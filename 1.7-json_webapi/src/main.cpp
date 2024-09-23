#include <Arduino.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include <WebServer.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRrecv.h>
#include <IRac.h>
#include <IRutils.h>
#include <ssid.hpp>

#define LED_PIN GPIO_NUM_27         // LEDピン
#define IR_SEND_PIN GPIO_NUM_26     // M5 IR赤外線送信ピン(Grove D2)
#define IR_RECEIVER_PIN GPIO_NUM_32 // 赤外線受信ピン

// LEDの色の定義
#define LED_COLOR_GREEN Adafruit_NeoPixel::Color(0, 32, 0)
#define LED_COLOR_BLUE Adafruit_NeoPixel::Color(0, 0, 32)
#define LED_COLOR_YELLOW Adafruit_NeoPixel::Color(32, 32, 0)

// LEDの定義（数、制御ピン、仕様）
Adafruit_NeoPixel led(1, LED_PIN, NEO_GRB + NEO_KHZ800);

// SSID、パスワードの文字列化
const char *ssid = WIFI_SSID;
const char *pass = WIFI_PASSWORD;

// 赤外線送信の定義
IRsend irsend(IR_SEND_PIN);

// 赤外線受信の定義
IRrecv irrecv(IR_RECEIVER_PIN, 1024, 50, true);

WebServer server(80);

// hexの文字列をuint64_tに変換
uint64_t hexStringToUint64(const char *hexString)
{
  if (hexString[0] == '0' && (hexString[1] == 'x' || hexString[1] == 'X'))
  {
    hexString += 2;
  }

  uint64_t result = strtoull(hexString, NULL, 16);

  return result;
}

// 16進数文字を対応する数値に変換
unsigned char hexCharToValue(char hexChar)
{
  if ('0' <= hexChar && hexChar <= '9')
  {
    return hexChar - '0';
  }
  else if ('a' <= hexChar && hexChar <= 'f')
  {
    return hexChar - 'a' + 10;
  }
  else if ('A' <= hexChar && hexChar <= 'F')
  {
    return hexChar - 'A' + 10;
  }
  else
  {
    return 0;
  }
}

// HEX文字列をバイト列に変換する関数
void hexStringToByteArray(const char *hexString, unsigned char *byteArray, size_t *byteArraySize)
{
  if (hexString[0] == '0' && (hexString[1] == 'x' || hexString[1] == 'X'))
  {
    hexString += 2;
  }

  size_t hexLen = strlen(hexString);
  *byteArraySize = hexLen / 2;

  for (size_t i = 0; i < hexLen / 2; ++i)
  {
    byteArray[i] = (hexCharToValue(hexString[2 * i]) << 4) | hexCharToValue(hexString[2 * i + 1]);
  }
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

// POST /ir/send
void handleIRSendAPI()
{
  Serial.println("access POST /ir/send");

  // 処理中を色で表示
  led.setPixelColor(0, LED_COLOR_YELLOW);
  led.show();

  JsonDocument resDoc;       // レスポンスのJSON
  JsonDocument reqDoc;       // リクエストのJSON
  char resBodyBuf[1024 * 2]; // レスポンスのバッファ

  // リクエストボディのJSONデシリアライズ
  String requestBody = server.arg("plain");
  Serial.print("request body:");
  Serial.println(requestBody);
  DeserializationError err = deserializeJson(reqDoc, requestBody);
  if (err)
  {
    // デシリアライズ失敗した場合はエラーを返す
    String errorMessage = "failed to deserialize Json.";
    resDoc["error"] = errorMessage;
    resDoc["success"] = false;
    serializeJson(resDoc, resBodyBuf, sizeof(resBodyBuf));
    Serial.println(errorMessage);
    server.send(401, "application/json", resBodyBuf);

    // 処理終了のLED表示
    led.setPixelColor(0, LED_COLOR_BLUE);
    led.show();
    return;
  }

  // 信号のタイプのプロパティを取得
  const char *type = reqDoc["type"];
  if (type == NULL || strlen(type) == 0)
  {
    String errorMessage = "type is required.";
    resDoc["error"] = errorMessage;
    resDoc["success"] = false;
    serializeJson(resDoc, resBodyBuf, sizeof(resBodyBuf));
    Serial.println(errorMessage);
    server.send(401, "application/json", resBodyBuf);

    // 処理終了のLED表示
    led.setPixelColor(0, LED_COLOR_BLUE);
    led.show();
    return;
  }
  //
  // 信号データのプロパティを取得
  const char *hexData = reqDoc["hex"];
  if (hexData == NULL || strlen(hexData) == 0)
  {
    // 取得できない場合エラーを返す
    String errorMessage = "hex is required.";
    resDoc["error"] = errorMessage;
    resDoc["success"] = false;
    serializeJson(resDoc, resBodyBuf, sizeof(resBodyBuf));
    Serial.println(errorMessage);
    server.send(401, "application/json", resBodyBuf);

    // 処理終了のLED表示
    led.setPixelColor(0, LED_COLOR_BLUE);
    led.show();
    return;
  }

  // タイプごとに送信
  if (strcmp(type, "SONY") == 0)
  {
    // SONY TV

    // hexテキストをuint64_tにパース
    uint64_t data_u64 = hexStringToUint64(hexData);
    Serial.printf("data: %x\r\n", data_u64);

    // データ長を特定（SONYは、12、15、20ビットの3種類）
    int16_t dataSize_bits;
    if (data_u64 <= 1 << 12)
    {
      dataSize_bits = 12;
    }
    else if (data_u64 <= 1 << 15)
    {
      dataSize_bits = 15;
    }
    else
    {
      dataSize_bits = 20;
    }

    // 送信
    irsend.sendSony(data_u64, dataSize_bits, 2);
  }
  else if (strcmp(type, "MITSUBISHI_AC") == 0)
  {
    // 三菱エアコン
    unsigned char data_bytes[64];
    size_t dataSize_bytes;

    // hexデータ
    hexStringToByteArray(hexData, data_bytes, &dataSize_bytes);
    Serial.printf("data: %llx\r\n", data_bytes);

    // 送信
    irsend.sendMitsubishiAC(data_bytes, dataSize_bytes);
  }
  else
  {
    // 不明なタイプ
    String errorMessage = "unknown type";
    resDoc["error"] = errorMessage;
    resDoc["success"] = false;
    serializeJson(resDoc, resBodyBuf, sizeof(resBodyBuf));
    Serial.println(resBodyBuf);
    server.send(401, "application/json", resBodyBuf);

    // 処理終了
    led.setPixelColor(0, LED_COLOR_BLUE);
    led.show();
    return;
  }

  // 成功時の応答
  resDoc["success"] = true;
  serializeJson(resDoc, resBodyBuf, sizeof(resBodyBuf));
  Serial.println(resBodyBuf);
  server.send(200, "application/json", resBodyBuf);

  // 処理終了
  led.setPixelColor(0, LED_COLOR_BLUE);
  led.show();
}

// GET /ir/send
void handleIRReceiveAPI()
{
  Serial.println("access GET /ir/receive");

  JsonDocument resDoc;       // レスポンスのJSON
  char resBodyBuf[1024 * 2]; // レスポンスのバッファ

  uint32_t start_time = millis();
  while (true)
  {
    if (millis() > start_time + 10000)
    {
      // タイムアウト
      resDoc["success"] = false;
      resDoc["error"] = "timeout";

      serializeJson(resDoc, resBodyBuf, sizeof(resBodyBuf));
      Serial.println(resBodyBuf);
      server.send(400, "application/json", resBodyBuf);

      // 処理終了のLED表示
      led.setPixelColor(0, LED_COLOR_BLUE);
      led.show();
      break;
    }

    // 受信デコード結果
    decode_results results;
    if (!irrecv.decode(&results))
    {
      // 受信できていない
      continue;
    }

    // 受信、デコード結果をシリアルに出力
    Serial.println("received:");
    Serial.println(resultToHumanReadableBasic(&results));

    // 受信、デコード結果をJSON形式でレスポンス
    resDoc["data"]["type"] = typeToString(results.decode_type, results.repeat);
    resDoc["data"]["hex"] = resultToHexidecimal(&results);
    resDoc["success"] = true;

    serializeJson(resDoc, resBodyBuf, sizeof(resBodyBuf));
    Serial.println(resBodyBuf);
    server.send(400, "application/json", resBodyBuf);

    // 処理終了のLED表示
    led.setPixelColor(0, LED_COLOR_BLUE);
    led.show();
    break;
  }
}

void setup()
{
  Serial.begin(115200);

  // LED
  led.begin();

  // セットアップ中は緑色にする
  led.setPixelColor(0, LED_COLOR_GREEN);
  led.show();

  // 赤外線送信の初期化
  irsend.begin();

  // 赤外線受信の初期化
  irrecv.setUnknownThreshold(12);
  irrecv.setTolerance(kTolerance);
  irrecv.enableIRIn();

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
  server.on("/ir/send", HTTP_POST, handleIRSendAPI);
  server.on("/ir/receive", HTTP_GET, handleIRReceiveAPI);
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
