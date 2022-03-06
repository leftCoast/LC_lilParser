/************************************************************************************
*************************************************************************************
                          10/15/2020 Damn pandemic still going on.
                                    
This is a really simple test/demo program for the Left Coast lilParser.
In the serial monitor..
      
      Type : on    to turn on the LED.
      Type : off   to turn it off again.
      
In order to run this you will need, along with this library : LC_baseTools
LC_baseTools can be found in in the Arduino library manager.

*************************************************************************************
*************************************************************************************/

#include "lilParser.h"

#define LED_PIN 13 // Pin for built in LED.

 
enum commands {   noCommand,  // ALWAYS start with noCommand. Or something simlar.
                  LEDOn,      // The rest is up to you. help would be a good one. Have it list
                  LEDOff,     // What the other commands are, how they work and what they do.
                  };          // Our list of commands.

lilParser   ourParser;        // The parser object.

void setup() {
   
   Serial.begin(9600);
   pinMode(LED_PIN, OUTPUT);        // Set up the LED pin.
   ourParser.addCmd(LEDOn,"on");    // Type on to turn on the LED.
   ourParser.addCmd(LEDOff,"off");  // Type off to turn it off again.
   Serial.println("Type on or off to control the LED.");
}


// Your loop where it parses out all your typings.
void loop(void) {

   char  inChar;
   int   command;
   
   if (Serial.available()) {                                   // If serial has some data..
      inChar = Serial.read();                                  // Read out a charactor.
      Serial.print(inChar);                                    // If using development machine, echo the charactor.
      command = ourParser.addChar(inChar);                     // Try parsing what we have.
      switch (command) {                                       // Check the results.
         case noCommand : break;                               // Nothing to report, move along.
         case LEDOn     : digitalWrite(LED_PIN, HIGH); break;  // Turn the LED on (HIGH is the voltage level)
         case LEDOff    : digitalWrite(LED_PIN, LOW);  break;  // Turn the LED off by making the voltage LOW
         default        : Serial.println("What?"); break;      // No idea. Try again?
      }
   }
}
