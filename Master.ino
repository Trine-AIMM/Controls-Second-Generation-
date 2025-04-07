#include <esp_now.h>
#include <WiFi.h>
#include <ESP32Servo.h>

Servo servoLeft;
Servo servoRight;

// Define the MAC addresses of the ESP-NOW devices
uint8_t neoAddress[] = {0x74, 0x4D, 0xBD, 0x7D, 0x26, 0xA4};
uint8_t gripperAddress[] = {0x48, 0x27, 0xE2, 0xFD, 0x7C, 0x08};
uint8_t tempSensorAddress[] = {0x34, 0x85, 0x18, 0x7B, 0x22, 0x38};
uint8_t payloadReleaseAddress[] = {0xDC, 0xDA, 0x0C, 0x21, 0x61, 0x44};
uint8_t thrustersAddress[] = {0x3C, 0x84, 0x27, 0xC3, 0x47, 0xE0};

int data = 0;
int tempData;

int servo1 = 18; 
int servo2 = 19;

float smoothedPower = 0;
float smoothedAngle = 0;
const float smoothingFactor = 0.1;  // Smaller = smoother, range: (0, 1]

bool sendTemperature = false;  // Flag to control temperature reporting

// Structure to receive controller data
struct ControllerData {
    int buttons;  // Button states (bitmask)
    int axisLX;   // Left stick X (-511 to 512)
    int axisLY;   // Left stick Y (-511 to 512)
    int axisRX;   // Right stick X (-511 to 512)
    int axisRY;   // Right stick Y (-511 to 512)
};

ControllerData controllerState;  // Holds received controller data

// Callback function when data is received
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
    // Try to interpret incoming data as an integer
    int receivedValue;
    memcpy(&receivedValue, incomingData, sizeof(receivedValue));

    // Check if the data is coming from the temperature sensor
    if (memcmp(mac, tempSensorAddress, 6) == 0) {  
        tempData = receivedValue;  // Store temperature data
    } else {
        memcpy(&controllerState, incomingData, sizeof(controllerState));  // Handle other control data
    }
}

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  char macStr[18];
//  Serial.print("Packet to: ");
  // Copies the sender mac address to a string
//  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
//           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
//  Serial.print(macStr);
//  Serial.print(" send status:\t");
//  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}


// Function to register a peer
void addPeer(uint8_t *peerAddress) {
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, peerAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  servoLeft.setPeriodHertz(50);   // Standard 50 Hz
  servoRight.setPeriodHertz(50);

  servoLeft.attach(servo1, 500, 2400);   // Min/max pulse width (tweak if needed)
  servoRight.attach(servo2, 500, 2400);


  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  // Register all peers correctly
  addPeer(neoAddress);
  addPeer(gripperAddress);
  addPeer(tempSensorAddress);
  addPeer(payloadReleaseAddress);
  addPeer(thrustersAddress);

  Serial.println("ESP-NOW Setup Complete.");
}

void sendData(uint8_t *address, int command) {
  esp_err_t result = esp_now_send(address, (uint8_t *) &command, sizeof(command));
  if (result == ESP_OK) {
//    Serial.println("Sent data successfully");
    // Serial.println(controllerState.buttons);
  } else {
//    Serial.println("Failed to send data");
  }
}

void loop() {

// Thruster Control (incorporate into "else if" statement for trolling motors with power/angle variables)
//  if (controllerState.axisLX < 0 || controllerState.axisLX > 10){
//    sendData(thrustersAddress, controllerState.axisLX);
//    delay(10);
//  }

  if (controllerState.axisLX > 0 && controllerState.axisLX < 10) {
    sendData(thrustersAddress, 4);
    servoLeft.write(90);
    servoRight.write(90);
  }

  // Net Controls: (Right trigger to drop/ Left trigger to lift)
  else if (controllerState.buttons == 128) {
    sendData(neoAddress, 1);
  } 
  else if (controllerState.buttons == 64){
    sendData(neoAddress, -1);
  }

  // Gripper Controls: Circle to drop sensor
  else if (controllerState.buttons == 2) {
    Serial.println(controllerState.buttons);
    sendData(gripperAddress, 2);
    delay(100);
  } 

  // Drone Controls: Square to release
  else if (controllerState.buttons == 4){
    sendData(gripperAddress, 5);
  }

  // Drone Payload Release Controls: Triangle to drop football
  else if (controllerState.buttons == 8) {
    sendData(payloadReleaseAddress, 4);
  } 

  // Temperature Sensor Controls: LB to start sending / RB to stop sending
  else if (controllerState.buttons == 16) {  // LB Button to start
      sendData(tempSensorAddress, 3);
      sendTemperature = true;  // Enable temperature reporting
  }

  else if (controllerState.buttons == 32) {  // RB Button to stop
      Serial.println("Temperature transmission halted");
      sendData(tempSensorAddress, -3);
      sendTemperature = false;  // Disable temperature reporting
  }

  // Trolling Motor Control:
  // controllerState.axisLX: [-508, 512]
  // controllerState.axisLY: [-508, 512]
  // power: [-127, 127];
  // angle: [-1,1]
  else if (controllerState.axisLY < 0 || controllerState.axisLY > 10 || controllerState.axisLX < 0 || controllerState.axisLX > 10) {
    int rawPower = map(controllerState.axisLY, -508, 512, -127, 127);
    float rawAngle = map(controllerState.axisLX, -508, 512, 0, 180);

  // Apply smoothing filter
  smoothedPower = (1 - smoothingFactor) * smoothedPower + smoothingFactor * rawPower;
  smoothedAngle = (1 - smoothingFactor) * smoothedAngle + smoothingFactor * rawAngle;

  int servoAngle = map(smoothedAngle, -1, 1, 0, 180);

  // Print smoothed values
  Serial.print((int)smoothedPower);
  Serial.print(" ");
  Serial.println(smoothedAngle);

  // Move both servos (or adjust individually if needed)
  servoLeft.write(smoothedAngle);
  servoRight.write(smoothedAngle);

  if (controllerState.axisLX < 0 || controllerState.axisLX > 10){
    sendData(thrustersAddress, controllerState.axisLX);
    delay(10);
  }
  }

  // Dead Zone
  else if ((controllerState.axisLY > 0 && controllerState.axisLY > 10) || (controllerState.axisLX < 0 && controllerState.axisLX > 10)) {
    if (controllerState.axisLY > 0 && controllerState.axisLY > 10) {
      Serial.println("0");
    }
    if (controllerState.axisLX < 0 && controllerState.axisLX > 10) {
      sendData(thrustersAddress, 4);
      servoLeft.write(90);
      servoRight.write(90);
      Serial.println("neutral");
    }
  }

  // If flag is true, print temperature
  if (sendTemperature) {
      Serial.print("Celsius temperature: ");
      Serial.println(tempData);
      delay(1001);
  }

  else if (controllerState.buttons == 0) {
    // Serial.println("no selection");
  }
  delay(50);

}

  }
  Serial.read();
  delay(500);
}
