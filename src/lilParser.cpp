#include <lilParser.h>
#include <strTools.h>


// ****************************************
// lilParser
// ****************************************


char*	paramBuff = NULL;
int	buffCount = 0;

// Create a parser.
lilParser::lilParser(size_t inBufSize)
	:linkList() {
	
	paramBuffSize = inBufSize;
	buffCount++;
	if (!paramBuff) {
		resizeBuff(paramBuffSize,&paramBuff);
	}
	reset();
}


// We are a linked list, so the list auto dumps when we destruct.
lilParser::~lilParser(void) {

	buffCount--;
	if (!buffCount) {
		resizeBuff(0,&paramBuff);
	}
	resizeBuff(0,&returnStr);
}


// Add a command we can parse for. Only command numbers >0 need apply.
void lilParser::addCmd(int inCmdNum, const char* inCmd) {

  cmdTemplate* command;

  if (inCmdNum > 0) {
    command = (cmdTemplate*) new cmdTemplate(inCmdNum, inCmd);
    if (command) {
      addToTop((linkListObj*)command);
    }
  }
}


// Pass in charactors and get back, -1 bad command, 0 still parsing or a command number.
int lilParser::addChar(char inChar) {

  cmdTemplate*  trace;

  if (sawEOL) {                           		// Means that LAST time we saw EOL.
    reset();                              		// Meaning, we are at a fresh beginning!
  }
  if (!firstLetter) {                     		// Not seen first letter.
    if (isspace(inChar)) {                		// White space?
      return 0;                           		// Burn off leading white space.
    } else {                              		// Oh, printable?
      firstLetter = true;                 		// Then we HAVE seen first letter
    }
  }
  if (inChar == EOL) {                    		// Ah! The end of the line!
    sawEOL = true;                        		// Well, it is true. Its right there.
    trace = (cmdTemplate*)theList;        		// Setup to see if any cmds are still valid..
    while (trace != NULL) {               		// Start looping.
      if (trace->validCmd()) {            		// Find the first cmd that's still valid.
        currentCmd = trace;               		// Save off who it is.
        trace->endParse();                		// Tell the valid one, "We're done here."
        return trace->cmdNumber();        		// And return its command number.
      }
      trace = (cmdTemplate*)trace->getNext();	// We're still here? Check the next cmd.
      
    }
    return -1;                            		// Saw EOL and no one took the bait? Bad inputted command.
  } else {                                		// Not seen EOL..
    trace = (cmdTemplate*)theList;        		// Setup to see if any cmds are still parsing..
    while (trace != NULL) {               		// Start looping.
      if (trace->parsing()) {             		// Are you still parsing?
        trace->addChar(inChar);           		// Parse this big boy!
      }
      trace = (cmdTemplate*)trace->getNext();	// Off to the next cmd.
    }
    return 0;                             		// Whatever, until EOL we're still parsin'
  }
}


int lilParser::numParams(void) {

  int count;
  int index;

  if (paramBuff[0] == '\0') return 0;                       		// Actually is a special case.
  count = 0;                                                		// Ready for looping.
  index = 0;
  while (paramBuff[index] != '\0' && index < paramBuffSize) {   // Until we run out of string.
    if (paramBuff[index] == EOL) {                              // Count up all the EOLs.
      count++;
    }
    index++;
  }
  return count + 1;
}


// Same as strlen 
int lilParser::getParamSize(void) {

	int index;
	int	count;
	
	count = 0;																			// Nothin' yet..
	if (currentCmd) {																	// Had successful parse.
		index = paramIndex;																// local copy of paramIndex.
		if (paramBuff[index] != '\0') {                                 	// Not at the end of the buffer.
			while (paramBuff[index] != '\0' && paramBuff[index] != EOL) {	// Loop through the next param.
				count++;																		// Counting..
				index++;
			}
		}
	}
	return count;																		// Tell the world.
}


// Returns the next param in the param string. Passes it back as a char* buffer that
// you should, depending on how you use it, make a local copy first thing.
char* lilParser::getParam(void) {
	
	int 	index;
	
	if (currentCmd) {																						// If we had successful parse.
		if (resizeBuff(getParamSize()+1,&returnStr)) {											// If we can get the memory.
			index = 0;																						// Ready to write in the chars..
			if (paramBuff[paramIndex] != '\0') {                                  		// Not looking at empty buffer.
				while (paramBuff[paramIndex] != '\0' && paramBuff[paramIndex] != EOL) { // Loop through to the next param.
					returnStr[index++] = paramBuff[paramIndex++];								// Filling the user buff.
				}																								//
				returnStr[index] = '\0';                                                // Cap off the new buff.
				if (paramBuff[paramIndex] == EOL) {                                 		// If EOL kicked us out.
					paramIndex++;																			// Hop over it.
				}
			}
  		}
	}
	return returnStr;                                                           		// Pass back the result.
}


// Ok, its not -really- the param buff. All whitespace is reduced to SINGLE spaces. Passes
// it back as a char* buffer that you should, depending on how you use it, make a local
// copy first thing.
char* lilParser::getParamBuff(void) {
	
	int	i;
	
	if (resizeBuff(getParamSize()+1,&returnStr)) {		// If we can get the memory.
		strcpy(returnStr,paramBuff);
		i=0;
		while(returnStr[i]!='\0') {
			if (returnStr[i]==EOL) {
				returnStr[i]=' ';
			}
			i++;
		}
	}
	return returnStr;
}


void lilParser::reset(void) {

  cmdTemplate*  trace;

  currentCmd = NULL;                    		// Deselect everyone.
  firstLetter = false;                  		// Not seen a first letter.
  sawEOL = false;                       		// Not seen the EOL yet, either.
  paramIndex = 0;                       		// Ready for the next reading.
  paramBuff[paramIndex] = '\0';         		// Cleared!
  trace = (cmdTemplate*)theList;        		// Setup to reset all the cmds.
  while (trace != NULL) {               		// Start looping.
    trace->reset();                     		// trace must be non NULL to get here so OK to call.
    trace = (cmdTemplate*)trace->getNext();	// Move to the next cmd.
  }
}



// ****************************************
// cmdTemplate
// ****************************************


cmdTemplate::cmdTemplate(int inCmdNum, const char* inCmd) {
	
	cmd = NULL;
	heapStr(&cmd,inCmd);
	cmdNum = inCmdNum;
	reset();
}


cmdTemplate::~cmdTemplate(void) { freeStr(&cmd); }


void cmdTemplate::addChar(char inChar) {

  if (parsingCmd) {                               // Ok, if parsing command.
    if (isspace(inChar)) {                        // And we have a white space. (cmd done)
      if (cmdIndex == (int)strlen(cmd)) {         // If we've parsed the same amount of chars as length.
        cmdOK = true;                             // Then its a match!
        parsingCmd = false;                       // No longer parsing the command.
      }
    } else if (cmd[cmdIndex] == inChar) {         // Else its a char, If its a match for us..
      cmdIndex++;                                 // Bump up the index and go on.
    } else {                                      // Else it was NOT a match.
      badChar = true;                             // BAD char!
      parsingCmd = false;                         // No longer parsing command.
    }
  } else if (cmdOK)  {                            // Not parsing a command, was it successful?
    if (!parsingParam) {                          // If we're NOT currently parsing a param.
      if (isspace(inChar)) {                      // Oh ho! Whitespace.
        return;                                   // Really nothing to do.
      } else {                                    // Else not whitespace.
        parsingParam = true;                      // Meaning we're now parsing a param,
        if (paramIndex) {                         //  We wern't, now we are. If index>0 its not the first.
          paramBuff[paramIndex++] = EOL;          //  So pop in a seperator.
        }
        paramBuff[paramIndex++] = inChar;         // Either way pop in the charactor.
      }
    } else {                                      // Else were in the middle of parsing a param.
      if (isspace(inChar)) {                      // A white space here means this param is complete.
        parsingParam = false;                     // So NOT parsing a param.
      } else {                                    // Else, parsing a param and not whitespace.
        paramBuff[paramIndex++] = inChar;         // Add the Char to the list.
      }
    }
  }
}


void cmdTemplate::endParse(void) {

  if (cmdOK) {                        // If we're the one with the valid parse.
    paramBuff[paramIndex] = '\0'; // Then close up the param buffer.
  }
}


bool cmdTemplate::parsing(void) {
  return !badChar;
}


// Two cases here. A) we've seen all the command chars and not hit a bad one.
// Or B) we already went through this and set cmdOK to true.
bool cmdTemplate::validCmd(void) { 
  return ((cmdIndex == (int)strlen(cmd) && !badChar) || cmdOK);
}


int  cmdTemplate::cmdNumber(void) {
  return cmdNum;
}


void cmdTemplate::reset(void) {
	
	if (cmd) {									// If we got the buffer
		parsingCmd = true;
		cmdIndex = 0;
		badChar = false;
		cmdOK = false;
		parsingParam = false;
		paramIndex = 0;
	} else {										// How many ways can we say "No, not doing it!"
		parsingCmd = false;
		badChar = true;
		cmdOK = false;
		parsingParam = false;
	}
}
