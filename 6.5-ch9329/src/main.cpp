#include <Arduino.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ssid.hpp>

#if defined(M5ATOM_BOARD)

#define LED_PIN GPIO_NUM_27       // LEDピン
#define CH9329_SERIAL Serial1     // 利用するSerial
#define CH9329_TX_PIN GPIO_NUM_26 // TXD
#define CH9329_RX_PIN GPIO_NUM_32 // RXD

#elif defined(IOT_SERVER_BOARD)

#define LED_PIN GPIO_NUM_2       // LEDピン
#define CH9329_SERIAL Serial1    // 利用するSerial
#define CH9329_TX_PIN GPIO_NUM_7 // TXD
#define CH9329_RX_PIN GPIO_NUM_8 // RXD

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

WebServer server(80);

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

void sendCH9329Key(uint8_t modifier, uint8_t keycodes[6])
{
  uint8_t buf[14];
  memset(buf, 0, sizeof(buf));

  buf[0] = 0x57;
  buf[1] = 0xAB;
  buf[2] = 0x00;
  buf[3] = 0x02;
  buf[4] = 0x08;
  // モディファイヤ
  buf[5] = modifier;
  buf[6] = 0x00;
  // キーコード1
  buf[7] = keycodes[0];
  // キーコード2
  buf[8] = keycodes[1];
  // キーコード3
  buf[9] = keycodes[2];
  // キーコード4
  buf[10] = keycodes[3];
  // キーコード5
  buf[11] = keycodes[4];
  // キーコード6
  buf[12] = keycodes[5];
  // チェックサム
  uint8_t checksum = 0;
  for (int i = 0; i < 13; i++)
  {
    checksum += buf[i];
  }
  buf[13] = checksum;

  for (int i = 0; i < 14; i++)
  {
    CH9329_SERIAL.write(buf[i]);
  }
}

void handlePostCH9329KeyboardAPI(void)
{
  Serial.println("access POST /ch9329/keyboard");

  // 処理中を色で表示
  led.setPixelColor(0, LED_COLOR_YELLOW);
  led.show();

  JsonDocument resDoc;
  JsonDocument reqDoc;
  char resBodyBuf[1024 * 2]; // レスポンスのバッファ

  // リクエストボディのJSONデシリアライズ
  String requestBody = server.arg("plain");
  Serial.print("request body:");
  Serial.println(requestBody);
  DeserializationError err = deserializeJson(reqDoc, requestBody);
  if (err)
  {
    // デシリアライズ失敗した場合のエラー
    resDoc["error"] = "failed to deserialize Json";
    resDoc["error_detail"] = err.c_str();
    resDoc["success"] = false;

    // JSONをレスポンス
    serializeJson(resDoc, resBodyBuf, sizeof(resBodyBuf));
    Serial.println(resBodyBuf);
    server.send(401, "application/json", resBodyBuf);

    // 処理終了のLED表示
    led.setPixelColor(0, LED_COLOR_BLUE);
    led.show();
    return;
  }

  // リクエストのJSONを読む
  const bool reqCtrl = reqDoc["ctrl"];
  const bool reqShift = reqDoc["shift"];
  const bool reqAlt = reqDoc["alt"];
  const bool reqWin = reqDoc["win"];
  const JsonArray reqKeycodeArray = reqDoc["keycode"];

  // モディファイアデータの作成
  uint8_t modifier = 0x00;
  if (reqCtrl)
    modifier |= 0x01;
  if (reqShift)
    modifier |= 0x02;
  if (reqAlt)
    modifier |= 0x04;
  if (reqWin)
    modifier |= 0x08;

  // JSONのリストからキーコードのバッファに読む
  Serial.printf("keycode: ");
  uint8_t keycodes[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  for (int i = 0; i < reqKeycodeArray.size() && i < 6; i++)
  {
    keycodes[i] = reqKeycodeArray[i].as<uint8_t>();
    Serial.printf("0x%02X ", keycodes[i]);
  }
  Serial.printf("\r\n");

  // キーコードの送信
  sendCH9329Key(modifier, keycodes);

  delay(100);

  // キーを離す操作を送信
  uint8_t freeKeycodes[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  sendCH9329Key(0x00, freeKeycodes);

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

  // Serial1の初期化
  CH9329_SERIAL.setPins(CH9329_RX_PIN, CH9329_TX_PIN);
  CH9329_SERIAL.begin(9600);

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
  server.on("/ch9329/keyboard", HTTP_POST, handlePostCH9329KeyboardAPI);
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
