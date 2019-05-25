#include <iostream>
#include <string>
#include <fstream>
#include "tokens.h"
#include "parse.h"
#include "tree.h"
using namespace std;

int main(int argc, char* argv[]){
  istream *in = &cin;
  ifstream infile;
  int linenum = 0;

  if(argc > 2){
    cerr << "TOO MANY FILENAMES" << endl;
    return -1;
  }
  
  else if(argc == 2 && in == &cin){
    infile.open(argv[1]);
    if(!infile.is_open()){
      cerr << "COULD NOT OPEN " << argv[1] << endl;
      return -1;
    }
    in = &infile;
  }

  ParseTree *prog = Prog(*in, linenum);
  if(prog == 0){
    return 0;
  }
  
  try{
    prog->Eval();
  }
  catch(std::logic_error& e){
    cout << "RUNTIME ERROR " << e.what() << endl;
  }
    
  
  return 0;
}