#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>
#include <esp_now.h>

// Data wire is conntec to the Arduino digital pin 14
#define ONE_WIRE_BUS 14

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

// Define Master MAC Address:
uint8_t masterAddress[] = {0xDC, 0xDA, 0x0C, 0x21, 0x60, 0xC0};
int myData = 0;
esp_now_peer_info_t peerInfo;

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
  memcpy(&myData, incomingData, sizeof(myData));
}

void setup(void) {
  // Start serial communication for debugging purposes
  Serial.begin(115200);

  // Start up the library
  sensors.begin();

  WiFi.mode(WIFI_STA);
  esp_now_init();

  // Register Send/Recieve Functions
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));

  // register peer
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;

  // register master Arduino 
  memcpy(peerInfo.peer_addr, masterAddress, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
  }
}

void sendTempData(){
  sensors.requestTemperatures(); 
  int data = sensors.getTempFByIndex(0);
  esp_now_send(0, (uint8_t *) &data, sizeof(data));
}

void loop(void){ 
  if (myData == 3){
    while (myData != -1){
      sendTempData();
      delay(1000);
    }
    myData = 0;
  }
}
