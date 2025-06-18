#define TINY_GSM_MODEM_SIM7600  // A7682S tương thích SIM7600
#define TINY_GSM_USE_UART true  // Bắt buộc khi dùng Serial2

#include <Arduino.h>
#include <TinyGsmClient.h>
#include <PubSubClient.h>

#define BAUDRATE            115200

#define LED_PIN             2
#define SIM_ENABLE_PIN      15
#define SIM_TX_PIN          17
#define SIM_RX_PIN          16
#define SERIAL_MONITOR      Serial
#define SERIAL_AT           Serial2

const char apn[]  = "m-wap.mobifone.com.vn"; // hoặc "m3-world"
const char user[] = "";
const char pass[] = "";

TinyGsm modem(SERIAL_AT);
TinyGsmClient client(modem);
PubSubClient mqtt(client);

const char* broker = "broker.emqx.io";
const int   port   = 1883;
const char* topic  = "LTE";

void enableModuleSIM();
void setPinLED();
void turnOnLED();
void mqttCallback(char* topic, byte* payload, unsigned int len);
const char* simStatusToStr(SimStatus s);


void setup() {
    SERIAL_MONITOR.begin(BAUDRATE);
    setPinLED();
    SERIAL_AT.begin(BAUDRATE, SERIAL_8N1, SIM_RX_PIN, SIM_TX_PIN);
    delay(2000);
    enableModuleSIM();

    SERIAL_MONITOR.println("[MODEM] Restarting....");
    modem.restart();

    SimStatus simStatus = modem.getSimStatus();
    SERIAL_MONITOR.print("[MODEM] Sim status: ");
    SERIAL_MONITOR.println(simStatusToStr(simStatus));

    SERIAL_MONITOR.println("[MODEM] Connecting to network....");
    modem.gprsConnect(apn, user, pass);

    if (modem.isGprsConnected()) {
        SERIAL_MONITOR.println("[MODEM] GPRS connected");
    }
    else {
        SERIAL_MONITOR.println("[ERROR] GPRS failed");
        return;
    }

    mqtt.setServer(broker, port);
    mqtt.setCallback(mqttCallback);

}

void loop() {
    if (!mqtt.connected()) {
        SERIAL_MONITOR.println("[MQTT] Connecting....");
        if (mqtt.connect("esp32Client")) {
            SERIAL_MONITOR.println("[MQTT] Connected!");
            mqtt.subscribe(topic);
        }
        else {
            SERIAL_MONITOR.println("[MQTT] Failed, retrying....");
            delay(5000);
            return;
        }
    }

    mqtt.loop();

    static unsigned long lastSent = 0;
    if (millis() - lastSent > 10000) {
        lastSent = millis();
        mqtt.publish(topic, "Hello from ESP32 + A7682S");
    }
}


//-----------------------Function implementation---------------------------
void enableModuleSIM() {
    pinMode(SIM_ENABLE_PIN, OUTPUT);
    digitalWrite(SIM_ENABLE_PIN, HIGH);
    delay(1000);
    digitalWrite(SIM_ENABLE_PIN, LOW);
}

void setPinLED() {
    pinMode(LED_PIN, OUTPUT);
}

void turnOnLED() {
    digitalWrite(LED_PIN, HIGH);
}

void mqttCallback(char* topic, byte* payload, unsigned int len) {
    SERIAL_MONITOR.print("[MQTT] Message: ");
    for (int i = 0; i < len; i++) {
        SERIAL_MONITOR.print((char)payload[i]);
    }
    SERIAL_MONITOR.println();
}

const char* simStatusToStr(SimStatus s) {
    switch (s)
    {
    case SimStatus::SIM_ERROR:      
        return "SIM_ERROR";
        break;
    case SimStatus::SIM_LOCKED:
        return "SIM_LOCKED";
        break;
    case SimStatus::SIM_READY:
        return "SIM_READY";
        break;
    case SimStatus::SIM_ANTITHEFT_LOCKED:
        return "SIM_ANTITHEFT_LOCKED";
        break;
    default:
        return "UNKNOWN";
        break;
    }
}

