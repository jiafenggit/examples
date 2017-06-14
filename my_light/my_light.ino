/*
 ESP8266 连接物林 MQTT 服务示例

 本代码演示 ESP8266 接入物林平台，根据接收到的控制信号开关LED的功能。

 基本流程为：
  - 连接WiFi
  - 连接MQTT服务器并订阅控制信道
  - 控制信道收到消息时，如果消息内容为 light=on ，则点亮 LED，否则关闭LED。

 更多信息请参考 http://doc.woolyn.net.cn 
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// LED 连接在 GPIO2 上
#define LED_PIN 2

// WiFi 配置，修改为自己的
const char* ssid = "CU_vxCa";
const char* password = "ypt9nbrf";

// 物林MQTT接入参数，参考设备配置修改
const char* mqtt_server = "mqtt.woolyn.net";
const char* ctrl_topic = "/d/my_light/ctrl";
const char* mqtt_client_id = "my_light";
const char* mqtt_username = "my_light";
const char* mqtt_passwd = "QfTUfL0PQhfs0xY8PIZvE-oyXZXWe5vF";

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  pinMode(LED_PIN, OUTPUT);     // 设置端口为输出模式
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
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
 * 收到控制消息后执行的回调函数
 * 
 * @param topic 收到的 Topic ，正常情况下是控制信道 Topic， 如 /d/my_light/ctrl
 * @param payload 收到的消息内容, 如 light=on 或 light=off
 * @param length 消息长度
 */
void callback(char* topic, byte* payload, unsigned int length) {
  char* msg = (char*)malloc(length + 1);
  memcpy(msg, payload, length);
  msg[length] = '\0';
  Serial.print("Message arrived :");
  Serial.println(msg);

  if(strncmp(msg, "light=", 6) == 0 && length >= 8 ) {
    if(msg[6] == 'o' && msg[7] == 'n') { // 判断收到的消息内容是否为 light=on
       digitalWrite(LED_PIN, HIGH);
    } else {
       digitalWrite(LED_PIN, LOW);
    }
  }
  
  free(msg);
}

/**
 * 自动重连
 */
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // 尝试连接 MQTT 服务器
    if (client.connect(mqtt_client_id, mqtt_username, mqtt_passwd)) {
      Serial.println("connected");
      // 连接成功则订阅控制信道（Topic)
      client.subscribe(ctrl_topic);
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
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
