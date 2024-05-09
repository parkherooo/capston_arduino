#include <SoftwareSerial.h>
SoftwareSerial gps(11,12);
char c = ""; 
String str = ""; 
String targetStr = "GPGGA" ;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  gps.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  
  if(gps.available()){
    c=gps.read(); 
    if(c=='\n'){
      if(targetStr.equals(str.substring(1,6))){
        Serial.println(str);
        int first = str.indexOf(",");
        int two = str.indexOf(",",first+1);
        int three = str.indexOf(",",two+1);
        int four = str.indexOf(",",three+1);
        int five = str.indexOf(",",four+1);
        String Lat = str.substring(two+1,three);
        String Long = str.substring(four+1,five);

        String Lat1 = Lat.substring(0,2);
        String Lat2 = Lat.substring(2);

        String Long1 = Long.substring(0,3);
        String Long2 = Long.substring(3);

        double LatF = Lat1.toDouble()+Lat2.toDouble()/60;
        float LongF = Long1.toFloat()+Long2.toFloat()/60;

        Serial.print("Lat(위도) :");
        Serial.println(LatF,15);
        Serial.print("Long(경도) :");
        Serial.println(LongF,15);
      }
    str = "";
    }
    else{
      str+=c;
    }
  }
}
