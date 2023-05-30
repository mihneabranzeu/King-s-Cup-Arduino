#include <esp_now.h>
#include <WiFi.h>

#define THUMB_MASTER_BUTTON_PIN 18
#define GENERAL_PURPOSE_BUTTON_PIN 19
#define FORCE_SENSOR_PIN 36
#define LED_PIN 22
#define LED2_PIN 17
#define BUZZER_PIN 23

#define IDLE_STATE 0
#define SHOULD_DRINK_STATE 1
#define SHOULD_SAY_STATE 2
#define THUMB_MASTER_STATE 3
#define WATERFALL_STATE 4

#define MESSAGE_DRINK 0
#define MESSAGE_CONFIRM 1
#define MESSAGE_SCREAM 2
#define MESSAGE_SAY 3
#define MESSAGE_GO_NEXT 4
#define MESSAGE_THUMB_MASTER 5
#define MESSAGE_WATERFALL_GO 6
#define MESSAGE_WATERFALL_STOP 7
#define MESSAGE_WATERFALL_GO_INITIATOR 8


#define GLASS_NOT_LIFTED 0
#define GLASS_LIFTED 1
#define GLASS_BACK 2

typedef struct message_t {
  int sender;
  int receiver;
  int messageType;
} message_t;

// MAC Address of the master ESP
uint8_t broadcastAddress[] = {0xE8, 0x31, 0xCD, 0x14, 0x21, 0x1C};

// The player ID
int myId = -1;

// Peer info structure
esp_now_peer_info_t peerInfo;

// The drinking flag
int currentState;

// The state of the buzzer
bool buzzerOn = false;

// The state of the glass
int glassState;

// Flag for the waterfall challenge
bool canPutBack;
bool isInitiator;

const int movingAvgLength = 10;
int movingAvgBuffer[movingAvgLength];
int movingAvgIndex = 0;

// Callback function executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  message_t message;

  memcpy(&message, incomingData, sizeof(message_t));
  Serial.print("Data received: ");
  Serial.println(len);
  Serial.print("Sender: ");
  Serial.println(message.sender);
  Serial.print("Message type: ");
  Serial.println(message.messageType == 0 ? "DRINK" : "CONFIRM");

  if (message.messageType == MESSAGE_DRINK) {
    currentState = SHOULD_DRINK_STATE;
    glassState = GLASS_NOT_LIFTED;
  }

  if (message.messageType == MESSAGE_SCREAM) {
    buzzerOn = true;
  }

  if (message.messageType == MESSAGE_SAY) {
    currentState = SHOULD_SAY_STATE;
  }

  if (message.messageType == MESSAGE_THUMB_MASTER) {
    currentState = THUMB_MASTER_STATE;
  }

  if (message.messageType == MESSAGE_WATERFALL_GO) {
    currentState = WATERFALL_STATE;
    glassState = GLASS_NOT_LIFTED;
    canPutBack = false;
    isInitiator = false;
  }

  if (message.messageType == MESSAGE_WATERFALL_GO_INITIATOR) {
    currentState = WATERFALL_STATE;
    glassState = GLASS_NOT_LIFTED;
    canPutBack = false;
    isInitiator = true;
  }

  if (message.messageType == MESSAGE_WATERFALL_STOP) {
    canPutBack = true;
  }

   // Check if it's the first message and update ID
  if (myId == -1) {
    myId = message.receiver;
  }

  
}

// Callback when data is sent
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

int calculateMovingAverage(int sensorValue) {
  movingAvgBuffer[movingAvgIndex] = sensorValue;
  movingAvgIndex = (movingAvgIndex + 1) % movingAvgLength;

  int sum = 0;
  for (int i = 0; i < movingAvgLength; i++) {
    sum += movingAvgBuffer[i];
  }

  return sum / movingAvgLength;
}



void setup() {
  // Initialize serial communication at 9600 bits per second:
  Serial.begin(9600);

  // Initialize ESP NOW stuff
  // Set ESP32 as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
 
  // Initilize ESP-NOW stuff
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Register callback function
  esp_now_register_recv_cb(OnDataRecv);
  esp_now_register_send_cb(OnDataSent);

  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }


  // Initialize the thumb master button
  pinMode(THUMB_MASTER_BUTTON_PIN, INPUT_PULLUP);
  pinMode(GENERAL_PURPOSE_BUTTON_PIN, INPUT_PULLUP);
  // Initialize the led
  pinMode(LED_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  // Initialize the buzzer
  pinMode(BUZZER_PIN, OUTPUT);

  currentState = IDLE_STATE;
}

void loop() {
  if (currentState == SHOULD_DRINK_STATE) {
    // Turn on the LED
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(LED2_PIN, LOW);

    if (buzzerOn) {
      digitalWrite(BUZZER_PIN, HIGH);
    }

    // Read the FSR value
    int forceSensorValue = calculateMovingAverage(analogRead(FORCE_SENSOR_PIN));

    
    if ((forceSensorValue <5) && glassState == GLASS_NOT_LIFTED) {
      glassState = GLASS_LIFTED;
    }

    if ((forceSensorValue > 100) && glassState == GLASS_LIFTED) {
      glassState = GLASS_BACK;
    }

    if (glassState == GLASS_BACK) {
      // Send the confirmation message
      // Create the message
      message_t message = {myId, 3, MESSAGE_CONFIRM};
  
      // Send the message
      esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &message, sizeof(message_t));

      // Check the result
      if (result == ESP_OK) {
        Serial.println("Sent with success");
        currentState = IDLE_STATE;
        buzzerOn = false;
      }
      else {
        Serial.println("Error sending the data");
      }
    }
  }

  if (currentState == SHOULD_SAY_STATE) {
    // Turn on the LED
    digitalWrite(LED2_PIN, HIGH);
    
    // Wait for the button press
    int generalPurposeButtonState = digitalRead(GENERAL_PURPOSE_BUTTON_PIN);

    if (!generalPurposeButtonState) {
      // Send the message to master to go to the next player
      message_t message = {myId, 3, MESSAGE_GO_NEXT};
      esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &message, sizeof(message_t));
      if (result == ESP_OK) {
        Serial.println("Sent with success");
        currentState = IDLE_STATE;
        buzzerOn = false;
      }
      else {
        Serial.println("Error sending the data");
      }
      digitalWrite(LED2_PIN, LOW);
    }
  }

  if (currentState == THUMB_MASTER_STATE) {
    int thumbMasterButtonState = digitalRead(THUMB_MASTER_BUTTON_PIN);
    if (!thumbMasterButtonState) {
      message_t message = {myId, 3, MESSAGE_THUMB_MASTER};
      esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &message, sizeof(message_t));
      if (result == ESP_OK) {
        Serial.println("Sent with success");
        currentState = IDLE_STATE;
        buzzerOn = false;
      }
      else {
        Serial.println("Error sending the data");
      }
    }
  }

  if (currentState == WATERFALL_STATE) {
    int forceSensorValue = calculateMovingAverage(analogRead(FORCE_SENSOR_PIN));
    if (forceSensorValue <5 && (glassState == GLASS_NOT_LIFTED || glassState == GLASS_BACK)) {
      glassState = GLASS_LIFTED;
    }

    if (forceSensorValue > 5 && glassState == GLASS_LIFTED) {
      glassState = GLASS_BACK;
      if (isInitiator) {
        canPutBack = true;
      }
    }

    if ((glassState == GLASS_NOT_LIFTED || glassState == GLASS_BACK) && !canPutBack) {
      // Turn on the alert
      digitalWrite(LED_PIN, HIGH);
      digitalWrite(BUZZER_PIN, HIGH);
    }

    if (glassState == GLASS_LIFTED && !canPutBack) {
      digitalWrite(LED_PIN, LOW);
      digitalWrite(BUZZER_PIN, LOW);
    }

    if (glassState == GLASS_BACK && canPutBack) {
      message_t message = {myId, 3, MESSAGE_WATERFALL_STOP};
      esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &message, sizeof(message_t));
      if (result == ESP_OK) {
        Serial.println("Sent with success");
        currentState = IDLE_STATE;
        buzzerOn = false;
      }
      else {
        Serial.println("Error sending the data");
      }
    }
  }

  if (currentState == IDLE_STATE) {
    // Turn off the LED
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
  }
}
