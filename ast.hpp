#ifndef __AST_HPP__
#define __AST_HPP__ 1

#include <string>
#include <vector>
using namespace std;

#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Support/FileSystem.h"

using namespace llvm;
using namespace llvm::legacy;

class ExprAST {
public:
  virtual ~ExprAST();
  virtual Value* codegen() const = 0;
};

class NumberExprAST : public ExprAST {
public:
  NumberExprAST(int v)
    :Val(v)
  {}
  Value* codegen() const;
private:
  int Val;
};

class VariableExprAST : public ExprAST {
public:
  VariableExprAST(string s)
    :Name(s)
  {}
  Value* codegen() const;
private:
  string Name;
};

class InnerExprAST : public ExprAST {
public:
  InnerExprAST(ExprAST* a);
  InnerExprAST(ExprAST* a, ExprAST* b);
  InnerExprAST(ExprAST* a, ExprAST* b, ExprAST* c);
  InnerExprAST(ExprAST* a, ExprAST* b, ExprAST* c, ExprAST* d);
  InnerExprAST(vector<ExprAST*> a);
  ~InnerExprAST();
protected:
  vector<ExprAST*> _nodes;
private:
  InnerExprAST(const InnerExprAST& i);
  InnerExprAST& operator=(const InnerExprAST& i);
};

class AndExprAST : public InnerExprAST {
public:
  AndExprAST(ExprAST* a, ExprAST* b)
    :InnerExprAST(a, b)
  {}
  Value* codegen() const;
};

class OrExprAST : public InnerExprAST {
public:
  OrExprAST(ExprAST* a, ExprAST* b)
    :InnerExprAST(a, b)
  {}
  Value* codegen() const;
};

class XorExprAST : public InnerExprAST {
public:
  XorExprAST(ExprAST* a, ExprAST* b)
    :InnerExprAST(a, b)
  {}
  Value* codegen() const;
};

class ShlExprAST : public InnerExprAST {
public:
  ShlExprAST(ExprAST* a, ExprAST* b)
    :InnerExprAST(a, b)
  {}
  Value* codegen() const;
};

class ShrExprAST : public InnerExprAST {
public:
  ShrExprAST(ExprAST* a, ExprAST* b)
    :InnerExprAST(a, b)
  {}
  Value* codegen() const;
};

class NotExprAST : public InnerExprAST {
public:
  NotExprAST(ExprAST* a)
    :InnerExprAST(a)
  {}
  Value* codegen() const;
};

class PrintExprAST : public InnerExprAST {
public:
  PrintExprAST(ExprAST* a)
    :InnerExprAST(a)
  {}
  Value* codegen() const;
};

class SetExprAST : public InnerExprAST {
public:
  SetExprAST(ExprAST* a, string n)
    :InnerExprAST(a), Name(n)
  {}
  Value* codegen() const;
private:
  string Name;
};

class SeqExprAST : public InnerExprAST {
public:
  SeqExprAST(vector<ExprAST*> a)
    :InnerExprAST(a)
  {}
  Value* codegen() const;
};

class CallExprAST : public InnerExprAST {
public:
  CallExprAST(string f, vector<ExprAST*> v)
    : InnerExprAST(v), _f(f)
  {}
  Value* codegen() const;
private:
  string _f;
};

class WhileExprAST : public InnerExprAST {
public:
  WhileExprAST(ExprAST* cond, ExprAST *n)
    :InnerExprAST(cond, n)
  {}
  Value* codegen() const;
};

class AddExprAST : public InnerExprAST {
public:
  AddExprAST(ExprAST* e1, ExprAST* e2)
    :InnerExprAST(e1, e2)
  {}
  Value* codegen() const;
};

class LessThenExprAST : public InnerExprAST {
public:
  LessThenExprAST(ExprAST* e1, ExprAST* e2)
    :InnerExprAST(e1, e2)
  {}
  Value* codegen() const;
};

class FunctionAST {
public:
  FunctionAST(string f, vector<string> args, ExprAST *e)
    :_f(f), _args(args), _e(e)
  {}
  ~FunctionAST();
  Function* codegen() const;
private:
  FunctionAST(const FunctionAST&);
  FunctionAST& operator=(const FunctionAST&);
  string _f;
  vector<string> _args;
  ExprAST *_e;
};

void InitializeModuleAndPassManager(void);
AllocaInst *CreateEntryBlockAlloca(Function *TheFunction, const string &VarName);


#endif

