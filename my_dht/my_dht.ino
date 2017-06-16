/*
 ESP8266 连接物林 MQTT 上报数据示例

 本代码演示 ESP8266 接入物林平台，向平台上报温湿度数据的功能。

 基本流程为：
  - 连接WiFi
  - 每隔60秒，查询温湿度并通过数据信道发送到物林平台。

 本代码基于Arduino ESP8266 开发环境测试。
 第三方库 PubSubClient 、SimpleDHT 分别用于连接物林MQTT服务、查询DHT温湿度传感器，
 二者均可通过Arduino软件的“管理库”功能搜索安装。
 
 更多信息请参考 http://doc.woolyn.net.cn 
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SimpleDHT.h>

#define DHT_PIN 2     // what digital pin we're connected to

// WiFi 配置，修改为自己的
const char* ssid = "YOUR_WIFI";
const char* password = "YOUR_WIFI_PASSWORD";

// 物林MQTT接入参数，参考设备配置修改
const char* mqtt_server = "mqtt.woolyn.net";
const char* data_topic = "/d/my_dht/data";
const char* mqtt_client_id = "my_dht";
const char* mqtt_username = "my_dht";
const char* mqtt_passwd = "YOUR_MQTT_PASSWORD";

WiFiClient espClient;
PubSubClient client(espClient);
SimpleDHT11 dht;

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

/**
 * 设置并连接WiFi
 */
void setup_wifi() {
  delay(10);
  
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  // 连接WiFi
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  //连接成功则通过串口打印 IP 地址
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


/**
 * 查询温湿度数据并发送到云端
 */
void queryAndSend() {
  byte tmp, hum;

  //读取温、湿度数据
  if (dht.read(DHT_PIN, &tmp, &hum, NULL)) {
    Serial.print("Read DHT11 failed.");
    return;
  }

  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // 尝试连接 MQTT 服务器
    if (client.connect(mqtt_client_id, mqtt_username, mqtt_passwd)) {
      Serial.println("connected");
      char mqttMsg[30];
      memset(mqttMsg, 0, 30);
      //按“变量名1=数据值1&变量名2=数据值2”的规则拼接温度、湿度数据
      snprintf(mqttMsg, 30, "tmp=%d&hum=%d", tmp, hum); 
      //发布到云端
      client.publish(data_topic, mqttMsg);
      Serial.println(mqttMsg);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // 连接失败则等5秒后重新连接
      delay(5000);
    }
  } 
}

void loop() {
  queryAndSend();
  delay(60000);//每分钟发送一次数据，不能频率过高
}
