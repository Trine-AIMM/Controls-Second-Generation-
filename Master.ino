#include <esp_now.h>
#include <WiFi.h>

uint8_t neoAddress[] = {0x74, 0x4D, 0xBD, 0x7D, 0x26, 0xA4};
uint8_t gripperAddress[] = {0x48, 0x27, 0xE2, 0xFD, 0x7C, 0x08};
uint8_t tempSensorAddress[] = {0x34, 0x85, 0x18, 0x7B, 0x22, 0x38};
uint8_t payloadReleaseAddress[] = {0xDC, 0xDA, 0x0C, 0x21, 0x61, 0x44};

int data = 0;
int tempData;
esp_now_peer_info_t peerInfo;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  char macStr[18];
  Serial.print("Packet to: ");
  // Copies the sender mac address to a string
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print(macStr);
  Serial.print(" send status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&tempData, incomingData, sizeof(tempData));
}

void setup() {
  Serial.begin(115200);
 
  WiFi.mode(WIFI_STA);
 
  if (esp_now_init() != ESP_OK) {
    
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));


  // register peer
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;

  // register neo motor 
  memcpy(peerInfo.peer_addr, neoAddress, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  // register servo gripper 
  memcpy(peerInfo.peer_addr, gripperAddress, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  // register temperature sensor peer  
  memcpy(peerInfo.peer_addr, tempSensorAddress, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  // register payload release peer  
  memcpy(peerInfo.peer_addr, payloadReleaseAddress, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
}
 
void loop() {
  Serial.println("Select an Option");
  Serial.println("1.) Deploy the net (Enter -1 to raise it back up)");
  Serial.println("2.) Drop the sensor");
  Serial.println("3.) Read temperature data (Enter -3 to stop reading data)");
  Serial.println("4.) Drop the football");

  while (Serial.available() == 0) {
    // Wait for input
  }

  data = Serial.parseInt();
  Serial.read();

  esp_err_t result = esp_now_send(0, (uint8_t *) &data, sizeof(data));
  
  while (data == 3) {
    Serial.print("Celsius temperature: ");
    Serial.println(tempData);
    delay(1001);
    if (Serial.available() != 0){
      int stopSignal = Serial.parseInt();
      if (stopSignal == -1){
        Serial.println("Transmission Halted");
        data = 0;
      }
    }
  }
  Serial.read();
  delay(500);
}
