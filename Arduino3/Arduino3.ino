// winアプリからの信号に基づいてArduinoを駆動する
// ポンプ制御は https://www.kdscientific.com/media/manuals/KDS%20Legato%20100%20Series_5617-006REV2.0.pdf を参照

#define PIN1 13
int pulse = 200; // パルスのミリ秒数
String a;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(PIN1, OUTPUT);  // デジタルピンを出力に設定
  digitalWrite(PIN1, LOW);
}

void repaint() {
  digitalWrite(PIN1,HIGH);
  delay(pulse);
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
    }
  }

  //ここで全てのバッファを消去しておく
  while(Serial.available()) Serial.read();

  digitalWrite(PIN1, LOW);
}
