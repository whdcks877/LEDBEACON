#include <SoftwareSerial.h>
#include <stdlib.h>


SoftwareSerial BTSerial(10,11);

char disi_result[2000];
int top=0;
int d1=0;
int i=0;
char buf[5];

int over;

String str="";

typedef struct _devices{
  String uuid;
  int major;
  int minor;
  int rssi;
}Device;

Device *Devices;

void setup()
{
 
  Serial.begin(9600);
  BTSerial.begin(9600); 
  start_at();
}
void loop(){ 
  str="";
  int device_num = 0;
  
  if(d1==0)
  {
    Serial.println("Scanning started");
    BTSerial.write("AT+DISI?");
    d1=1;   
  }
  if(d1==1){
   
    if (BTSerial.available()){
      while(BTSerial.available()){
        disi_result[i]=BTSerial.read();   
        i++;
        if(BTSerial.overflow()) {
          Serial.println("overflow");
          return;
        }
      }

    }

    int j;
    
    for(j=i-8;j<=i;j++)
    {
      str=str+disi_result[j];
    }

    if(str.length()  > 8){
      str = str.substring(0,8);
    }

    if(str=="OK+DISCE" ){
      d1=2;
    }

  }
  if(d1==2)
  {
    int j=0;
      for(j=0;j<=i;j++){
       Serial.print(disi_result[j]);
      }
      Serial.print('\n');
      String disi_str = disi_result;

      int device_num_t = (i - 16)/78;
      Devices = (Device *)calloc(sizeof(Device),device_num_t);
      
      for(int k=0; k < device_num_t; k++){
        if(disi_str.substring(16+(78*k),24+(k*78)) == "00000000"){
          continue;
        }

        
        Devices[device_num].uuid = disi_str.substring(25+(78*k),57+(k*78));
        disi_str.substring(58+(78*k),62+(k*78)).toCharArray(buf,5);
        Devices[device_num].major = StrToHex(buf);
        disi_str.substring(62+(78*k),66+(k*78)).toCharArray(buf,5);
        Devices[device_num].minor = StrToHex(buf);
        disi_str.substring(82+(78*k),86+(k*78)).toCharArray(buf,5);
        Devices[device_num].rssi = StrToHex(buf);

        Serial.print("uuid : ");
        Serial.println(Devices[device_num].uuid);
        Serial.print("major : ");
        Serial.println(Devices[device_num].major);
        Serial.print("minor : ");
        Serial.println(Devices[device_num].minor);
        Serial.print("rssi : ");
        Serial.println(Devices[device_num].rssi);
        device_num++;
        /*
        Serial.println(disi_str.substring(16+(78*k),24+(k*78))); //factory id
        Serial.println(disi_str.substring(25+(78*k),57+(k*78))); //uuid
        Serial.println(disi_str.substring(58+(78*k),62+(k*78))); //major
        Serial.println(disi_str.substring(62+(78*k),66+(k*78))); //minor
        Serial.println(disi_str.substring(66+(78*k),68+(k*78))); //txpower
        Serial.println(disi_str.substring(82+(78*k),86+(k*78))); //rssi
        */

       
        
      }

      
    i=0;
    d1=0;

   free(Devices);
    
  }

 delay(100);
}

void start_at(){
  String c="";
  do{
    BTSerial.print("AT");
    delay(50);
    if (BTSerial.available())
    {
      while(BTSerial.available())
      {
        c=c+char(BTSerial.read());
      }
 
    }
  }while(c!="OK");
}

int StrToHex(char str[])
{
  return (int) strtol(str, 0, 16);
}
