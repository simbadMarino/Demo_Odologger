
//Codigo Demo para la manufactura de Odologger
//v0.5

//HEADER FILES##############################################
#include <SPI.h>
#include <MFRC522.h>
#include "nRF24L01.h"
#include "RF24.h"
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>
#include <SD.h>
#include "Timer.h"
#include<stdlib.h>
#include "printf.h"
//#################################################################

//SW Version########################################################
String SWver = "SW ver: 2.5 Date: 28/01/16";
String diagCmd = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete


//SD Card DEfinitions###########################
File gpsLogFile;
File root;
const int chipSelect = 11;
bool headerFlag=0;
//##############################################

//LED Definitions############################################################
const int NOK_LED = 10;
const int OK_LED = 6;

//GPS Definitions and Vars###################################################
static const int RXPin = 62, TXPin = 63;
static const uint32_t GPSBaud = 9600;

// The TinyGPS++ object
TinyGPSPlus gps;

boolean firstTime=1;
// The serial connection to the GPS device
SoftwareSerial ss(RXPin, TXPin);

//RF_ID Definitions##########################################################
#define SS_PIN 7    //Arduino Uno
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);        // Create MFRC522 instance.
//#######################################################################

#define SendButton 8


//RF24 Variables and CFG##########################################################################################

int rfReturnVal = 0;
boolean rfDataflag = 0;

// Set up nRF24L01 radio on SPI bus plus pins 5(CE) & 4(CS)
RF24 radio(5,4);

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };

// The various roles supported by this sketch
typedef enum { role_sender = 1, role_receiver } role_e;

// The debug-friendly names of those roles
const char* role_friendly_name[] = { "invalid", "Ping out", "Pong back"};

// The role of the current running sketch
role_e role;

static uint32_t message_count = 0;
//char SendPayload[31] = "#ERROR|ERROR|K00000|V00|D01#";




//RFID COnfig##########################################################################



    byte dataBlock[]    = {
        0x00, 0x00, 0x00, 0x00, //  0, 0, 0, 0,
        0x00, 0x00, 0x00, 0x00, //  0, 0, 0, 0,
        0x00, 0x00, 0x00, 0x00, //  0, 0, 0, 0,
        0x00, 0x00, 0x00, 0x00  //  0, 0, 0, 0
    };
int succesCounter=0;
boolean flagClearSector;
byte sector;
byte trailerBlock;
int BlockNumber;
int SectorToClear;
int cardIsonPosition= 1;
char data_to_write[16];
String strCmd = "";

//byte buffer[34];  
//byte status, len;
byte blockAddr;
//byte size = sizeof(buffer);
int i;
boolean flagGps=1;
boolean flagGpsAlive=0;
MFRC522::MIFARE_Key key;
//###########################################################################################


void setup() {

 //SERIAL Protocol####################################################################
  Serial.begin(115200); //Inicializa Serial a 115200 baudios
 //###################################################################################

  //CFG GPS Module#####################################################################
  ss.begin(GPSBaud);
 //###################################################################################

 //CFG LED Pins#######################################################################
 pinMode(NOK_LED,OUTPUT);
 pinMode(OK_LED,OUTPUT);
 digitalWrite(NOK_LED,HIGH);
 digitalWrite(OK_LED,HIGH);
 //###################################################################################

 //CFG RF module######################################################################

  //Choose Tx, Odologger will Tx to the station
  role = role_receiver;
  radio.begin();
  printf_begin();
  //Setting Data rate to 2Mbps
  radio.setDataRate(RF24_2MBPS);
  
  // optionally, increase the delay between retries & # of retries( 0 = Delay, 5 = # of retries)
  radio.setRetries(0,2);

  radio.setPayloadSize(32);
  radio.setPALevel(RF24_PA_MAX);
  //radio.setChannel(70);
  radio.enableDynamicPayloads();

  
  radio.openWritingPipe(pipes[0]);
 // radio.openReadingPipe(1,pipes[1]);

     radio.startListening();
  // Dump the configuration of the rf unit for debugging


   //attachInterrupt(4, check_radio, FALLING);
 //###################################################################################


//CFG rfID module####################################################################
 SPI.begin();                // Init SPI bus
 //mfrc522.PCD_Init();        // Init MFRC522 card

  //IRQ!!!
  /* mfrc522.PCD_WriteRegister(MFRC522::ComIrqReg, 0x80); //Clear interrupts
  mfrc522.PCD_WriteRegister(MFRC522::ComIEnReg, 0x7F); //Enable all interrupts
  mfrc522.PCD_WriteRegister(MFRC522::DivIEnReg, 0x14);
  attachInterrupt(5, isr, FALLING);*/
  
 //###################################################################################


  
 //CFG Digital Pin Interrupt##########################################################
    pinMode(SendButton, INPUT_PULLUP);       //Use internal pullup resistor for the switch
 //###################################################################################




tc_01_LedsBasicTest();
tc_02_SDcardConnectTest();
tc_03_rfIDbasicTest();
tc_04_RFTest();

  Serial.println("");
  Serial.println("");
  Serial.println("-----TEST 5: FUNCIONAMIENTO BASICO DE GPS-----");
  Serial.println("");

}

void loop() 
{
  
 /*if(flagGps)
 { Serial.println("flag");
   for(i=0;i<10;i++)
   {*/
   tc_05_GPSbasicTest();
   /*Serial.println("GPS");
   }
 */
}


int tc_01_LedsBasicTest()
{
  Serial.println("-----TEST 1: FUNCIONAMIENTO BASICO DE LEDS------");
  Serial.println("");
  Serial.println("LED AZUL de POWER encendido? ");
  delay(2000);
  Serial.println("Ahora encenderan los leds Rojo y Verde...");
  delay(2000);
  digitalWrite(NOK_LED,LOW);
  digitalWrite(OK_LED,LOW);
  Serial.println("LED ROJO y VERDE encendidos? ");
  delay(2000);
  Serial.println("Fin de la prueba de LEDs");

}

int tc_02_SDcardConnectTest()
{
  Serial.println("");
  Serial.println("");
  Serial.println("-----TEST 2: FUNCIONAMIENTO BASICO DE SD-----");
    Serial.println("");
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Falla en comunicacion, asegurate que la uSD esté insertada e intenta de nuevo");
    Serial.println("FAIL");
    return 0;
  }
  else
    {
      Serial.println("Tarjeta uSD Detectada");
      Serial.println("PASS");
      return 1;
    }

}

int tc_03_rfIDbasicTest()
{
  Serial.println("");
  Serial.println("");
  Serial.println("-----TEST 3: FUNCIONAMIENTO BASICO DE RFid-----");
  Serial.println("");
  mfrc522.PCD_Init();                             // Init MFRC522 chip again per each write attemp  
  mfrc522.PCD_DumpVersionToSerial();              // Show details of PCD - MFRC522 Card Reader details
   
    // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    Serial.println("Targeta fuera de alcance");
    
  }
  else
  {
    Serial.println("Targeta dentro de alcance");
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    Serial.println("Falla en lectura de tarjeta");
    
  }
  else
  {
    Serial.println("RFid OK :) !");
    Serial.println("PASS");
  }

  
   
}


int tc_04_RFTest()
{
  Serial.println("");
  Serial.println("");
  Serial.println("-----TEST 4: FUNCIONAMIENTO BASICO DE RF-----");
  Serial.println("");
  radio.printDetails();
  
}


int tc_05_GPSbasicTest()
{

 /* Serial.println("");
  Serial.println("");
  Serial.println("-----TEST 5: FUNCIONAMIENTO BASICO DE GPS-----");
  Serial.println("");*/
  


   while (ss.available())
{
 int c = ss.read();
 gps.encode(c);
}
  if (gps.location.isUpdated())
  {
      gps.location.lat();
      gps.location.lng();
    
  }

else if (gps.satellites.isUpdated())
  {
    Serial.println(F("GPS Alive, Sats Available="));
    Serial.println(gps.satellites.value());
       digitalWrite(NOK_LED,flagGpsAlive);
        digitalWrite(OK_LED,flagGpsAlive);
        flagGpsAlive=!flagGpsAlive;
  }


 
  
}

