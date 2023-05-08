# TCP-ESP8266
Nine-axis data is sent over tcp using esp8266
The mage 2560 board is used for this project, which is burned on the Arduino IDE
The car is driven by the L298N, remote control
void chuansong(String str)
{
  
   String string="AT+CIPSEND=";
   int len=str.length();
   string.concat(String(len));
   Serial3.println(string);
   delay(100);
   Serial3.print(str);
   return str;
}
The code above converts the specified data to String and sends it over wifi
