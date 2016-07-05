//digisoft.h
/*****************************************************************************************************
ProMiniLoRaTracker_V1 Programs

Copyright of the author Stuart Robinson - 02/07/2015 15:00

These programs may be used free of charge for personal, recreational and educational purposes only.

This program, or parts of it, may not be used for or in connection with any commercial purpose without
the explicit permission of the author Stuart Robinson.

The programs are supplied as is, it is up to individual to decide if the programs are suitable for the
intended purpose and free from errors.

Noramlly used with Nokia 5110 display and Arduino backapack, display is 6 lines of 14 characters. 
******************************************************************************************************
*/



const byte DigiSoftDelay = 5;

void Digisoft_Cls()
{
  //adds text to current position of display
  ss.print("CL");   //print text
}

void Digisoft_Setfont(byte lfont)
{
  //adds text to current position of display
  ss.print("SF");   //print text
  ss.write(char(lfont)); //print data
  delay(DigiSoftDelay);
}

void Digisoft_text(String Ltext)
{
  //adds text to current position of display
  ss.print("TT");   //print text
  ss.print(Ltext);   //print text
  ss.write(char(13)); //print data
  delay(DigiSoftDelay);
}

void Digisoft_SetCurPos(byte lcol, byte lrow)
{
  //Sets the current cursor position
  ss.print("TP");   //print text
  ss.write(char(lcol)); //print data
  ss.write(char(lrow)); //print data
  delay(DigiSoftDelay);
}

void Digisoft_SetContrast(byte lcont)
{
  //Sets the current cursor position
  ss.print("CT");   //print text
  ss.write(char(lcont)); //print data
  delay(DigiSoftDelay);
}

void Digisoft_printint(int Ltemp)
{
  //adds text to current position of display
  ss.print("TT");   //print text
  ss.print(Ltemp);   //print number
  ss.write(char(13)); //print data
  delay(DigiSoftDelay);
}


void Digisoft_printfloat(float Ltemp, byte Lnumdecimals)
{
  //adds text to current position of display
  ss.print("TT");   //print text
  ss.print(Ltemp,Lnumdecimals);   //print number
  ss.write(char(13)); //print data
  delay(DigiSoftDelay);
}



