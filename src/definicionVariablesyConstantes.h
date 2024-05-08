#ifndef VARIABLES_H
#define VARIABLES_H
#include <Arduino_MachineControl.h>
#include <queue>
#include <LittleFS_Portenta_H7.h>
using namespace machinecontrol;

//===============================Selección de la linea==================
#define verisionOfSoftware "V1.1.1-20-04-2024"
#define LINEA 10 // Posibles valores***************************************************************************************************************************
// Estos valores permiten configurar rapidamente todos los parametros a
// a definir por Adium desde IT

#define dnsAdium "192.168.1.236"    //
#define gatewayAdium "10.66.20.254" //"10.66.20.254"
#define maskAdium "255.255.255.0"
String deviceIdx = "APIGENBA"; //"APIADIUM";
//=========================Configuracion de Lineas No tocar=========
#if LINEA == 1
const char *ip "10.66.20.56" const char *dns1 dnsAdium const char *gateway gatewayAdium const char *mask maskAdium
    uint8_t mac[] = {0x44, 0x00, 0x21, 0xE1, 0x80, 0x00};
int inputNumberx[] = {43, 44, 45, 46, 47, 48};

#elif LINEA == 10
const char *ip = "192.168.137.223";
const char *dns1 = "200.40.30.245";
const char *dns2 = "200.40.220.245";
const char *gateway = "192.168.137.1";
const char *mask = "255.255.255.0";
// uint8_t mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
int inputNumberx[] = {1, 2, 3, 4, 5, 6}; //{1, 2, 3, 4, 5, 6};

#endif

//----------------------------------------------------------------Logica del sensor + o menos----------------------
// La linea 16 utiliza el sensor de descarte estuchado con logica negativa
// 1 logica positiva
// 0 logica negativa
/*

#define logica_embazadora_1 0 // 0
#define logica_embazadora_2 1
#define logica_transporte_1 1
#define logica_transporte_2 1
#define logica_final_transporte_lineal 1
#define logica_balanza_activa 1
*/

#define logica_Descarte_1 1 // 0
#define logica_Descarte_2 1
#define logica_Descarte_3 1
#define logica_Descarte_4 1
#define logica_Descarte_5 1
#define logica_Descarte_6 1
#define logica_Descarte_Manual 1
#define logica_Paso 1
//----------------------------------------------------------------Definicion de pin----------------------
/*Definimos los pines que van a cada entrada*/
#define Pin_Descarte_1 DIN_READ_CH_PIN_00
#define Pin_Descarte_2 DIN_READ_CH_PIN_01
#define Pin_Descarte_3 DIN_READ_CH_PIN_02
#define Pin_Descarte_4 DIN_READ_CH_PIN_03
#define Pin_Descarte_5 DIN_READ_CH_PIN_04
#define Pin_Descarte_6 DIN_READ_CH_PIN_05
#define Pin_Descarte_Manual DIN_READ_CH_PIN_06
#define Pin_paso DIN_READ_CH_PIN_07
#define Pin_led_Verde 0
#define Pin_led_Rojo 1

//----------------------------------------------------------------Estado del sensor----------------------

bool actualEstado_Descarte_1 = false;
bool actualEstado_Descarte_2 = false;
bool actualEstado_Descarte_3 = false;
bool actualEstado_Descarte_4 = false;
bool actualEstado_Descarte_5 = false;
bool actualEstado_Descarte_6 = false;
bool actualEstado_Descarte_Manual = false;
bool actualEstado_Paso = false;


bool previoEstado_Descarte_1 = false;
bool previoEstado_Descarte_2 = false;
bool previoEstado_Descarte_3 = false;
bool previoEstado_Descarte_4 = false;
bool previoEstado_Descarte_5 = false;
bool previoEstado_Descarte_6 = false;
bool previoEstado_Descarte_Manual = false;
bool previoEstado_Paso = false;

// Tiempo actual donde se esta ejecutando las cuentas

int16_t actual_contador_producto_bueno_durante_un_paso = 0;
int16_t actual_contador_descarte_en_un_paso = 0;
int16_t actual_contador_Descarte_Manual_por_batch = 0;
int16_t actual_contador_producto_bueno_por_batch = 0;
int16_t actual_contador_descarte_por_batch = 0;

// cuando pasa el tiempo especificado se copian los contadores actuales a estos para poder
// absorver alguna cuenta equivocada que se halla realizado previamente

int16_t previoEnvio_contador_producto_bueno_por_batch = 0;
int16_t previoEnvio_contador_descarte_por_batch = 0;

// cuando pasa el tiempo especificado se copian los contadores previoEnvio a estos para poder
// ser enviados al servidor

int16_t listoEnviar_contador_producto_bueno_por_batch = 0;
int16_t listoEnviar_contador_descarte_por_batch = 0;
int16_t listoEnviar_contador_producto_total = 0;

/*
Estos contadores lo usaremos como ayuda para poder visualizar todos los paquetes enviados
Al enviar los datos se sumara el el dato enviado a este contador y se reiniciaran haciendo uso
del serial.
II inicia el contador general
IR reinicia el contador general
IP imprime el contador general
*/

int16_t contadorGeneral_Descarte_Manual = 0;
int16_t contadorGeneral_producto_bueno_por_batch = 0;
int16_t contadorGeneral_descarte_por_batch = 0;

int producto_por_paso = 6; // This value indicates the maximum product in a batch

// Constantes de funcionamiento
uint16_t tiempoParaFiltro = 5; // 20 ms para asegurarnos de que fue presionado correctametne
float thresholdsParaFiltro = 0.65;
// Variables para control
int16_t contador_de_pasos_por_batch = 0;
int16_t contador_de_Batch_por_ciclo = 0;
//----------------------------------------------------------------Temporizadores----------------------------------------------------------------
#define ConstanteMillisToSeconds 1000
#define setpointTiempoEnvio 60 * ConstanteMillisToSeconds
#define tiempoImprimirInputs 0.3 * ConstanteMillisToSeconds
unsigned long timerImprimirInputs = 0;
#define timeoutForSendingData 10 * ConstanteMillisToSeconds
#define timeTosleepSerialControl 50ms
unsigned long timerToprintActualCounter = 0;
unsigned long time_system_active_duration = 0;
#define timeToprintActualCounter 0.5 * ConstanteMillisToSeconds
#define timeToStop 70 * ConstanteMillisToSeconds
#define timeToStopReceptionSSl 5 * ConstanteMillisToSeconds
#define timeToWaitForReceptionSSl 3 * ConstanteMillisToSeconds
#define timeToQuerryQuantityOfPackages 10 * 60 * ConstanteMillisToSeconds
#define timeToQuerryKeepAlive 4.1 * 60 * ConstanteMillisToSeconds

//----------------------------------------------------------------Keys----------------------------------------------------------------
bool keyImprimir = true;
bool keyImprimirJSON = false;
bool keyImprimirTestDescarte = false;
bool keyImprimirEstadosSensor = false;
bool keyImprimirContadorListoEnviar = true;
bool keyImprimirMac = false;
bool keyImprimirSocketData = true;
bool keyImprimirKeepAlive = true;
bool flagDescarteBalanza = false;
bool flagEnviarAevocon = false;
bool flagEnvioRealizadoAEvocon = false;
bool flagNotAllZero = false;
bool flagActivarControlSerial = true;
bool flagEnviarSiempre = false;          // solo para pruebas
bool flagInputControlFromSerial = false; // This flag is used to control the inputs using the serial port without the input hardware signal
bool flagUpdatePrevioDato = false;
bool flagBuckupJson = false;
bool flagGeneralCounter = false;         // This flag is used to eneable the general counter
bool flagfirstStart = true;              // This flag is used to know if this is the first time the system is turn on
bool flagFirstEmpaquetadoPacage = true;  // This flag is used to determine if a package passes through an S3 sensor
bool flagTransportador_3_Manual = false; // This flag is used to simulate the signal from scale
bool flagImpresionConteoActual = false;  // This flag is used to print the current number of the actual counter
bool flagWasRuningButStop = true;
bool flagStartTemporizationStop = false;         // This flag is used to
bool flagCheckTheProductQuantity = false;        // This flag is used to check the product quantity in the evocon server
bool flagCheckTheProductQuantityUpdated = false; // This flag is updated when the product quantity is recived
bool flagUnixTimeWasUpdated = false;             // This flag is true when the unix time is updated correctly
bool flagEncenderRenvioPorError = true;          // This flag is used to eneable re send information if the reques is fail
bool flagFuncionando = false;                    // "This flag indicates when the system detects the input "Paso" within the last minute before the time finishes. Once the time is up, this flag is set to off."
//----------------------------------------------------------------Serial----------------------------------------------------------------
#define CharkeyImprimirEstadosSensor 's'
#define CHarkeyImprimirSocketData 'd'
#define CharkeyImprimirContadorListoEnviar 'c'
#define CharkeyImprimirContadorActual 'A'
#define CharkeyImprimir 'i'
#define CharMostrarLetras 't'
#define CharSendContinuo 'u'
#define CharSendMac 'm'
#define CharJSONVew 'f'
#define charControlPorSerial 'H'
#define charSerialActivarControlSerial 0
#define charSerialDESCARTE_1 1
#define charSerialDESCARTE_2 2
#define charSerialDESCARTE_3 3
#define charSerialDESCARTE_4 4
#define charSerialDESCARTE_5 5
#define charSerialDESCARTE_6 6
#define charSerialDESCARTE_MANUAL 7
#define charSerial_PASO 8
#define charSerialFile 'F'
#define charSerialGeneralCounter 'G'
#define charSerialGeneralCounterSTART 'G'
#define charSerialGeneralCounterRESET 'R'
#define charSerialGeneralCounterPRINT 'P'
#define charSerialVersionOftheSoftware 'V'
#define charSerialCheckProductQuantity 'Q'
#define charSerialCheckProductQuantity_Actual '0'
#define charSerialCheckProductQuantity_New '1'
#define CharkeyImprimirEstadosWifi 'W'
#define CharkeyImprimirUnixTime 'U'
#define CharkeyKeepAlive 'K'
//----------------------------------------------------------------Evocon----------------------------------------------------------------
// int multiplicador_de_producto_X_embazado = 6;

#define numberOfmaximumTests 4
char serverTime[] = "api.evocon.com";
char serverToSendData[] = "devices.evocon.com";
char server3[] = "eoe3m066beg7c7s.m.pipedream.net"; //"192.168.1.4"; //"10.66.20.121"; //"192.168.1.21";
int port = 80;
String encabezadoMuchos = "/shards?multiple=true "; // https://api.evocon.com/shards?multiple=true
String requestToTime = "/EvoconReportingServer/rest/v1/time/APIGENBA";
String requestToBatch = "/api/reports/activeproduct?stationId=1";
String autentification = "Authorization: Basic YXBpdXNlckBzYW1hbjp0NXMxTlVKWXFFZjN6dVFKVEN0NlBlT3hSQ1ZZRlQ=";
String requestToKeepAlive = "/EvoconReportingServer/rest/v1/time/" + deviceIdx;
String unixTime = "1681579521";
int counter_Unix_Time_start = 10; // maxima cantidad de veces que se testeara que la hora unix sea actualizada
std::queue<String> dataQueue;
bool receptionStatus = false;
String text;
String jsonForRequest = "";

bool ethernetConnected = true;

//========================Estados de luz==============
#define cableRedDesconectado 0
#define noComunicacionSocket 1
#define balanzaApagada 2
#define seEnvioDato 3

// bit 0 cable Red Desconectado
// bit 1 no Comunicacion Socket
// bit 2 balanza apagada
// bit 3 seEnvioDato
byte bitestadoYalarma = 0;
// estadoDelSistemaVisual=0 == Cable de red desconectado
// estadoDelSistemaVisual=1 == Cable de red conectado pero hay error en la comunicacion con socket
// estadoDelSistemaVisual=2 == Balanza apagada
// estadoDelSistemaVisual=3 == energizado y balanza ok
// estadoDelSistemaVisual=4 == envio dato correcto
byte estadoDelSistemaVisual = 0;
//========================File system data ==============
// This is not used in this version of the program, but will be used in the next version
LittleFS_MBED *myFS;
char fileName1[] = MBED_LITTLEFS_FILE_PREFIX "/evocon.txt";
uint32_t FILE_SIZE_KB = 64;
//========================Wifi data ==============
bool flagActivarWifi = true;      // flag to activate wifi connection
bool flagWifiWithoutDHCP = false; // flag to activate fixed ip
char ssid[] = "HUAWEI_311_1061";  //"Chiquito-2.4GHz";  //"HUAWEI_311_1061";  //"Robot Envasadora"; //"Redmi9T";//  your network SSID (name)
char pass[] = "genba2024";        //"ClaRe..2022";      //"genba2024";        //"GD!6#aGRD5";       // "9715174c9cd6";// your network password (use for WPA, or use as key for WEP)
byte mac[6];                      // the MAC address of your Wifi shield
//=========================Function declaration========================

#endif