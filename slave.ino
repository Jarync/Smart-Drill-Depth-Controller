#include <ESP8266WiFi.h>
#include <espnow.h>

const int relayPin = 12;  // Relay control pin (GPIO12)
const int buttonPin = 0;  // Reset button pin (GPIO0)

void setup() {
  Serial.begin(9600);
  pinMode(relayPin, OUTPUT); // Set relay pin as output
  pinMode(buttonPin, INPUT_PULLUP); // Set button pin as input with internal pull-up
  digitalWrite(relayPin, HIGH);  // Keep the socket powered ON initially
  
  WiFi.mode(WIFI_STA); // Set WiFi to Station mode
  WiFi.disconnect();   // Disconnect any existing WiFi connections
  
  // Initialize ESP-NOW
  if (esp_now_init() != 0) 
  {
    Serial.println("ESP-NOW Initialization Failed");
    return;
  }

  // Set this device as Slave role
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  
  // Register callback function for receiving data
  esp_now_register_recv_cb(onDataReceived);
}

void loop() 
{
  // Check if the manual reset button is pressed
  if (digitalRead(buttonPin) == LOW) 
  {
    Serial.println("Button pressed, resetting the device.");
    delay(1000); // Debounce delay (1 second) to prevent accidental multiple triggers
    ESP.restart(); // Restart the ESP8266
  }
}

// Callback function executed when data is received
void onDataReceived(uint8_t *mac, uint8_t *incomingData, uint8_t len)
{
  // Check if the received data code is 2 (Shutdown Signal)
  if (incomingData[0] == 2) 
  { 
    Serial.println("Received shutdown signal, turn off the plug.");
    digitalWrite(relayPin, LOW); // Cut off relay to turn off power
  }
}