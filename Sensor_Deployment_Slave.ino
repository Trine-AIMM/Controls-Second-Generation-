#include <esp_now.h>
#include <WiFi.h>
#include <ESP32Servo.h> 

int myData;
Servo servo;
const int servoPin = 18;
const int relayPin = 6;

//callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("x: ");
  Serial.println(myData);
  Serial.println();
}

void open(){
  Serial.println("Opening");
  for (int angle = 100; angle <= 135; angle += 1) {  
    servo.write(angle);
    delay(10);
  }
}

void close() {
  Serial.println("Closing");
  for (int angle = 135; angle >= 100; angle -= 1) {  
    servo.write(angle);
    delay(10);
  }
}
 
void setup() {
  //Initialize Serial Monitor
  Serial.begin(115200);
  
  //Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  //Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));

  servo.attach(servoPin, 500, 2500);
  pinMode(relayPin, OUTPUT);
}
 
void loop() {
  if (myData == 2) {
    open();
    delay(1000);
    close();
    myData = 0;
    Serial.println(myData);
  } else if (myData == 5){
    digitalWrite(relayPin, HIGH);
    delay(5000);
    digitalWrite(relayPin, LOW);
    myData = 0;
  }
}
