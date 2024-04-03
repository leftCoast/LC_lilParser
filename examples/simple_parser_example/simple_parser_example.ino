/************************************************************************************
*************************************************************************************
                          10/15/2020 Damn pandemic still going on.
                                    
This is a really simple test/demo program for the Left Coast lilParser.
In the serial monitor..
      
      Type : on    		to turn on the LED.
      Type : off   		to turn it off again.
      Type : on NUMBER	to turn it on for that many seconds.
      
In order to run this you will need, along with this library : LC_baseTools.
LC_baseTools can also be found in in the Arduino library manager.

*************************************************************************************
*************************************************************************************/

#include <lilParser.h>
#include <timeObj.h>


#define LED_PIN 13 // Pin for built in LED.


// Our list of commands.
enum commands {
	noCommand,		// ALWAYS start with noCommand. Or something simlar.
	LEDOn,			// The rest is up to you. help would be a good one. Have it list
	LEDOff,			// What the other commands are, how they work and what they do.
};						

lilParser   ourParser;        	// The parser object.
timeObj		LEDTimer(10,false);	// The timer to shut off the LED when timed.	
	
void setup() {
   
   Serial.begin(57600);
   pinMode(LED_PIN, OUTPUT);        // Set up the LED pin.
   ourParser.addCmd(LEDOn,"on");    // Type on to turn on the LED.
   ourParser.addCmd(LEDOff,"off");  // Type off to turn it off again.
   Serial.println();
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
         case LEDOn     : handleLEDOn(); 					break;	// Call a function for this one..
         case LEDOff    : digitalWrite(LED_PIN, LOW);	break;	// Turn the LED off by making the voltage LOW
         default        : Serial.println("What?");		break;	// No idea. Try again?
      }																			//
   }																				// Done dealing with incoming chars.
   if (LEDTimer.ding()) {													// If the timer has expired..
   	digitalWrite(LED_PIN, LOW);										// Shut off the LED
   	LEDTimer.reset();														// Reset the timer.
   }
}


// When we get the LED on message lets see it it's on forever or for a specified time. If
// a time is specified, set the timer for that amount of time. And, of course turn on the
// LED.
void handleLEDOn(void) {

	float seconds;
	
	if (ourParser.numParams()) {							// If there are any params after the command..
		seconds = atof(ourParser.getNextParam());		// Get the first param and interpret it as a float.
		LEDTimer.setTime(seconds * 1000);				// Set our timer to the required milliseconds.
	}																//
	digitalWrite(LED_PIN, HIGH);							// Either way, turn the LED on.
}

		
