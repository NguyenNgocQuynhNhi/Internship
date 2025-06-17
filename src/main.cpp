#include <HardwareSerial.h>

#define BAUDRATE        115200
#define SIM_ENABLE_PIN  15
#define SIM_RX_PIN      16
#define SIM_TX_PIN      17

// ================== MQTT Config ==================
#define MQTT_CLIENT_ID   "mqttx_e3ab8d86"
#define MQTT_BROKER_URL  "broker.emqx.io"
// #define MQTT_TOPIC       "emqx/esp32-test"
// #define MQTT_PAYLOAD     "hello from ESP32"
const char* MQTT_TOPIC   = "LTE";
const char* MQTT_PAYLOAD = "ESP32";

#define STRINGIFY(x) STRINGIFY2(x)
#define STRINGIFY2(x) #x
#define MQTT_KEEPALIVE   60
     

HardwareSerial SerialAT(2);  // UART2: dùng GPIO 16 (RX), 17 (TX)

void enableSIMModule();
void sendCommand(String cmd, int delayMs = 1000);
bool sendCommandAndWaitFor(String expected, String cmd = "", int timeout = 5000);

void setup() {
    Serial.begin(BAUDRATE);
    SerialAT.begin(BAUDRATE, SERIAL_8N1, SIM_RX_PIN, SIM_TX_PIN);
    delay(3000);

    enableSIMModule();

    Serial.println("🚀 Bắt đầu gửi lệnh AT đến A7682S...");

    sendCommand("AT");
    sendCommand("AT+CGDCONT=1,\"IP\",\"m-wap\"");
    sendCommand("AT+CGACT=1,1");

    sendCommand("AT+CSQ");      // Kiểm tra chất lượng sóng
    sendCommand("AT+CREG?");    // Đăng ký mạng GSM
    sendCommand("AT+CGATT?");   // GPRS đã gắn chưa
    sendCommand("AT+CGPADDR=1"); // Đã cấp IP chưa
    sendCommand("AT+PING=\"8.8.8.8\"");  // Ping Google DNS
    sendCommand("AT+CMEE=2");           // Bật báo lỗi chi tiết




    sendCommand("AT+CMQTTSTART");
    sendCommand("AT+CMQTTACCQ=0,\"esp32_client\"");
    sendCommand("AT+CMQTTCONNECT=0,\"tcp://broker.emqx.io:1883\",60,0");
    // sendCommand("AT+CMQTTCONNECT=0,\"tcp://broker.emqx.io:1883\",60,0,\"nhiTestLTE\",\"123456\"");


    // Kiểm tra xem kết nối MQTT thành công chưa
    if (!sendCommandAndWaitFor("+CMQTTCONNECT: 0,0")) {
        Serial.println("❌ Không kết nối được MQTT broker!");
    }
    else {
        Serial.println("Kết nối được MQTT broker!");
    }


    // Gửi topic
    sendCommand("AT+CMQTTTOPIC=0," + String(strlen(MQTT_TOPIC)));
    if (!sendCommandAndWaitFor(">", MQTT_TOPIC)) return;

    // Gửi payload
    sendCommand("AT+CMQTTPAYLOAD=0," + String(strlen(MQTT_PAYLOAD)));
    if (!sendCommandAndWaitFor(">", MQTT_PAYLOAD)) return;


    // // Publish với QoS = 0, retain = 0
    // if (!sendCommandAndWaitFor("OK", "AT+CMQTTPUBLISH=0,0,0", 3000)) {
    //     Serial.println("❌ Publish thất bại!");
    // } else {
    //     Serial.println("✅ Đã publish thành công! Mở MQTTX và kiểm tra.");
    // }

    // Publish với QoS = 0, retain = 0
    if (!sendCommandAndWaitFor("OK", "AT+CMQTTPUBLISH=0,0,0", 3000)) {
        Serial.println("❌ Publish thất bại!");
    } else {
        Serial.println("✅ Đã publish thành công! Mở MQTTX và kiểm tra.");
    }



    // Wait & disconnect gracefully
    delay(2000);

    sendCommand("AT+CMQTTDISC=0,60");
    sendCommand("AT+CMQTTREL=0");
    sendCommand("AT+CMQTTSTOP");
}

void loop() {
    while (SerialAT.available()) {
        Serial.write(SerialAT.read());
    }
}

// ------------------- Function implemention -------------------

void enableSIMModule() {
    pinMode(SIM_ENABLE_PIN, OUTPUT);
    digitalWrite(SIM_ENABLE_PIN, HIGH);
    delay(1000);
    digitalWrite(SIM_ENABLE_PIN, LOW);
    delay(3000);
}

void sendCommand(String cmd, int delayMs) {
    Serial.print(">> ");
    Serial.println(cmd);
    SerialAT.println(cmd);
    delay(delayMs);
}

bool sendCommandAndWaitFor(String expected, String cmd, int timeout) {
    long start = millis();
    if (cmd != "") {
        Serial.print(">> ");
        Serial.println(cmd);
        SerialAT.println(cmd);
    }

    String response = "";
    while (millis() - start < timeout) {
        if (SerialAT.available()) {
            response += SerialAT.readStringUntil('\n');
            Serial.println(response);
            if (response.indexOf(expected) != -1) return true;
        }
    }
    Serial.println("⚠️ Timeout đợi '" + expected + "'");
    return false;
}
