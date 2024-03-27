#ifndef lilParser_h
#define lilParser_h

#include <lists.h>

// 8/2019 - I must have been on something when I wrote this. It all works, but WOW I can't
// make heads or tails of it now.
//
//
// A Command is: CMD params.. \n
// Parameters are seperated by whitespace.
// Succesful parsed commands return positive integers.
// Commands "in process" return 0. IE not end of text.
// Unparsable commands return -1
// Meaning? Your enum of commands should start with noCommand. IE a 0 to skip over in your
// switch statment.
//
//
// 3/2022 - Rewriting this to take away the dynamic memory issues. The plan is to pass a
// string pointer back for the params that is reallocated over and over internally so the
// user doesn't do the freening of it.
//
// 3/2023 - Pokergeist John J Added the ability to set your param buffer to a different
// size when creating a parser. I changed the #define to DEF_BUFF_SIZE 'cause it made more
// sense than the old name.
//
// 3/2024 - Testing the textBuff library code, I found that when run "hard" this thing
// would fail. And, as I said before, it was written when I was "in the zone" and
// extremely hard to follow.
//
// So I ended up rewriting the entire thing from scratch. Now, as far as I can tell, it
// is way more logical and maintainable. It even handles limited memory better, and throws
// errors as apposed to just crashing when it accidentally runs out.
//
// It also can run along side of sibling parsers nothing is shared anymore. And I kept 
// Pokergeist John J addition of a user selectable param buffer size.


#define EOL '\n'					// Set this to match your system.
#define DEF_BUFF_SIZE	20		// Default size buffer to hold the param string.
#define PARSE_ERR			-1		// Basically.. We have no idea. Possibly a typo?
#define CONFIG_ERR		-2		// Most likely we ran out of RAM for setup.
#define PARAM_ERR			-3		// The param buff was too small for the incoming string.



class cmdTemplate;

class lilParser : public linkList {

	public:
	
				enum parseState { startParse, parsingCmd, parsingParams, dumpingParams, configErr };
				
				lilParser(int inBufSize=DEF_BUFF_SIZE);
	virtual	~lilParser(void);
		
				void	addCmd(int inCmdNum, const char* inCmd);
				int	addChar(char inChar);
				int	numParams(void);
				char*	getNextParam(void);		// COPY/READ THIS STRING ASAP. IT'S A TEMP.
				char*	getParamBuff(void);		// COPY/READ THIS STRING ASAP. IT'S A TEMP.
	private:
				void	resetParse(void);
				int	brokenParse(char inChar);
				int	parseStart(char inChar);
				int	parseCmd(char inChar);
				int	parseParam(char inChar);
				int	dumpParam(char inChar);
				
				parseState		ourState;
				char*				paramBuff;
				int				paramBuffSize;
				int				paramIndex;
				int				cmd;
				bool				haveParamBuff;
				int				tokenIdx;
				char*				resultStr;
};


class cmdTemplate : public linkListObj {

	public:
				cmdTemplate(int inCmdNum, const char* inCmd);
	virtual	~cmdTemplate(void);

				bool	isOK(void);
				void	addChar(char inChar);
				int	isMatch(void);
				void	reset(void);
				
				int	cmdNum;
				char*	cmdStr;
				int	numChars;
				int	index;
				bool	ready;
				bool	fail;
};

#endif

