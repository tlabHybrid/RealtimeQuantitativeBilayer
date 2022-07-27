// winアプリからの信号に基づいてArduinoを駆動する
// ステッピングモータ制御は https://burariweb.info を参照

String a;
int state = LOW;

#include <Stepper.h>
#define PIN1 2  // 青
#define PIN2 3  // ピンク
#define PIN3 4  // 黄
#define PIN4 5  // オレンジ
#define STEP 2048  // ステッピングモーター(出力軸)が1回転するのに必要なステップ数
Stepper stepper1(STEP, PIN1, PIN3, PIN2, PIN4);  // オブジェクトを生成

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);

  stepper1.setSpeed( 15 );  // 1分間当たりの回転数を設定(rpm)
  pinMode(PIN1, OUTPUT);  // デジタルピンを出力に設定
  pinMode(PIN2, OUTPUT);
  pinMode(PIN3, OUTPUT);
  pinMode(PIN4, OUTPUT);
  digitalWrite(PIN1, LOW);
  digitalWrite(PIN2, LOW);
  digitalWrite(PIN3, LOW);
  digitalWrite(PIN4, LOW);
}

void loop() {
  // put your main code here, to run repeatedly:
  while(Serial.available()) {
    a = Serial.readStringUntil('\n');
    if(a == "repaint") {
      // 30°回転
      state = !state;
      digitalWrite(13, state);
      if(state==HIGH){
        stepper1.step( 171 );    // 30°回転させる(2048/360*30ステップ)
      }else{
        stepper1.step( -171 );    // -30°回転させる(2048/360*30ステップ)
      }
    }else if(a == "a"){
      //指示が止まるまで回転を続ける
      stepper1.step( 5 );    // 少しずつ回転させる
    }
  }
  digitalWrite(PIN1, LOW);  // 出力を停止(モーターへの電流を止め発熱を防ぐ)
  digitalWrite(PIN2, LOW); 
  digitalWrite(PIN3, LOW); 
  digitalWrite(PIN4, LOW);   
}
