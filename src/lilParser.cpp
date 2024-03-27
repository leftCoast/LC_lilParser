#include <lilParser.h>
#include <strTools.h>



// Handy little guy that decides the type of each char. We have 3 types. text is basically
// printable. space is whitespace. endline is either EOL or '\0'.

enum charType { text, space, endline };

charType getType(char inChar) {

	if (inChar=='\0'||inChar==EOL) return endline;
	if (isspace(inChar)) return space;
	return text;
}


// Create a new parser. The buff size is for the param list. If you are not using commands
// with params? You may want to just set this maybe 2 to save the memory.
lilParser::lilParser(int inBufSize) {

	resultStr = NULL;									// Call a function for a string output? This is the string you get.
	paramBuffSize = 0;								// Default this to zero. If we get one, we'll update this.
	haveParamBuff = false;							// And no, we don't have one yet. May never have one.
	paramBuff = NULL;									// ALL pointers get initialized to NULL here.
	if (resizeBuff(inBufSize,&paramBuff)) {	// Lets see if we can allocate the param buffer.
		haveParamBuff = true;						// Ok we got one.
		paramBuffSize = inBufSize;					// And it's this many bytes.
		resetParse();									// Lets setup for our first parse.
	} else {												// Else we didn't get one.. 
		ourState = configErr;						// Set our state to configErr.
	}
}


// And our destructor. Basically, release what we allocated. Will this ever be used?
// Probably never. But if someone wants to spawn a parser on the fly. Then delte it? No
// worries. It'll clean up after itself as it should.
lilParser::~lilParser(void) {

	resizeBuff(0,&paramBuff);
	resizeBuff(0,&resultStr);
}


// Add a command ID  and matching type-able string. Multiple strings can match with an ID.
// But you dn't want more than one ID to match a string. That won'y work.	
void lilParser::addCmd(int inCmdNum, const char* inCmd) {
	
	cmdTemplate*	newCmd;
	
	if (ourState != configErr) {
		newCmd = new cmdTemplate(inCmdNum,inCmd);
		if (newCmd) {
			if (newCmd->isOK()) {
				addToTop(newCmd);
			}
		} else {
			ourState = configErr;
		}
	}
}


// A char has arrived! Let's run it through the parsing machine and see what the result is.
int lilParser::addChar(char inChar) {

	switch(ourState) {
		case startParse		: return parseStart(inChar);
		case parsingCmd		: return parseCmd(inChar);
		case parsingParams	: return parseParam(inChar);
		case dumpingParams	: return dumpParam(inChar);
		case configErr			: return brokenParse(inChar);
	}
	return 0;
}	

// This resets everything for a new parse.
void lilParser::resetParse(void) {

	cmdTemplate*	trace;
	
	if (ourState!=configErr) {								// If we were able to setup shop..
		trace = (cmdTemplate*)getFirst();				// Grab a pointer to the top of the list.
		while(trace) {											// While we have a non-NULL list pointer.
			trace->reset();									// Reset the templates for new parse.
			trace = (cmdTemplate*)trace->getNext();	// Hop to the next template on the list.
		}															//
		paramIndex = 0;										// Reset the param buff writing index.
		ourState = startParse;								// Ok, ready to parse!
	}
}


// This is called if we couldn't set up the parser. Probably not enough RAM. It basically
// eats chars 'till an endline then outputs the error message.
int lilParser::brokenParse(char inChar) {

	if (getType(inChar)==endline) return CONFIG_ERR;
	return 0;
}


// This is basically a white space eater for the beginning of command strings.		
int lilParser::parseStart(char inChar) {

	tokenIdx = 0;							// After the first char, reset the param token index.
	if (haveParamBuff) {					// And if we have a param buff..
		paramBuff[0] = '\0';				// Clear the damn thing out.
	}
	switch(getType(inChar)) {			// Check the kind of char we got..
		case text		:					// Got text
			ourState = parsingCmd;		// Our state is now parsing a command.
			return parseCmd(inChar);	// Pass in this non whitespace char.
		case space		: 					//
		case endline	: return 0;		// Got space or endlne..
	}											//
	return 0;								// Won't happen. Shuts up compiler.
}


// At this point we knocked off all the leading blank chars. This is at least the first
// or subsequent printable char. Looking for a valid command or a fail to parse.
int lilParser::parseCmd(char inChar) {
	
	cmdTemplate*	trace;
	
	trace = (cmdTemplate*)getFirst();						// We are parsing a command. Get top of list.
	switch(getType(inChar)) {									// Check the kind of char we got..
		case text		:											// Got text..
			while(trace) {											// While trace is not NULL..	
				trace->addChar(inChar);							// Every template gets a look at the character.
				trace = (cmdTemplate*)trace->getNext();	// Hop on to the next guy.
			}															//
			return 0;												// Keep 'me coming. We're still listening.
		case space		:											// Got a space..
			cmd = 0;													// Reset the command.
			while(trace && !cmd) {								// While trace is not NULL and no command has been claimed..
				cmd = trace->isMatch();							// See if what we point at is a match.
				trace = (cmdTemplate*)trace->getNext();	// Hop to the next template on the list.
			}															//
			if (!cmd) {												// If we didn't get a parse success..
				cmd = PARSE_ERR;									// We note this as a parsing error.
				ourState = dumpingParams;						// Dump the rest.
				return 0;											// Return our basic, "keep it up, furball'.
			} else {													// Else we have valid command..
				if (haveParamBuff) {								// If we have a param buffer..
					ourState = parsingParams;					// Now we're parsing a param list.
					return 0;										// Tell 'em to keep going, we're listening.				
				} else {												// Else we have command, but no param buffer..
					ourState = dumpingParams;					// Dump the rest.
					return 0;										// Return our basic, "keep it up, furball'.
				}														//
			}															//
		case endline	:											// Got an end of line..
			cmd = 0;													// Reset the command.
			while(trace && !cmd) {								// While trace is not NULL and no command has been claimed..
				cmd = trace->isMatch();							// See if what we point at is a match.
				trace = (cmdTemplate*)trace->getNext();	// Hop to the next template on the list.
			}															//
			resetParse();											// We clear out for next parse.
			if (cmd) {												// If we got a command..
				return cmd;											// Return the command ID.
			} else {													// Else, no one claimed it..
				return PARSE_ERR;									// Pass back a parse error.
			}															//
	}																	//
	return 0;														// Just in case. But we should have coved all cases.																//
}
	

// To be here, we must have found a valid command.
int lilParser::parseParam(char inChar) {

	switch(getType(inChar)) {							// Check the kind of char we got..
		case text		:									// Got text..
		case space 		:									// Or a space..
			if (paramBuffSize>paramIndex+1) {		// If the buffer is big enough..
				paramBuff[paramIndex] = inChar;		// Stuff in the char.	
				paramIndex++;								// Bump up indes to next slot
				paramBuff[paramIndex] = '\0';			// Stuff in a line ending. Just in case..
				return 0;									// Return, "keep 'em coming! If you got 'em."
			} else {											// Else the buffer was NOT big enough..
				cmd = PARAM_ERR;							// We switch the command to a param error.
				ourState = dumpingParams;				// Setup to dump the rest of the input.
				return dumpParam(inChar);				// Start the dumping process.
			}													//
		case endline	:									// Got endLine..
			resetParse();									// We done! Do reset.
			return cmd;										// Pass back the command we got from before.
	}															//
	return 0;												// Won't happen. Shuts up compiler.
}
							
			
// To be here, we must NOT have found a valid command. We may have had an error.	
int lilParser::dumpParam(char inChar) {
	
	if (getType(inChar)==endline) {		// If we got to the end..
		resetParse();							// Reset the parse.
		return cmd;								// return whatever command was stored.
	}												//
	return 0;									// All other cases just ask for more.
}
	

// Count the beginnings of non white char blocks.			
int lilParser::numParams(void) {

	int	i;
	int	count;
	
	i = 0;															// Point at the beginning.
	count = 0;														// As yet, we have none.
	while(true) {													// loop 'till we drop.
		while(getType(paramBuff[i])==space) i++;			// Run the string while a space..
		switch(getType(paramBuff[i])) {						// So, what are we lookin' at?
			case space		: return 0;							// It can't be space. Return 0.
			case text		:										// We got text..
				count++;												// Bump up the count.
				while(getType(paramBuff[i])==text) i++;	// Run the string while printable..
				if (getType(paramBuff[i])==endline) {		// If we hit the end..
					return count;									// Return the count.
				}														//
			break;													// Din't hit the end, keep going.
			case endline	: return count;					// We hit the end? Return the count.
		}
	}
}


// We keep track of the last param handed out. This starts the search for the next param
// from there. Then the location of this param's end is saved for the next getParam call.
char* lilParser::getNextParam(void) {

	int	endIdx;
	int	numBytes;
	int	outIdx;
	
	while(getType(paramBuff[tokenIdx])==space) tokenIdx++;	// Run across all the white space.
	if (getType(paramBuff[tokenIdx])==endline) return NULL;	// If sitting on the endline return NULL. We're done.
	endIdx = tokenIdx;													// We must be pointing at text. Bring up endIdx.
	while(getType(paramBuff[endIdx])==text) endIdx++;			// Run endIdx across the text.
	numBytes = endIdx - tokenIdx + 2;								// Calculate the number bytes needed for this param.
	if (resizeBuff(numBytes,&resultStr)) {            			// If we can allocated the memory..
		outIdx = 0;															// outIdx set to start of output string.
		for(int i=tokenIdx;i<endIdx;i++) {							// For every char in the param..
			resultStr[outIdx] = paramBuff[i];						// Add the char into the output string.
			outIdx++;														// Bump up the output string's index.
		}																		//
		resultStr[outIdx] = '\0';										// Looping's over. Pop in the trailing '\0'.
		tokenIdx = endIdx;												// Update the tokenIdx for the next call.
		return resultStr;													// Hand the param string back to the caller.
	} else {																	// Else the allocation failed..
		return NULL;														// In this case all we can do is return NULL.
	}
}
		
		
char* lilParser::getParamBuff(void) { return paramBuff; }
				
				
				
// ************* cmdTemplate *************


cmdTemplate::cmdTemplate(int inCmdNum, const char* inCmd) {

	ready = false;
	cmdStr = NULL;
	if (heapStr(&cmdStr,inCmd)) {
		cmdNum = inCmdNum;
		ready = true;
		reset();
	}
}


cmdTemplate::~cmdTemplate(void) { freeStr(&cmdStr); }


bool cmdTemplate::isOK(void) { return ready; }


void cmdTemplate::addChar(char inChar) {
	
	if (ready && !fail) {
		if (cmdStr[index]==inChar) {
			index++;
		} else {
			fail = true;
		}
	}	
}


int cmdTemplate::isMatch(void) {
	
	if (ready && !fail) {
		if (cmdStr[index]=='\0') {
			return cmdNum;
		}
	}
	return 0;
}


void cmdTemplate::reset(void) {

	if (ready) {
		index = 0;
		fail	= false;
	}
}
