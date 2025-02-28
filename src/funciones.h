#ifndef FUNCIONES_H
#define FUNCIONES_H
#include <Arduino.h>
#include "definicionVariablesyConstantes.h"
#include <Arduino_MachineControl.h>
#include <LittleFS_Portenta_H7.h>
#include <Arduino_JSON.h>
#include <queue>
#include "EthernetInterface.h"
#include "WiFi.h"
#include <WiFiSSLClient.h>

using namespace machinecontrol;
using namespace std::chrono_literals;
/*
This function only prints a line
*/
void printLine()
{
    Serial.println("====================================================");
}
bool funcionFiltroDeLecturas(int pin, unsigned int tiempo, float threshold)
{
    float lecturaAcumulada = 0;
    float numeroLecturas = 0;

    unsigned long tiempoActual = HAL_GetTick();
    while (HAL_GetTick() - tiempoActual < tiempo)
    { // Mientras no haya pasado el tiempo deseado
        int valorLectura = digital_inputs.read(pin);
        lecturaAcumulada += valorLectura;
        numeroLecturas++;
    }

    int promedioLecturas = lecturaAcumulada / numeroLecturas;
    bool estadoFinaldeLectura = 0;

    if (!flagInputControlFromSerial)
    {
        if (promedioLecturas > threshold)
        {
            estadoFinaldeLectura = true;
        }
    }
    else
    {
        estadoFinaldeLectura = true;
    }
    return estadoFinaldeLectura;
}

/*
Esta funcion permite actualizar el valor del estado anterior de los sensores
para poder ver si realmente cambiaron de estado
*/
void funcionActualizarEstadoDeSensoresAnterior()
{
    previoEstado_Descarte_1 = actualEstado_Descarte_1;
    previoEstado_Descarte_2 = actualEstado_Descarte_2;
    previoEstado_Descarte_3 = actualEstado_Descarte_3;
    previoEstado_Descarte_4 = actualEstado_Descarte_4;
    previoEstado_Descarte_5 = actualEstado_Descarte_5;
    previoEstado_Descarte_6 = actualEstado_Descarte_6;
    previoEstado_Descarte_Manual = actualEstado_Descarte_Manual;
    previoEstado_Paso = actualEstado_Paso;
}

void funcionPrintListoAenviarData()
{
    Serial.println("Estado del contador listo a enviar - Producto bueno : " + String(listoEnviar_contador_producto_bueno_por_batch));
    Serial.println("Estado del contador listo a enviar - Descarte : " + String(listoEnviar_contador_descarte_por_batch));
    Serial.println("Estado del contador listo a enviar - Total de producto: " + String(listoEnviar_contador_producto_total));
    Serial.println();
}
/*
Funcion para realizar un reset de los valores de listo a enviar
para eliminar cualquier mala inicializacion de los mismos
*/
void resetListoAEnviar()
{
    listoEnviar_contador_producto_bueno_por_batch = 0;
    listoEnviar_contador_descarte_por_batch = 0;
    listoEnviar_contador_producto_total = 0; // evocon precisa el numero total de lo que se produjo durante ese periodo de tiempo
}
/*
Esta funcion permite actualizar el valor de los contadores de previo a enviar a listo a enviar
*/
void funcionActualizarPrevioEnvioToListoAEnviar()
{
    listoEnviar_contador_producto_bueno_por_batch = previoEnvio_contador_producto_bueno_por_batch;
    listoEnviar_contador_descarte_por_batch = previoEnvio_contador_descarte_por_batch;
    listoEnviar_contador_producto_total = listoEnviar_contador_producto_bueno_por_batch + listoEnviar_contador_descarte_por_batch;
    if (keyImprimirContadorListoEnviar)
    {
        funcionPrintListoAenviarData();
    }
}
/*
this function is called when we need to read the digital input from the sensors
  if (((inputs & (1 << Pin_)) >> Pin_) and previoEstado_)
  {
    // aplico filtro para estar seguro de que se detecto un objeto
    actualEstado_ = funcionFiltroDeLecturas(Pin_, tiempoParaFiltro, thresholdsParaFiltro);
  }
*/
void funcionActualizarEstadoDeSensoresActual(uint32_t dataIn)
{
    uint32_t inputs = dataIn; // digital_inputs.readAll();
    if ((inputs & (1 << Pin_Descarte_1)) >> Pin_Descarte_1)
    {
        // aplico filtro para estar seguro de que se detecto un objeto
        actualEstado_Descarte_1 = logica_Descarte_1 ? funcionFiltroDeLecturas(Pin_Descarte_1, tiempoParaFiltro, thresholdsParaFiltro) : !funcionFiltroDeLecturas(Pin_Descarte_1, tiempoParaFiltro, thresholdsParaFiltro);
    }
    else
    {
        // pongo a verdadero por logica Negativa del sensor
        logica_Descarte_1 ? (actualEstado_Descarte_1 = false) : (actualEstado_Descarte_1 = true);
    }
    if ((inputs & (1 << Pin_Descarte_2)) >> Pin_Descarte_2)
    {
        // aplico filtro para estar seguro de que se detecto un objeto
        actualEstado_Descarte_2 = logica_Descarte_2 ? funcionFiltroDeLecturas(Pin_Descarte_2, tiempoParaFiltro, thresholdsParaFiltro) : !funcionFiltroDeLecturas(Pin_Descarte_2, tiempoParaFiltro, thresholdsParaFiltro);
    }
    else
    {
        logica_Descarte_2 ? actualEstado_Descarte_2 = false : actualEstado_Descarte_2 = true;
    }
    if ((inputs & (1 << Pin_Descarte_3)) >> Pin_Descarte_3)
    {
        // aplico filtro para estar seguro de que se detecto un objeto
        actualEstado_Descarte_3 = logica_Descarte_3 ? funcionFiltroDeLecturas(Pin_Descarte_3, tiempoParaFiltro, thresholdsParaFiltro) : !funcionFiltroDeLecturas(Pin_Descarte_3, tiempoParaFiltro, thresholdsParaFiltro);
    }
    else
    {
        logica_Descarte_3 ? actualEstado_Descarte_3 = false : actualEstado_Descarte_3 = true;
    }
    if ((inputs & (1 << Pin_Descarte_4)) >> Pin_Descarte_4)
    {
        // aplico filtro para estar seguro de que se detecto un objeto
        actualEstado_Descarte_4 = logica_Descarte_4 ? funcionFiltroDeLecturas(Pin_Descarte_4, tiempoParaFiltro, thresholdsParaFiltro) : !funcionFiltroDeLecturas(Pin_Descarte_4, tiempoParaFiltro, thresholdsParaFiltro);
    }
    else
    {
        logica_Descarte_4 ? actualEstado_Descarte_4 = false : actualEstado_Descarte_4 = true;
    }
    if ((inputs & (1 << Pin_Descarte_5)) >> Pin_Descarte_5)
    {
        // aplico filtro para estar seguro de que se detecto un objeto
        actualEstado_Descarte_5 = logica_Descarte_5 ? funcionFiltroDeLecturas(Pin_Descarte_5, tiempoParaFiltro, thresholdsParaFiltro) : !funcionFiltroDeLecturas(Pin_Descarte_5, tiempoParaFiltro, thresholdsParaFiltro);
    }
    else
    {
        logica_Descarte_5 ? actualEstado_Descarte_5 = false : actualEstado_Descarte_5 = true;
    }
    if ((inputs & (1 << Pin_Descarte_6)) >> Pin_Descarte_6)
    {
        // aplico filtro para estar seguro de que se detecto un objeto
        actualEstado_Descarte_6 = logica_Descarte_6 ? funcionFiltroDeLecturas(Pin_Descarte_6, tiempoParaFiltro, thresholdsParaFiltro) : !funcionFiltroDeLecturas(Pin_Descarte_6, tiempoParaFiltro, thresholdsParaFiltro);
    }
    else
    {
        logica_Descarte_6 ? actualEstado_Descarte_6 = false : actualEstado_Descarte_6 = true;
    }
    if ((inputs & (1 << Pin_Descarte_Manual)) >> Pin_Descarte_Manual)
    {
        // aplico filtro para estar seguro de que se detecto un objeto
        actualEstado_Descarte_Manual = logica_Descarte_Manual ? funcionFiltroDeLecturas(Pin_Descarte_Manual, tiempoParaFiltro, thresholdsParaFiltro) : !funcionFiltroDeLecturas(Pin_Descarte_Manual, tiempoParaFiltro, thresholdsParaFiltro);
    }
    else
    {
        logica_Descarte_Manual ? actualEstado_Descarte_Manual = false : actualEstado_Descarte_Manual = true;
    }
    if ((inputs & (1 << Pin_paso)) >> Pin_paso)
    {
        // aplico filtro para estar seguro de que se detecto un objeto
        actualEstado_Paso = logica_Paso ? funcionFiltroDeLecturas(Pin_paso, tiempoParaFiltro, thresholdsParaFiltro) : !funcionFiltroDeLecturas(Pin_paso, tiempoParaFiltro, thresholdsParaFiltro);
    }
    else
    {
        logica_Paso ? actualEstado_Paso = false : actualEstado_Paso = true;
    }
    /*
        Esto lo apago porque no va a ser necesario en este caso por ser un sistema que no cuenta con un pin
        que nos indique si esta prendido o no el sistema, sino que nos guiamos por el paso.

        if (!flagInputControlFromSerial) // Esto permite el control desde el Serial cuando se activa el control por serie se simula como que la balanza esta siempre prendida.
        {
            if (((inputs & (1 << Pin_paso)) >> Pin_paso))
            {
                // aplico filtro para estar seguro de que se detecto un objeto
                actualEstado_Paso = funcionFiltroDeLecturas(Pin_paso, tiempoParaFiltro, thresholdsParaFiltro);
                if (actualEstado_Paso)
                {
                    bitestadoYalarma = bitestadoYalarma & ~(1 << balanzaApagada);
                }
                else
                {
                    bitestadoYalarma = bitestadoYalarma | (1 << balanzaApagada);
                }
            }
            else
            {
                actualEstado_Paso = false;
                bitestadoYalarma = bitestadoYalarma | (1 << balanzaApagada);
            }
        }
        else
        {
            actualEstado_Sensor_transporte_3 = true;
            bitestadoYalarma = bitestadoYalarma & ~(1 << balanzaApagada);
        }
    */
    if (keyImprimirEstadosSensor and keyImprimir)
    {
        if (HAL_GetTick() - timerImprimirInputs > tiempoImprimirInputs)
        {

            printLine();
            Serial.println("Estado Sensor_Descarte 1        : " + String(actualEstado_Descarte_1));
            Serial.println("Estado Sensor_Descarte 2        : " + String(actualEstado_Descarte_2));
            Serial.println("Estado Sensor_Descarte 3        : " + String(actualEstado_Descarte_3));
            Serial.println("Estado Sensor_Descarte 4        : " + String(actualEstado_Descarte_4));
            Serial.println("Estado Sensor_Descarte 5        : " + String(actualEstado_Descarte_5));
            Serial.println("Estado Sensor_Descarte 6        : " + String(actualEstado_Descarte_6));
            Serial.println("Estado Sensor_Descarte manual   : " + String(actualEstado_Descarte_Manual));
            Serial.println("Estado Sensor_Paso              : " + String(actualEstado_Paso));
            Serial.println();
            timerImprimirInputs = HAL_GetTick();
            printLine();
        }
    }
}

/*
Function to print the actual status of the previoEnvio variabl
*/
void funcionPrintPrevioAenviarData()
{
    printLine();
    Serial.println("Estado del contador previo a enviar - Descarte por lote: " + String(previoEnvio_contador_descarte_por_batch));
    Serial.println("Estado del contador previo a enviar - Prodcuto bueno por lote: " + String(previoEnvio_contador_producto_bueno_por_batch));
    printLine();
}
void funcionPrintConteoActual()
{
    printLine();
    Serial.println("Estado del contador producto bueno por paso  : " + String(actual_contador_producto_bueno_durante_un_paso));
    Serial.println("Estado del contador Producto bueno por batch : " + String(actual_contador_producto_bueno_por_batch));
    Serial.println("Estado del contador descarte por paso \t: " + String(actual_contador_descarte_en_un_paso));
    Serial.println("Estado del contador descarte por batch\t: " + String(actual_contador_descarte_por_batch));
    Serial.println("Estado del contador descarte manual por batch: " + String(actual_contador_Descarte_Manual_por_batch));
    printLine();
}
void funcionPrintEstadoDeSensores()
{
    printLine();
    Serial.println("Estado sensor DESCARTE 1: " + String(actualEstado_Descarte_1));
    Serial.println("Estado sensor DESCARTE 2: " + String(actualEstado_Descarte_2));
    Serial.println("Estado sensor DESCARTE 3: " + String(actualEstado_Descarte_3));
    Serial.println("Estado sensor DESCARTE 4: " + String(actualEstado_Descarte_4));
    Serial.println("Estado sensor DESCARTE 5: " + String(actualEstado_Descarte_5));
    Serial.println("Estado sensor DESCARTE 6: " + String(actualEstado_Descarte_6));
    Serial.println("Estado sensor DESCARTE MANUAL: " + String(actualEstado_Descarte_Manual));
    Serial.println("Estado sensor PASO: " + String(actualEstado_Paso));
    printLine();
}
void upgradeGeneralcounters()
{
    printLine();
    contadorGeneral_producto_bueno_por_batch += +listoEnviar_contador_producto_bueno_por_batch;
    contadorGeneral_descarte_por_batch += listoEnviar_contador_descarte_por_batch;
    printLine();
}

/*
This function is called when we need an actualization of the counters,
Here we do the actualization with the algorithm of count
*/
void funcionAlgoritmoDeConteoDeProductos()
{
    flagUpdatePrevioDato = true;
    contador_de_Batch_por_ciclo += 1;

    previoEnvio_contador_producto_bueno_por_batch = actual_contador_producto_bueno_por_batch - actual_contador_Descarte_Manual_por_batch;
    previoEnvio_contador_descarte_por_batch = actual_contador_Descarte_Manual_por_batch + actual_contador_descarte_por_batch;

    actual_contador_producto_bueno_por_batch = 0;
    actual_contador_Descarte_Manual_por_batch = 0;
    actual_contador_descarte_por_batch = 0;

    flagUpdatePrevioDato = false;
    return;
}
/*
Esta funcion cuenta la cantidad de cambios de franco ascendentes que tenemos
en todos los sensores. Se calcula conteos de productos en cada paso y se suman al
conteo por batch, siendo este ultimo reiniciado al enviar a evocon el dato
*/
void conteo_de_Productos_dentro_del_batch() // Finished
{

    if (flagFuncionando)
    {
        if (!flagUpdatePrevioDato)
        {

            if (actualEstado_Paso and !previoEstado_Paso)
            {
                actual_contador_producto_bueno_durante_un_paso = 0; // Initialize the variable 'actual_contador_producto_bueno_durante_un_paso' to 0.
                contador_de_pasos_por_batch += 1;

                if (actualEstado_Descarte_1 and !previoEstado_Descarte_1)
                {
                    // Count the number of actual good production
                    if (producto_por_paso > 0){
                        actual_contador_producto_bueno_durante_un_paso += 1;
                    }
                }
                if (actualEstado_Descarte_2 and !previoEstado_Descarte_2)
                {
                    // Count the number of actual good production
                    if (producto_por_paso > 1){
                        actual_contador_producto_bueno_durante_un_paso += 1;
                    }
                }
                if (actualEstado_Descarte_3 and !previoEstado_Descarte_3)
                {
                    // Count the number of actual good production
                    if (producto_por_paso > 2){
                        actual_contador_producto_bueno_durante_un_paso += 1;
                    }
                }
                if (actualEstado_Descarte_4 and !previoEstado_Descarte_4)
                {
                    // Count the number of actual good production
                    if (producto_por_paso > 3){
                        actual_contador_producto_bueno_durante_un_paso += 1;
                    }
                }
                if (actualEstado_Descarte_5 and !previoEstado_Descarte_5)
                {
                    // Count the number of actual good production
                    if (producto_por_paso > 4){
                        actual_contador_producto_bueno_durante_un_paso += 1;
                    }
                }
                if (actualEstado_Descarte_6 and !previoEstado_Descarte_6)
                {
                    // Count the number of actual good production
                    if (producto_por_paso > 5){
                        actual_contador_producto_bueno_durante_un_paso += 1;
                    }
                }

                actual_contador_producto_bueno_por_batch += producto_por_paso - actual_contador_producto_bueno_durante_un_paso;
                actual_contador_descarte_en_un_paso += actual_contador_producto_bueno_durante_un_paso;
                actual_contador_descarte_por_batch += actual_contador_descarte_en_un_paso;

                actual_contador_descarte_en_un_paso = 0;
                actual_contador_producto_bueno_durante_un_paso = 0;
            }
            if (actualEstado_Descarte_Manual and !previoEstado_Descarte_Manual)
            {
                // Count the number of actual good production
                actual_contador_Descarte_Manual_por_batch += 1;
            }
            return;
        }
    }

    return;
}

String dataExtraction(String response)
{

    const String clTag = "Content-Length: ";
    auto clIndex = response.indexOf(clTag);
    String content = response.substring(clIndex);
    clIndex += clTag.length();
    auto cl = response.substring(clIndex, clIndex + content.indexOf("\r\n"));
    String dataRetrun = "no data";
    if (keyImprimirSocketData)
    {
        Serial.println("el valor de clindex es: " + String(clIndex));
        Serial.println("el valor de cl es: " + String(cl));
    }
    const String bodyTag = "\r\n\r\n";
    auto bodyIndex = response.indexOf(bodyTag);
    if (bodyIndex != -1)
    {
        bodyIndex += bodyTag.length();
        auto body = response.substring(bodyIndex, bodyIndex + cl.toInt());
        if (keyImprimir and keyImprimirSocketData)
        {
            Serial.print("Data recived in de body is : ");
            Serial.println(body);
        }

        if ((body != ""))
        {
            if (keyImprimir and keyImprimirSocketData)
            {
                Serial.println("El dato ha llegado correctamente ");
            }
            receptionStatus = true;
            bitestadoYalarma = bitestadoYalarma & ~(1 >> noComunicacionSocket);
            estadoDelSistemaVisual = 4;
        }

        else
        {
            if (keyImprimir and keyImprimirSocketData)
            {
                Serial.println("El dato no llego correctamente ");
            }
            receptionStatus = false;
            bitestadoYalarma = bitestadoYalarma | (1 >> noComunicacionSocket);
        }
        dataRetrun = body;
    }
    return dataRetrun;
}
String funcionJsonCreator(String diviceIdx, int inputNumberx[], String epochTimex)
{
    JSONVar jsonArray;

    jsonArray[0]["deviceId"] = diviceIdx;
    jsonArray[0]["inputNumber"] = inputNumberx[0];
    jsonArray[0]["eventTime"] = epochTimex;
    jsonArray[0]["signal"] = listoEnviar_contador_producto_total;

    jsonArray[1]["deviceId"] = diviceIdx;
    jsonArray[1]["inputNumber"] = inputNumberx[1];
    jsonArray[1]["eventTime"] = epochTimex;
    jsonArray[1]["signal"] = listoEnviar_contador_descarte_por_batch;

    String jsonString = JSON.stringify(jsonArray);
    if (keyImprimirJSON)
    {
        Serial.println(jsonString);
    }
    return jsonString;
}
void parseJsonData(String data, bool enablePrint = true)
{
    if (enablePrint)
    {
        Serial.println("parse");
        Serial.println(data);
    }

    // Parsea la cadena JSON
    JSONVar myArray = JSON.parse(data);

    // Verifica si el tipo del objeto es un array
    if (JSON.typeof(myArray) != "array")
    {
        if (enablePrint)
        {
            Serial.println("Input is not an array!");
        }
        return;
    }

    // Itera sobre los elementos del array
    for (uint16_t i = 0; i < myArray.length(); i++)
    {
        JSONVar myObject = myArray[i];

        // Verifica si el tipo del objeto dentro del array es un objeto
        if (JSON.typeof(myObject) != "object")
        {
            if (enablePrint)
        {
                Serial.println("\nArray element is not an object!");
            }
            continue;
        }

        // Verifica si el objeto tiene la propiedad "productId" 
        if (myObject.hasOwnProperty("productId"))
        {
            if (enablePrint)
            {
                printLine();
                Serial.print("Element ");
                Serial.print(i);
                Serial.print(", productId = ");
                Serial.println((bool)myObject["productId"]);
                printLine();
            }
        }

        // Verifica si el objeto tiene la propiedad "unitQty" "productAlternativeUnitConversionMultiplier"
        if (myObject.hasOwnProperty("productAlternativeUnitConversionMultiplier"))
        {
            flagCheckTheProductQuantityUpdated = true; // update the product quantity
            producto_por_paso = (int)myObject["productAlternativeUnitConversionMultiplier"];
            product_id = (int)myObject["productId"];

            if (enablePrint)
            {
                printLine();
                Serial.print("Element ");
                Serial.print(i);
                Serial.print(", productAlternativeUnitConversionMultiplier = ");
                Serial.print(myObject["productAlternativeUnitConversionMultiplier"]);
                Serial.print(", productId = ");
                Serial.println(myObject["productId"]);
                printLine();
            }
        }

        // Puedes agregar más propiedades según sea necesario

        // Imprime el objeto JSON completo
        if (enablePrint)
        {
            printLine();
            Serial.print("Element ");
            Serial.print(i);
            Serial.print(", myObject = ");
            Serial.println(myObject);
            printLine();
        }
    }

    if (enablePrint)
    {
        Serial.println();
    }
}
String funcionRequestTosend(char *serverToSend, String headerToSend, String get_post, String dataToSend)
{
    String request;
    request += get_post + headerToSend + " HTTP/1.1\r\n";
    if (headerToSend == requestToBatch)
    {
        request += "Authorization: Basic YXBpdXNlckBzYW1hbjp0NXMxTlVKWXFFZjN6dVFKVEN0NlBlT3hSQ1ZZRlQ=\r\n";
    }
    request += "Accept: */*\r\n";
    request += "Host: " + String(serverToSend) + "\r\n";
    request += "User-Agent: arduino-ethernet\r\n";

    if (dataToSend != "")
    {

        request += "Content-Type: text/plain\r\n";
        request += "Connection: close\r\n";
        request += "Content-Length: " + String(dataToSend.length()) + "\r\n";
    }
    else
    {
        request += "Connection: close\r\n";
        request += "Content-Length: 0\r\n";
    }
    request += "\r\n";
    if (dataToSend != "")
    {
        request += dataToSend;
    }
    return request;
}
int funcionConnectAndSendSocketSSL(char *servidor, int port, String tipoDeSolicitud, String get_post, String autentification)
{
    WiFiSSLClient clientSSl;
    delay(100);
    if (keyImprimir)
    {
        Serial.println("\nStarting connection to server...");
    }
    if (clientSSl.connect(servidor, port)) // The port isn't 80 we need to connect to 443
    {
        Serial.println("connected to server");
        // Make a HTTP request:
        clientSSl.print("GET ");
        clientSSl.print(tipoDeSolicitud);
        clientSSl.println(" HTTP/1.1");
        if (autentification != "")
        { // creo esta parte para poder solicitar sin autorizacion
            clientSSl.println(autentification);
        }
        clientSSl.print("Host: ");
        clientSSl.println(servidor);
        clientSSl.println("Connection: close");
        clientSSl.println();
    }
    else
    {
        Serial.print("connected to server failed ");
        Serial.println(servidor);
        return -1;
    }
    const size_t rlen{1024};
    char rbuffer[rlen + 1]{};
    size_t rcount;
    String rec = "";
    String response = "";
    unsigned long tiempoMaximo = millis();

    while (millis() - tiempoMaximo < timeToWaitForReceptionSSl and !clientSSl.available())
    {
        // only to do some sleep for a while
    }

    while (clientSSl.available())
    {
        char c = clientSSl.read();
        // Serial.write(c);

        rcount += c;
        response += c;
        memset(rbuffer, 0, rlen);
        if (millis() - tiempoMaximo > timeToStopReceptionSSl)
        {
            if (keyImprimir and keyImprimirSocketData)
            {
                Serial.println("Cerrando recepcion por exceso de tiempo maximo");
                return -1;
            }
            break;
        }
    }

    parseJsonData(dataExtraction(response));
    //  if the server's disconnected, stop the client
    if (!clientSSl.connected())
    {
        if (keyImprimir)
        {
            Serial.println();
            Serial.println("disconnecting from server.");
        }
        clientSSl.stop();
        return -1;
    }
    return 0;
}

String funcionConnectAndSendSocketHTTP(char *servidor, int port, String tipoDeSolicitud, String get_post, String dataToSend)
{
    WiFiClient client;
    client.connect(servidor, port);
    String request = funcionRequestTosend(servidor, tipoDeSolicitud, get_post, dataToSend); // Create the request to send
    auto scount = client.println(request);                                                  // Send the request
                                                                                            //=====================Cola por si falla el envio de datos=========
    if (flagEncenderRenvioPorError)
    {
        // Como se esta dando el error al yo leer el dato que ellos me envian no voy a poner más en la cola
        if (!scount)
        {
            // Si falla el envío, se almacena en la cola
            dataQueue.push(request);
            Serial.println("Error to send data using the socket");
            return String("");
        }
    }
    //================================Print the Sent data================================
    if (keyImprimir and keyImprimirSocketData)
    {
        Serial.print("Sent ");
        Serial.print(scount);
        Serial.println(" bytes: ");
        Serial.print(request);
    }
    //================================Receive a simple HTTP response=====================
    delay(2000);
    const size_t rlen{1024};
    char rbuffer[rlen + 1]{};
    size_t rcount;

    // size_t rec{0};
    String rec = "";
    String response = "";
    unsigned long tiempoMaximo = millis();

    while (client.available())
    {
        char c = client.read();
        Serial.write(c);
        rcount += c;
        response += c;
        memset(rbuffer, 0, rlen);
        if (millis() - tiempoMaximo > 5000)
        {
            if (keyImprimir and keyImprimirSocketData)
            {
                Serial.println("Cerrando recepcion por exceso de tiempo maximo");
            }
            break;
        }
    }

    if (keyImprimir and keyImprimirSocketData)
    {
        Serial.print("Received ");
        Serial.print(rcount);
        Serial.println(" bytes: ");
        Serial.println(response);
        Serial.println("=========================================");
    }
    const String clTag = "Content-Length: ";
    auto clIndex = response.indexOf(clTag);
    clIndex += clTag.length();
    auto cl = response.substring(clIndex, clIndex + 2);
    if (keyImprimirSocketData)
    {
        Serial.println("el valor de clindex es: " + String(clIndex));
        Serial.println("el valor de cl es: " + String(cl));
    }
    const String bodyTag = "\r\n\r\n";
    auto bodyIndex = response.indexOf(bodyTag);
    if (bodyIndex != -1)
    {
        bodyIndex += bodyTag.length();
        auto body = response.substring(bodyIndex, bodyIndex + cl.toInt());
        if (keyImprimir and keyImprimirSocketData)
        {
            Serial.print("My public IPv4 Address is: ");
            Serial.println(body);
        }
        if ((body == "\"OK\"" or dataToSend != ""))
        {
            if (keyImprimir and keyImprimirSocketData)
            {
                Serial.println("El dato ha llegado correctamente ");
            }
            receptionStatus = true;
            bitestadoYalarma = bitestadoYalarma & ~(1 >> noComunicacionSocket);
            estadoDelSistemaVisual = 4;
        }

        else
        {
            if (keyImprimir and keyImprimirSocketData)
            {
                Serial.println("El dato no llego correctamente ");
            }
            receptionStatus = false;
            bitestadoYalarma = bitestadoYalarma | (1 >> noComunicacionSocket);
        }
        if (!client.connected())
        {

            Serial.println();

            Serial.println("disconnecting from server.");

            client.stop();
        }
        return body;
    }
    // Close the socket to return its memory and bring down the network interface
    return "";
}
String funcionConnectAndSendSocket(EthernetInterface &net, SocketAddress &addr, char *servidor, int port, String tipoDeSolicitud, String get_post, String dataToSend)
{
    TCPSocket socket; // TCPSocket socket; // If we dont want to connect encrypt the socket
    // TLSSocket socket; //connect to 443 port
    socket.open(&net);
    net.gethostbyname(servidor, &addr);
    addr.set_port(port);
    socket.connect(addr);
    String request = funcionRequestTosend(servidor, tipoDeSolicitud, get_post, dataToSend); // Create the request to send
    auto scount = socket.send(request.c_str(), request.length());                           // Send the request

    //=====================Cola por si falla el envio de datos=========
    if (flagEncenderRenvioPorError)
    {
        if (!scount)
        {
            // Si falla el envío, se almacena en la cola
            dataQueue.push(request);
            Serial.println("Error to send data using the socket");
            return String("");
        }
    }
    //================================Print the Sent data================================
    if (keyImprimir and keyImprimirSocketData)
    {
        Serial.print("Sent ");
        Serial.print(scount);
        Serial.println(" bytes: ");
        Serial.print(request);
    }
    //================================Receive a simple HTTP response=====================
    const size_t rlen{1024};
    char rbuffer[rlen + 1]{};
    size_t rcount;
    size_t rec{0};
    String response = "";
    unsigned long tiempoMaximo = millis();
    while ((rec = socket.recv(rbuffer, rlen)) > 0)
    {
        rcount += rec;
        response += rbuffer;
        memset(rbuffer, 0, rlen);
        if (millis() - tiempoMaximo > 1000)
        {
            if (keyImprimir and keyImprimirSocketData)
            {
                Serial.println("Cerrando recepcion por exceso de tiempo maximo");
            }
            break;
        }
    }

    if (keyImprimir and keyImprimirSocketData)
    {
        Serial.print("Received ");
        Serial.print(rcount);
        Serial.println(" bytes: ");
        Serial.println(response);
        Serial.println("=========================================");
    }
    const String clTag = "Content-Length: ";
    auto clIndex = response.indexOf(clTag);
    clIndex += clTag.length();
    auto cl = response.substring(clIndex, clIndex + 2);
    if (keyImprimirSocketData)
    {
        Serial.println("el valor de clindex es: " + String(clIndex));
        Serial.println("el valor de cl es: " + String(cl));
    }
    const String bodyTag = "\r\n\r\n";
    auto bodyIndex = response.indexOf(bodyTag);
    if (bodyIndex != -1)
    {
        bodyIndex += bodyTag.length();
        auto body = response.substring(bodyIndex, bodyIndex + cl.toInt());
        if (keyImprimir and keyImprimirSocketData)
        {
            Serial.print("My public IPv4 Address is: ");
            Serial.println(body);
        }
        if ((body == "\"OK\"" or dataToSend != ""))
        {
            if (keyImprimir and keyImprimirSocketData)
            {
                Serial.println("El dato ha llegado correctamente ");
            }
            receptionStatus = true;
            bitestadoYalarma = bitestadoYalarma & ~(1 >> noComunicacionSocket);
            estadoDelSistemaVisual = 4;
        }

        else
        {
            if (keyImprimir and keyImprimirSocketData)
            {
                Serial.println("El dato no llego correctamente ");
            }
            receptionStatus = false;
            bitestadoYalarma = bitestadoYalarma | (1 >> noComunicacionSocket);
        }
        socket.close();
        return body;
    }
    // Close the socket to return its memory and bring down the network interface
    return "";
}
String funcionConnectAndSendSocketEthernetTLS(EthernetInterface &net, SocketAddress &addr, char *servidor, int port, String tipoDeSolicitud, String get_post, String dataToSend)
{
    // TCPSocket socket; // TCPSocket socket; // If we dont want to connect encrypt the socket
    TLSSocket socket; // connect to 443 port
    socket.open(&net);
    net.gethostbyname(servidor, &addr);
    addr.set_port(port);
    socket.connect(addr);
    String request = funcionRequestTosend(servidor, tipoDeSolicitud, get_post, dataToSend); // Create the request to send
    auto scount = socket.send(request.c_str(), request.length());                           // Send the request

    //=====================Cola por si falla el envio de datos=========
    if (flagEncenderRenvioPorError)
    {
        if (!scount)
        {
            // Si falla el envío, se almacena en la cola
            dataQueue.push(request);
            Serial.println("Error to send data using the socket");
            return String("");
        }
    }
    //================================Print the Sent data================================
    if (keyImprimir and keyImprimirSocketData)
    {
        Serial.print("Sent ");
        Serial.print(scount);
        Serial.println(" bytes: ");
        Serial.print(request);
    }
    //================================Receive a simple HTTP response=====================
    const size_t rlen{1024};
    char rbuffer[rlen + 1]{};
    size_t rcount;
    size_t rec{0};
    String response = "";
    unsigned long tiempoMaximo = millis();
    while ((rec = socket.recv(rbuffer, rlen)) > 0)
    {
        rcount += rec;
        response += rbuffer;
        memset(rbuffer, 0, rlen);
        if (millis() - tiempoMaximo > 1000)
        {
            if (keyImprimir and keyImprimirSocketData)
            {
                Serial.println("Cerrando recepcion por exceso de tiempo maximo");
            }
            break;
        }
    }

    if (keyImprimir and keyImprimirSocketData)
    {
        Serial.print("Received ");
        Serial.print(rcount);
        Serial.println(" bytes: ");
        Serial.println(response);
        Serial.println("=========================================");
    }
    const String clTag = "Content-Length: ";
    auto clIndex = response.indexOf(clTag);
    clIndex += clTag.length();
    auto cl = response.substring(clIndex, clIndex + 2);
    if (keyImprimirSocketData)
    {
        Serial.println("el valor de clindex es: " + String(clIndex));
        Serial.println("el valor de cl es: " + String(cl));
    }
    const String bodyTag = "\r\n\r\n";
    auto bodyIndex = response.indexOf(bodyTag);
    if (bodyIndex != -1)
    {
        bodyIndex += bodyTag.length();
        auto body = response.substring(bodyIndex, bodyIndex + cl.toInt());
        if (keyImprimir and keyImprimirSocketData)
        {
            Serial.print("My public IPv4 Address is: ");
            Serial.println(body);
        }
        if ((body == "\"OK\"" or dataToSend != ""))
        {
            if (keyImprimir and keyImprimirSocketData)
            {
                Serial.println("El dato ha llegado correctamente ");
            }
            receptionStatus = true;
            bitestadoYalarma = bitestadoYalarma & ~(1 >> noComunicacionSocket);
            estadoDelSistemaVisual = 4;
        }

        else
        {
            if (keyImprimir and keyImprimirSocketData)
            {
                Serial.println("El dato no llego correctamente ");
            }
            receptionStatus = false;
            bitestadoYalarma = bitestadoYalarma | (1 >> noComunicacionSocket);
        }
        socket.close();
        return body;
    }
    // Close the socket to return its memory and bring down the network interface
    return "";
}
/*
Esta funcion activa las banderas para el envio del dato a evocon
Si no se produce el envio en un tiempo determiando por la varieable timeoutForSendingData
se guada en una cola de datos para luego  enviarse en la proxima oportunidad
*/
void funcionEnviarAEvocon()
{
    unsigned long timeoutSend = HAL_GetTick();
    flagEnviarAevocon = true;
    flagEnvioRealizadoAEvocon = false;
    if (flagGeneralCounter)
    {
        upgradeGeneralcounters();
    }
    if (flagEncenderRenvioPorError)
    {

        while (!flagEnvioRealizadoAEvocon) //
        {
            if (HAL_GetTick() - timeoutSend > timeoutForSendingData)
            {
                time_t tiempo = rtc_controller.getEpoch();
                char timestampString[20]; // Tamaño suficiente para almacenar la cadena de caracteres
                sprintf(timestampString, "%lu", (unsigned long)tiempo);
                String respaldo = funcionJsonCreator(deviceIdx, inputNumberx, String(timestampString));
                dataQueue.push(respaldo);
                Serial.println("\n");
                Serial.println("========================================= ");
                Serial.println("Hay un error en la comunicacion con evocon");
                Serial.print("La cola de datos tiene ");
                Serial.println(dataQueue.size());
                Serial.println("========================================= ");

                flagBuckupJson = true;
                break;
            }
        }
    }
}
void funcionVerificacionNoCero()
{
    if (listoEnviar_contador_producto_total != 0)
    {
        flagNotAllZero = true;
        if (keyImprimir)
        {
            Serial.println("Se van a enviar datos ya que hay datos medidos que no son cero");
        }
    }
    else
    {
        flagNotAllZero = false;
        if (keyImprimir)
        {
            Serial.println("Todos los datos medidos son cero");
        }
    }
}

void readCharsFromFile(const char *path)
{
    Serial.print("readCharsFromFile: ");
    Serial.print(path);

    FILE *file = fopen(path, "r");

    if (file)
    {
        Serial.println(" => Open OK");
    }
    else
    {
        Serial.println(" => Open Failed");
        return;
    }

    char c;

    while (true)
    {
        c = fgetc(file);

        if (feof(file))
        {
            break;
        }
        else
            Serial.print(c);
    }

    fclose(file);
}

void readFile(const char *path)
{
    Serial.print("Reading file: ");
    Serial.print(path);

    FILE *file = fopen(path, "r");

    if (file)
    {
        Serial.println(" => Open OK");
    }
    else
    {
        Serial.println(" => Open Failed");
        return;
    }

    char c;
    uint32_t numRead = 1;

    while (numRead)
    {
        numRead = fread((uint8_t *)&c, sizeof(c), 1, file);

        if (numRead)
            Serial.print(c);
    }

    fclose(file);
}

void writeFile(const char *path, const char *message, size_t messageSize)
{
    Serial.print("Writing file: ");
    Serial.print(path);

    FILE *file = fopen(path, "w");

    if (file)
    {
        Serial.println(" => Open OK");
    }
    else
    {
        Serial.println(" => Open Failed");
        return;
    }

    if (fwrite((uint8_t *)message, 1, messageSize, file))
    {
        Serial.println("* Writing OK");
    }
    else
    {
        Serial.println("* Writing failed");
    }

    fclose(file);
}

void appendFile(const char *path, const char *message, size_t messageSize)
{
    Serial.print("Appending file: ");
    Serial.print(path);

    FILE *file = fopen(path, "a");

    if (file)
    {
        Serial.println(" => Open OK");
    }
    else
    {
        Serial.println(" => Open Failed");
        return;
    }

    if (fwrite((uint8_t *)message, 1, messageSize, file))
    {
        Serial.println("* Appending OK");
    }
    else
    {
        Serial.println("* Appending failed");
    }

    fclose(file);
}

void deleteFile(const char *path)
{
    Serial.print("Deleting file: ");
    Serial.print(path);

    if (remove(path) == 0)
    {
        Serial.println(" => OK");
    }
    else
    {
        Serial.println(" => Failed");
        return;
    }
}

void renameFile(const char *path1, const char *path2)
{
    Serial.print("Renaming file: ");
    Serial.print(path1);
    Serial.print(" to: ");
    Serial.print(path2);

    if (rename(path1, path2) == 0)
    {
        Serial.println(" => OK");
    }
    else
    {
        Serial.println(" => Failed");
        return;
    }
}

void testFileIO(const char *path)
{
    Serial.print("Testing file I/O with: ");
    Serial.print(path);

#define BUFF_SIZE 512

    static uint8_t buf[BUFF_SIZE];

    FILE *file = fopen(path, "w");

    if (file)
    {
        Serial.println(" => Open OK");
    }
    else
    {
        Serial.println(" => Open Failed");
        return;
    }

    size_t i;
    Serial.println("- writing");

    uint32_t start = millis();

    size_t result = 0;

    // Write a file with FILE_SIZE_KB
    for (i = 0; i < FILE_SIZE_KB * 2; i++)
    {
        result = fwrite(buf, BUFF_SIZE, 1, file);

        if (result != 1)
        {
            Serial.print("Write result = ");
            Serial.println(result);
            Serial.print("Write error, i = ");
            Serial.println(i);

            break;
        }
    }

    Serial.println("");
    uint32_t end = millis() - start;

    Serial.print(i / 2);
    Serial.print(" Kbytes written in (ms) ");
    Serial.println(end);

    fclose(file);

    printLine();

    /////////////////////////////////

    file = fopen(path, "r");

    start = millis();
    end = start;
    i = 0;

    if (file)
    {
        start = millis();
        Serial.println("- reading");

        result = 0;

        fseek(file, 0, SEEK_SET);

        // Read a file with FILE_SIZE_KB
        for (i = 0; i < FILE_SIZE_KB * 2; i++)
        {
            result = fread(buf, BUFF_SIZE, 1, file);

            if (result != 1)
            {
                Serial.print("Read result = ");
                Serial.println(result);
                Serial.print("Read error, i = ");
                Serial.println(i);

                break;
            }
        }

        Serial.println("");
        end = millis() - start;

        Serial.print((i * BUFF_SIZE) / 1024);
        Serial.print(" Kbytes read in (ms) ");
        Serial.println(end);

        fclose(file);
    }
    else
    {
        Serial.println("- failed to open file for reading");
    }
}

void resetGeneralcounters()
{
    contadorGeneral_Descarte_Manual = 0;
    contadorGeneral_producto_bueno_por_batch = 0;
    contadorGeneral_descarte_por_batch = 0;
}
String getNewQuantityBatch(int query)
{
    flagCheckTheProductQuantityUpdated = false;
    String dataToReturn = "";
    int countOfTests = 0;
    switch (query)
    {
    case 0:
        for (int i = 0; i < numberOfmaximumTests; i++)
        {
            countOfTests += 1;

            if (flagCheckTheProductQuantityUpdated)
            {
                break;
            }
            else
            {
                funcionConnectAndSendSocketSSL(serverTime, 443, requestToBatch, "GET ", autentification);
            }
            if (keyImprimir)
            {
                printLine();
                Serial.print("This is the test number ");
                Serial.println(countOfTests);
                printLine();
            }
        }
        break;
    case 1:
        dataToReturn = funcionConnectAndSendSocketHTTP(serverTime, 443, requestToBatch, "GET ", "");
        break;
    case 2:
        funcionConnectAndSendSocketSSL(server3, 443, "/", "GET ", autentification);
        break;
    case 3:
        dataToReturn = funcionConnectAndSendSocketHTTP(server3, 80, requestToBatch, "GET ", "");
        break;
    default:
        break;
    }
    if (keyImprimir)
    {

        printLine();
        Serial.print("El multiplicador es ahora = ");
        Serial.println(producto_por_paso);
        printLine();
    }
    flagCheckTheProductQuantity = true;
    return dataToReturn;
}

void printWifiStatus()
{
    Serial.println("Connected to wifi");
    // print the SSID of the network you're attached to:

    Serial.print("SSID: ");

    Serial.println(WiFi.SSID());

    // print your WiFi shield's IP address:

    IPAddress ip = WiFi.localIP();

    Serial.print("IP Address: ");

    Serial.println(ip);

    // print the received signal strength:

    long rssi = WiFi.RSSI();

    Serial.print("signal strength (RSSI):");

    Serial.print(rssi);

    Serial.println(" dBm");
}
/*
Testea el dataIn si es una hora unxi correcta

*/
bool functionQuestUnixTimeCorrect(String dataIn)
{
    // Convierte el string a un número entero
    bool unixTimeRecibed = false;
    int horaUnix = dataIn.toInt();

    // Verifica si el número es plausible como hora Unix
    if (horaUnix > 0 && horaUnix < 2147483647)
    {
        if (keyImprimir)
        {
            Serial.println("Es una hora Unix válida.");
            // Puedes realizar otras acciones aquí si es necesario
        }
        unixTimeRecibed = true;
    }
    else
    {
        if (keyImprimir)
        {
            Serial.println("No es una hora Unix válida.");
        }
    }
    return unixTimeRecibed;
}
/*
Devuelve string con hora unix interna
*/
String funcionObtenerUnixTimeInterno()
{
    if (keyImprimir)
    {
        Serial.println("Usando una hora Unix interna...");
    }
    unsigned long ulUnixTime = static_cast<unsigned long>(rtc_controller.getEpoch());

    return String(ulUnixTime);
}
/*
Esta funcion permite mandar siempre redondeando el tiempo para que sea un minuto justo
*/
String redondeoUnixTime(const String &data)
{
    for (size_t i = 0; i < data.length(); i++)
    {
        if (!isdigit(data[i]))
        {
            // Manejar error de entrada no numérica
            return "-1"; // O un valor que indique un error
        }
    }
    // Convertir a tiempo Unix
    unsigned long ulUnixTime = data.toInt();

    // Redondear al minuto más cercano
    unsigned long resto = ulUnixTime % 60;
    if (resto > 30)
    {
        ulUnixTime += (60 - resto);
    }
    else if (resto == 0)
    {
        return String(ulUnixTime);
    }
    else
    {
        ulUnixTime -= resto;
    }

    return String(ulUnixTime);
}

/*Esta funcion genera un keepAlive del dispositivo para saber que esta funcionando*/
void funcionKeepAlive()
{
    if (funcionConnectAndSendSocketSSL(serverToSendData, 443, requestToKeepAlive, "GET ", "") == 0)
    {
        if (keyImprimir && keyImprimirKeepAlive)
        {
            printLine();
            Serial.print("La hora del keepAlive es : ");
            Serial.println(funcionObtenerUnixTimeInterno());
            printLine();
        }
    }
    else
    {
    }
}
/*esta funcion permite enviar periodicamente el keepAlive */
void funcionActivarKeepAlive(unsigned long *tiempokeepAlive)
{
    if (HAL_GetTick() - *tiempokeepAlive > timeToQuerryKeepAlive)
    {
        /*
        Here we will send the keepAlive messenge to the Evocon Server.
            */
        funcionKeepAlive();

        *tiempokeepAlive = HAL_GetTick();
    }
}
#endif