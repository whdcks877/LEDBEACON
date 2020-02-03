#include <stdlib.h>


#define HEIGHT 0.1
#define R_INTEVER 1
#define TX_POWER -53
#define LED1 22
#define LED2 24
#define LED3 26

//비콘에서 받을 수 있는 신호
typedef struct _devices{
  String uuid;
  int major;
  int minor;
  int rssi;
  double distance;
}Device;

class Receiver {
    public:
    HardwareSerial *BTSerial; // 블루투스 통신 하드웨어 포트
    String name;  // 출력시 블루투스 센서 이름
    int d1 = 0;   // 모드
    String disi_result="";  // 블루투스 결과값 버퍼
    char buf[5];
    int device_num_t; //  총 장치 중 비콘을 걸러내기 위한 버퍼
    int device_num ; // 걸러낸 비콘의 수
    Device *Devices;
    int LEDpin;
    int StrToHex(char str[])
    {
        return (int) strtol(str, 0, 16); // 16진수 문자열을 10진수 int형으로
    }

    Receiver(HardwareSerial *HS,String _name,int _LEDpin) { //시리얼과 오브젝트 연결, 이름 붙여줌(초기화)
        BTSerial = HS;  
        name = _name;
        LEDpin = _LEDpin;
    }

    void start_at();  //블루투스 동작 확인

    void begin(int baudrate); // 
    void BTloop();  // 실질적인 main함수
};


Receiver Receiver1(&Serial1,"R1",LED1);
Receiver Receiver2(&Serial2,"R2",LED2);
Receiver Receiver3(&Serial3,"R3",LED3);

double min_max_to_velocity(Device BTR1);
double rssi_to_meter(Device BTR1);

void setup() {
  pinMode(LED1,OUTPUT);pinMode(23,OUTPUT);
  pinMode(24,OUTPUT);pinMode(25,OUTPUT);
  pinMode(26,OUTPUT);pinMode(27,OUTPUT);
  
  digitalWrite(LED1,LOW);
  Receiver1.begin(9600);
  Receiver2.begin(9600);
  Receiver3.begin(9600);
  Serial.begin(9600);
  Receiver1.start_at();
  Receiver2.start_at();
  Receiver3.start_at();
}

void loop() {
  Receiver1.BTloop();
  Receiver2.BTloop();
  Receiver3.BTloop();
  

}

//
//AT치면 OK가 뜨는지 확인
//
void Receiver::start_at() {
    String c="";
  do{
    BTSerial->print("AT");
    delay(50);
    if (BTSerial->available())
    {
      while(BTSerial->available())
      {
        c=c+char(BTSerial->read());
      }
 
    }
  } while(c!="OK");
}
//
//블루투스 정해진 baudrate로 시리얼 시작
//
void Receiver::begin(int baudrate){
    BTSerial->begin(baudrate);
}
//
//d1의 값에 따라 수행하는 일이 변한다.
//d1 = 0일 시 AT+disi명령어 보내서 블루투스가 검색 수행
//d1 = 1일 시 블루투스가 ok+disce를 보낼 때 까지 disi_result 버퍼에다가 문자 삽입
//d1 = 2일 시 disi_result버퍼의 값을 비콘 형식에 맞게 추출, 속도와 거리 출력
//
void Receiver::BTloop(){

  if(d1==0) // at+disi? 명령어를 블루투스에 보내줌
  {
    BTSerial->flush();
    Serial.println("Scanning started");
    BTSerial->write("AT+DISI?");
    d1 = 1;
  }
  if(d1 == 1){  // disi_result에 블루투스의 값을 읽어들임
      if (BTSerial->available()){
        // Serial RX buffer의 overflow가 의심 되면 다시 스캔
        if(BTSerial->available() == SERIAL_RX_BUFFER_SIZE){
          d1 = 0;
          return;
        }
        while(BTSerial->available()){
          disi_result += (char)BTSerial->read();   
        }
      }
    
      if(disi_result.length()>=8 && (disi_result.substring((disi_result.length()-8),(disi_result.length()))=="OK+DISCE") || 
         (disi_result.substring((disi_result.length()-9),(disi_result.length()-1))=="OK+DISCE")){
        d1=2;
        BTSerial->flush();
        BTSerial->write("AT+DISI?");
      }
  }
  
  if(d1==2)
  {
    device_num_t = 0;
    device_num = 0 ;

    int device_num_t = (disi_result.length() - 16)/78; //OK+DISIS와 OK+DISCE 문자열 제거 
    Devices = (Device *)calloc(sizeof(Device),device_num_t);  // 메모리 잡힌 장치수만큼 할당
      
    for(int k=0; k < device_num_t; k++){
    if(disi_result.substring(16+(78*k),24+(k*78)) == "00000000"){ // 00000000은 비콘이 아니라 버린다
        continue;
    }
    Devices[device_num].uuid = disi_result.substring(25+(78*k),57+(k*78)); //UUID : 10-25바이트
    disi_result.substring(58+(78*k),62+(k*78)).toCharArray(buf,5);  // major : 26-27바이트
    Devices[device_num].major = StrToHex(buf);
    disi_result.substring(62+(78*k),66+(k*78)).toCharArray(buf,5);  // minor : 28-29바이트
    Devices[device_num].minor = StrToHex(buf);
    //disi_result.substring(82+(78*k),86+(k*78)).toCharArray(buf,5);
    Devices[device_num].rssi = disi_result.substring(82+(78*k),86+(k*78)).toInt();  //  rssi값
    
    Serial.println(name);
    Serial.print("uuid : ");
    Serial.println(Devices[device_num].uuid);
    Serial.print("major : ");
    Serial.println(Devices[device_num].major);
    Serial.print("minor : ");
    Serial.println(Devices[device_num].minor);
    Serial.print("rssi : ");
    Serial.println(Devices[device_num].rssi);
    min_max_to_velocity(Devices[device_num]); // 속도출력
    rssi_to_meter(Devices[device_num]); // 거리출력
    Devices[device_num].distance = sqrt(pow(rssi_to_meter(Devices[device_num]),2) - pow(HEIGHT,2));
      
    if((rssi_to_meter(Devices[device_num])) < 0.4){
      digitalWrite(LEDpin,HIGH);
      if(LEDpin != LED3){
        digitalWrite(LEDpin+1,HIGH);
      }
    } else {
      digitalWrite(LEDpin,LOW);
      if(LEDpin != LED3){
        digitalWrite(LEDpin+1,LOW);
      }
    }
 
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
    d1=1;
    disi_result = ""; //버퍼 초기화
    free(Devices);
  }
}


double min_max_to_velocity(Device BTR1){ // 
  int minor = BTR1.minor;
  int major = BTR1.major;
  char to_value[10];
  double value;

  sprintf(to_value, "%X%X", major, minor); // major와 minor값을 하나의 문자열로

  for(int i=0; i<10; i++){
    if(to_value[i] == 'A'){ // 보낼 때 소수점을 A로 바꿔 보내 이를 다시 .으로 변환해줌
        to_value[i] = '.';
      }
      else if(to_value[i] == 'B'){
        to_value[i] = '-';
      }
  }

  value = atof(to_value); //문자열을 double형 소수로
  Serial.print("velocity: ");
  Serial.println(value);
  return value;
  
}

double rssi_to_meter(Device BTR1) {
  double N_value = 2;
  double distance = pow(10, (TX_POWER - (double)BTR1.rssi) / (10*N_value)); // d = 10 ^ ((txPower - rssi) / 10 * n)
  Serial.print("distance: ");
  Serial.println(distance);
  return distance;
  
}
