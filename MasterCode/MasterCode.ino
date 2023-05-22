#include <LiquidCrystal_I2C.h>

#define BUTTON_1_PIN 33
#define BUTTON_2_PIN 25
#define BUTTON_3_PIN 26
#define BUTTON_NEXT_PIN 32

#define NEW_GAME_STATE 0
#define NEW_CARD_STATE 1
#define WAITING_FOR_DRINKS_STATE 2
#define GAME_OVER_STATE 3
#define IDLE_STATE 4

#define CARDS_PER_NUMBER 1

// The left amount of each card
int cardFrequencies[15];
int cardsLeft = (CARDS_PER_NUMBER) * 12;

// The current state of the game
int currentState;

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x3F for a 16 chars and 2 line display

void lcdPrintAnimation() {
  lcd.clear();
  int i, j;
  lcd.setCursor(0, 0);
  for (int i = 0; i < 15; i++) {
    for (int j = 0; j < i; j++) {
      lcd.setCursor(j, 0);
      lcd.print("=");
      lcd.setCursor(15 - j, 1);
      lcd.print("=");
    }
    delay(100);
  }
}

void lcdPrint(char *message, int row, int shouldClear) {
  if (shouldClear) {
    lcd.clear();
  }
  lcd.setCursor(2, row);
  lcd.print(message);
}

void twoChallenge() {
  lcdPrint("2 IS YOU", 0, 1);
  lcdPrint("Choose Someone", 1, 0);
}

void threeChallenge() {
  lcdPrint("3 IS ME", 0, 1);
  lcdPrint("Drink!!", 1, 0);
}

void fourChallenge() {
  lcdPrint("4 IS GIRLS", 0, 1);
  lcdPrint("Girls Drink", 1, 0);
  
}

void fiveChallenge() {
  lcdPrint("THUMB MASTER", 0, 1);
}

void sixChallenge() {
  lcdPrint("6 IS GUYS", 0, 1);
  lcdPrint("Boys Drink", 1, 0);
}

void eightChallenge() {
  lcdPrint("8 IS MATE", 0, 1);
  lcdPrint("Choose Someone", 1, 0);
}

void nineChallenge() {
  lcdPrint("9 IS RHYME", 0, 1);

}

void tenChallenge() {
  lcdPrint("10 IS CATEGORY", 0, 1);
}

void JChallenge() {
  lcdPrint("J IS RULE", 0, 1);
  lcdPrint("Choose a Rule", 1, 0);

}

void QChallenge() {
  lcdPrint("Q IS QUESTION", 0, 1);
}

void KChallenge() {
  lcdPrint("K IS EVERYONE", 0, 1);
  lcdPrint("Everyone Drinks", 1, 0);
}

void AChallenge() {
  lcdPrint("A IS WATERFALL", 0, 1);
}

void startRound(int card) {
  // Update the game state
  currentState = WAITING_FOR_DRINKS_STATE;

  // Check the value of the card and proceed accordingly
  switch (card) {
    case 2:
      twoChallenge();
      break;
    case 3:
      threeChallenge();
      break;
    case 4:
      fourChallenge();
      break;
    case 5:
      fiveChallenge();
      break;
    case 6:
      sixChallenge();
      break;
    case 8:
      eightChallenge();
      break;
    case 9:
      nineChallenge();
      break;
    case 10:
      tenChallenge();
      break;
    case 11:
      JChallenge();
      break;
    case 12:
      QChallenge();
      break;
    case 13:
      KChallenge();
      break;
    case 14:
      AChallenge();
      break;
  }
  
}

int canMoveToNext() {
  return 1;
}

void setup() {
  Serial.begin(9600);
  // LCD CODE
  lcd.init();
  lcd.clear();         
  lcd.backlight();      // Make sure backlight is on
  
  // Print a message on both lines of the LCD.
  lcd.setCursor(2,0);   //Set cursor to character 2 on line 0
  lcd.print("King's Cup!");
  
  lcd.setCursor(2,1);   //Move cursor to character 2 on line 1
  lcd.print("Press NEXT");
  //---------------------------

  //BUTTON SETUP
  pinMode(BUTTON_1_PIN, INPUT_PULLUP);
  pinMode(BUTTON_2_PIN, INPUT_PULLUP);
  pinMode(BUTTON_3_PIN, INPUT_PULLUP);
  pinMode(BUTTON_NEXT_PIN, INPUT_PULLUP);

  //----------------------------

  // Initialize the card frequencies
  for (int i = 2; i < 15; i++) {
    cardFrequencies[i] = CARDS_PER_NUMBER;
  }
  // 7 card won't be played
  cardFrequencies[7] = 0;

  // Initialize the current state of the game
  currentState = NEW_GAME_STATE;
  


}

void loop() {
  // //BUTTON CODE
  // int button1Value = digitalRead(BUTTON_1_PIN);
  // int button2Value = digitalRead(BUTTON_2_PIN);
  // int button3Value = digitalRead(BUTTON_3_PIN);
  // Serial.print("1: ");
  // Serial.print(button1Value);
  // Serial.print(" ; 2: ");
  // Serial.print(button2Value);
  // Serial.print(" ; 3: ");
  // Serial.println(button3Value);
  // //-------------------------

  // delay(1000);

  if (currentState == NEW_GAME_STATE) {
    // Check if the button was pressed
    int nextButtonValue = digitalRead(BUTTON_NEXT_PIN);
    if (!nextButtonValue) {
      // Switch into the new card state
      currentState = NEW_CARD_STATE;
    }
  }

  if (currentState == NEW_CARD_STATE) {
    // Check if game should be over
    if (cardsLeft == 0) {
      currentState = GAME_OVER_STATE;
    } else {
      lcdPrintAnimation();
      // Generate a new card
      int cardValue;
      while (1) {
        cardValue = random(2, 15);
        if (cardFrequencies[cardValue] > 0) {
          cardFrequencies[cardValue]--;
          cardsLeft--;
          break;
        }
      }
      Serial.print("Card Value: ");
      Serial.println(cardValue);

      // Start the new round depening on the card
      startRound(cardValue);
    }
  }

  if (currentState == WAITING_FOR_DRINKS_STATE) {
    // Check the next button state
    int nextButtonValue = digitalRead(BUTTON_NEXT_PIN);
    if (!nextButtonValue) {
      // Check if everybody is ready
      if (canMoveToNext()) {
        currentState = NEW_CARD_STATE;
      }
    }
  }

  if (currentState == GAME_OVER_STATE) {
    lcdPrint(" GAME OVER", 0, 1);
    currentState = IDLE_STATE;
  }




}