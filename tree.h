#ifndef TREE_H_
#define TREE_H_

#include <vector>
#include <map>
#include <string>
#include "value.h"
using std::vector;
using std::map;
using std::string;

static map<string, Value> SymbolTable;
// NodeType represents all possible types
enum NodeType { ERRTYPE, INTTYPE, STRTYPE };

// a "forward declaration" for a class to hold values
class Value;

class ParseTree {
	int		linenum;

protected:
	ParseTree	*left;
	ParseTree	*right;

public:
	ParseTree(int linenum, ParseTree *l = 0, ParseTree *r = 0)
		: linenum(linenum), left(l), right(r) {}

	virtual ~ParseTree() {
		delete left;
		delete right;
	}

	int GetLinenum() const { return linenum; }
	ParseTree *GetLeft() const { return left; }
	ParseTree *GetRight() const { return right; }

	int NodeCount() const {
		int nc = 0;
		if(left){
      nc += left->NodeCount();
    }
		if(right){
      nc += right->NodeCount();
    }
		return nc + 1;
	}

	int LeafCount() const {
    int lc = 0;
    if(left){
      lc += left->LeafCount();
    }
    if(right){
      lc += right->LeafCount();
    }
    if(left == 0 && right == 0){
      lc++;
    }
    return lc;
  }

	int OpsCount() const{
		int Opcnt = 0;
		if(left){
      Opcnt += left->OpsCount();
    }
		if(right){
      Opcnt += right->OpsCount();
    }
		if(IsOperator()){
			Opcnt++;
    }
		return Opcnt;
	}

	int StringCount() const {
		int cnt = 0;
		if(left){
      cnt += left->StringCount();
    }
		if(right){
      cnt += right->StringCount();
    }
		if(IsString()){
			cnt++;
    }
		return cnt;
	}

	int MaxDepth() const{
    int lDepth = 0;
    int rDepth = 0;
		if(left){
      lDepth += left->MaxDepth();
    }
		if(right){
      rDepth += right->MaxDepth();
    }
    if(lDepth > rDepth){
      return(lDepth + 1);
    }
    else{
      return(rDepth + 1);  
    }
  }    

  virtual bool IsString() const { return false; }
	virtual bool IsOperator() const { return false; }
	virtual Value Eval () const = 0;
};

class StmtList : public ParseTree{

public:
	StmtList(ParseTree *l, ParseTree *r) : ParseTree(0, l, r) {}
	 Value Eval() const {
		left->Eval();
		if(right){
      right->Eval();
		}
		return Value();
	}
};

class IfStatement : public ParseTree {
public:
	IfStatement(int line, ParseTree *ex, ParseTree *stmt) : ParseTree(line, ex, stmt) {}

	Value Eval() const {
    Value leftVal = left->Eval();
    if(leftVal.isInt()){
      if(left->Eval().GetInt()!= 0){
        right->Eval();
      }
    }
    else{
      throw std::logic_error("Conditional is not an integer");
    }
    return Value();
	}
};

class SetStatement : public ParseTree{
  string id;
public:
	SetStatement(int line, string id, ParseTree *expr) : ParseTree(line, expr), id(id) {}
	Value Eval() const{
    SymbolTable[id] = left->Eval();
		return Value();
	}
};

class PrintStatement : public ParseTree {
public:
	PrintStatement(int line, ParseTree *e) : ParseTree(line, e) {}

	Value Eval() const {Value val = left->Eval();
		cout << val;
		return val;
	}
};

class LoopStatement : public ParseTree {
public:
	LoopStatement(int line, ParseTree *ex, ParseTree *stmt) : ParseTree(line, ex, stmt) {}

	Value Eval() const {
		if(left->Eval().isInt()){
			while(left->Eval().GetInt() != 0){
				right->Eval();
			}
		}
		else{
			throw std::logic_error("Type loop is not of integer type");
		}
		return Value();
	}
};

class Addition : public ParseTree {
public:
	Addition(int line, ParseTree *l, ParseTree *r) : ParseTree(line,l,r) {}
	bool IsOperator() const { return true; }

	Value Eval() const {
		auto a1 = left->Eval();
		auto a2 = right->Eval();

		if(a1.isInt() && a2.isInt()){
			return Value(a1.GetInt() + a2.GetInt());
		}
		else if(a1.isStr() && a2.isStr()){
			return Value(a1.GetStr() + a2.GetStr());
		}
		else{
			throw std::logic_error("Type mismatch for arguments of +");
		}
	}
};

class Subtraction : public ParseTree {
public:
	Subtraction(int line, ParseTree *l, ParseTree *r) : ParseTree(line,l,r) {}
	bool IsOperator() const { return true; }

	Value Eval() const {
		auto a1 = left->Eval();
		auto a2 = right->Eval();

		if(a1.isInt() && a2.isInt()){
			return Value(a1.GetInt() - a2.GetInt());
		}
		else{
			throw std::logic_error("Type mismatch for arguments of -");
		}
	}
};

class Multiplication : public ParseTree {
public:
	Multiplication(int line, ParseTree *l, ParseTree *r) : ParseTree(line,l,r) {}
	bool IsOperator() const { return true; }

	Value Eval() const {
		auto a1 = left->Eval();
		auto a2 = right->Eval();

		if(a1.isInt() && a2.isInt()){
			return Value(a1.GetInt() * a2.GetInt());
		}
		else if(a1.isStr() && a2.isInt()){
          string ret;
          if (a2.GetInt()<0){
            throw std::logic_error("Repetition count less than 0");
          }
          for(int i=0; i<a2.GetInt(); i++){
            ret += a1.GetStr();
          }
			return Value(ret);
		}
        else if(a1.isInt() && a2.isStr()){
          string ret;
          if (a1.GetInt()<0){
            throw std::logic_error("Repetition count less than 0");
          }
          for(int i=0; i<a1.GetInt(); i++){
            ret += a2.GetStr();
          }
			return Value(ret);
        }
      
		else{
			throw std::logic_error("Type mismatch for arguments of *");
		}
		return Value();
	}
};

class Division : public ParseTree {
public:
	Division(int line, ParseTree *l, ParseTree *r) : ParseTree(line,l,r) {}
	bool IsOperator() const { return true; }
	
	Value Eval() const {
		auto a1 = left->Eval();
		auto a2 = right->Eval();

		if(a1.isInt() && a2.isInt()){
          if (a2.GetInt()!= 0){
			return Value(a1.GetInt() / a2.GetInt());
          }
          else{
            throw std::logic_error("Divide by zero error");
		  }
        }
		else{
			throw std::logic_error("Type mismatch for arguments of /");
		}
	}
};

class IConst : public ParseTree {
	int val;

public:
	IConst(Token& t) : ParseTree(t.GetLinenum()) {
		val = stoi(t.GetLexeme());
	}

	Value Eval() const {return Value(val);}
};

class SConst : public ParseTree {
	string val;

public:
	SConst(Token& t) : ParseTree(t.GetLinenum()) {
		val = t.GetLexeme();
	}
    
    Value Eval() const {return Value(val);}
	bool IsString() const { return true; }
};

class Ident : public ParseTree {
	string id;

public:
	Ident(Token& t) : ParseTree(t.GetLinenum()), id(t.GetLexeme()) {}
    Value Eval() const {
      if(SymbolTable.find(id) == SymbolTable.end()){
        throw std::logic_error("Symbol " + id + " not defined");
      }
      else{
        return SymbolTable[id];
      }
    }
	bool IsIdent() const { return true; }
};

#endif /* TREE_H_ */
