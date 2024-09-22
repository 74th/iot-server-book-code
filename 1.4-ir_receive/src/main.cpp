#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

#define LED_PIN GPIO_NUM_27         // LEDピン
#define BUTTON_PIN GPIO_NUM_39      // ボタンピン
#define IR_RECEIVER_PIN GPIO_NUM_32 // 赤外線受信ピン

// LEDの色の定義
#define LED_COLOR_BLUE Adafruit_NeoPixel::Color(0, 0, 32)
#define LED_COLOR_YELLOW Adafruit_NeoPixel::Color(32, 32, 0)

// LEDの定義（数、制御ピン、仕様）
Adafruit_NeoPixel led(1, LED_PIN, NEO_GRB + NEO_KHZ800);

// 赤外線受信の定義
IRrecv irrecv(IR_RECEIVER_PIN, 1024, 50, true);

void setup()
{
  // シリアルコンソールの有効化
  Serial.begin(115200);

  // ボタン
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  irrecv.setUnknownThreshold(12);
  irrecv.setTolerance(kTolerance);
  irrecv.enableIRIn();

  // LED
  led.begin();
  led.setPixelColor(0, LED_COLOR_BLUE);
  led.show();
}

bool ir_receive_mode = false;          // 受信中モード
uint32_t ir_receive_mode_start_ms = 0; // 受信中モード開始時刻
decode_results results;                // 受信結果を入れる構造体

void loop()
{
  bool button = digitalRead(BUTTON_PIN);
  if (!ir_receive_mode && button == LOW)
  {
    // ボタン押下検出
    // 受信モードに入る
    ir_receive_mode = true;
    ir_receive_mode_start_ms = millis();

    led.setPixelColor(0, LED_COLOR_YELLOW);
    led.show();

    Serial.println("Start receive");
  }

  if (ir_receive_mode)
  {
    // 受信中
    if (millis() - ir_receive_mode_start_ms > 10000)
    {
      // 10秒受信がなければ終了
      ir_receive_mode = false;

      led.setPixelColor(0, LED_COLOR_BLUE);
      led.show();

      Serial.println("receive failed");

      return;
    }

    // 受信、デコード
    if (!irrecv.decode(&results))
    {
      // 受信できていない
      return;
    }

    // 受信、デコード結果をシリアルに出力
    Serial.println("received:");
    Serial.println(resultToHumanReadableBasic(&results).c_str());

    // モードを戻す
    ir_receive_mode = false;

    led.setPixelColor(0, LED_COLOR_BLUE);
    led.show();
  }
}
