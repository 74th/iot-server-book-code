#include <Arduino.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ssid.hpp>

#if defined(M5ATOM_BOARD)

#define LED_PIN GPIO_NUM_27 // LEDピン
#define I2C_SCL_PIN GPIO_NUM_32
#define I2C_SDA_PIN GPIO_NUM_26

#elif defined(IOT_SERVER_BOARD)

#define LED_PIN GPIO_NUM_2 // LEDピン
#define I2C_SCL_PIN GPIO_NUM_0
#define I2C_SDA_PIN GPIO_NUM_1

#endif

// LEDの色の定義
#define LED_COLOR_GREEN Adafruit_NeoPixel::Color(0, 32, 0)
#define LED_COLOR_BLUE Adafruit_NeoPixel::Color(0, 0, 32)
#define LED_COLOR_YELLOW Adafruit_NeoPixel::Color(32, 32, 0)

// SHT30 のI2Cアドレス
#define SHT30_I2C_ADDR 0x44

// LEDの定義（数、制御ピン、仕様）
Adafruit_NeoPixel led(1, LED_PIN, NEO_GRB + NEO_KHZ800);

// SSID、パスワードの文字列化
const char *ssid = WIFI_SSID;
const char *pass = WIFI_PASSWORD;

WebServer server(80);

void setupSHT31(void)
{
  // ソフトリセット
  Wire.beginTransmission(SHT30_I2C_ADDR);
  Wire.write(0x30);
  Wire.write(0xA2);
  Wire.endTransmission();

  delay(300);
}

// GET /
void handleRootAPI()
{
  Serial.println("access GET /");
  server.send(200, "application/json", "{}");
}

// 404 Not Found
void handleNotFoundAPI()
{
  server.send(404, "application/json", "{\"message\": \"Not Found.\"}");
}

void handleGetSHT30API(void)
{
  Serial.println("access GET /sht31");

  // 処理中を色で表示
  led.setPixelColor(0, LED_COLOR_YELLOW);
  led.show();

  JsonDocument resDoc;       // レスポンスのJSON
  char resBodyBuf[1024 * 2]; // レスポンスのバッファ

  // One Shot のデータ取得
  Wire.beginTransmission(SHT30_I2C_ADDR);
  Wire.write(0x24);
  Wire.write(0x00);
  Wire.endTransmission();

  delay(300);

  Wire.requestFrom(SHT30_I2C_ADDR, 6);
  uint8_t read_buf[6];
  for (int i = 0; i < 6; i++)
  {
    read_buf[i] = Wire.read();
  }
  Wire.endTransmission();

  int32_t tempRaw, humRaw;
  float_t temp, hum;
  tempRaw = (read_buf[0] << 8) | read_buf[1];   // 上位バイトと下位バイトを結合
  temp = (float_t)(tempRaw) * 175 / 65535 - 45; // ℃に変換
  humRaw = (read_buf[3] << 8) | read_buf[4];    // 上位バイトと下位バイトを結合
  hum = (float_t)(humRaw) / 65535 * 100;        // %に変換

  resDoc["data"]["temperature"] = temp;
  resDoc["data"]["humidity"] = hum;
  resDoc["data"]["temperature_raw"] = tempRaw;
  resDoc["data"]["humidity_raw"] = humRaw;

  Serial.print("temperature：");
  Serial.print(temp);
  Serial.print(" humidity：");
  Serial.println(hum);

  resDoc["success"] = true;

  // JSONをレスポンス
  serializeJson(resDoc, resBodyBuf, sizeof(resBodyBuf));
  Serial.println(resBodyBuf);
  server.send(200, "application/json", resBodyBuf);

  // 完了後は青色に戻す
  led.setPixelColor(0, LED_COLOR_BLUE);
  led.show();
}

void setup()
{
  Serial.begin(115200);

  // LED
  led.begin();

  // セットアップ中は緑色にする
  led.setPixelColor(0, LED_COLOR_GREEN);
  led.show();

  // I2Cの初期化
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

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

  // Webサーバの開始
  server.on("/", HTTP_GET, handleRootAPI);
  server.on("/sht30", HTTP_GET, handleGetSHT30API);
  server.onNotFound(handleNotFoundAPI);
  server.begin();

  // セットアップ後は青色にする
  led.setPixelColor(0, LED_COLOR_BLUE);
  led.show();
}

// 最後のモニター出力
uint32_t last_monitor_time = 0;

void loop()
{
  if (millis() - last_monitor_time > 1000)
  {
    // モニター出力を1秒ごとにする
    last_monitor_time = millis();
    Serial.print("IP:");
    Serial.println(WiFi.localIP());
  }

  // サーバのリクエストを処理する
  server.handleClient();
}
