// winアプリからの信号に基づいてArduinoを駆動する
// ソレノイド制御は https://highso.hatenablog.com/entry/2019/02/25/180000 を参照

#define PIN1 13 
int note = 200; //8分音符のミリ秒数
int claptime = 50; //「カチ」のミリ秒数
int rest = note - claptime;
String a;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(PIN1, OUTPUT);  // デジタルピンを出力に設定
  digitalWrite(PIN1, LOW);
}

void repaint() {
  digitalWrite(PIN1,HIGH);
  delay(claptime);
  digitalWrite(PIN1,LOW);
}

void loop() {
  // put your main code here, to run repeatedly:

  //1回の通信では1回分しか処理しない (pC側アプリがしばしば処理落ちして2個3個同時に指示を送ってくる事がある)
  if(Serial.available()) {
    a = Serial.readStringUntil('\n');
    
    //Re-paintingする
    if(a == "r"){
      Serial.println("OK");
      repaint();
      delay(rest);
      delay(note);
    }
  }

  //ここで全てのバッファを消去しておく
  while(Serial.available()) Serial.read();

  digitalWrite(PIN1, LOW);
}
