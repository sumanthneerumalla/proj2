// File: Sally.cpp
//
// CMSC 341 Spring 2017 Project 2
//
// Implementation of member functions of Sally Forth interpreter
//

#include <iostream>
#include <string>
#include <list>
#include <stack>
#include <stdexcept>
#include <cstdlib>
using namespace std ;

#include "Sally.h"


// Basic Token constructor. Just assigns values.
//
Token::Token(TokenKind kind, int val, string txt) {
   m_kind = kind ;
   m_value = val ;
   m_text = txt ;
}


// Basic SymTabEntry constructor. Just assigns values.
//
SymTabEntry::SymTabEntry(TokenKind kind, int val, operation_t fptr) {
   m_kind = kind ;
   m_value = val ;
   m_dothis = fptr ;
}


// Constructor for Sally Forth interpreter.
// Adds built-in functions to the symbol table.
//
Sally::Sally(istream& input_stream) :
   istrm(input_stream)  // use member initializer to bind reference
{

   symtab["DUMP"]    =  SymTabEntry(KEYWORD,0,&doDUMP) ;

   symtab["+"]    =  SymTabEntry(KEYWORD,0,&doPlus) ;
   symtab["-"]    =  SymTabEntry(KEYWORD,0,&doMinus) ;
   symtab["*"]    =  SymTabEntry(KEYWORD,0,&doTimes) ;
   symtab["/"]    =  SymTabEntry(KEYWORD,0,&doDivide) ;
   symtab["%"]    =  SymTabEntry(KEYWORD,0,&doMod) ;
   symtab["NEG"]  =  SymTabEntry(KEYWORD,0,&doNEG) ;

   symtab["."]    =  SymTabEntry(KEYWORD,0,&doDot) ;
   symtab["SP"]   =  SymTabEntry(KEYWORD,0,&doSP) ;
   symtab["CR"]   =  SymTabEntry(KEYWORD,0,&doCR) ;

   symtab["DUP"] = SymTabEntry(KEYWORD, 0, &doDUP);
   symtab["DROP"] = SymTabEntry(KEYWORD, 0, &doDROP);
   symtab["SWAP"] = SymTabEntry(KEYWORD, 0, &doSWAP);
   symtab["ROT"] = SymTabEntry(KEYWORD, 0, &doROT);

   symtab["=="] = SymTabEntry(KEYWORD, 0, &checkEE);
   symtab["!="] = SymTabEntry(KEYWORD, 0, &checkNE);
   symtab["<"] = SymTabEntry(KEYWORD, 0, &checkLT);
   symtab["<="] = SymTabEntry(KEYWORD, 0, &checkLTE);
   symtab[">"] = SymTabEntry(KEYWORD, 0, &checkGT);
   symtab[">="] = SymTabEntry(KEYWORD, 0, &checkGTE);

   symtab["SET"] = SymTabEntry(KEYWORD, 0, &doSET);
   symtab["@"] = SymTabEntry(KEYWORD, 0, &doAT);
   symtab["!"] = SymTabEntry(KEYWORD, 0, &doEX);

   symtab["AND"] = SymTabEntry(KEYWORD, 0, &doAND);
   symtab["OR"] = SymTabEntry(KEYWORD, 0, &doOR);
   symtab["NOT"] = SymTabEntry(KEYWORD, 0, &doNOT);

   symtab["IFTHEN"] = SymTabEntry(KEYWORD, 0, &doIFTHEN);
   symtab["ELSE"] = SymTabEntry(KEYWORD, 0, &doELSE);
   symtab["ENDIF"] = SymTabEntry(KEYWORD, 0, &doENDIF);

   symtab["DO"] = SymTabEntry(KEYWORD, 0, &doDO);
   symtab["UNTIL"] = SymTabEntry(KEYWORD, 0, &doUNTIL);

   recorder = false;
}



// This function should be called when tkBuffer is empty.
// It adds tokens to tkBuffer.
//
// This function returns when an empty line was entered 
// or if the end-of-file has been reached.
//
// This function returns false when the end-of-file was encountered.
// 
// Processing done by fillBuffer()
//   - detects and ignores comments.
//   - detects string literals and combines as 1 token
//   - detetcs base 10 numbers
// 
//
bool Sally::fillBuffer() {
   string line ;     // single line of input
   int pos ;         // current position in the line
   int len ;         // # of char in current token
   long int n ;      // int value of token
   char *endPtr ;    // used with strtol()


   while(true) {    // keep reading until empty line read or eof

      // get one line from standard in
      //
      getline(istrm, line) ;   

      // if "normal" empty line encountered, return to mainLoop
      //
      if ( line.empty() && !istrm.eof() ) {
         return true ;
      }

      // if eof encountered, return to mainLoop, but say no more
      // input available
      //
      if ( istrm.eof() )  {
         return false ;
      }


      // Process line read

      pos = 0 ;                      // start from the beginning

      // skip over initial spaces & tabs
      //
      while( line[pos] != '\0' && (line[pos] == ' ' || line[pos] == '\t') ) {
         pos++ ; 
      }

      // Keep going until end of line
      //
      while (line[pos] != '\0') {

         // is it a comment?? skip rest of line.
         //
         if (line[pos] == '/' && line[pos+1] == '/') break ;

         // is it a string literal? 
         //
         if (line[pos] == '.' && line[pos+1] == '"') {

            pos += 2 ;  // skip over the ."
            len = 0 ;   // track length of literal

            // look for matching quote or end of line
            //
            while(line[pos+len] != '\0' && line[pos+len] != '"') {
               len++ ;
            }

            // make new string with characters from
            // line[pos] to line[pos+len-1]
            string literal(line,pos,len) ;  // copy from pos for len chars

            // Add to token list
            //
            tkBuffer.push_back( Token(STRING,0,literal) ) ;  

            // Different update if end reached or " found
            //
            if (line[pos+len] == '\0') {
               pos = pos + len ;
            } else {
               pos = pos + len + 1 ;
            }

         } else {  // otherwise "normal" token

            len = 0 ;  // track length of token

            // line[pos] should be an non-white space character
            // look for end of line or space or tab
            //
            while(line[pos+len] != '\0' && line[pos+len] != ' ' && line[pos+len] != '\t') {
               len++ ;
            }

            string literal(line,pos,len) ;   // copy form pos for len chars
            pos = pos + len ;

            // Try to convert to a number
            //
            n = strtol(literal.c_str(), &endPtr, 10) ;

            if (*endPtr == '\0') {
               tkBuffer.push_back( Token(INTEGER,n,literal) ) ;
            } else {
               tkBuffer.push_back( Token(UNKNOWN,0,literal) ) ;
            }
         }

         // skip over trailing spaces & tabs
         //
         while( line[pos] != '\0' && (line[pos] == ' ' || line[pos] == '\t') ) {
            pos++ ; 
         }

      }
   }
}



// Return next token from tkBuffer.
// Call fillBuffer() if needed.
// Checks for end-of-file and throws exception 
//
Token Sally::nextToken() {
      Token tk ;
      bool more = true ;

      

      while(more && tkBuffer.empty() ) {
         more = fillBuffer() ;
      }

      if ( !more && tkBuffer.empty() ) {
         throw EOProgram("End of Program") ;
      }

      
      tk = tkBuffer.front() ;
      
      if(recorder ==true){
	toDoList[toDoList.size()-1].push_back(tk);
      }

      tkBuffer.pop_front() ;
      return tk ;
}


// The main interpreter loop of the Sally Forth interpreter.
// It gets a token and either push the token onto the parameter
// stack or looks for it in the symbol table.
//
//
void Sally::mainLoop() {

   Token tk ;
   map<string,SymTabEntry>::iterator it ;

   try {
      while( 1 ) {
         tk = nextToken() ;
         if (tk.m_kind == INTEGER || tk.m_kind == STRING) {

            // if INTEGER or STRING just push onto stack
            params.push(tk) ;

         } else { 
            it = symtab.find(tk.m_text) ;
            
            if ( it == symtab.end() )  {   // not in symtab

               params.push(tk) ;

            } else if (it->second.m_kind == KEYWORD)  {

               // invoke the function for this operation
               //
               it->second.m_dothis(this) ;   
               
            } else if (it->second.m_kind == VARIABLE) {

               // variables are pushed as tokens
               //
               tk.m_kind = VARIABLE ;
               params.push(tk) ;

            } else {

               // default action
               //
               params.push(tk) ;

            }
         }
      }

   } catch (EOProgram& e) {

      cerr << "End of Program\n" ;
      if ( params.size() == 0 ) {
         cerr << "Parameter stack empty.\n" ;
      } else {
         cerr << "Parameter stack has " << params.size() << " token(s).\n" ;
      }

   } catch (out_of_range& e) {

      cerr << "Parameter stack underflow??\n" ;

   } catch (...) {

      cerr << "Unexpected exception caught\n" ;

   }
}

// -------------------------------------------------------


void Sally::doPlus(Sally *Sptr) {
   Token p1, p2 ;

   if ( Sptr->params.size() < 2 ) {
      throw out_of_range("Need two parameters for +.") ;
   }
   p1 = Sptr->params.top() ;
   Sptr->params.pop() ;
   p2 = Sptr->params.top() ;
   Sptr->params.pop() ;
   int answer = p2.m_value + p1.m_value ;
   Sptr->params.push( Token(INTEGER, answer, "") ) ;
}


void Sally::doMinus(Sally *Sptr) {
   Token p1, p2 ;

   if ( Sptr->params.size() < 2 ) {
      throw out_of_range("Need two parameters for -.") ;
   }
   p1 = Sptr->params.top() ;
   Sptr->params.pop() ;
   p2 = Sptr->params.top() ;
   Sptr->params.pop() ;
   int answer = p2.m_value - p1.m_value ;
   Sptr->params.push( Token(INTEGER, answer, "") ) ;
}


void Sally::doTimes(Sally *Sptr) {
   Token p1, p2 ;

   if ( Sptr->params.size() < 2 ) {
      throw out_of_range("Need two parameters for *.") ;
   }
   p1 = Sptr->params.top() ;
   Sptr->params.pop() ;
   p2 = Sptr->params.top() ;
   Sptr->params.pop() ;
   int answer = p2.m_value * p1.m_value ;
   Sptr->params.push( Token(INTEGER, answer, "") ) ;
}


void Sally::doDivide(Sally *Sptr) {
   Token p1, p2 ;

   if ( Sptr->params.size() < 2 ) {
      throw out_of_range("Need two parameters for /.") ;
   }
   p1 = Sptr->params.top() ;
   Sptr->params.pop() ;
   p2 = Sptr->params.top() ;
   Sptr->params.pop() ;
   int answer = p2.m_value / p1.m_value ;
   Sptr->params.push( Token(INTEGER, answer, "") ) ;
}


void Sally::doMod(Sally *Sptr) {
   Token p1, p2 ;

   if ( Sptr->params.size() < 2 ) {
      throw out_of_range("Need two parameters for %.") ;
   }
   p1 = Sptr->params.top() ;
   Sptr->params.pop() ;
   p2 = Sptr->params.top() ;
   Sptr->params.pop() ;
   int answer = p2.m_value % p1.m_value ;
   Sptr->params.push( Token(INTEGER, answer, "") ) ;
}


void Sally::doNEG(Sally *Sptr) {
   Token p ;

   if ( Sptr->params.size() < 1 ) {
      throw out_of_range("Need one parameter for NEG.") ;
   }
   p = Sptr->params.top() ;
   Sptr->params.pop() ;
   Sptr->params.push( Token(INTEGER, -p.m_value, "") ) ;
}


void Sally::doDot(Sally *Sptr) {

   Token p ;
   if ( Sptr->params.size() < 1 ) {
      throw out_of_range("Need one parameter for .") ;
   }

   p = Sptr->params.top() ;
   Sptr->params.pop() ;

   if (p.m_kind == INTEGER) {
      cout << p.m_value ;
   } else {
      cout << p.m_text ;
   }
}


void Sally::doSP(Sally *Sptr) {
   cout << " " ;
}


void Sally::doCR(Sally *Sptr) {
   cout << endl ;
}

void Sally::doDUMP(Sally *Sptr) {
   // do whatever for debugging
} 


void Sally::doDUP(Sally *Sptr) {
  Token p;

  if (Sptr->params.size() < 1) {
    throw out_of_range("Need atleast one parameter for DUP");
  }

  p = Sptr->params.top();
  //add a copy of the item on the top of the stack to the top of the stack
  Sptr->params.push(p);
}

void Sally::doDROP(Sally *Sptr) {

  if (Sptr->params.size() < 1) {
    throw out_of_range("Need atleast one parameter for DROP");
  }

  Sptr->params.pop();
}

void Sally::doSWAP(Sally *Sptr) {
  Token p;
  Token q;

  if (Sptr->params.size() < 2) {
    throw out_of_range("Need atleast two parameters for DUP");
  }

  //take two items off the top of the stack
  p = Sptr->params.top();
  Sptr->params.pop();

  q = Sptr->params.top();
  Sptr->params.pop();

  //push them back in, with the first one now.
  //previous: p,q,stack...
  Sptr->params.push(p);
  Sptr->params.push(q);
  //now: q,p,stack...

}

void Sally::doROT(Sally *Sptr) {
  Token p;
  Token q;
  Token r;

  if (Sptr->params.size() < 3) {
    throw out_of_range("Need atleast three parameters for SWAP");
  }

  //take two items off the top of the stack
  p = Sptr->params.top();
  Sptr->params.pop();

  q = Sptr->params.top();
  Sptr->params.pop();

  r = Sptr->params.top();
  Sptr->params.pop();

  //push them back in, with the first one now.
  //previous: p,q,r,  stack...
  Sptr->params.push(q);
  Sptr->params.push(p);
  Sptr->params.push(r);

  //now: q,p,stack...

}


void Sally::checkEE(Sally *Sptr) {
  Token p1, p2;

  if (Sptr->params.size() < 2) {
    throw out_of_range("Need two parameters for ==");
  }
  p1 = Sptr->params.top();
  Sptr->params.pop();
  p2 = Sptr->params.top();
  Sptr->params.pop();

  //should i check for text or value here?
  //settling for value since the documentation doesn't mention comparing text
  int answer = (p2.m_value == p1.m_value);
  Sptr->params.push(Token(INTEGER, answer, ""));
}

void Sally::checkNE(Sally *Sptr) {
  Token p1, p2;

  if (Sptr->params.size() < 2) {
    throw out_of_range("Need two parameters for !=");
  }
  p1 = Sptr->params.top();
  Sptr->params.pop();
  p2 = Sptr->params.top();
  Sptr->params.pop();

  //should i check for text or value here?
  //settling for value since the documentation doesn't mention comparing text
  int answer = (p2.m_value != p1.m_value);
  Sptr->params.push(Token(INTEGER, answer, ""));
}

void Sally::checkLT(Sally *Sptr) {
  Token p1, p2;

  if (Sptr->params.size() < 2) {
    throw out_of_range("Need two parameters for <");
  }
  p1 = Sptr->params.top();
  Sptr->params.pop();
  p2 = Sptr->params.top();
  Sptr->params.pop();


  //settling for value since the documentation doesn't mention comparing text
  int answer = (p2.m_value < p1.m_value);
  Sptr->params.push(Token(INTEGER, answer, ""));
}

void Sally::checkLTE(Sally *Sptr) {
  Token p1, p2;

  if (Sptr->params.size() < 2) {
    throw out_of_range("Need two parameters for <=");
  }
  p1 = Sptr->params.top();
  Sptr->params.pop();
  p2 = Sptr->params.top();
  Sptr->params.pop();

  //settling for value since the documentation doesn't mention comparing text
  int answer = (p2.m_value <= p1.m_value);
  Sptr->params.push(Token(INTEGER, answer, ""));
}


void Sally::checkGT(Sally *Sptr) {
  Token p1, p2;

  if (Sptr->params.size() < 2) {
    throw out_of_range("Need two parameters for >");
  }
  p1 = Sptr->params.top();
  Sptr->params.pop();
  p2 = Sptr->params.top();
  Sptr->params.pop();

  //settling for value since the documentation doesn't mention comparing text
  int answer = (p2.m_value > p1.m_value);
  Sptr->params.push(Token(INTEGER, answer, ""));
}

void Sally::checkGTE(Sally *Sptr) {
  Token p1, p2;

  if (Sptr->params.size() < 2) {
    throw out_of_range("Need two parameters for >=");
  }
  p1 = Sptr->params.top();
  Sptr->params.pop();
  p2 = Sptr->params.top();
  Sptr->params.pop();

  //settling for value since the documentation doesn't mention comparing text
  int answer = (p2.m_value >= p1.m_value);
  Sptr->params.push(Token(INTEGER, answer, ""));
}


void Sally::doSET(Sally *Sptr) {
  Token p1, p2;

  if (Sptr->params.size() < 2) {
    throw out_of_range("Need two parameters for SET");
  }


  p1 = Sptr->params.top();
  Sptr->params.pop();
  p2 = Sptr->params.top();
  Sptr->params.pop();

  map<string,SymTabEntry>::iterator it ;
  it =  Sptr->symtab.find(p1.m_text);

  //if the variable is not already in the symbol table then add it to the symbol table
  if(it == Sptr->symtab.end()){
    string newVar = p1.m_text;
    Sptr->symtab[newVar] = SymTabEntry(VARIABLE,p2.m_value,NULL);
  }
  
  //otherwise say that the variable is already defined
  else{
    cout<< "variable: " << p1.m_text<< "has already been set"<< endl;
  }

}


void Sally::doAT(Sally *Sptr) {
  Token p1 ;

  if (Sptr->params.size() < 1) {
    throw out_of_range("Need atleast one parameters for @");
  }

  //get the variable
  p1 = Sptr->params.top();
  Sptr->params.pop();

  //make an iterator to search for the variable 
  map<string,SymTabEntry>::iterator it ;
  it =  Sptr->symtab.find(p1.m_text);
  
  //see if the variable is in the symbol table first, if not print error
  if(it == Sptr->symtab.end()){
    cout<< "variable not found"<<endl;
  }

  //otherwise the item is in the stack and we need to add teh value to the stack
  Sptr->params.push( Token(INTEGER,it->second.m_value , "") ) ;

}



void Sally::doEX(Sally *Sptr) {
  Token p1, p2;

  if (Sptr->params.size() < 2) {
    throw out_of_range("Need two parameters for !");
  }

  //get teh variable and the value to set it to
  p1 = Sptr->params.top();
  Sptr->params.pop();
  p2 = Sptr->params.top();
  Sptr->params.pop();

  //create an iterator to search for the variable
  map<string,SymTabEntry>::iterator it ;
  it =  Sptr->symtab.find(p1.m_text);

  //if the variable is not already in the symbol table then add it to the symbol table
  if(it == Sptr->symtab.end()){
    cout<< "variable has not been declared yet"<< endl;
  }
  
  //otherwise the variable exists and we can redefine its value
  else{
    string newVar = p1.m_text;
    Sptr->symtab[newVar] = SymTabEntry(VARIABLE,p2.m_value,NULL);

  }

}


void Sally::doAND(Sally *Sptr) {
  Token p1, p2;

  //ERRORCHECK
  if (Sptr->params.size() < 2) {
    throw out_of_range("Need two parameters for AND");
  }

  p1 = Sptr->params.top();
  Sptr->params.pop();
  p2 = Sptr->params.top();
  Sptr->params.pop();

  //true only if p1 and p2 are true
  if((p2.m_value == 1) and (p1.m_value == 1)){
    //push a true onto the stack
    Sptr->params.push(Token(INTEGER, 1, ""));    
  }
  else{
    //otherwise push a false onto the stack
      Sptr->params.push(Token(INTEGER, 0, ""));
  }

}

void Sally::doOR(Sally *Sptr) {
  Token p1, p2;

  //ERRORCHECK
  if (Sptr->params.size() < 2) {
    throw out_of_range("Need two parameters for OR");
  }

  p1 = Sptr->params.top();
  Sptr->params.pop();
  p2 = Sptr->params.top();
  Sptr->params.pop();

  //false only if p1 and p2 are false
  if((p2.m_value == 0) and (p1.m_value == 0)){
    //push a false onto the stack
    Sptr->params.push(Token(INTEGER, 0, ""));    
  }
  else{
    //otherwise push a true onto the stack
      Sptr->params.push(Token(INTEGER, 1, ""));
  }

}

void Sally::doNOT(Sally *Sptr) {
  Token p1;

  //ERRORCHECK
  if (Sptr->params.size() < 1) {
    throw out_of_range("Need atleast one parameters for NOT");
  }

  p1 = Sptr->params.top();
  Sptr->params.pop();

  //if p1 = 0 , set to 1
  if((p1.m_value == 0)){
    //push a 1 onto the stack
    Sptr->params.push(Token(INTEGER, 1, ""));    
  }
  else{
    //otherwise push a 0 onto the stack
      Sptr->params.push(Token(INTEGER, 0, ""));
  }
}

void Sally::doIFTHEN(Sally *Sptr) {
  Token p1;

  //ERRORCHECK
  if (Sptr->params.size() < 1) {
    throw out_of_range("Need atleast one parameter for IFTHEN");
  }

  p1 = Sptr->params.top();
  Sptr->params.pop();

  //these were added in order to handle for nested ifthens
  bool notDone = true;
  int myCounter = 0;

  //if true
  if((p1.m_value != 0)){
    return;
    //this way we continue to the rest of the code
  }
  else{
    //otherwise we continue popping off the stack until we hit else
    Token tk = Token(INTEGER, 0, "");
      

    //loop until done
    while(notDone){

      tk = Sptr->nextToken();
    
      if(tk.m_text == "IFTHEN"){
	myCounter++;
	}
      else if( tk.m_text == "ELSE"){
	myCounter--;
      }
      if(myCounter<0){
	notDone = false;
      }
    }

  }
  //once the while loop stops running, it means we passed enough matching elses
}



void Sally::doELSE(Sally *Sptr) {
  
  //if we actually encounter an else, it means that the IFTHEN has already
  //executed.

  //this means we just consume tokens until the next endif
  
  Token tk = Token(INTEGER, 0, "");
    
    //consume tokens until endif
    while(tk.m_text != "ENDIF"){
      
      tk = Sptr->nextToken();
      
    }
 //once the while loop stops running, it means we have consumed a matching endif
//and we can return to the main loop

}
 

void Sally::doENDIF(Sally *Sptr) {
  //do nothing here
}



void Sally::doDO(Sally *Sptr) {
  //set recording to true and clear the old list in case a previous loop
  //happened

  Sptr->recorder = true;

  list<Token> anotherList;

  Sptr-> toDoList.push_back(anotherList);
  

}

void Sally::doUNTIL(Sally *Sptr) {

  Token t1;

  t1 = Sptr->params.top();
  Sptr->params.pop();

  //we continue the loop while the previous variable is false
  if(t1.m_value == 0){

    //if we need to do the loop again we put the contents of the list into 
    //the beginning of the buffer
    Sptr->tkBuffer.splice(Sptr->tkBuffer.begin(), Sptr->toDoList[Sptr->toDoList.size()-1]);

  }
  else{

    Sptr->toDoList.pop_back();

    if(Sptr->toDoList.size() ==0){
    Sptr->recorder = false;
    }
  }


}
