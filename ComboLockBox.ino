/*

Hardware consist of 
  - 4 momentary buttons with pull down resistors (Brown-Black-Orange Gold, 10k ohm 5% tolerance)
  - 1 Potentiometer 
  - 4 Red LEDs w/ resistors (Yellow-Violet-Brown Gold, 470 ohm 5% tolerance) 
  - 1 Green LED w/ resistor (same as above) 
  - 1 Yellow LED w/ resistor (same as above) 

  Buttons provide 8-bit binary value input 
  Potentiometer allows for selecting between:
    - Far counter clockwise = Set code 
    - Left (counter clockwise) of center = buttons set low bits (1 2 4 8) 
        and green light on (pin 11, aka LED__BITS_LOW_PORT) 
    - Right (clockwise) of center = buttons set high bits (16 32 64 128) 
        and yellow light on (pin 12, aka LED__BITS_HIGH_PORT)
    - Far clockwise = Check code 
        and red LEDs either flash all together (wrong code) or in sequence left to right (correct code)
        
*/

#define LED__BITS_LOW_PORT 11
#define LED__BITS_HIGH_PORT 12

#define LED__BANK_BASE_PORT 7
#define LED__BANK_COUNT 4

#define BUTTON__BASE_PORT 2
#define BUTTON__COUNT 4

#define ANALOG_PORT 5
#define ANALOG_THRESHOLD 512

int anaVal; // analog port value
int baseButton = 0;
int prevBaseButton = 0;

int currentButtons[8];
int storedButtons[8];
int buttonsChanged = false;

int combo = 42;
int bDoComboCheck = false;
int bDoComboSet = false;
int bLocked = true;

int idx; // shared loop counter
int waitTime;


void setup() {
  
  // Setup Debug output
  Serial.begin(9600);
  
  // Setup LEDs
  pinMode(LED__BITS_LOW_PORT, OUTPUT);
  pinMode(LED__BITS_HIGH_PORT, OUTPUT);

  for (idx=LED__BANK_BASE_PORT; idx<(LED__BANK_BASE_PORT+LED__BANK_COUNT); idx++)
    pinMode(idx, OUTPUT);

  // Setup Buttons
  for (idx=BUTTON__BASE_PORT; idx<(BUTTON__BASE_PORT+BUTTON__COUNT); idx++)
    pinMode(idx, INPUT);

  clearButtons();  

} // setup

void loop() {

  setBaseButton();
  sampleButtons();
  
  if (isButtonChangeDetected()) {
    updateBankLEDs();
    buttonsChanged = false;
    
  } // if buttonsChanged 
  
  // wait half second before looping
  delay(waitTime);

  // if requested, do combo check
  if (bDoComboCheck) 
    doComboCheck();
    
  if (bDoComboSet)
    doComboSet();

} // loop

void clearButtons() {
  
    // Setup button status storage
  for (idx=0; idx<8; idx++) {
    currentButtons[idx] = false;
    storedButtons[idx] = false;
  }

} // clearButtons

int setBaseButton() {
   // read analog dial and based on val, set base button
   
   anaVal = analogRead(ANALOG_PORT); 
   
   // set base low for above threshold, high for below
   // Note: this seems 'reversed' due to the layout on the test board.
   baseButton = (anaVal > ANALOG_THRESHOLD) ? 0 : 4;
   
   // set waitTime to zero if anaVal is greater than 1000 or below 23
   if (anaVal < 23) 
     bDoComboSet = true;
   
   if ((anaVal > 1000) && (bLocked))
     bDoComboCheck = true;
   
   if ((anaVal > 1000) || (anaVal < 23))      
     waitTime = 0;
   else {
     waitTime = 500;
     bLocked = true;
   }
   
   // Set Hi/Low LED
   if (baseButton == 0) {
     digitalWrite(LED__BITS_LOW_PORT, HIGH);
     digitalWrite(LED__BITS_HIGH_PORT, LOW);
     
   }
   else {
     digitalWrite(LED__BITS_LOW_PORT, LOW);
     digitalWrite(LED__BITS_HIGH_PORT, HIGH);
     
   }
   
   // See if LED bank should be updated
   if (prevBaseButton != baseButton) {
     updateBankLEDs();
     prevBaseButton = baseButton; 
   }

   return baseButton;

} // setBaseButton

void sampleButtons() {
   // read button bank (based on baseButton) into currentBtns[].
   
   // UNO only has room for 4 buttons
   for (idx=0; idx<BUTTON__COUNT; idx++)
     currentButtons[baseButton+idx] = digitalRead(BUTTON__BASE_PORT + idx);

}

int isButtonChangeDetected() {
  // if currentButtons differ from prevButtons, return true
  // update prevButtons to equal current if change detected.
  
  int bChanged = false;
  
  for (idx=0; idx<8; idx++) {
    if (currentButtons[idx] == HIGH) {
      // button is pressed
      storedButtons[idx] = (storedButtons[idx] == LOW) ? HIGH : LOW;
      bChanged = true;
    }
        
  } // for idx

  return bChanged;
  
} // isButtonChangeDetected

void updateBankLEDs() {
   // update the display LEDs based on button presses

  for(idx=0; idx<LED__BANK_COUNT; idx++) {
    digitalWrite(idx+LED__BANK_BASE_PORT, storedButtons[baseButton+idx]);
  }

  // Update to debug console for now
  for(idx=7; idx>=0; idx--)
    Serial.print(storedButtons[idx]);
  Serial.println();
  
}

int calcCodeTotal() {

  int twoPow[8] = {1, 2, 4, 8, 16, 32, 64, 128}; 
  
  int total = 0;
  for (idx=0; idx<8; idx++)
    total = total + twoPow[idx] * storedButtons[idx];

  return total;
  
}

void doComboCheck() {

  int total = calcCodeTotal();

  if (total == combo)
    doUnlock();
  else
    doBadCombo();

  Serial.println();
  Serial.println(total);
  Serial.println();

  bDoComboCheck = false;
  
}

void doComboSet() {

  int total = calcCodeTotal();

  // prevent setting combo to zero.
  // prevents turn full left then full right to unlock.
  if (total == 0) {
    doBadCombo();
    bDoComboSet = false;
    return;
  }

  combo = total;
  clearButtons();
  updateBankLEDs();
 
  int x;
  for (x=0; x<3; x++) {
    
    for(idx=(LED__BANK_BASE_PORT+LED__BANK_COUNT-1); idx>=LED__BANK_BASE_PORT; idx--) {
      delay(125);
      digitalWrite(idx, HIGH);
    }
    delay(250);
  
    for(idx=(LED__BANK_BASE_PORT+LED__BANK_COUNT-1); idx>=LED__BANK_BASE_PORT; idx--) {
      delay(125);
      digitalWrite(idx, LOW);
    }
  
  } // for x
  
  bDoComboSet = false;

}

void doUnlock() {
 
  // For now, just animate the lights
  clearButtons();
  updateBankLEDs();
  
  int x;
  for (x=0; x<5; x++) {
    
    for(idx=LED__BANK_BASE_PORT; idx<(LED__BANK_BASE_PORT+LED__BANK_COUNT); idx++) {
      delay(125);
      digitalWrite(idx, HIGH);
    }
    delay(250);
  
    for(idx=LED__BANK_BASE_PORT; idx<(LED__BANK_BASE_PORT+LED__BANK_COUNT); idx++) {
      delay(125);
      digitalWrite(idx, LOW);
    }
  
  } // for x
  
  bLocked = false;
  
}

void doBadCombo() {

  // For now, just animate the lights
  clearButtons();
  updateBankLEDs();
  
  int x;
  for (x=0; x<5; x++) {
    
    for(idx=LED__BANK_BASE_PORT; idx<(LED__BANK_BASE_PORT+LED__BANK_COUNT); idx++) {
      digitalWrite(idx, HIGH);
    }
    delay(250);
  
    for(idx=LED__BANK_BASE_PORT; idx<(LED__BANK_BASE_PORT+LED__BANK_COUNT); idx++) {
      digitalWrite(idx, LOW);
    }
    delay (250);
  
  } // for x
  
}

void debugMsg(int msg) {
 
   Serial.println(msg);
   
} // debugMsg
