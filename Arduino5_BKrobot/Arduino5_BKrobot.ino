// タンパク質の信号をフィードバックしてロボットの移動方向と速度をリアルタイムに制御する
// サーボモータなのでservo.hを使う
// 推定した電圧の大きさと正比例した回転数に (負の方向も含む) 動かせるようにしておく
// https://miraluna.hatenablog.com/entry/rasen

#include <Servo.h>
Servo leftWheel;
Servo rightWheel;

int analogPinA = 2;
int analogPinB = 3;
String a;
int motSpeed = 90;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  leftWheel.attach(analogPinA);
  rightWheel.attach(analogPinB);
  
}

void loop() {
  // put your main code here, to run repeatedly:
  
  if(Serial.available()) {
    a = Serial.readStringUntil('\n');

    // 360°サーボの場合は，0で正転最速，90で停止，180で反転最速
    if(a=="0") motSpeed = 90;
    else if(a=="1") motSpeed = 50;
    else if(a=="2") motSpeed = 60;
    else if(a=="3") motSpeed = 70;
    else if(a=="4") motSpeed = 80;
    else if(a=="5") motSpeed = 90;
    else if(a=="6") motSpeed = 100;
    else if(a=="7") motSpeed = 110;
    else if(a=="8") motSpeed = 120;
    else if(a=="9") motSpeed = 130;  
    
  }
  //ここで全てのバッファを消去しておく
  while(Serial.available()) Serial.read();
    
  //速度変更
  leftWheel.write(180-motSpeed); // サーボの回転向きが逆
  rightWheel.write(motSpeed);
  delay(500);
}
