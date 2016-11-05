#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "scanner.h"

FILE*	fHandle = NULL;

int	scanner_openFile(char* fileName)
{
	fHandle = fopen(fileName,"r");	
	if(fHandle)
		return 1;
	return 0;
}

int	scanner_closeFile()
{
	fclose(fHandle);
	return 0;
}


//TODO: implement a single linked list to store tokens
int	scanner_rewind()
{
        rewind(fHandle);	
return 1;
}

t_token	g_lastToken;

char*	createString(const char* str)
{
	char* nstr = malloc(strlen(str)+1);
	if(nstr)
	{
		strcpy(nstr,str);
		return nstr;
	}
	return NULL;
}

char*	createString2(const char* str, const char* second)
{
	char* nstr = malloc(strlen(str)+strlen(second)+2);
	if(nstr)
	{
		sprintf(nstr,"%s.%s",str,second);
		return nstr;
	}
	return NULL;
}


// processes text literals such as "text" or "tex\t"
int	process_literal()
{
	fgetc(fHandle);
	// a normal state is used when awaiting regular ASCII input
	// SPECIAL state is reached after receiving '\' 
	enum States {NORMAL, SPECIAL,OCTAL};
	int c, state = NORMAL;

	// TODO: provide a better string datatype which would allow 'unlimited' strings
	char tempString[1001] = "\0";
	int i = 0;

	int octBase= 64;
	char sum = 0;
	while((c = fgetc(fHandle)) != EOF)
	{
		switch(state)
		{
			case NORMAL:
				if(c == '\"')
				{
					// when the end of terminal is reached
					g_lastToken.type = TOK_LITERAL;
					g_lastToken.data.string = createString(tempString);
					// TODO: add pointer to symbol
					return TOK_LITERAL;

				} else if(c == '\\')
				{
					state = SPECIAL;
					break;
				} else {
					tempString[i++] = c;
				}
				break;
			// process escape sequences (e.g. \n) or report invalid ones
			case SPECIAL:
				switch(c)
				{
					case '0': case '1': case '2': case '3': 
						ungetc(c,fHandle);
						state = OCTAL;
						break;
					case '\\':
					case '\"':
						tempString[i++] = c;
						state = NORMAL;
						break;
					case 'n':
						tempString[i++] = '\n';
						state = NORMAL;
						break;
					case 't':
						tempString[i++] = '\t';
						state = NORMAL;
						break;
					default:

						// TODO: report an error - invalid escape sequence
						fprintf(stderr, "Error while reading literal\n");
						return TOK_ERROR;
						break;
				}
				break;
			// special case for processing \ddd
			case OCTAL:
				if(isdigit(c))
				{
					// convert ASCII char to a number of <0,9>
					int digit = c-'0';
					if(digit > 7)
						return TOK_ERROR;
					// and multiply it with N power of 8
					// => conversing octal-to-decal with one buffer sum 
					sum += octBase*digit;
					octBase /= 8;
					// if we have read 3 numbers
					if(octBase == 0)
					{
						//concatenate a new char
						tempString[i++] = sum;
						// reload default values 
						octBase= 64;
						sum = 0;
						// and continue reading the rest of literal
						state = NORMAL;
					}
				} else {
					return TOK_ERROR;
				}	
				break;
		}
	}
	return TOK_ERROR;
}

// Utility function
// TODO: implement a faster way of searching for strings (tree)
//

// Returns 1 if ID is an keyword (type of keyword is returned in 2nd param)
// Returns 0 otherwise
int	isKeyword(const char* str,int* typeOfKeyword)
{
	static char* keywords[] = {"boolean","break","class","continue","do","double",
	"else","false","for","if","int","return","String","static","true","void","while",NULL};

	for(int i = 0; keywords[i] != NULL; i++)
	{
		if(strcmp(keywords[i],str) == 0)
		{
			(*typeOfKeyword) = i;
			return 1;
		}
	}
	return 0;
}

int	process_identifier()
{
	//TODO: unlimited length of ID
	char first[256] = {0,};
	char second[256] = {0,};

	int i = 0,c;
	// nonAlpha is toggled to 1 if at least one character is non-alphanumerical
	// -> useful for skipping keyword comparing 
	int isNonAlpha = 0;

	enum states {FIRST,SECOND};
	int state = FIRST;
	while((c = fgetc(fHandle)) != EOF)
	{
		int res = isalnum(c);
	
		// if character fullfills ID requirements 
		if(res || c == '$' || c == '_')
		{
			isNonAlpha |= (res == 0);
		
			// store the incoming char into either first or second part of ID
			if(state == FIRST)
				first[i++] = c;
			else
				second[i++] = c;
		} else {
			// if it is a ID splitter (FIRST.SECOND)
			if(c == '.')
			{
				// then change the state to second
				if(state == FIRST)
					state = SECOND;
				else {
					// already processing the second part of ID
					// > lex. error
					fprintf(stderr,"Error: Multiple '.' in identifier.\n");
					return TOK_ERROR;
				}
				i = 0;
				continue;
			}
			
			// the last character is probably a part of another token->return back
			ungetc(c,fHandle);

			// if we captured only a single word, it migh be a keyword
			if(state != SECOND)
			{
				// verify if string isn't a keyword
				if(!isNonAlpha)
				{
					int typeOfKeyword;
					// if ID is in set of keywords
					if(isKeyword(first,&typeOfKeyword))
					{
						g_lastToken.type = TOK_KEYWORD;
						g_lastToken.data.integer = typeOfKeyword; 
						return TOK_KEYWORD;
					}
					
				}
			}
			// otherwise, it's an ID
			g_lastToken.type = TOK_ID;
			
			// if it's a special ID (ID.ID)
			if(state == SECOND)
			{
				// if the second part of ID fullfills requirements
				if(i > 0 && isdigit(second[0]) == 0)
				{
					g_lastToken.data.string = createString2(first,second);
					g_lastToken.type = TOK_SPECIAL_ID;
				}
				else {
					// error in the second part of ID
					return TOK_ERROR;
				}
			} else 
				g_lastToken.data.string = createString(first);
				return g_lastToken.type;
		}
	}
	return TOK_ERROR;
	
}

int	process_number()
{
	//TODO: improve solution for larger numbers
	//TODO: a question: is overflow/underflow taken as a scanner error ?
	char buff[256] = {0, };
	int c,i = 0;

	enum numberType {INT, DOT,DOUBLE,EXP,EXP_SIGN,EXP_RADIX};
	int state = INT;
	while((c = fgetc(fHandle)) != EOF)
	{
		switch(state)
		{
			case INT:
				if(isdigit(c) || tolower(c) == 'e' || c == '.')
				{
					buff[i++] = c;
					if(c == '.')
						state = DOT;
					else if(tolower(c) == 'e')
						state = EXP;
				} else {
					ungetc(c,fHandle);
					g_lastToken.type = TOK_CONST;
					g_lastToken.data.integer = atoi(buff);
					return TOK_CONST;
				}
				break;
			case DOT:
				if(isdigit(c))
				{
					buff[i++] = c;
					state = DOUBLE;
				} else {
					// emit error, number ends with '.' without any following digit
					return TOK_ERROR;
				}
				break;
			case DOUBLE:
				if(!isdigit(c) && tolower(c) != 'e')
				{
					// new float
					ungetc(c,fHandle);
					g_lastToken.type = TOK_DOUBLECONST;
					g_lastToken.data.real= atof(buff);
					return TOK_DOUBLECONST;
				} else {
					buff[i++] = c;	
					if(tolower(c) == 'e')
						state = EXP;
				}
				break;
			case EXP:
				if(c == '+' || c == '-')
				{
					buff[i++] = c;
					state = EXP_SIGN;
				} else if(isdigit(c))
				{
					buff[i++] = c;
					state = EXP_RADIX;
				} else {
					return TOK_ERROR;
				}
				break;
			case EXP_SIGN:
				if(isdigit(c))
				{
					buff[i++] = c;
					state = EXP_RADIX;
				} else {
					return TOK_ERROR;
				}
				break;
			case EXP_RADIX:
				if(isdigit(c) || c == '+' || c == '-')
					buff[i++] = c;
				else 
				{
					//new float
					ungetc(c,fHandle);
					g_lastToken.type = TOK_DOUBLECONST;
					g_lastToken.data.real= atof(buff);
					return TOK_DOUBLECONST;
				}
				break;
				
		}
	}
	return TOK_ERROR;
}

int	process_operator(char op)
{
	switch(op)
	{
		case '*':
			g_lastToken.type = TOK_MUL;		
			break;
		case '/':
			g_lastToken.type = TOK_DIV;		
			break;
		case '+':
			g_lastToken.type = TOK_PLUS;		
			break;
		case '-':
			g_lastToken.type = TOK_MINUS;		
			break;
		default:
			fprintf(stderr,"Unknown operator '%c'\n",op);
			return TOK_ERROR;
	}
	g_lastToken.data.op= op;
	return g_lastToken.type;
}

int	process_relation(char c)
{
	int nextc = fgetc(fHandle);
	// TODO: what exactly should we do upon receiving an EOF ?
	if(nextc == EOF)
		return TOK_ERROR;
	switch(c)
	{
		case '=':
			if(nextc == '=')
			{
				g_lastToken.type = TOK_EQ;
			} else {
				// assigment
				ungetc(nextc,fHandle);
				g_lastToken.type = TOK_ASSIGN;
			}
			return g_lastToken.type;
			break;
		case '!':
			if(nextc == '=')
				g_lastToken.type = TOK_NOTEQ;
			else
				return TOK_ERROR;
			return g_lastToken.type;
			break;
		case '<':
			if(nextc == '=')
				g_lastToken.type = TOK_LE;
			else {
				ungetc(nextc,fHandle);
				g_lastToken.type = TOK_LESS;
			}
			return g_lastToken.type;
			break;
		case '>':
			if(nextc == '=')
				g_lastToken.type = TOK_GE;
			else {
				ungetc(nextc,fHandle);
				g_lastToken.type = TOK_GREATER;
			}
			return g_lastToken.type;
			break;
		default:
			break;	
	}
	return TOK_ERROR;
}
// maps ASCII symbols to token types
int	process_symbol(char op)
{
	int type = 0;
	switch(op)
	{
		case '(':
			type = TOK_LEFT_PAR;
			break; 
		case ')':
			type = TOK_RIGHT_PAR;
			break; 
		case '{':
			type = TOK_LEFT_BRACE;
			break; 
		case '}':
			type = TOK_RIGHT_BRACE;
			break; 
		case ';':
			type = TOK_DELIM;
			break; 
		case ',':
			type = TOK_LIST_DELIM;
			break;
		default:
			return TOK_ERROR;
	}		
	g_lastToken.type = type;
	return g_lastToken.type;
}

// isBlock 	if func should process block comment
int	process_comments(int isBlock)
{
	// These states are used while waiting for a sequence to terminate
	// block commentary
	enum waitingState {FOR_END, NORMAL};
	enum commentaryType {LINE,BLOCK};
	int c,state = NORMAL;
	while((c = fgetc(fHandle)) != EOF)
	{
		switch(isBlock)
		{
			case LINE:
				// loop through text untill the end of line 
				if(c == '\n')
					return g_lastToken.type;
				break;
			case BLOCK:
				// either await a part of commentary or
				// wait for '/' in FOR_END after receiving '*'
				switch(state)
				{
					case NORMAL:
						if(c == '*')
							state = FOR_END;
						break;
					case FOR_END:
						if(c == '/')
							return g_lastToken.type;
						else if(c == '*')
							break;
						else
							state = NORMAL;
						break;
				}
		}
	}
	return TOK_ERROR;	
}

int	getToken()
{
	// let's get a character from source code's stream
	int c;
	while((c = fgetc(fHandle)) != EOF)
	{
		// if the stream is at its end, either return EOF token
		// or report error (multiple EOF tokens reached)
		if(c == EOF)
			break;
		switch(c)
		{
			// literals start with "
			case '\"':
				ungetc(c,fHandle);
				return  process_literal();
				break;
			// skip these chars
			case ' ':
			case '\t': 
			case '\n':
				break;
			// start of commentaries or / operator
			case '/':
				c = fgetc(fHandle);
				switch(c)
				{
					case EOF:
						break;
					case '/':
					case '*':
						process_comments(c == '*');
						break;
					default:
						// it was an operator /
						ungetc(c,fHandle);
						return process_operator('/');
				}
				break;
			// math operators
			case '+':
			case '-':
			case '*':
				return process_operator(c);	
			case '{':
			case '}':
			case '(':
			case ')':
			case ';':
			case ',':
				return process_symbol(c);
			case '<':
			case '>':
			case '=':
			case '!':
				return process_relation(c);
			default:
				if(isalpha(c) || c == '_' || c == '$')
				{
					ungetc(c,fHandle);
					return process_identifier();
				} else if(isdigit(c))
				{
					ungetc(c,fHandle);
					return process_number();
				} else {
					fprintf(stderr,"Error: No token defined for 0x%X character\n",c);
					return TOK_ERROR;
				}
				break;
				
		}
	}
	// if there has already been an EOF token, report ERROR
	if(g_lastToken.type == TOK_EOF)
		return TOK_ERROR;

	// report the first occurence of EOF
	g_lastToken.type = TOK_EOF;
	return TOK_EOF;
}
