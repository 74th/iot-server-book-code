#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>

#define LED_PIN GPIO_NUM_27     // LEDピン
#define BUTTON_PIN GPIO_NUM_39  // ボタンピン
#define IR_SEND_PIN GPIO_NUM_26 // M5 IR赤外線送信ピン(Grove D2)
// #define IR_SEND_PIN GPIO_NUM_12 // Atom内蔵赤外線送信ピン

// LEDの色の定義
#define LED_COLOR_BLUE Adafruit_NeoPixel::Color(0, 0, 32)
#define LED_COLOR_YELLOW Adafruit_NeoPixel::Color(32, 32, 0)

// LEDの定義（数、制御ピン、仕様）
Adafruit_NeoPixel led(1, LED_PIN, NEO_GRB + NEO_KHZ800);

// 赤外線送信の定義
IRsend irsend(IR_SEND_PIN);

uint64_t sony_tv_power_code = 0xA90;

void setup()
{
  // ボタン
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // 赤外線送信の初期化
  irsend.begin();

  // LED
  led.begin();
  led.setPixelColor(0, LED_COLOR_BLUE);
  led.show();
}

bool latest_button_state = HIGH;

void loop()
{
  bool button = digitalRead(BUTTON_PIN);
  if (button == LOW && latest_button_state == HIGH)
  {
    // ボタン押下検出
    printf("Start send\n");

    led.setPixelColor(0, LED_COLOR_YELLOW);
    led.show();

    irsend.sendSony(sony_tv_power_code, 12, 2);

    printf("Done send\n");

    led.setPixelColor(0, LED_COLOR_BLUE);
    led.show();
  }

  latest_button_state = button;
}
