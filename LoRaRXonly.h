//LoRaRXonly.h
/*****************************************************************************************************
ProMiniLoRaTracker_V2 Programs

Copyright of the author Stuart Robinson - 15/07/2015 15:00

These programs may be used free of charge for personal, recreational and educational purposes only.

This program, or parts of it, may not be used for or in connection with any commercial purpose without
the explicit permission of the author Stuart Robinson.

The programs are supplied as is, it is up to individual to decide if the programs are suitable for the
intended purpose and free from errors.
******************************************************************************************************
*/

//byte Variables
byte  lora_RXStart;			//start of packet data in RXbuff
byte  lora_RXEnd;			//end of packet data in RXbuff
byte  lora_FCRCerror;			//flag, set to 1 if there is a packet CRC error
byte  lora_FRXTimeout;			//flag, set to 1 if there is a RX timeout
byte  lora_FRXOK;			//flag, set to 1 if RX is OK
byte  lora_RXPacketType;		//type number of received packet
byte  lora_RXDestination;		//destination address of received packet
byte  lora_RXSource;			//source address of received packet
byte  lora_BackGroundRSSI;		//measured background noise level
byte  lora_PacketRSSI;			//RSSI of received packet
byte  lora_PacketSNR;			//signal to noise ratio of received packet
byte  lora_RXPacketL;			//length of packet received, includes source, destination and packet type.

//byte arrays
byte  lora_RXBUFF[128];			//buffer where received packets are stored

//Integer variables, more than byte
long lora_RXpacketCount;		//count of valid packets received
long lora_CRCerrorcount;		//count of packets received with CRC errors

/*
**************************************************************************************************
Library Functions
**************************************************************************************************
*/

byte lora_RXBuffPrint(byte lora_LPrint)
{
  //Print contents of RX buffer as ASCII,decimal or HEX
  //Serial.print("lora_RXBuffPrint() ");
  //Serial.print(lora_LPrint);
  //Serial.print(") ");
  //Serial.print(lora_RXStart);
  //Serial.print(" ");
  //Serial.print(lora_RXEnd);
  //Serial.print(" Start>>");                           // print start marker so we can be sure where packet data starts

  byte lora_LLoop;
  byte lora_LData;
  byte lcount;

  for (lora_LLoop = lora_RXStart; lora_LLoop <= lora_RXEnd; lora_LLoop++)
  {
    lcount++;
    lora_LData = lora_RXBUFF[lora_LLoop];
    if (lora_LPrint == 0)
    {
      Serial.write(lora_LData);
    }
    if (lora_LPrint == 1)
    {
      Serial.print(lora_LData);
      Serial.print(" ");
    }

    if (lora_LPrint == 2)
    {
      Serial.print(lora_LData, HEX);
      Serial.print(" ");
    }
  }
 //Serial.print("<<End   ");                                 // print end marker so we can be sure where packet data ends
  Serial.println();
}


void lora_RXOFF()
{
  //turns off receiver
  //Serial.print("lora_RXOFF()");
  //Serial.println();
  lora_Write(lora_RegOpMode, 0x89);                         // TX and RX to sleep, in direct mode
}


void lora_RXPKTInfo()
{
  //print the information for packet last received
  byte lora_Lvar1;
  char lora_LChar;
  //Serial.print("lora_RXPKTInfo() ");
  Serial.print("RXtype,");
  Serial.print(lora_RXPacketType);
  Serial.print(",Destination,");
  Serial.print(lora_RXDestination);
  Serial.print(",Source,");
  Serial.print(lora_RXSource);
  Serial.print(",Length,");
  Serial.print(lora_RXPacketL);
  Serial.print(",RXCount,");
  Serial.print(lora_RXpacketCount);
  Serial.print(",CRCErrors,");
  Serial.print(lora_CRCerrorcount);
  Serial.println();

  lora_Lvar1 = 137 - lora_PacketRSSI;
  Serial.print("RSSI -");
  Serial.print(lora_Lvar1);
  Serial.print("dBm");
  Serial.println();

  lora_Lvar1 = 137 - lora_BackGroundRSSI;
  Serial.print("Noise -");
  Serial.print(lora_Lvar1);
  Serial.print("dBm");
  Serial.println();

  if (lora_PacketSNR > 127)
  {
    lora_Lvar1 =  (255 - lora_PacketSNR) / 4;
    lora_LChar = '-';
  }
  else
  {
    lora_Lvar1 = lora_PacketSNR / 4;
    lora_LChar = '+';
  }

  Serial.print("SNR ");
  Serial.print(lora_LChar);
  Serial.print(lora_Lvar1);
  Serial.print("dB");
  Serial.println();
}


void lora_ReadPacket()
{
  byte lora_Lvar1;
  byte lora_LRegData;
  lora_RXpacketCount++;
  lora_RXPacketL = lora_Read(lora_RegRxNbBytes);
  lora_PacketRSSI = lora_Read(lora_RegPktRssiValue);
  lora_PacketSNR = lora_Read(lora_RegPktSnrValue);
  lora_PacketRSSI = lora_Read(lora_RegPktRssiValue);

  //lora_Lvar1 = lora_Read(lora_RegFifoRxByteAddr);
  //lora_Write(lora_RegFifoAddrPtr,lora_Lvar1);  	          // set FIFO ptr
  lora_Write(lora_RegFifoAddrPtr,0);  	          // set RX FIFO ptr
    
  digitalWrite(lora_PNSS, LOW);		                  // start the burst read
  SPI.transfer(lora_RegFifo);			          // address for burst read
  lora_RXPacketType = SPI.transfer(0);
  lora_RXDestination = SPI.transfer(0);
  lora_RXSource = SPI.transfer(0);
  lora_RXStart = 0;
  lora_RXEnd = lora_RXPacketL - 4;                       // adjust for destination, packettype and source bytes

  //Serial.print("RXbuff ");
  //Serial.print(lora_RXStart);
  //Serial.print(" ");
  //Serial.print(lora_RXEnd);
  //Serial.println();
  
  for (lora_Lvar1 = lora_RXStart; lora_Lvar1 <= lora_RXEnd; lora_Lvar1++)
  {
    lora_LRegData = SPI.transfer(0);
    lora_RXBUFF[lora_Lvar1] = lora_LRegData;
  }
  digitalWrite(lora_PNSS, HIGH);		           // finish, turn off LoRa device
}


void lora_RXtoReady()
{
  //'puts SX1278 into listen mode and receives packet exits with packet in array lora_RXBUFF(256)
  Serial.println("lora_RXtoReady()");
  byte lora_Lvar1, lora_LRegData, lora_LLoop;
  long lora_Lvar2;
  lora_FRXOK = 1;                                        // Start assuming packet received OK, unless flag cleared
  lora_FCRCerror = 0;
  lora_RXPacketL = 0;
  lora_RXPacketType = 0;
  lora_RXDestination = 0;
  lora_RXSource = 0;
  lora_RXStart = 0;
  lora_RXEnd = 0;
  lora_Write(lora_RegOpMode, 0x09);
  lora_Write(lora_RegFifoRxBaseAddr, 0x00);
  lora_Write(lora_RegFifoAddrPtr, 0x00);
  lora_Write(lora_RegIrqFlagsMask, 0x9F);                // only allow rxdone and crc error
  lora_Write(lora_RegIrqFlags, 0xFF);
  lora_Write(lora_RegOpMode, 0x8D);
  lora_BackGroundRSSI = lora_Read(lora_RegRssiValue);    // get the background noise level
}


byte lora_readRXready()
{
  byte lora_LRegData;
  lora_LRegData = lora_Read(lora_RegIrqFlags);
  return lora_LRegData;
}


