#include <HardwareSerial.h>

#define BAUDRATE        115200

#define LED_PIN         2
#define SIM_ENABLE_PIN  15
#define SIM_TX_PIN      17
#define SIM_RX_PIN      16



// HardwareSerial SerialAT(1); // UART1
HardwareSerial SerialAT(2); // UART2

void enableModuleSIM();
void setPinLED();
void turnOnLED();
void sendATCommand(String command, int wait = 1000);

void setup() {
    Serial.begin(BAUDRATE);
    setPinLED();
    SerialAT.begin(BAUDRATE, SERIAL_8N1, SIM_RX_PIN, SIM_TX_PIN); // Rx, Tx của ESP32
    delay(3000);

    enableModuleSIM();

    Serial.println("Send AT command to module A7682S...");
    //SerialAT.println("AT"); // Gửi lệnh kiểm tra
    sendATCommand("AT");
    // sendATCommand("AT+CGDCONT=1,\"IP\",\"internet\"");          //configuration APN
    sendATCommand("AT+CGDCONT=1,\"IP\",\"v-internet\"");        //configuration APN
    delay(500);
    sendATCommand("AT+CGACT=1,1");                              //activate PDP context
    delay(500);
    sendATCommand("AT+CGPADDR=1");                              //check IP

    sendATCommand("AT+CMQTTSTART");                             //initialize MQTT
    sendATCommand("AT+CMQTTACCQ=0,\"mqttx_1a834cef\"");         //create client
    sendATCommand("AT+CMQTTCONNECT=0,\"tcp://broker.emqx.io:1883\",60,1");      //connect MQTT broker

    //send data
    sendATCommand("AT+CMQTTTOPIC=0,18"); // 18 là độ dài topic
    delay(100);
    SerialAT.print("emqx/esp32-test");

    sendATCommand("AT+CMQTTPAYLOAD=0,17"); // 17 là độ dài payload
    delay(100);
    SerialAT.print("hello from ESP32");

    sendATCommand("AT+CMQTTPUBLISH=0,1,60");





    // sendATCommand("AT+SMCONF=\"URL\",\"broker.emqx.io\",1883"); //MQTT address and MQTT  port
    // sendATCommand("AT+SMCONF=\"KEEPTIME\",60");                 //connecting time
    // sendATCommand("AT+SMCONF=\"CLEANSS\",1");                   //clean session
    // sendATCommand("AT+SMCONF=\"mqttx_1a834cef\",\"esp_mqtt_nhi\"");     //ID Client MQTT
    // sendATCommand("AT+SMCONN");                                 //send command to connect MQTT


    // sendATCommand("AT+SMCONF=\"URL\",\"broker.emqx.io\",1883");     //MQTT address and MQTT  port
    // sendATCommand("AT+SMCONF=\"KEEPTIME\",60");                     //connecting time
    // sendATCommand("AT+SMCONF=\"CLEANSS\",1");                       //clean session
    // sendATCommand("AT+SMCONF=\"CLIENTID\",\"mqttx_1a834cef\"");     //ID Client MQTT
    // sendATCommand("AT+SMCONF=\"USERNAME\",\"\"");                   // Nếu không cần xác thực
    // sendATCommand("AT+SMCONF=\"PASSWORD\",\"\"");                   // Nếu không cần xác thực
    // sendATCommand("AT+SMCONN");

    // sendATCommand("AT+SMPUB=\"emqx/esp32-test\",17,1,0");
    // delay(100);
    // SerialAT.print("hello from ESP32");




    // delay(1000);
    // sendATCommand("AT+SMPUB=\"emqx/esp32-test\",17,1,0");
    // delay(100);
    // SerialAT.print("hello from ESP32");
}

void loop() {
    // while (SerialAT.available()) {
    //     Serial.write(SerialAT.read()); // In phản hồi từ module
    //     // char c = SerialAT.read();
    //     // Serial.print(c);
    //     turnOnLED();
    // }
}

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

void sendATCommand(String command, int wait) {
    Serial.println(">>" + command);
    SerialAT.println(command);
    delay(wait);
    while (SerialAT.available()) {
        turnOnLED();
        Serial.write(SerialAT.read());
    }
}
