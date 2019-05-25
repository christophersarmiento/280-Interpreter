#include "parse.h"
#include "gettoken.h"
#include "tree.h"

static int error_count = 0;

void ParseError(int line, string msg){
  ++error_count;
  cout << line+1 << ": " << msg << endl;
}

// Prog is an Slist
ParseTree *Prog(istream& in, int& line){
  ParseTree *sl = Slist(in, line);
    
  if(GetToken::Get(in, line) != DONE){
    ParseError(line, "Unrecognized statement");
  }
  if(sl == 0){
    ParseError(line, "No statements in program");
  }
  if(error_count){
    return 0;
  }
  return sl;
}

// Ssep { Slist } | Stmt Ssep { Slist }
ParseTree *Slist(istream& in, int& line) {
  ParseTree *s = Stmt(in, line);
  if(s == 0){
    return 0;
  }

  Token t;
  if((t = GetToken::Get(in, line)) != SC && t != NL){
    ParseError(line, "Missing statement separator");
    return 0;
  }
    return new StmtList(s, Slist(in,line));
}

ParseTree *Stmt(istream& in, int& line) {
  ParseTree *s = 0;

  Token t = GetToken::Get(in, line);
  switch( t.GetTokenType() ) {
  case IF:
    s = IfStmt(in, line);
    break;

  case PRINT:
    s = PrintStmt(in, line);
    break;

  case SET:
    s = SetStmt(in, line);
    break;

  case LOOP:
    s = LoopStmt(in, line);
    break;

  case ERR:
    ParseError(line, "Invalid token");
    break;

  case DONE:
    break;

  case NL:
  case SC:
    // an empty statement; get the next one
    s = Stmt(in, line);
    break;

  default:
    GetToken::PushBack(t);
    break;
  }

  return s;
}

ParseTree *IfStmt(istream& in, int& line) {
  ParseTree* expr = Expr(in, line);
  if(expr == 0){
    ParseError(line, "Expected expression after IF");
    return 0;
  }
  Token t = GetToken::Get(in, line);
  if(t != BEGIN){
    ParseError(line, "Expected BEGIN token after IF");
    return 0;
  }
  ParseTree *ifList = Slist(in, line);
  if(ifList == 0){
    ParseError(line, "Expected statement(s) after then");
    return 0;
  }
  t = GetToken::Get(in, line);
  if(t != END){
    ParseError(line, "Expected END after statement");
    return 0;
  }
  return new IfStatement(line, expr, ifList);
}

//LoopStmt := LOOP Expr BEGIN Slist END
ParseTree *LoopStmt(istream& in, int& line) {
  ParseTree* expr = Expr(in, line);
  if(expr == 0){
    ParseError(line, "Expected expression after LOOP");
    return 0;
  }
  Token t = GetToken::Get(in, line);
  if(t != BEGIN){
    ParseError(line, "Expected BEGIN after LOOP");
    return 0;
  }
  ParseTree *loopList = Slist(in, line);
  if(loopList == 0){
    ParseError(line, "Expected statement after LOOP");
    return 0;
  }
  t = GetToken::Get(in, line);
  if(t != END){
    ParseError(line, "Expected END after statement");
    return 0;
  }
  return new LoopStatement(line, expr, loopList);
}
// SetStmt := SET IDENT Expr
ParseTree *SetStmt(istream& in, int& line) {
  Token t = GetToken::Get(in, line);
  if(t != ID){
    cout << t << endl;
    ParseError(line, "Expected Identifier");
    return 0;
  }
  ParseTree* expr = Expr(in, line);
  if(expr == 0){
    ParseError(line, "Expected expression after PRINT");
    return 0;
  }
  return new SetStatement(line, t.GetLexeme(), expr);
}
// PrintStmt := PRINT Expr
ParseTree *PrintStmt(istream& in, int& line) {
  ParseTree* expr = Expr(in, line);
  if(expr == 0){
    ParseError(line, "Expected expression after PRINT");
    return 0;
  }
  return new PrintStatement(line, expr);
}

ParseTree *Expr(istream& in, int& line) {
  ParseTree *t1 = Prod(in, line);
  if(t1 == 0){
    return 0;
  }

  while(true){
    Token t = GetToken::Get(in, line);

    if(t != PLUS && t != MINUS){
      GetToken::PushBack(t);
      return t1;
    }

    ParseTree *t2 = Prod(in, line);
    if(t2 == 0) {
      ParseError(line, "Missing expression after operator");
      return 0;
    }

    if(t == PLUS){
      t1 = new Addition(t.GetLinenum(), t1, t2);
    }
    else{
      t1 = new Subtraction(t.GetLinenum(), t1, t2);
    }
  }
}

ParseTree *Prod(istream& in, int& line) {
  ParseTree *t1 = Primary(in, line);

  if(t1 == 0){
    return 0;
  }

  while(true){
    Token t = GetToken::Get(in, line);

    if(t != SLASH && t != STAR){
      GetToken::PushBack(t);
      return t1;
    }

    ParseTree *t2 = Primary(in, line);
    if(t2 == 0) {
      ParseError(line, "Missing expression after operator");
      return 0;
    }

    if(t == STAR){
      t1 = new Multiplication(t.GetLinenum(), t1, t2);
    }
    else{
      t1 = new Division(t.GetLinenum(), t1, t2);
    }
  }
      
}

ParseTree *Primary(istream& in, int& line) {
  Token t = GetToken::Get(in, line);
  
  switch(t.GetTokenType()){
  case ID:
    return new Ident(t);
    break;

  case SCONST:
    return new SConst(t);
    break;

  case ICONST:
    return new IConst(t);
    break;
      
  case LPAREN:{
    ParseTree *expr = Expr(in, line);
    if(expr == 0) {
      ParseError(line, "Missing expression after (");
      return 0;
      break;
    }
    if(GetToken::Get(in, line) == RPAREN){
      return expr;
    }
    ParseError(line, "Missing ) after expression");
    return 0;}
      
  default:
    ParseError(line, "Expected Primary token");
    return 0;
  }
}