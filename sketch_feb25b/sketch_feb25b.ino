/* ###追加ここから### */
#include <WiFi.h>                           //WiFi接続ライブラリ
#include <PubSubClient.h>
#define TOPIC "isc/pressure/01"
const char* ssid = "HUAWEI_E5785_46CC";               //WiFiのSSID
const char* password = "Y7JG5RG9B5B";          //WiFiのパスワード
const char* mqtt_server = "mqtt.isc.ac.jp";  //中継サーバアドレス
WiFiClient espClient;
PubSubClient client(espClient);      //MQTTライブラリ
/* ###追加ここまで### */

const int pressureSensorPin = 36;
int maxPressureValue = 0;
unsigned long startTime = 0;
const unsigned long detectionInterval = 500;  // データを記録する時間（ミリ秒）
unsigned long lastDisplayTime = 0;
const unsigned long displayInterval = 500;  // 表示する間隔（ミリ秒）


// 閾値
const int threshold = 100;

void setup() {
  Serial.begin(9600);

/* ###追加ここから### */
  WiFi.begin(ssid, password);               //WiFi接続開始
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED) {   //WiFiが未接続の間待つ
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());      //割り当てられたIPアドレスを出力

  client.setServer(mqtt_server, 1883);      //MQTT Brokerアドレスなど
  client.setCallback(callback);             //メッセージ受信時のｺｰﾙﾊﾞｯｸ
/* ###追加ここまで### */
}

void loop() {
/* ###追加ここから### */
  if (!client.connected()) {                //MQTT接続済みか確認
    reconnect();                            //未接続なら接続処理
  }
  client.loop();                            //MQTT処理
/* ###追加ここまで### */

  int pressureValue = analogRead(pressureSensorPin);
  // 一定以上の圧力を検知した場合
  if (pressureValue > 50) {  // ここで適切な閾値を設定
    unsigned long currentTime = millis();
    // 最初の検知または一定時間が経過した場合
    if (maxPressureValue == 0 || currentTime - startTime > detectionInterval) {
      maxPressureValue = pressureValue;
      startTime = currentTime;
    } else {
      // 最大値を更新
      if (pressureValue > maxPressureValue) {
        maxPressureValue = pressureValue;
      }
    }
  }
  // 一定以上の数値の場合のみシリアル通信に表示
  if (pressureValue > 50) {  // 適切な閾値を設定
    unsigned long currentDisplayTime = millis();
    if (currentDisplayTime - lastDisplayTime > displayInterval) {
      Serial.print("Max Pressure Value: ");
      Serial.println(maxPressureValue);
      lastDisplayTime = currentDisplayTime;
      // もし受信されたシリアル通信が100以上だった場合、BOX2Dを処理する
      if (maxPressureValue >= threshold) {
/* ###追加ここから### */
        String message = "{\"pressure\":\"" + String(maxPressureValue) + "\"}";
        char charBuf[64];
        message.toCharArray(charBuf, 64);
        client.publish(TOPIC, charBuf);
/* ###追加ここまで### */
      }
    }
  }
  delay(50);  // データ取得の間隔を設定
}

/* ###追加ここから### */
// MQTT Broker接続処理関数
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // ランダムなクライアントIDを作る
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    // MQTT Brokerに接続
    if (client.connect(clientId.c_str(), "isc", "iwasaki3_")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

// MQTTメッセージ受信時に呼び出されるコールバック関数
void callback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0';
  String msg = String((char*) payload);
  Serial.println(msg);
}
/* ###追加ここまで### */