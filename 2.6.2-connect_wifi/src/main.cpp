#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include <WiFi.h>
#include <ssid.hpp>

#if defined(M5ATOM_BOARD)

#define LED_PIN GPIO_NUM_27 // LEDピン

#elif defined(IOT_SERVER_BOARD)

#define LED_PIN GPIO_NUM_2 // LEDピン

#endif

// LEDの色の定義
#define LED_COLOR_GREEN Adafruit_NeoPixel::Color(0, 32, 0)
#define LED_COLOR_BLUE Adafruit_NeoPixel::Color(0, 0, 32)
#define LED_COLOR_YELLOW Adafruit_NeoPixel::Color(32, 32, 0)

// LEDの定義（数、制御ピン、仕様）
Adafruit_NeoPixel led(1, LED_PIN, NEO_GRB + NEO_KHZ800);

// SSID、パスワードの文字列化
const char *ssid = WIFI_SSID;
const char *pass = WIFI_PASSWORD;

void setup()
{
  Serial.begin(115200);

  // LED
  led.begin();

  // セットアップ中は緑色にする
  led.setPixelColor(0, LED_COLOR_GREEN);
  led.show();

  Serial.print("setup wifi.");
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println("connected");
  Serial.print("IP:");
  Serial.println(WiFi.localIP());

  // セットアップ後は青色にする
  led.setPixelColor(0, LED_COLOR_BLUE);
  led.show();
}

void loop()
{
  Serial.print("IP:");
  Serial.println(WiFi.localIP());
  delay(1000);
}
