# LC_lilParser
A command line parser.

Need to control a program from a command line? This should take most of the drudgery out of it.

**Depends on**  
[LC_baseTools](https://github.com/leftCoast/LC_baseTools)

**1 : Create your parser.** 

```
lilParser ourParser; // Typically you create this as a global.
lilParser longLineParser(80); // Set the param buffer size if you need more than 20 bytes. Minimum would be 2.
```

**2 : Define your commands.** Set up an enum to list the commands you would like to use. Make sure the first command is noCommand or something similar. Why is this? Because, as the parser is running through inputted text, it returns 0 for "Not done parsing yet". It passes back -1 for "I can't parse this!" and all other numbers are valid commands. So to lessen confusion your first (0) command should be something like "noCommand" or "stillParsing". Your enum should looks something like this..
```
enum commands { noCommand, command1, command2 }; // This gives you two commands command1 & command2.
```
All these names you can change, it doesn't care what you call things.

**3 : Setting up the parser.** Typically this is done in your setup() function. This is where you link what your going to type to your command. For example..
```
ourParser.addCmd(command1,"reset"); // command1 will now be returned when the user types reset.
ourParser.addCmd(command2,"run");   // command2 will now be returned when the user types run.
ourParser.addCmd(command2,"y");   // command2 will now be returned when the user types y. (Synonyms are ok)
```
**4 : Run the parser.** Typically this is what goes on in your loop() function. Every time through loop(), check if there's a new character ready to read. If so, pass that into the parser and run its output through a switch statement. Here is a simple example..
```
void loop(void) {

   char  inChar;
   int   command;
   
   if (Serial.available()) {                                // If serial has some data..
      inChar = Serial.read();                               // Read out a charactor.
      Serial.print(inChar);                                 // *Optional* echo the charactor.
      command = ourParser.addChar(inChar);                  // Try parsing what we have.
      switch (command) {                                    // Check the results.
         case noCommand : break;                            // Nothing to report, move along.
         case command1   : handleCommand1();       break;   // Call the handler function for comman1
         case command2   : handleCommand2();       break;   // Call the handler function for comman1
         default        : Serial.println("What?"); break;   // No idea. Try again?
      }
   }
}
```
**5 : Handling commands.** Handlers are the functions that are called to execute the commands. They can be simple commands that are just triggers for actions. Or they can contain parameters to add more info. Here is an example of a command that creates a new folder on an SD drive. This is so you can see how to deal with a simple parameter string.
```
void makeDirectory(void) {

   char* param;                                    // A pointer for the folder name string.
   char  pathBuff[PATH_CHARS];                     // A buffer to hold the full path to the new folder.
   
   if (ourParser.numParams()) {                    // If they typed in somethng past the command.
      param = ourParser.getnextParam();            // We get the first parameter, assume its the new folder's name.
      strcpy(pathBuff,wd);                         // Start building up the full path. Starting with working directory.
      strcat(pathBuff,charBuff);                   // Add in the user's parameter.
      if (!SD.mkdir(pathBuff)) {                   // If we can not create the folder.
         Serial.println("Can't create folder.");   // We'll send back an error.
      }
   }
}
```
numParams() actually returns the number of parameters that the user typed in. If you are expecting more than one, you may want to use a for-loop for grabbing parameters? Up to you.  

getNextParam() Each time getNextParam() is called, it (Or any other command that wants a string to hand back) re-allocates a c-string for its own use. This means, the pointer you have been handed, is temporary. Therefore it may not last long. So, if you need that information for awhile, you should think about making a copy of it for yourself.

getParamBuff() Passes back the entire typed in param string as a string. Good for passing text blocks.

That's about it. Give it a try and see if it helps.

-jim lee
