#include <Adafruit_NeoPixel.h>
#include <Arduino.h>

// LEDピン
#define LED_PIN GPIO_NUM_27

// LEDの色の定義
#define LED_COLOR_BLUE Adafruit_NeoPixel::Color(0, 0, 32)
#define LED_COLOR_YELLOW Adafruit_NeoPixel::Color(32, 32, 0)

// LEDの定義（数、制御ピン、仕様）
Adafruit_NeoPixel led(1, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup()
{
  // LEDの初期化
  led.begin();
}

uint32_t loop_count = 0;

void loop()
{
  // 色設定
  if (loop_count % 2 == 0)
  {
    printf("Blue!\n");
    led.setPixelColor(0, LED_COLOR_BLUE);
  }
  else
  {
    printf("Yellow!\n");
    led.setPixelColor(0, LED_COLOR_YELLOW);
  }
  // LEDの表示
  led.show();

  loop_count++;
  delay(200);
}
