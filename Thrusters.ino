#include <ESP32Servo.h>

Servo motorA;
Servo motorB;

const int neutralPWM = 1500;      
const int fullForwardPWM = 2000;  
const int fullReversePWM = 1000;  
const int rampStep = 10;  // Increase/decrease speed in steps
const int rampDelay = 30; // Time delay between steps

int currentSpeedA = neutralPWM;
int currentSpeedB = neutralPWM;
int targetSpeedA = neutralPWM;
int targetSpeedB = neutralPWM;
char command = 's';  // Default to stop

void setup() {
    Serial.begin(115200);
    motorA.attach(18);  
    motorB.attach(17);  

    Serial.println("Initializing ESCs...");
    motorA.writeMicroseconds(neutralPWM);
    motorB.writeMicroseconds(neutralPWM);
    delay(3000);  

    Serial.println("Ready to receive commands.");
}

void loop() {
    if (Serial.available() > 0) {
        command = Serial.read();
        Serial.flush();
        
        // Set target speeds based on command
        if (command == 'd') {  
            Serial.println("Turning Right");
            targetSpeedA = fullForwardPWM;
            targetSpeedB = fullReversePWM;
        } 
        else if (command == 'a') {  
            Serial.println("Turning Left");
            targetSpeedA = fullReversePWM;
            targetSpeedB = fullForwardPWM;
        } 
        else if (command == 's') {  
            Serial.println("Stopping");
            targetSpeedA = neutralPWM;
            targetSpeedB = neutralPWM;
        }
    }

    // Gradually adjust speed for both motors **at the same time**
    if (currentSpeedA != targetSpeedA || currentSpeedB != targetSpeedB) {
        if (currentSpeedA < targetSpeedA) currentSpeedA += rampStep;
        else if (currentSpeedA > targetSpeedA) currentSpeedA -= rampStep;

        if (currentSpeedB < targetSpeedB) currentSpeedB += rampStep;
        else if (currentSpeedB > targetSpeedB) currentSpeedB -= rampStep;

        motorA.writeMicroseconds(currentSpeedA);
        motorB.writeMicroseconds(currentSpeedB);
    }

    delay(rampDelay);  // Small delay for smoother updates
}
