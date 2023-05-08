#include <MPU9250_WE.h>
#include <Wire.h>
#define MPU9250_ADDR 0x68
#define pressures   false
#define rumble      false
#include <PS2X_lib.h>//ps2库函数


String str="";
String id="yourid";
String password="youridpassword";
String wifi = "AT+CWJAP=\"id\",\"youridpassword\"";  //连接wifi


PS2X ps2x;
int val = 50; //限速，防止pwm值超过255
int valA = 0; //a电机pwm速度控制变量
int valB = 0; //b电机pwm速度控制变量
MPU9250_WE myMPU9250 = MPU9250_WE(MPU9250_ADDR);

int error=0;
byte type=0;
byte vibrate=0;

#define rxPin 50
#define txPin 51

/*******ps2手柄接口************/
#define PS2_DAT        33  
#define PS2_CMD        31  
#define PS2_SEL        32  
#define PS2_CLK        30  

/*********L298N接口*********/

int EA1 = 2;   //左边轮子
int EA2 = 3;   
int ENA = 4;  
int EB1 = 5;   //右边轮子
int EB2 = 6;   
int ENB = 7;  
int state=0,num=0,Mood;

int value=120;

void setup() {
  Serial.begin(115200); //arduino端口
  Serial3.begin(115200); //esp8266端口
  pinMode(ENA, OUTPUT);
  pinMode(EA1, OUTPUT);
  pinMode(EA2, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(EB1, OUTPUT);
  pinMode(EB2, OUTPUT);
  delay(100);

//  while(true)
//  {
//    Serial3.println("AT+RST");
//   
//    Serial.println(waitWifiAnswer(3000));
//    delay(100);
//    if(Serial3.find("OK")==true)
//    {
//      break;
//    }
//  }
  Serial3.println("AT+CWLAP");
  delay(100);
  while(Serial3.available()>0)
  {
    char a=Serial3.read();
    str.concat(a);
  }
  Serial.println(str);
  str ="";
  Serial3.println(wifi);
  delay(100);
  while(Serial3.available()>0)
  {
    char a=Serial3.read();
    str.concat(a);
  }
  Serial.println(str);
  str ="";
  Serial3.println("AT+CIFSR");
  delay(100);
  while(Serial3.available()>0)
  {
    char a=Serial3.read();
    str.concat(a);
  }
  Serial.println(str);
  delay(50);
  Serial3.println("AT+CIPSTART=\"TCP\",\"192.168.1.120\",50010");//连接TCP服务
  Serial.println(waitWifiAnswer(300));
  delay(50);
  Serial3.println("AT+CIPSEND=5");//发送长度为5的数据
  delay(50);
  
  error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, pressures, rumble);
  Serial.print("\n error=");
  Serial.print(error);

  Wire.begin();
  if(!myMPU9250.init())
  {
    Serial.println("MPU9250 does not respond");
  }
  else
  {
    Serial.println("MPU9250 is connected");
  }
  Serial.println("Position you MPU9250 flat and don't move it - calibrating...");
  delay(1000);

  if(!myMPU9250.initMagnetometer())
  {
    Serial.println("Magnetometer does not respond");
  }
  else
  {
    Serial.println("Magnetometer is connected");
  }
  
   myMPU9250.autoOffsets();
   Serial.println("Done!");

   myMPU9250.setMagOpMode(AK8963_CONT_MODE_100HZ);
   delay(200);
   
   myMPU9250.setSampleRateDivider(5);
   myMPU9250.setAccRange(MPU9250_ACC_RANGE_2G);
   myMPU9250.enableAccDLPF(true);
   myMPU9250.setAccDLPF(MPU9250_DLPF_6);
  
   myMPU9250.enableGyrDLPF();
   myMPU9250.setGyrDLPF(MPU9250_DLPF_6);
   myMPU9250.setSampleRateDivider(99);
   myMPU9250.setGyrRange(MPU9250_GYRO_RANGE_250);
  }

void loop() {
  ps2x.read_gamepad(false, vibrate);
  /**************获取左右摇杆数值***************/
  int Y1 = ps2x.Analog(PSS_LY);
  int Y2 = ps2x.Analog(PSS_RY);
  Serial.print("\n Y1:Y2      ");
  Serial.print(Y1);
  Serial.print(":");
  Serial.println(Y2);
  /**************左摇杆控制左轮前进后退*****************/
  valA = abs(Y1 - 127) * 2 - val;
  valA = valA>0?valA:0;
  if (Y1 < 100) 
  {
    go_r();
   }
  else if (Y1 > 150) 
  {
     refund_r();
  } 
  else 
  {
    pause_r(); 
  }
  /**************右摇杆控制右轮前进后退*****************/
  valB = abs(Y2 - 127) * 2 - val;
  valB = valB>0?valB:0;
  if (Y2 < 100) 
  {
    go_l();
  } 
  else if (Y2 > 150) 
  {
    refund_l();
  } 
  else 
  {
    pause_l();
  }

  /*********智能车速度限制*********/
  
  if (ps2x.ButtonReleased(PSB_SQUARE))               //1档□形
    val = 150;
  if (ps2x.Button(PSB_TRIANGLE))                   //2档△形
    val = 100;
  if (ps2x.ButtonPressed(PSB_CIRCLE))              //3档○形
    val = 50;
  if (ps2x.NewButtonState(PSB_CROSS))              //4档×形
    val = 0;

  xyzFloat accRaw = myMPU9250.getAccRawValues();
  xyzFloat accCorrRaw = myMPU9250.getCorrectedAccRawValues();
  xyzFloat gValue = myMPU9250.getGValues();
  xyzFloat angle = myMPU9250.getAngles();
  float resultantG = myMPU9250.getResultantG(gValue);
  xyzFloat magValue = myMPU9250.getMagValues(); // returns magnetic flux density [µT] 
  Serial.println(myMPU9250.getOrientationAsString());
  float pitch = myMPU9250.getPitch();
  float roll  = myMPU9250.getRoll();
  float yaw = (atan2(angle.z, angle.x) * 180.0 )/ M_PI;
  float ROll = fmod(roll + 360, 360);
  float PITch = fmod(pitch + 360, 360);
  float YAW = fmod(yaw + 360, 360);
  
  delay(1000);
  
  String Yaw = String (YAW);
  String Roll = String (ROll);
  String Pitch = String (PITch);
  
  delay(100);
  chuansong("Pitch"+Pitch);
  Serial.println(Pitch);
  Serial.println("111111111111111111");
  delay(200);
  chuansong("ROll"+Roll);
  Serial.println(Roll);
  Serial.println("3333333333333333333333");
  delay(200);
  chuansong("YAW"+Yaw);
  Serial.println(Yaw);
  Serial.println("2222222222222222222");
  delay(200);
  
}

//右轮前进
void go_r() {
  analogWrite(ENA, valA);
  digitalWrite(EA1, HIGH);
  digitalWrite(EA2, LOW);
}
//右轮后退
void refund_r() {
  analogWrite(ENA, valA);
  digitalWrite(EA1, LOW);
  digitalWrite(EA2, HIGH);
}
//右轮停止
void pause_r() {
  digitalWrite(EA1, LOW);
  digitalWrite(EA2, LOW);
}
//左轮前进
void go_l() {
  analogWrite(ENB, valB);
  digitalWrite(EB1,HIGH );
  digitalWrite(EB2, LOW);
}
//左轮后退
void refund_l() {
  analogWrite(ENB, valB);
  digitalWrite(EB1, LOW);
  digitalWrite(EB2, HIGH);
}
//左轮停止
void pause_l() {
  digitalWrite(EB1, LOW);
  digitalWrite(EB2, LOW);
}

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
String waitWifiAnswer(uint32_t maxWaitTime)
{
  uint32_t previousTime=millis(),currentTime=millis();
  if(maxWaitTime<0)
  {
    maxWaitTime=0;
  }
  String recv="",tmpRecv="";
  char readOne;
  uint32_t buf_size=Serial3.available();
  int ipdStart=0,colon,dot;
  bool hasCmd=false;
  int cmdLen=0;
  while(buf_size==0)
  {
    buf_size=Serial3.available();
  }
  while((buf_size>0||(recv.indexOf("OK")==-1&&recv.indexOf("ERROR")==-1))&&currentTime-previousTime<maxWaitTime)
  {
    currentTime=millis();
    buf_size=Serial3.available();
    
    for(uint32_t i=0;i<buf_size;i++)
    {
      readOne=Serial3.read();
      recv.concat(readOne);
      previousTime=currentTime;
      cmdLen+=1;
    }
  }
  /*
  while(ipdStart!=-1)
  {
    ipdStart=recv.indexOf("+IPD,",ipdStart);
    if(ipdStart==-1)
    {
      break;
    }
    if(recv.indexOf(":",ipdStart)!=-1)
    {
      int strLen=recv.length();
      colon=recv.indexOf(":",ipdStart+5);
      dot=recv.indexOf(",",ipdStart+5);
      String cmdLenStr=recv.substring(dot+1,colon);
      cmdLen=cmdLenStr.toInt();
      if(cmdLenStr.length()<5&&cmdLen>0)
      {
        for(int i=0;i<cmdLen;i++)
        {
          cmdBuf.writeToBuf((byte)recv.charAt(colon+1+i));
          recv.setCharAt(colon+1+i,1);
        }
      }
      ipdStart+=1;
    }
  }
  */
  return recv;
}
