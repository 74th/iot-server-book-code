#include <Adafruit_NeoPixel.h>
#include <Arduino.h>

#if defined(M5ATOM_BOARD)

// LEDピン
#define LED_PIN GPIO_NUM_2

#elif defined(IOT_SERVER_BOARD)

// LEDピン
#define LED_PIN GPIO_NUM_27

#endif

// LEDの色の定義
#define LED_COLOR_BLUE Adafruit_NeoPixel::Color(0, 0, 32)
#define LED_COLOR_YELLOW Adafruit_NeoPixel::Color(32, 32, 0)

// LEDの定義（数、制御ピン、仕様）
Adafruit_NeoPixel led(1, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup()
{
  // シリアルコンソールの有効化
  Serial.begin(115200);

  // LEDの初期化
  led.begin();
}

uint32_t loop_count = 0;

void loop()
{
  // 色設定
  if (loop_count % 2 == 0)
  {
    Serial.println("Blue!");
    led.setPixelColor(0, LED_COLOR_BLUE);
  }
  else
  {
    Serial.println("Yellow!");
    led.setPixelColor(0, LED_COLOR_YELLOW);
  }
  // LEDの表示
  led.show();

  loop_count++;
  delay(200);
}
