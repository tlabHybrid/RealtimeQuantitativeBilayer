// タンパク質の信号をフィードバックしてモータの速度を制御する
// モータ速度はただのPWM
// 一応verapamil濃度と正比例した回転数に

int analogPinA = 2;
int analogPinB = 3;
String a;
int motSpeed = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(Serial.available()) {
    a = Serial.readStringUntil('\n');

    if(a=="0") motSpeed = 0;
    else if(a=="1") motSpeed = 10;
    else if(a=="2") motSpeed = 20;
    else if(a=="3") motSpeed = 30;
    else if(a=="4") motSpeed = 40;
    else if(a=="5") motSpeed = 50;
    else if(a=="6") motSpeed = 60;
    else if(a=="7") motSpeed = 70;
    else if(a=="8") motSpeed = 80;
    else if(a=="9") motSpeed = 90;
    
  }
  //ここで全てのバッファを消去しておく
  while(Serial.available()) Serial.read();
    
  //速度変更
  analogWrite(analogPinA, motSpeed);  //analogWriteの値は0から255まで
  analogWrite(analogPinB, 0);
  delay(500);
}
