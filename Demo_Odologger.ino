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
//#################################################################

//SW Version########################################################
String SWver = "SW ver: 2.4 Date: 21/01/16";
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
typedef enum { role_ping_out = 1, role_pong_back } role_e;

// The debug-friendly names of those roles
const char* role_friendly_name[] = { "invalid", "Ping out", "Pong back"};

// The role of the current running sketch
role_e role;

static uint32_t message_count = 0;
//char SendPayload[31] = "#ERROR|ERROR|K00000|V00|D01#";


void check_radio(void);

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
  role = role_ping_out;
  radio.begin();
  
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
  radio.printDetails();

   attachInterrupt(4, check_radio, FALLING);
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

}

void loop() {



}


int tc_01_LedsBasicTest()
{
  Serial.println("-----TEST 1: FUNCIONAMIENTO BÁSICO DE LEDS------");
  Serial.println("");
  Serial.println("LED AZUL de POWER encendido? s/n");
  delay(3000);
  Serial.println("Ahora encenderan los leds Rojo y Azul...");
  delay(5000);
  digitalWrite(NOK_LED,LOW);
  digitalWrite(OK_LED,LOW);
  Serial.println("LED ROJO y VERDE encendidos? s/n");
  delay(5000);
  Serial.println("Fin de la prueba de LEDs");
  
}

int tc_02_SDcardConnectTest()
{
  Serial.println("-----TEST 2: FUNCIONAMIENTO BÁSICO DE SD-----");
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



void check_radio(void)
{
  //Serial.println("IRQ");
  // What happened?
  bool tx,fail,rx;
  
  radio.whatHappened(tx,fail,rx);

  rfDataflag = 1;
  // Have we successfully transmitted?
  if ( tx )
  {
   

  }
    else
    {
    //  Serial.println("Cards not available");
     /* digitalWrite(NOK_LED,LOW);
      delay(500);
      digitalWrite( NOK_LED,HIGH);*/
    //  rfReturnVal = -1;   //We transmited Data but somethig is wrong with tags
    //}
      
  }

  // Have we failed to transmit?
  if ( fail )
  {
       /*digitalWrite(NOK_LED,LOW);
       delay(500);
       digitalWrite( NOK_LED,HIGH);
       Serial.println("Send:Failed\n\r");*/
     //  rfReturnVal = 0;   //Definetly we failed transmission of DATA 
  }



  // Did we receive a message?
  if ( rx )
  {
   
      radio.read(&message_count,sizeof(message_count));
      Serial.print("ACK: ");
      Serial.println(message_count);
    

    
  }
}


