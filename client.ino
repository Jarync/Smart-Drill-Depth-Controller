#include <ESP8266WiFi.h>
#include <espnow.h>

// Define variables
String receiveData = "";    // String variable to buffer received Serial data
String nanoSignal = "sent"; // Signal flag to identify commands sent from Arduino Nano

unsigned long previousMillis = 0; // Timer variable for non-blocking delays (do not use delay in loop)

// Define Slave MAC Address (Receiver ESP8266)
uint8_t slaveMac[] = {0x08, 0xF9, 0xE0, 0x6B, 0x88, 0x40}; 

bool connectionStatus = false; // Flag to store connection status

void setup() {
  Serial.begin(9600); 
  WiFi.mode(WIFI_STA); // Set WiFi to Station mode
  WiFi.disconnect();   // Disconnect any existing WiFi connections

  // Initialize ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("ESP-NOW Initialization Failed");
    return;
  }

  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER); // Set this ESP8266 as the Controller (Client)

  esp_now_add_peer(slaveMac, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);   // Add the Slave peer

  esp_now_register_send_cb(onDataSent); // Register callback for transmission status

  // Send initial data to slave to check connection status
  uint8_t data = 1;
  esp_now_send(slaveMac, &data, sizeof(data));
}

void loop() {

  // Check if Serial data is available from Nano/UNO
  while (Serial.available() > 0) 
  {
    char receivedChar = char(Serial.read()); // Read incoming character
    receiveData += receivedChar; // Append to buffer

      // Check buffer length
    if (receiveData.length() >= nanoSignal.length()) {
      // Check if the buffer ends with the specific signal string
      if (receiveData.endsWith(nanoSignal)) {
        uint8_t data = 2; // Define shutdown signal code (2)
        esp_now_send(slaveMac, &data, sizeof(data)); // Send shutdown signal to Slave
      }
      receiveData = ""; // Clear buffer to prevent overflow
    }
  }
  connect(); // Periodically check connection status
}

// Callback function executed when data is sent
void onDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  // sendStatus 0 means success
  if (sendStatus == 0) {
    connectionStatus = true;
  } else {
    connectionStatus = false;
  }
}

// Function to handle WiFi connection status reporting
void connect()
{
  const long interval = 2000; // Check interval (2000ms)
  unsigned long currentMillis = millis(); // Get current runtime

  // Non-blocking delay logic
  if(currentMillis - previousMillis >= interval )
  {
    previousMillis = currentMillis;
    if (connectionStatus) {
      Serial.println(F("WiFi")); // Send status back to Nano via Serial
    } else {
      Serial.println(F("Disc")); // Disconnected
    }
  }
}