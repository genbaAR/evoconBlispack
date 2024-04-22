/*
You need to update de wifi firmware and the bootloader firmware for the lastest update
*/

#define LFS_MBED_PORTENTA_H7_VERSION_MIN_TARGET "LittleFS_Portenta_H7 v1.2.0"
#define LFS_MBED_PORTENTA_H7_VERSION_MIN 1002000
#define _LFS_LOGLEVEL_ 1
#define FORCE_REFORMAT false

#include <Arduino.h>
#include <mbed.h>
#include "EthernetInterface.h"
#include <Arduino_MachineControl.h>
#include "definicionVariablesyConstantes.h"
#include "funciones.h"
#include <Arduino_JSON.h>
#include <queue>
#include "nsapi_dns.h"
#include <LittleFS_Portenta_H7.h>
#include <WiFi.h>

using namespace mbed;
using namespace rtos;
using namespace machinecontrol;
using namespace std::chrono_literals;

Mutex semaforoMutex;
Thread t1;
Thread t2;
Thread t3;
Thread t4;
Thread t5;

/*
Este hilo se encarga de:
  - Leer entradas digitales
  - Filtrar las entradas digitales
  - Contar cantidad de cajas en cada posible descarte
  - Contar las cajas correctas
*/
void hilo_Para_Lectura_Pin_IN()
{
    for (;;)
    { // Repeat forever
        if (!flagInputControlFromSerial)
        {
            funcionActualizarEstadoDeSensoresActual(digital_inputs.readAll());
        }
        conteo_de_Productos_por_Paso();
        funcionActualizarEstadoDeSensoresAnterior();
        ThisThread::sleep_for(10ms); // Wait for 10 miliseconds
    }
}
/*
Est hilo se encarga del flujo del programa
*/
void hiloControl()
{
    unsigned long timeoutStop = 0;
    unsigned long tiempoEnvio = HAL_GetTick();
    unsigned long tiempoQuerry = 0;
    unsigned long tiempokeepAlive = 0;
    u_int16_t timeOut = setpointTiempoEnvio - timeoutForSendingData;
    for (;;)
    { // Repeat forever

        funcionActivarKeepAlive(&tiempokeepAlive); // This function is called to inform Evocon that It's alive.
        /*
        In this step, we detect the first
        activation of the 'Paso' sensor and prepare everything
        to initiate the process of counting and sending data.
        */
        if (actualEstado_Paso && !flagFuncionando)
        {
            resetListoAEnviar();
            getNewQuantityBatch(0); // Update the quantity of products per batch
            flagFuncionando = true;
            tiempoQuerry = HAL_GetTick();
            time_system_active_duration = HAL_GetTick();
        }

        if (flagFuncionando)
        {
            if (actualEstado_Paso)
            {

                time_system_active_duration = HAL_GetTick(); // Update the time of the active system
            }

            /*
            Here we will check wich is the quantity of products/package
            In the variable timeToQuerryQuantityOfPackages we set the time
            In this moment i put 10 minutes.
            */
            if (HAL_GetTick() - tiempoQuerry > timeToQuerryQuantityOfPackages)
            {
                getNewQuantityBatch(0); // Update the quantity of products per batch

                tiempoQuerry = HAL_GetTick();
            }

            if (HAL_GetTick() - tiempoEnvio > timeOut)
            {
                funcionAlgoritmoDeConteoDeProductos();
                if (!flagUpdatePrevioDato)
                {
                    funcionActualizarPrevioEnvioToListoAEnviar();
                    funcionVerificacionNoCero();
                    if (flagFirstEmpaquetadoPacage)
                    {
                        /*
                        Hasta que no pase un paquete por S3 incrementaremos los contadores de la
                        de paquetes producidos por las empaquetadoras S1 y S2, al parar T3 estos seran
                        comparados con el acarreo para igualar marcar correctamente los descartes
                        y paquetes prducidos
                        */
                        funcionActualizarContadorParaCompararConAcarreo();
                    }
                    else
                    {
                        if (flagNotAllZero or flagEnviarSiempre)
                        {
                            funcionEnviarAEvocon();
                            funcionPrintListoAenviarData();
                        }
                    }
                }
                tiempoEnvio = HAL_GetTick();
            }
        }
        /*
        When the elapsed time exceeds the 'timeToStop' because the variable
        'time_system_active_duration' was not updated, set the 'flagFuncionando' to false.
        */
        if (flagFuncionando and (HAL_GetTick() - time_system_active_duration > timeToStop))
        {
            flagFuncionando = false;
        }
        ThisThread::sleep_for(100ms);
    }
}

void hiloEvocon()
{
    ThisThread::sleep_for(3s);
    //============================Bring up the ethernet interface====================
    if (keyImprimir and keyImprimirSocketData)
    {
        Serial.println("Inciando la configuracion del servidor");
    }
    int keyIndex = 0; // your network key Index number (needed only for WEP)

    int status = WL_IDLE_STATUS;
    int estado = 0;
    int estadoanterior = 1;

    if (flagActivarWifi)
    {
        if (flagWifiWithoutDHCP)
        {
            WiFi.config(ip, mask, gateway); // Change this
            WiFi.setDNS(dns1, dns2);        // Change this
        }
        if (WiFi.status() == WL_NO_SHIELD)
        {
            Serial.println("WiFi shield not present");

            // don't continue:

            while (true)
                ;
        }

        // attempt to connect to Wifi network:

        while (status != WL_CONNECTED)
        {
            Serial.print("Attempting to connect to SSID: ");

            Serial.println(ssid);

            // Connect to WPA/WPA2 network. Change this line if using open or WEP network:

            status = WiFi.begin(ssid, pass);

            // wait 10 seconds for connection:

            ThisThread::sleep_for(5s);
        }

        printWifiStatus();

        WiFi.macAddress(mac);
        estado = WiFi.status();
    }
    else
    {

        // == == == == == = This is used if we want to connecte using the ethernet interface EthernetInterface net;
        if (keyImprimir and keyImprimirSocketData)
        {
            Serial.println("Inciando la configuracion del servidor");
        }
        EthernetInterface net;
        net.set_network(ip, mask, gateway);
        net.add_dns_server(dns1, NULL);
        //  Show the network address
        net.connect();
        SocketAddress addr;
        net.get_ip_address(&addr);
        Serial.print("IP address: ");
        Serial.println(addr.get_ip_address() ? addr.get_ip_address() : "None");
        std::memcpy(mac, net.get_mac_address(), sizeof(mac));
        estado = net.get_connection_status();
    }
    //=========================Update Unix Time
    int contTest = 0;
    while (counter_Unix_Time_start > 0)
    {
        String time_Unix = funcionConnectAndSendSocketHTTP(serverTime, 80, requestToTime, "GET ", "");
        flagUnixTimeWasUpdated = functionQuestUnixTimeCorrect(time_Unix);
        if (flagUnixTimeWasUpdated)
        {
            unixTime = time_Unix;
            rtc_controller.setEpoch(unixTime.toInt());
            if (keyImprimir)
            {
                printLine();
                Serial.println("Se a actualizado la hora unix");
                printLine();
            }
            break;
        }
        else
        {
            contTest += 1;
            counter_Unix_Time_start -= 1;
            if (keyImprimir)
            {
                Serial.print("No se ha podido actualizar la hora unix esta es el intento número : ");
                Serial.println(contTest);
            }
        }
        ThisThread::sleep_for(10ms);
    }
    if (!flagUnixTimeWasUpdated)
    {
        printLine();
        Serial.println("**************No se ha podio a actualizado la hora unix*************");
        printLine();
    }
    for (;;)
    {

        if (keyImprimirMac)
        {
            Serial.print("valor es = ");
            Serial.print(mac[5], HEX);
            Serial.print(":");
            Serial.print(mac[4], HEX);
            Serial.print(":");
            Serial.print(mac[3], HEX);
            Serial.print(":");
            Serial.print(mac[2], HEX);
            Serial.print(":");
            Serial.print(mac[1], HEX);
            Serial.print(":");
            Serial.println(mac[0], HEX);
            keyImprimirMac = false;
        }
        if (estado != estadoanterior)
        {
            if (estado == WL_CONNECTED)
            {
                bitestadoYalarma = bitestadoYalarma & ~(1 << cableRedDesconectado);
                ethernetConnected = true;
            }
            else
            {
                if (ethernetConnected)
                {
                    /*
                    The status of the connection might be:
                    WL_NO_SHIELD = 255,
                    WL_NO_MODULE = 255,
                    WL_IDLE_STATUS = 0,
                    WL_NO_SSID_AVAIL, =1
                    WL_SCAN_COMPLETED, =2
                    WL_CONNECTED, =3
                    WL_CONNECT_FAILED, =4
                    WL_CONNECTION_LOST =5
                    WL_DISCONNECTED =6
                    WL_AP_LISTENING =7
                    WL_AP_CONNECTED=8
                    WL_AP_FAILED =9
                    */
                    Serial.println("El error de la red es " + String(WiFi.status()));
                }
                ethernetConnected = false;
                bitestadoYalarma = bitestadoYalarma | (1 << cableRedDesconectado);
            }
        }
        // mandar request cada x tiempo
        if (flagEnviarAevocon && ethernetConnected)
        {

            unixTime = funcionConnectAndSendSocketHTTP(serverTime, 80, requestToTime, "GET ", "");
            bool pruebaRecibidoUnixTime = functionQuestUnixTimeCorrect(unixTime);
            if (!pruebaRecibidoUnixTime)
            {
                unixTime = funcionObtenerUnixTimeInterno();
                if (keyImprimir)
                {
                    printLine();
                    Serial.println("No hemos podido recibir correctamente la hora unix, se utilizara la interna");
                    printLine();
                }
            }
            else
            {
                rtc_controller.setEpoch(unixTime.toInt());
                if (keyImprimir and keyImprimirSocketData)
                {
                    Serial.println("=============El dato es correcto = " + unixTime);
                }
            }

            String datosJson = funcionJsonCreator(deviceIdx, inputNumberx, unixTime);
            String estadoRecepcion2 = funcionConnectAndSendSocketHTTP(serverToSendData, 80, encabezadoMuchos, "POST ", datosJson);
            int cuentasPrueba = 0;
            while (!dataQueue.empty())
            {
                cuentasPrueba += 1;
                String datas = dataQueue.front();
                Serial.println("**************************Estoy enviando un dato anterior que no se mando ");
                Serial.print("El numero de dato para enviar es de : ");
                Serial.println(cuentasPrueba);
                if (funcionConnectAndSendSocketHTTP(serverToSendData, 80, encabezadoMuchos, "POST ", datas))
                {
                    // Si se envía correctamente, se quita de la cola
                    dataQueue.pop();
                }
                else
                {
                    // Si falla el envío, se vuelve a intentar en la siguiente iteración
                    break;
                }
            }
            if (keyImprimir and keyImprimirSocketData)
            {
                Serial.println("=============El dato recibido  es " + estadoRecepcion2);
                Serial.println("Done");
            }
            // net.disconnect();
            flagEnviarAevocon = false;
            flagEnvioRealizadoAEvocon = true;
        }

        if (flagCheckTheProductQuantity)
        {
            flagCheckTheProductQuantity = false;
            //   String dataRecib = funcionConnectAndSendSocket(net, addr, serverTime, 80, requestToBatch, "GET ", "");

            if (keyImprimir and keyImprimirSocketData)
            {
                //   Serial.println("=============El dato recibido  es " + dataRecib);
                Serial.println("Done");
            }
        }
        estadoanterior = estado;
        ThisThread::sleep_for(250ms);
    }
}

void hiloEstados()
{
    unsigned long timerLights = HAL_GetTick();
    byte bitestadoYalarmaAnterior = 5;
    bool estadoVerde = false; // to generating the toggle
    bool estadoRojo = false;  // to generating the toggle
    for (;;)
    {
        // verifico el estado de los bits de error, estan ordenados dependiendo de la importancia
        if (estadoDelSistemaVisual != bitestadoYalarmaAnterior && keyImprimir)
        {
            Serial.println("\t\t\t\tEl estado del sistema es =" + String(estadoDelSistemaVisual));
            bitestadoYalarmaAnterior = estadoDelSistemaVisual;
        }
        if (!actualEstado_Sensor_transporte_3)
        {
            // Serial.println("\t\t\t\tla balanza esta apagada!!!");
            estadoDelSistemaVisual = balanzaApagada;
        }
        else if (bitestadoYalarma & (1 << cableRedDesconectado) >> cableRedDesconectado)
        {
            estadoDelSistemaVisual = cableRedDesconectado;
        }
        else if (bitestadoYalarma & (1 << noComunicacionSocket) >> noComunicacionSocket)
        {
            estadoDelSistemaVisual = noComunicacionSocket;
        }
        else if (estadoDelSistemaVisual == 4)
        {
            ; // se activa desde el envio de datos
        }
        else
        {
            estadoDelSistemaVisual = seEnvioDato;
        }
        switch (estadoDelSistemaVisual)
        // if ((estadoDelSistemaVisual & (1 << cableRedDesconectado)) >> cableRedDesconectado)
        {
        case 0:
            // Cable de red desconectado
            // luz verde apagada
            // Luz roja parpadea con una frecuencia de 500ms
            if (HAL_GetTick() - timerLights > 500)
            {
                estadoRojo = !estadoRojo;
                timerLights = HAL_GetTick();
            }
            digital_outputs.set(Pin_led_Verde, estadoVerde);
            digital_outputs.set(Pin_led_Rojo, estadoRojo);
            break;
        case 1:
            // Cable de red conectado pero hay error en la comunicacion con socket
            // luz verde apagada
            // Luz roja parpadea con una frecuencia de 500ms
            if (HAL_GetTick() - timerLights > 500)
            {
                estadoRojo = !estadoRojo;
                timerLights = HAL_GetTick();
            }
            digital_outputs.set(Pin_led_Verde, HIGH);
            digital_outputs.set(Pin_led_Rojo, estadoRojo);
            break;
        case 2:
            // Balanza apagada
            // luz verde apagada
            // Luz roja prendida

            digital_outputs.set(Pin_led_Verde, LOW);
            digital_outputs.set(Pin_led_Rojo, HIGH);
            break;
        case 3:
            // energizado y balanza ok
            // luz verde prendida
            // Luz roja apagada
            digital_outputs.set(Pin_led_Verde, HIGH);
            digital_outputs.set(Pin_led_Rojo, LOW);
            break;
        case 4:
            // envio dato correcto
            // luz verde prendida
            // Luz roja apagada
            digital_outputs.set(Pin_led_Verde, LOW);
            digital_outputs.set(Pin_led_Rojo, LOW);

            for (int i = 0; i < 2; i++)
            {
                digital_outputs.set(Pin_led_Verde, LOW);  // Encendemos el LED
                ThisThread::sleep_for(200ms);             // Esperamos 200ms
                digital_outputs.set(Pin_led_Verde, HIGH); // Apagamos el LED
                ThisThread::sleep_for(200ms);             // Esperamos 200ms
            }

            estadoDelSistemaVisual = 3;
            break;
        default:
            // statements
            Serial.println("\t\t\t\t============Estado default del switch");
            break;
        }
        ThisThread::sleep_for(150ms);
    }
}
void hiloSerialControl()
{
    int bitGeneratorTocontrol = 0;
    timerToprintActualCounter = HAL_GetTick();
    for (;;)
    {
        if (Serial.available() > 0)
        {
            char entrada = Serial.read();

            if (entrada == CharkeyImprimir)
            {
                keyImprimir = !keyImprimir;
                Serial.println("valor es = " + String(keyImprimir));
            }
            else if (entrada == charControlPorSerial)
            {
                entrada = Serial.parseInt();
                switch (entrada)
                {
                case charSerialActivarControlSerial:
                    flagInputControlFromSerial = !flagInputControlFromSerial;
                    Serial.println("El control por serial se encuentra = " + String(flagInputControlFromSerial));
                    break;
                case charSerialEmbolzadora_1:
                    bitGeneratorTocontrol = 0;
                    bitGeneratorTocontrol = bitGeneratorTocontrol | (1 << Pin_sensor_embazadora_1);
                    funcionActualizarEstadoDeSensoresActual(bitGeneratorTocontrol);
                    Serial.println("****************Se a creado un paquete de Embolzadora 1****************" + String(bitGeneratorTocontrol));
                    ThisThread::sleep_for(timeTosleepSerialControl);
                    funcionActualizarEstadoDeSensoresActual(0);
                    break;
                case charSerialEmbolzadora_2:
                    bitGeneratorTocontrol = 0;
                    bitGeneratorTocontrol = bitGeneratorTocontrol | (1 << Pin_sensor_embazadora_2);
                    Serial.println("****************Se a creado un paquete de Embolzadora 2****************" + String(bitGeneratorTocontrol));
                    funcionActualizarEstadoDeSensoresActual(bitGeneratorTocontrol);
                    ThisThread::sleep_for(timeTosleepSerialControl);
                    funcionActualizarEstadoDeSensoresActual(0);
                    break;
                case charSerialTransporte_1:
                    bitGeneratorTocontrol = 0;
                    bitGeneratorTocontrol = bitGeneratorTocontrol | (1 << Pin_sensor_transporte_1);
                    funcionActualizarEstadoDeSensoresActual(bitGeneratorTocontrol);
                    Serial.println("**************** Se ha prendido el transporte 1 ****************" + String(bitGeneratorTocontrol));
                    ThisThread::sleep_for(timeTosleepSerialControl);
                    funcionActualizarEstadoDeSensoresActual(0);
                    break;
                case charSerialTransporte_2:
                    bitGeneratorTocontrol = 0;
                    bitGeneratorTocontrol = bitGeneratorTocontrol | (1 << Pin_sensor_transporte_2);
                    Serial.println("****************Se ha prendido el transporte 2 ****************" + String(bitGeneratorTocontrol));
                    funcionActualizarEstadoDeSensoresActual(bitGeneratorTocontrol);
                    ThisThread::sleep_for(timeTosleepSerialControl);
                    funcionActualizarEstadoDeSensoresActual(0);
                    break;
                case charSerialFinDeLinea:
                    bitGeneratorTocontrol = 0;
                    bitGeneratorTocontrol = bitGeneratorTocontrol | (1 << Pin_Sensor_final_transporte_lineal);
                    funcionActualizarEstadoDeSensoresActual(bitGeneratorTocontrol);
                    Serial.println("**************** Fin de Linea ****************" + String(bitGeneratorTocontrol));
                    ThisThread::sleep_for(timeTosleepSerialControl);
                    funcionActualizarEstadoDeSensoresActual(0);
                    break;
                case charSerialTransporte_3:
                    flagTransportador_3_Manual = !flagTransportador_3_Manual;
                    Serial.println("**************** La simulación de la balanza ha sido ****** " + String(flagTransportador_3_Manual));
                    break;
                default:
                    break;
                }
                funcionPrintConteoActual(); // funcionPrintListoAenviarData();
            }
            else if (entrada == CharkeyKeepAlive)
            {
                funcionKeepAlive();
            }
            else if (entrada == CharkeyImprimirEstadosWifi)
            {
                printWifiStatus();
            }
            else if (entrada == CharkeyImprimirEstadosSensor)
            {
                keyImprimirEstadosSensor = !keyImprimirEstadosSensor;
                Serial.println("valor es = " + String(keyImprimirEstadosSensor));
            }
            else if (entrada == charSerialVersionOftheSoftware)
            {
                Serial.println("La version de sofware es = " + String(verisionOfSoftware));
            }
            else if (entrada == CharkeyImprimirContadorListoEnviar)
            {
                keyImprimirContadorListoEnviar = !keyImprimirContadorListoEnviar;
                Serial.println("valor es = " + String(keyImprimirContadorListoEnviar));
            }
            else if (entrada == CHarkeyImprimirSocketData)
            {
                keyImprimirSocketData = !keyImprimirSocketData;
                Serial.println("valor es = " + String(keyImprimirSocketData));
            }
            else if (entrada == CharSendContinuo)
            {
                flagEnviarSiempre = !flagEnviarSiempre;
                Serial.println("valor es = " + String(flagEnviarSiempre));
            }
            else if (entrada == CharkeyImprimirContadorActual)
            {
                flagImpresionConteoActual = !flagImpresionConteoActual;
                funcionPrintConteoActual();
                Serial.println("valor es = " + String(flagImpresionConteoActual));
            }
            else if (entrada == CharMostrarLetras)
            {
                Serial.println("=============================================");
                Serial.println("Activar/desactivar impresion de datos \t\t\t== i");
                Serial.println("Activar/desactivar impresion de Socket \t\t\t== d");
                Serial.println("Activar/desactivar impresion de estados de sensores \t== s");
                Serial.println("Activar/desactivar impresion de contador para enviar \t== c");
                Serial.println("Activar/desactivar envio siempre datos por socket \t== u");
                Serial.println("Get Unixtime \t\t\t\t\t\t== U");
                Serial.println("Wifi Status \t\t\t\t\t\t== W");
                Serial.println("Imprimir Mac \t\t\t\t\t\t== m");
                Serial.println("Imprimir Version \t\t\t\t\t== V");
                Serial.println("Imprimir Contador Actual \t\t\t\t== A");
                Serial.println("Activar/desactivar simulación de entradas por serial \t== H0");
                Serial.println("\t\t Simular Embolzadora 1  \t\t== H1");
                Serial.println("\t\t Simular Embolzadora 2  \t\t== H2");
                Serial.println("\t\t Simular Transporte 1 \t\t\t== H3");
                Serial.println("\t\t Simular Transporte 2 \t\t\t== H4");
                Serial.println("\t\t Simular fin de linea \t\t\t== H5");
                Serial.println("\t\t Simular Transporte 3 \t\t\t== H6");
                Serial.println("Activar/desactivar Contador General de productos \t== GG");
                Serial.println("\t\t Reset the general counter \t\t== GR");
                Serial.println("\t\t Print the general counter \t\t== GP");
                Serial.println("Print the acual quantity of pruduct per batch \t\t== Q0");
                Serial.println("Find and print the new quantity of pruduct per batch \t== Q1");
                Serial.println("Send keep-Alive \t\t\t== K");
                Serial.println("=============================================");
            }
            else if (entrada == CharkeyImprimirUnixTime)
            {
                unixTime = funcionConnectAndSendSocketHTTP(serverTime, 80, requestToTime, "GET ", "");
                // unixTime = "";
                Serial.println("\t\t==============*****UnixTime*****==================");
                bool pruebaRecibido = functionQuestUnixTimeCorrect(unixTime);
                if (!pruebaRecibido)
                {
                    unixTime = funcionObtenerUnixTimeInterno();
                }
                Serial.println(unixTime);
                Serial.println("\t\t==============******************==================");
            }
            else if (entrada == CharSendMac)
            {
                keyImprimirMac = !keyImprimirMac;
                Serial.println("valor es = " + String(keyImprimirMac));
            }
            else if (entrada == CharJSONVew)
            {
                keyImprimirJSON = !keyImprimirJSON;
                Serial.println("valor es = " + String(keyImprimirJSON));
            }
            else if (entrada == charSerialFile)
            {
                entrada = Serial.read();
                if (entrada == 'D')
                {
                    deleteFile(fileName1);
                    printLine();
                }
                else if (entrada == 'N')
                {
                    String data = Serial.readStringUntil('*');
                    Serial.println(data);

                    char miCharArray[data.length() + 1];
                    data.toCharArray(miCharArray, sizeof(miCharArray));
                    writeFile(fileName1, miCharArray, sizeof(miCharArray));
                    printLine();
                }
                else if (entrada == 'P')
                {
                    readFile(fileName1);
                    printLine();
                }
            }
            else if (entrada == charSerialGeneralCounter)
            {
                entrada = Serial.read();
                if (entrada == charSerialGeneralCounterSTART)
                {
                    flagGeneralCounter = !flagGeneralCounter;
                    Serial.print("General counter is ");
                    Serial.println(flagGeneralCounter ? "Start" : "Stop");
                }
                else if (entrada == charSerialGeneralCounterRESET)
                {
                    resetGeneralcounters();
                    Serial.println("****Reset the general counter for the production****");
                }
                else if (entrada == charSerialGeneralCounterPRINT)
                {
                    Serial.println("=============================GENERL COUNTER=================================");
                    Serial.println("Embazadora 1 : " + String(contadorGeneral_embazadora_1));
                    Serial.println("Embazadora 2 : " + String(contadorGeneral_embazadora_2));
                    Serial.println("Descarte Embazadora : " + String(contadorGeneral_descarte_embazado));
                    Serial.println("Producto bueno: " + String(contadorGeneral_producto_bueno));
                    Serial.println("Producto total: " + String(contadorGeneral_producto_total));
                    Serial.println("=================================================================================");
                    Serial.println();
                }
                else
                {
                    Serial.println("Serial command not recognized");
                }
            }
            else if (entrada == charSerialCheckProductQuantity)
            {
                entrada = Serial.read();
                if (entrada == charSerialCheckProductQuantity_Actual)
                {
                    Serial.print("The actual quantity per batch is ");
                    Serial.println(multiplicador_de_producto_X_embazado);
                }
                else if (entrada == charSerialCheckProductQuantity_New)
                {
                    // multiplicador_de_producto_X_embazado
                    String dt = getNewQuantityBatch(0);
                    Serial.print("Api evocon con seguridad ");
                    Serial.println(dt);
                }
                else if (entrada == '2')
                {
                    // multiplicador_de_producto_X_embazado
                    String dt = getNewQuantityBatch(1);
                    Serial.print("Api evocon sin seguridad ");
                    Serial.println(dt);
                }
                else if (entrada == '3')
                {
                    // multiplicador_de_producto_X_embazado
                    String dt = getNewQuantityBatch(2);
                    Serial.print("Api pipedream con segurida");
                    Serial.println(dt);
                }
                else if (entrada == '4')
                {
                    // multiplicador_de_producto_X_embazado
                    String dt = getNewQuantityBatch(3);
                    Serial.print("Api pipedream sin segurida");
                    Serial.println(dt);
                }
                else
                {
                    Serial.println("Serial command not recognized");
                }
            }
            else
            {
                Serial.println("Serial command not recognized");
            }
        }
        if (keyImprimir)
        {

            if (flagImpresionConteoActual)
            {
                if (HAL_GetTick() - timerToprintActualCounter > timeToprintActualCounter)
                {
                    funcionPrintConteoActual();
                    timerToprintActualCounter = HAL_GetTick();
                }
            }
            if (keyImprimirEstadosSensor)
            {
                if (HAL_GetTick() - timerImprimirInputs > tiempoImprimirInputs)
                {
                    funcionPrintEstadoDeSensores();
                    timerImprimirInputs = HAL_GetTick();
                }
            }
        }
        ThisThread::sleep_for(150ms);
    }
}
void setup()
{
    digital_outputs.setLatch();
    digital_outputs.setAll(0);
    Wire.begin();
    if (!digital_inputs.init())
    {
        Serial.println("Digital input GPIO expander initialization fail!!");
    }
    Serial.begin(115200); // Start Serial communication
    myFS = new LittleFS_MBED();

    if (!myFS->init())
    {
        Serial.println("LITTLEFS Mount Failed");

        return;
    }
    Serial.println("Initialization of RTC clock");
    if (!rtc_controller.begin())
    {
        Serial.println("Initialization fail!");
    }
    Serial.println("Initialization Done!");
    t1.start(hilo_Para_Lectura_Pin_IN); //
    t2.start(hiloControl);              // Pass func2 to t2 to start executing it as an independant thread
    t3.start(hiloEvocon);
    t3.set_priority(osPriorityAboveNormal);
    t4.start(hiloEstados);
    if (flagActivarControlSerial)
    {
        t5.start(hiloSerialControl);
    }
    ThisThread::sleep_for(1500ms);
    Serial.println("Se esta iniciando el sistema");
    Serial.println("Presione la letra m para ver opciones");
}

void loop()
{
}