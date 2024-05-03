/*********************************************
 * Id: walk5700
 *
 * Compile: gcc -Wall
 * Run: ./a.out input.txt
 *
 * This program tokenizes an input file, using spaces as deliminators, and prints each token.
 *********************************************/

#define MAXTOKEN 256

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Assign meaning to tokens
// @param line      The line to extract a token from
// @param length    The length of line
// @param start     The starting index of the token
// @param end       The ending index of the token
// @param type      String representing type of token
// @param multiLine A flag that is set when the token is multiline.
// @note different values of multiLine mean different things, 0 = not multiLine, 1 = start, 2 = middle, 3 = end
// @note The types of token include: Token, String, Character, Comment
void lex(char *line, int start, int end, char *type, int multiLine);

// Break the string down into tokens
// @param line   The line to tokenize
// @param length Length of line
void tokenize(char *line, int length);

//takes a string of 2 chars, and checks if it is an operator
//@param char1 the first char
//@param char2 the second char
//@return integer flags; 0 = not an operator, 1 = single char operator, 2 = double char operator
int isOperator(char char1, char char2);

int main(int argc, char *argv[])
{
   if(argc<2)
   {
      printf("Please specify input file.\n");
      printf("%s /y/shared/Engineering/cs-drbc/assignments/cs210/w01_in1.txt\n", argv[0]);
      return 1;
   }
   FILE * fp;
   char * line = NULL;
   size_t len = 0;
   ssize_t read;

   fp = fopen(argv[1], "r");
   if (fp == NULL){
      printf("Error: Could not open file %s\n", argv[1]);
      exit(EXIT_FAILURE);
  }

   while ((read = getline(&line, &len, fp)) != -1) {
      tokenize(line, read);
   }

   fclose(fp);
   if (line) {
      free(line);
   }
   exit(EXIT_SUCCESS);
}

//--------Helper/Auxillory(?) Functions------------

//Checks if a token is a keyword
//@param *token String containing token to check
//@param size   The size of token
//@return 0 if token is not a keyword, 1 if it is a keyword
int isKeyword(char *token, int size);

//Checks if a token is a numeric literal
//@param *token String containing token to check
//@param size   The size of token
//@return 0 if token is not a numeric literal, 1 if it is
int isNumeric(char *token, int size);

//Checks if a token is an identifier
//@param *token String containing token to check
//@param size   The size of token
//@return 0 if token is not an identifier, 1 if it is
int isIdentifier(char *token, int size);

//--------Function Defintions---------

// Break the string down into tokens
void tokenize(char *line, int length) {
   int start = 0;               //index of the start of a token. Set to where end was last
   int end;                     //index of the end of a token. Set to the space after a token
   int foundFlag = 0;           //True when a token has been found, false otherwise
   int opFlag = 0;              //stores the return value of isOperator()
   static int multiLine = 0;    //Look at lex documentation
   static int commentFlag = 0;  //Flag set when in comment 

   for(end = 0; end < length; end++) {
      //remove starting spaces from previous lex
      if(foundFlag && !multiLine) {
         while(end < length && isspace(line[end])) {
            end ++;
         }
         //move start
         start = end;
         foundFlag = 0;
      }
      //check for stuff to parse
      opFlag = isOperator(line[end], line[end + 1]);
      if((line[end] == '/' && line[end + 1] == '*') || commentFlag) { //Comment - work
         foundFlag = 1;
         if(start < end) { //lex any unresolved tokens
            lex(line, start, end, "Token", multiLine);
         }
         if (!multiLine) {
            end += 2;
            start = end;
         }
         while(end < length && !(line[end] == '*' && line[end + 1] == '/')) { //move end to index of final *
            end ++;
         }
         if(!(line[end] == '*' && line[end + 1] == '/')) { //update multiLine flag
            commentFlag = 1;
            if(!multiLine) {
               multiLine = 1;
            } else
            {
               multiLine = 2;
            }
         } else 
         if(multiLine) {
            multiLine = 3;
         }
         lex(line, start, end, "Comment", multiLine); //lex comment
         end ++; //move end
      } else
      if(opFlag) { //operator
         foundFlag = 1;
         if(start < end) { //lex any unresolved tokens
            lex(line, start, end, "Token", multiLine);
         }
         start = end ++;
         if(opFlag == 2) {
            end ++;
         }
         lex(line, start, end, "Operator", multiLine);
         end --;
      } else
      if(line[end] == '\"') { //String
         foundFlag = 1;
         if(start < end) { //lex any unresolved tokens before
            lex(line, start, end, "Token", multiLine);
         }
         start = ++end; //set start to the index after the initial double quote
         while(end < length && line[end] != '\"') { //move end to ending double quote or end of line
            if(line[end] == '\\') {
               end += 2;
            } else
            {
               end ++;
            }
         }
         if(end + 1 == length && line[end] != '\"') { //check for multiline string
            printf("Error: Multiline String!");
         }
         lex(line, start, end, "String", multiLine); //lex a string
      } else 
      if(line[end] == '\'') { //Character
         foundFlag = 1;
         if(start < end) { //lex any unresoved tokens
            lex(line, start, end, "Token", multiLine);
         }
         start = ++end;
         if(line[end] == '\\') { //check if escaped
            end += 2;
         } else
         {
            end ++;
         }
         lex(line, start, end, "Char", multiLine); //lex the characters
      } else
      if(isspace(line[end])) { //unresolved token
         foundFlag = 1;
         lex(line, start, end, "Token", multiLine);
      }
      //check multiLine and reset
      if(multiLine == 3) {
         multiLine = 0;
         commentFlag = 0;
      }
   }
}

// Assign meaning to tokens
void lex(char *line, int start, int end, char *type, int multiLine) {
   char token[MAXTOKEN]; //the token getting parsed
   //check if there is actually a token
   if(start == end && isspace(line[start])) {
      return;
   }
   //copy token into var:token
   strncpy(token, &line[start], end - start);
   token[end - start] = '\0';
   //if type is Token, then check for keyword
   if(type[0] == 'T') {
      if(isKeyword(token, strlen(token))) {
         type = "Keyword";
      } else
      if(isNumeric(token, strlen(token))) {
         type = "Numeric";
      } else
      if(isIdentifier(token, strlen(token))) {
         type = "Identifier";
      }
   }
   //print type
   printf("%s: ", type);
   //print token
   if(type[0] == 'S') { //string
      printf("\"%s\"\n", token);
   } else
   if(type[0] == 'T' || type[0] == 'K' || type[0] == 'O' || type[0] == 'N' || type[0] == 'I') { //unresolved token or keyword
      printf("%s\n", token);
   } else
   if (type[0] == 'C') {
      if(type[1] == 'h') { //character
         printf("\'%s\'\n", token);
      } else
      if (type[1] == 'o') { //comment
         if (multiLine == 0 || multiLine == 1) {
            printf("/*");
         }
         printf("%s", token);
         if(multiLine == 0 || multiLine == 3) {
            printf("*/\n");
         }
      }
   } else
   {
      printf("Error: Unknown token type");
   } 
}

//Checks if two operators are an operator
int isOperator(char char1, char char2) {
   const int numOpD = 12; //the number of operators to check for
   const int numOpS = 20; //number of of single character operators
   char* operatorsD[] = {":=", "..", "<<", ">>", "<>", "<=", ">=", "**", "!=", "=>", "{:", "}:"}; //double character symbols representing operators
   char operatorsS[] = {'<', '>', '(', ')', '+', '-', '*', '/', '|', '&', ';', ',', ':', '=', '$', '@', '[', ']', '{', '}'}; //single character symbols representing the operators
   //check double character operators
   for(int i = 0; i < numOpD; i ++) {
      if(char1 == operatorsD[i][0] && char2 == operatorsD[i][1]) {
         return 2;
      }
   }
   //check single character operators
   for(int i = 0; i < numOpS; i ++) {
      if(operatorsS[i] == char1) {
         return 1;
      }
   }
   //if previous checks were false then characters are not an operator
   return 0;
}

//Checks if a token is a keyword
int isKeyword(char *token, int size) {
   const char *keywords[] = {"accessor", "and", "array", "bool", "case", "character", "constant", "else", "elsif", "end", "exit", "float", "func", "if", "ifc", "in", "integer", "is", "mutator", "natural", "null", "of", "or", "others", "out", "pkg", "positive", "proc", "ptr", "range", "subtype", "then", "type", "when", "while"}; //array of all keywords
   const int numKeywords = 35;  //number of keywords contained in *keywords
   
   for(int i = 0; i < numKeywords; i ++) {
      if(strlen(keywords[i]) == size && 
         !strncmp(token, keywords[i], size)) {
         return 1;
      }
   }
   return 0;
}

//Checks if a token is a numeric literal
int isNumeric(char *token, int size) {
   int decFlag = 0; //Flag set when '.' encountered and unset otherwise
   //check each character
   for(int i = 0; i < size; i ++) {
      if(!isxdigit(token[i])) { //check digit for numeric-ness (?)
         if(token[i] != '.' || decFlag) { //if not a hex digit, check for decimal, and whether a decimal has been enountered before
            return 0; //not hex digit, decimal, or decimal has been encountered before
         } else
         {
            decFlag = 1; //digit is a decimal, and no decimals have been encountered yet
         }
      }
   }
   //if here then token is numeric literal
   return 1;
}

//Checks if a token is an identifier
int isIdentifier(char *token, int size) {
   //check first character
   if(!isalpha(token[0])) { //is alphabetical?
      return 0;
   }
   //check the rest
   for(int i = 1; i < size; i ++) {
      if(!isalpha(token[i]) && !isdigit(token[i]) && token[i] != '_') { //is alphabetical, numerical, or underscore?
         return 0;
      }
   }
   //if here, everything checks out and token meats requirements for an identifier
   return 1;
}
