// winアプリからの信号に基づいてArduinoを駆動する
// ステッピングモータ制御は https://burariweb.info/electronic-work/arduino-learning/arduino-stepping-motor-control.html を参照

String a;
//int state = false;
int rotate_until_stop = 0;  // 0:stop, -1:left, 1:right

#include <Stepper.h>
#define PIN1 12  // 青
#define PIN2 11  // ピンク
#define PIN3 10  // 黄
#define PIN4 9  // オレンジ
#define STEP 2048  // ステッピングモーター(出力軸)が1回転するのに必要なステップ数
Stepper stepper1(STEP, PIN1, PIN3, PIN2, PIN4);  // オブジェクトを生成

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
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

  //1回の通信では1回分しか処理しない (pC側アプリがしばしば処理落ちして2個3個同時に指示を送ってくる事がある)
  if(Serial.available()) {
    a = Serial.readStringUntil('\n');
    // 位置合わせ系の処理
    if(a == "z"){
      //止めるまで左に一定間隔で回し続ける
      rotate_until_stop = -1;
    }else if(a == "x"){
      //1ステップだけ左に回す
      stepper1.step( -10 );
      rotate_until_stop = 0;
    }else if(a == "c"){
      //止める
      rotate_until_stop = 0;
    }else if(a == "v"){
      //1ステップだけ右に回す
      rotate_until_stop = 0;
      stepper1.step( 10 );
    }else if(a == "b"){
      // 止めるまで右に一定間隔で回し続ける
      rotate_until_stop = 1;
    }
    //Re-paintingする
    else if(a == "r"){
      // 2022/7/5 軸の遊びが原因で，回転がきれいに指示通りの値にならない．どうしようか
      // 2022/7/12 ギアのバックラッシが原因のようなので，大きめに動かすことで補正する．
      //    まず左回しでしっかり位置合わせする．右回しした場合は大きく右に動かし，最後を左回しで合わせる．
      //    続いて，「x」をn回押す．これでバックラッシの分ちょうど良い場所に来る．
      //    ...と思ったら，初めに位置をずらさなくてもちょうどよかった．なんで？30°で充分バックラッシを打ち消せるのか．
      //    その後，下の挙動で膜を張りなおす．
      stepper1.step( 171 );    // -30°回転させる(2048/360*30ステップ)
      stepper1.step( -171 );    // 大きく動かす事でちょうど合う．

      //無理そうならこれでもよい．
      //stepper1.step( -2048 );    // 一周ぐるっと回す
    }
  }

  //ここで全てのバッファを消去しておく
  while(Serial.available()) Serial.read();

  //止めるまで回転系の処理を実行
  if(rotate_until_stop == -1){
    //左
    stepper1.step( -3 );
  }else if(rotate_until_stop == 1){
    //右
    stepper1.step( 3 );
  }

  //最後にはモータの出力を止める (モーターへの電流を止め発熱を防ぐ)
  digitalWrite(PIN1, LOW);
  digitalWrite(PIN2, LOW); 
  digitalWrite(PIN3, LOW); 
  digitalWrite(PIN4, LOW);   
}
