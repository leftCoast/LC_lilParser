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
// I think I'm going to do a buff = realloc(buff,size); thing here.
// Need to add numBytes for params.
//
// 3/2022 - Rewriting this to take away the dynamic memory issues. The plan is to pass a
// string pointer back for the params that is reallocated over and over internally so the
// user doesn't do the freening of it.
//
// 3/2023 - Pokergeist John J Added the ability to set your param buffer to a different
// size when creating a parser. I changed the #define to DEF_BUFF_SIZE 'cause it made more
// sense than the old name.


#define EOL '\n'				// Set this to match your system.
#define DEF_BUFF_SIZE 40	// One buff to fit them all..

class cmdTemplate;

class lilParser : public linkList {

	public:
				lilParser(size_t inBufSize=DEF_BUFF_SIZE);
	virtual	~lilParser(void);
		
				void	addCmd(int inCmdNum, const char* inCmd);
				int	addChar(char inChar);
				int	numParams(void);
				int	getParamSize(void);
				char*	getParam(void);			// COPY THIS STRING ASAP. IT'S A TEMP.
				char*	getParamBuff(void);		// COPY THIS STRING ASAP. IT'S A TEMP.
				void	reset(void);
    
				cmdTemplate*	currentCmd;
				bool				firstLetter;
				bool				sawEOL;
				int				paramIndex;
				size_t			paramBuffSize;
};


class cmdTemplate : public linkListObj {

	public:
				cmdTemplate(int inCmdNum, const char* inCmd);
	virtual	~cmdTemplate(void);

				void	addChar(char inChar);
				void  endParse(void);
				bool  parsing(void);
				bool  validCmd(void);
				int   cmdNumber(void);
				void	reset(void);

				int   cmdNum;
				char*	cmd;
				bool  parsingCmd;
				int   cmdIndex;
				bool  badChar;
				bool  cmdOK;
				bool  parsingParam;
				byte	paramIndex;
};

#endif

