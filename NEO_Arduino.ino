#include <esp_now.h>
#include <WiFi.h>

//Create a struct_message called myData
int myData;
int triggerPin = 47;

//callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("data: ");
  Serial.println(myData);
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

  pinMode(triggerPin, OUTPUT);
}
 
void loop() {
  if (myData == 1) {
    digitalWrite(triggerPin, HIGH);
    delay(1000);
    digitalWrite(triggerPin, LOW);
    myData = 0;
  }
}
