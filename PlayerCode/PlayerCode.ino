#define THUMB_MASTER_BUTTON_PIN 18
#define GENERAL_PURPOSE_BUTTON_PIN 19
#define FORCE_SENSOR_PIN 36
#define LED_PIN 22
#define BUZZER_PIN 23



void setup() {
  // Initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  // Initialize the thumb master button
  pinMode(THUMB_MASTER_BUTTON_PIN, INPUT_PULLUP);
  pinMode(GENERAL_PURPOSE_BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);





}

void loop() {
  // BUTTON CODE
  // read the state of the switch/button:
  int thumbMasterButtonState = digitalRead(THUMB_MASTER_BUTTON_PIN);
  int generalPurposeButtonState = digitalRead(GENERAL_PURPOSE_BUTTON_PIN);


  // print out the button's state
  Serial.print("Thumb Master Button State: ");
  Serial.print(thumbMasterButtonState);
  Serial.print(" ; General Purpose Button State: ");
  Serial.println(generalPurposeButtonState);
  //-----------------------------------------

  // FORCE SENSOR CODE
  int analogReading = analogRead(FORCE_SENSOR_PIN);

  Serial.print("The force sensor value = ");
  Serial.print(analogReading); // print the raw analog reading

  if (analogReading < 10)       // from 0 to 9
    Serial.println(" -> no pressure");
  else if (analogReading < 200) // from 10 to 199
    Serial.println(" -> light touch");
  else if (analogReading < 500) // from 200 to 499
    Serial.println(" -> light squeeze");
  else if (analogReading < 800) // from 500 to 799
    Serial.println(" -> medium squeeze");
  else // from 800 to 1023
    Serial.println(" -> big squeeze");


  // LED CODE
  digitalWrite(LED_PIN, HIGH);

  // BUZZER CODE
  // digitalWrite (BUZZER_PIN, HIGH);

  delay(1000);

  digitalWrite(LED_PIN, LOW);
  // digitalWrite (BUZZER_PIN, LOW);

  delay(1000);




}



