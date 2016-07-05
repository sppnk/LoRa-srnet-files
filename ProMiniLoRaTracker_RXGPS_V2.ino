//ProMiniLoRaTracker_RXGPS_V2.ino
/*****************************************************************************************************
ProMiniLoRaTracker_RXGPS_V2 Programs

Copyright of the author Stuart Robinson - 23/07/2015 15:00

These programs may be used free of charge for personal, recreational and educational purposes only.

This program, or parts of it, may not be used for or in connection with any commercial purpose without
the explicit permission of the author Stuart Robinson.

The programs are supplied as is, it is up to individual to decide if the programs are suitable for the
intended purpose and free from errors.

This receiver program requires an attached GPS. The program will receive the GPS co-ordinates of the
tracker transmitter, and using the local co-ordinates from the attached GPS, will calculate and display
the distance and direction to the tracker transmitters last known position.
******************************************************************************************************
*/

//Hardware definitions
const byte lora_PNSS = 10;	//pin number where the NSS line for the LoRa device is connected.
const byte PLED1 = 8;           //pin number for LED on Tracker
const byte lora_PReset = 9;	//pin where LoRa device reset line is connected
const byte lora_PPWMCH = 3;     //pin number for tone generation, connects to LoRa device DIO2.
const byte Buzz1 = 4;           //pin number for buzzer
const byte contrast = 32;       //Contrast value for this LCD, varies between modules, range noramlly 35-65
const byte RXPin = A5, TXPin = A2;           //pins for soft serial


//Program constants
const byte f1 = 108, f2 = 153, f3 = 153;    //Set the LoRa frequency, 434.400 Mhz
const byte GPSNumLoop = 10;                 //number orf GPS reads to do before sending fix info
const uint32_t GPSBaud = 9600;              //GPS baud rate
const byte thisnode = 6;                    //Node number for this device


//Program Variables
const int PayloadArraySize = 3;             //max number of comma sperated values in packet data
byte GPSLoop;
byte TXGPSfix = 0, RXGPSfix = 0;
int Talt;
String results[PayloadArraySize];
String InputString = "";     //data on buff is copied to this string
String Outputstring = "";
float Tlat, Tlon; //define the GPS lat and long variables for tracker
float Tdistance, Tdirection;
byte ScreenMode = 0;               //Screen mode, 0 for Small text, 1 for Large test
byte DataMode = 0;                 //Data mode, 0 for Low data rate (98bps), 1 for High data rate (1042bps)


//Includes
#include <SPI.h>
#include "LoRaCommon.h"
#include "LoRaRXonly.h"
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
SoftwareSerial ss(RXPin, TXPin);   // The serial connection to the GPS device
#include "Digisoft.h"
TinyGPSPlus gps;


void loop()
{
  while (ss.available() > 0)
    if (gps.encode(ss.read()))
    {
      processGPS();
      checkforpacket();
      ss.begin(GPSBaud);                          //Re-start soft serial for GPS
    }
  if (millis() > 7500 && gps.charsProcessed() < 10)
  {
    //Serial.println("TX GPS ERROR");
    LCD_clearset6();
    Digisoft_text("TX GPS ERROR");
    systemerror();
  }
}


void processGPS()
{
  byte i;
  byte ltemp;
  Outputstring = "";

  if (gps.location.isValid())
  {
    RXGPSfix = 1;
    //ss.end();

    GPSLoop++;                                               //increment the GPS loop counter
    if  (GPSLoop > GPSNumLoop)                               //software keeps the GPS reading and saving fixes, after a specific amount of loops, update local GPS data
    {
      GPSLoop = 0;
      UpdateLCD();                                           //put latest local GPS co-ordiantes on screen 
    }

    delay(250);                                 //take each GPS read up to circa 1 second
  }
  else
  {
    RXGPSfix = 0;
    if (TXGPSfix == 0);                         //if we have had a TX location fix, allow the Lat and Lon display to stay on LCD.
    {
    digitalWrite(PLED1, HIGH);
    digitalWrite(Buzz1, HIGH);
    //Serial.println("No RX GPS Fix");
    LCD_clearset6();
    Digisoft_text("No RX GPS Fix");
    digitalWrite(PLED1, LOW);
    digitalWrite(Buzz1, LOW);
    delay(2000);
    }
  }
}


float convertstring(String inString)
{
  char buf[20];
  inString.toCharArray(buf, inString.length());
  float val = atof(buf);
  return val;
}


void FillPayloadArray(byte llora_RXStart, byte llora_RXEnd)
//fills payload array from RC buffer
{
  byte i = 0;
  byte j = 0;
  byte lvar1 = 0;
  String tempstring = "";
  for (i = llora_RXStart; i <= llora_RXEnd; i++)
  {
    lvar1 = lora_RXBUFF[i];
    
    if (lvar1 == 44)
    {
      results[j] = tempstring;
      j++;
      tempstring = "";
      if (j > PayloadArraySize)
      {
        Serial.print("ERROR To many Fields");
        Serial.println(j);
        break;
      }
    }
    else
    {
      tempstring = tempstring + char(lvar1);
    }
  }
  Serial.println();
}


void directionto()
{
  Tdirection = TinyGPSPlus::courseTo(gps.location.lat(), gps.location.lng(), Tlat, Tlon);
  Serial.print("Dir ");
  Serial.println(Tdirection, 0);
}


void printLatLon()
{
  Serial.print("Hlat ");
  Serial.println(gps.location.lat(), 6);
  Serial.print("Hlon ");
  Serial.println(gps.location.lng(), 6);
  Serial.print("Tlat ");
  Serial.println(Tlat, 6);
  Serial.print("Tlon ");
  Serial.println(Tlon, 6);
  Serial.print("Talt ");
  Serial.println(Talt);
  Serial.println();
}


void distanceto()
{
  Tdistance = TinyGPSPlus::distanceBetween(gps.location.lat(), gps.location.lng(), Tlat, Tlon);
  Serial.print("Dist ");
  Serial.println(Tdistance, 0);
}


void checkforpacket()
{
  byte lora_LRegData;
  byte lora_Ltemp;
  String floatstring;

  lora_Ltemp = lora_readRXready();

  if (lora_Ltemp == 0)
  {
    Serial.print("NP.");
  }

  if (lora_Ltemp == 64)
  {
    Serial.println();
    lora_ReadPacket();
    lora_RXPKTInfo();                    //print the info on received packet
    processPacket();
    lora_RXtoReady();                    // ready for next and clear flags
  }

  if (lora_Ltemp == 96)
  {
    Serial.println();
    Serial.println("CRC Error");
    lora_RXtoReady();                    // ready for next and clear flags
  }
}


void UpdateLCD()
{
  //prints the local and tracker lat and long
  byte lvar1;
  String lstring;
  LCD_clearset6();
  Digisoft_text("HLA ");
  Digisoft_printfloat(gps.location.lat(), 5);
  Digisoft_SetCurPos(0, 1);
  Digisoft_text("HLO ");
  Digisoft_printfloat(gps.location.lng(), 5);
  Digisoft_SetCurPos(0, 2);
  Digisoft_text("TLA ");
  Digisoft_printfloat(Tlat, 5);
  Digisoft_SetCurPos(0, 3);
  Digisoft_text("TLO ");
  Digisoft_printfloat(Tlon, 5);
  Digisoft_SetCurPos(0, 4);
  Digisoft_printfloat(Tdistance, 0);
  Digisoft_text("m  ");
  Digisoft_printfloat(Tdirection, 0);
  Digisoft_text("dg");
  
  
  //here are the updates applicable to bot screen modes

  Digisoft_Setfont(6);               //all these updates are small text
  Digisoft_SetCurPos(0, 5);
  //Digisoft_text("AL ");
  Digisoft_printint(Talt);
  Digisoft_text("m  ");
  
  if (lora_PacketSNR > 127)          //Calculate the SNR
  {
    lvar1 =  ((255 - lora_PacketSNR) / 4);
    lstring = "-";
  }
  else
  {
    lvar1 =  (lora_PacketSNR / 4);
    lstring = "+";
  }

  Digisoft_text(lstring);              //now print the SNR
  Digisoft_printint(lvar1);
  Digisoft_text("dB");
  
  Digisoft_SetCurPos(13, 0);           //Position cursor last char of top line

  if (lora_RXPacketType == 64)
  {
    Digisoft_text("R");
  }
  else
  {
    Digisoft_text("D");
  }
  
  Digisoft_SetCurPos(13, 1);           //Position cursor last char of second line
  if (ScreenMode == 0)
  {
  Digisoft_text("S")
  }
  else
  {
  Digisoft_text("L")
  }

  Digisoft_SetCurPos(13, 2);           //Position cursor last char of third line
  if (DataMode == 0)
  {
  Digisoft_text("L")
  }
  else
  {
  Digisoft_text("H")
  }
}


void processPacket()
{
  if ((lora_RXPacketType == 60) || (lora_RXPacketType == 64))
  {
    TXGPSfix = 1;
    lora_RXBuffPrint(0);                 //print packet contents as ASCII
    FillPayloadArray(lora_RXStart, lora_RXEnd);
    Tlat = convertstring(results[0]);
    Tlon = convertstring(results[1]);
    Talt = results[2].toInt();
    printLatLon();
    if (TXGPSfix == 1 && RXGPSfix == 1)
    {
      distanceto();
      directionto();
    }
    UpdateLCD();
    delay(2000);
  }

  if (lora_RXPacketType == 2)
  {
    lora_RXBuffPrint(0);
    FillPayloadArray(lora_RXStart, lora_RXEnd);
    LCD_clearset6();
    Digisoft_text("TX Batt ");
    Digisoft_text(results[0]);
    Digisoft_text("V");
    delay(1500);
  }

  if (lora_RXPacketType == 30)
  {
    LCD_clearset6();
    Digisoft_text("TX GPS ERROR");
    delay(1500);
  }

  if (lora_RXPacketType == 28)
  {
    LCD_clearset6();
    Digisoft_text("No TX GPS Fix");
    delay(1500);
  }
}


void systemerror()
{
  while (1)
  {
    digitalWrite(PLED1, HIGH);
    delay(100);
    digitalWrite(PLED1, LOW);
    delay(100);
  }
}


void LCD_clearset6()
{
  Digisoft_SetContrast(contrast);
  Digisoft_Cls();
  //delay(25);
  Digisoft_Setfont(6);
  Digisoft_SetCurPos(0, 0);
}


void setup()
{
  Serial.begin(9600);                       //Serial console ouput
  Serial.println("ProMiniLoRaTracker_RXGPS_V2.ino");
  Serial.println("Stuart Robinson - 20/07/15");
  Serial.println();
  pinMode(lora_PReset, OUTPUT);			// RFM98 reset line
  digitalWrite(lora_PReset, LOW);		// Reset RFM98
  pinMode (lora_PNSS, OUTPUT);			// set the slaveSelectPin as an output:
  digitalWrite(lora_PNSS, HIGH);
  pinMode(PLED1, OUTPUT);			// for shield LED
  pinMode(Buzz1, OUTPUT);			// for shield LED
  SPI.begin();					// initialize SPI:
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(MSBFIRST);
  if (lora_CheckDevice() == 1)
  {
    //Serial.println("RFM98 Error");
    //LCD_clearset6();
    Digisoft_text("RFM98 Error");
    systemerror();
  }
  lora_Setup();				//Do the initial LoRa Setup
  lora_SetFreq(f1, f2, f3);
  lora_Tone(1000, 1000, 10);             //Transmit an FM tone
  delay(500);
  lora_SetModem(lora_BW41_7, lora_SF12, lora_CR4_5, lora_Explicit, lora_LowDoptON);		//Setup the LoRa modem parameters
  lora_PrintModem();                    //Print the modem parameters
  
  lora_RXtoReady();
  ss.begin(GPSBaud);                          //Startup soft serial for GPS


  int BattRead = analogRead(A3);
  float BattVolts = BattRead * (6.6 / 1023.0);
  
  Serial.print("RX Battery ");
  Serial.print(BattVolts, 2);
  Serial.println("V");
  
  LCD_clearset6();
  Digisoft_text("RX Batt ");
  Digisoft_printfloat(BattVolts, 2);
  Digisoft_text("V");
  
  delay(1500);
}


