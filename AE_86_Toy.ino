// AE-86 toy code by Daniel Chu
//
//Button Usage:
//	Press: Play next track
//	Hold: Switch play mode (ad card/song)
//	Double Press: Stop current track

#include "Arduino.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"

//pin remap
//11 => 4
//10 => 3

#define BUTTONPIN 2  
#define SONGCOUNT 2
#define ADCOUNT 21
#define FOLDERCOUNT 2


//DFplayer vals
SoftwareSerial mySoftwareSerial(3,4);//remapped RX, TX
DFRobotDFPlayerMini myDFPlayer;
int currentTrack = 1;
int currentMax = ADCOUNT;
int currentFolder = 0;

void setup() {
  //button
  delay(700);
  pinMode(BUTTONPIN, INPUT);
  mySoftwareSerial.begin(9600);


  if (!myDFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
    while(true);
  }
  myDFPlayer.volume(20);  //Set volume value. From 0 to 30
}

void loop() {
  int b = checkButton();
   if (b == 1) clickEvent();
   if (b == 2) doubleClickEvent();
   if (b == 3) holdEvent();
   if (b == 4) longHoldEvent();
}

//=================================================
// Events to trigger

void clickEvent() {
   myDFPlayer.playFolder(currentFolder + 1, currentTrack);
   currentTrack =  ((currentTrack + 1) % currentMax);
}

void doubleClickEvent() {
   myDFPlayer.pause();
}

void holdEvent() {
   myDFPlayer.playFolder(3, 1);
   currentFolder = ((currentFolder + 1) % FOLDERCOUNT);
   
   if (currentFolder == 1){
    currentMax = ADCOUNT;
    }
   if (currentFolder == 2){
    currentMax = SONGCOUNT;
    }
    currentTrack = 1;
}
void longHoldEvent() {
//   Serial.println("Long Hold!");
}

//=================================================
//  MULTI-CLICK:  One Button, Multiple Events

// Button timing variables
int debounce = 20;          // ms debounce period to prevent flickering when pressing or releasing the button
int DCgap = 250;            // max ms between clicks for a double click event
int holdTime = 1500;        // ms hold period: how long to wait for press+hold event
int longHoldTime = 3000;    // ms long hold period: how long to wait for press+hold event

// Button variables
boolean buttonVal = HIGH;   // value read from button
boolean buttonLast = HIGH;  // buffered value of the button's previous state
boolean DCwaiting = false;  // whether we're waiting for a double click (down)
boolean DConUp = false;     // whether to register a double click on next release, or whether to wait and click
boolean singleOK = true;    // whether it's OK to do a single click
long downTime = -1;         // time the button was pressed down
long upTime = -1;           // time the button was released
boolean ignoreUp = false;   // whether to ignore the button release because the click+hold was triggered
boolean waitForUp = false;        // when held, whether to wait for the up event
boolean holdEventPast = false;    // whether or not the hold event happened already
boolean longHoldEventPast = false;// whether or not the long hold event happened already

int checkButton() {    
   int event = 0;
   buttonVal = digitalRead(BUTTONPIN);
   // Button pressed down
   if (buttonVal == HIGH && buttonLast == LOW && (millis() - upTime) > debounce)
   {
       downTime = millis();
       ignoreUp = false;
       waitForUp = false;
       singleOK = true;
       holdEventPast = false;
       longHoldEventPast = false;
       if ((millis()-upTime) < DCgap && DConUp == false && DCwaiting == true)  DConUp = true;
       else  DConUp = false;
       DCwaiting = false;
   }
   // Button released
   else if (buttonVal == LOW && buttonLast == HIGH && (millis() - downTime) > debounce)
   {        
       if (not ignoreUp)
       {
           upTime = millis();
           if (DConUp == false) DCwaiting = true;
           else
           {
               event = 2;
               DConUp = false;
               DCwaiting = false;
               singleOK = false;
           }
       }
   }
   // Test for normal click event: DCgap expired
   if ( buttonVal == LOW && (millis()-upTime) >= DCgap && DCwaiting == true && DConUp == false && singleOK == true && event != 2)
   {
       event = 1;
       DCwaiting = false;
   }
   // Test for hold
   if (buttonVal == HIGH && (millis() - downTime) >= holdTime) {
       // Trigger "normal" hold
       if (not holdEventPast)
       {
           event = 3;
           waitForUp = true;
           ignoreUp = true;
           DConUp = false;
           DCwaiting = false;
           //downTime = millis();
           holdEventPast = true;
       }
       // Trigger "long" hold
       if ((millis() - downTime) >= longHoldTime)
       {
           if (not longHoldEventPast)
           {
               event = 4;
               longHoldEventPast = true;
           }
       }
   }
   buttonLast = buttonVal;
   return event;
}