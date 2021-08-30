#include "ast.hpp"

#include <iostream>


LLVMContext TheContext;
map<string, AllocaInst*> NamedValues;
IRBuilder<> Builder(TheContext);
Module* TheModule;
llvm::legacy::FunctionPassManager *TheFPM;

ExprAST::~ExprAST() {

}

InnerExprAST::InnerExprAST(ExprAST* a) {
  _nodes.resize(0);
  _nodes.push_back(a);
}

InnerExprAST::InnerExprAST(ExprAST* a, ExprAST* b) {
  _nodes.resize(0);
  _nodes.push_back(a);
  _nodes.push_back(b);
}

InnerExprAST::InnerExprAST(ExprAST* a, ExprAST* b, ExprAST* c) {
  _nodes.resize(0);
  _nodes.push_back(a);
  _nodes.push_back(b);
  _nodes.push_back(c);
}

InnerExprAST::InnerExprAST(ExprAST* a, ExprAST* b, ExprAST* c, ExprAST* d) {
  _nodes.resize(0);
  _nodes.push_back(a);
  _nodes.push_back(b);
  _nodes.push_back(c);
  _nodes.push_back(d);
}

InnerExprAST::InnerExprAST(vector<ExprAST*> a) {
  _nodes = a;
}

InnerExprAST::~InnerExprAST() {
  for (unsigned i = 0; i < _nodes.size(); i++)
    delete _nodes[i];
}

Value* NumberExprAST::codegen() const {
  return ConstantInt::get(TheContext, APInt(32, Val));
}

Value* VariableExprAST::codegen() const {
  AllocaInst* tmp = NamedValues[Name];
  if (tmp == NULL) {
    cerr << "Promenljiva " + Name + " nije definisana" << endl;
    return NULL;
  }
  return Builder.CreateLoad(Type::getInt32Ty(TheContext), tmp, Name.c_str());
}

Value* AndExprAST::codegen() const {
  Value *l = _nodes[0]->codegen();
  Value *d = _nodes[1]->codegen();
  if (l == NULL || d == NULL)
    return NULL;
  return Builder.CreateAnd(l, d, "andtmp");
}

Value* OrExprAST::codegen() const {
  Value *l = _nodes[0]->codegen();
  Value *d = _nodes[1]->codegen();
  if (l == NULL || d == NULL)
    return NULL;
  return Builder.CreateOr(l, d, "ortmp");
}

Value* XorExprAST::codegen() const {
  Value *l = _nodes[0]->codegen();
  Value *d = _nodes[1]->codegen();
  if (l == NULL || d == NULL)
    return NULL;
  return Builder.CreateXor(l, d, "xortmp");
}

Value* ShlExprAST::codegen() const {
  Value *l = _nodes[0]->codegen();
  Value *d = _nodes[1]->codegen();
  if (l == NULL || d == NULL)
    return NULL;
  return Builder.CreateShl(l, d, "shltmp");
}

Value* ShrExprAST::codegen() const {
  Value *l = _nodes[0]->codegen();
  Value *d = _nodes[1]->codegen();
  if (l == NULL || d == NULL)
    return NULL;
  return Builder.CreateLShr(l, d, "shrtmp");
}

Value* NotExprAST::codegen() const {
  Value *l = _nodes[0]->codegen();
  if (l == NULL)
    return NULL;
  return Builder.CreateNot(l, "nottmp");
}

extern Value* Str;
extern Value* Str1;
extern Function *PrintfFja;

Value* PrintExprAST::codegen() const {
  Value *l = _nodes[0]->codegen();
  if (l == NULL)
    return NULL;

  /* globalni stringovi potreban za ispis */
  if (Str == NULL) {
    Str = Builder.CreateGlobalStringPtr("%d\n");
  }  
  if (Str1 == NULL) {
    Str1 = Builder.CreateGlobalStringPtr("0x%x\n");
  }
  
  AllocaInst* Alloca = NamedValues["flag"];
  if (Alloca == NULL) {
    Function *F = Builder.GetInsertBlock()->getParent();
    Alloca = CreateEntryBlockAlloca(F, "flag");
    NamedValues["flag"] = Alloca;
    Builder.CreateStore(ConstantInt::get(TheContext, APInt(32, 0)), Alloca);
  }
  
  Value *tmp = Builder.CreateLoad(Type::getInt32Ty(TheContext), Alloca, "flag");
  Value *CondV = Builder.CreateICmpEQ(tmp, ConstantInt::get(TheContext, APInt(32, 0)));

  Function *TheFunction = Builder.GetInsertBlock()->getParent();
  BasicBlock *ThenBB =
    BasicBlock::Create(TheContext, "then", TheFunction);
  BasicBlock *ElseBB = BasicBlock::Create(TheContext, "else");
  BasicBlock *MergeBB = BasicBlock::Create(TheContext, "ifcont");
  
  Builder.CreateCondBr(CondV, ThenBB, ElseBB);
  Builder.SetInsertPoint(ThenBB);
  vector<Value *> ArgsV;
  ArgsV.push_back(Str);
  ArgsV.push_back(l);
  Builder.CreateCall(PrintfFja, ArgsV, "printfCall");
  Builder.CreateBr(MergeBB);
  ThenBB = Builder.GetInsertBlock();

  TheFunction->getBasicBlockList().push_back(ElseBB);
  Builder.SetInsertPoint(ElseBB);
  ArgsV.clear();
  ArgsV.push_back(Str1);
  ArgsV.push_back(l);
  Builder.CreateCall(PrintfFja, ArgsV, "printfCall");
  Builder.CreateBr(MergeBB);
  ElseBB = Builder.GetInsertBlock();

  TheFunction->getBasicBlockList().push_back(MergeBB);
  Builder.SetInsertPoint(MergeBB);
  return l;
}

Value* SetExprAST::codegen() const {
  Value *l = _nodes[0]->codegen();
  if (l == NULL)
    return NULL;
  AllocaInst* Alloca = NamedValues[Name];
  if (Alloca == NULL) {
    Function *F = Builder.GetInsertBlock()->getParent();
    Alloca = CreateEntryBlockAlloca(F, Name);
    NamedValues[Name] = Alloca;
  }
  return Builder.CreateStore(l, Alloca);
}

Value* SeqExprAST::codegen() const {
  Value *tmp = NULL;
  for (unsigned i = 0; i < _nodes.size(); i++) {
    tmp = _nodes[i]->codegen();
    if (tmp == NULL)
      return NULL;
  }
  return tmp;
}

Value* CallExprAST::codegen() const {
  Function* F = TheModule->getFunction(_f);
  if (!F) {
    cerr << "Poziv funkcije " << _f << " koja nije definisana";
    exit(EXIT_FAILURE);
  }

  if (F->arg_size() != _nodes.size()) {
    cerr << "Funkcija " << _f << " ocekuje " << F->arg_size() << " argumenata" ;
    exit(EXIT_FAILURE);
  }

  vector<Value*> a;
  for (unsigned i = 0; i < _nodes.size(); i++)
    a.push_back(_nodes[i]->codegen());

  return Builder.CreateCall(F, a, "calltmp");
}

Value* AddExprAST::codegen() const {
  Value *l = _nodes[0]->codegen();
  Value *d = _nodes[1]->codegen();
  if (!l || !d)
    return NULL;
  return Builder.CreateAdd(l, d, "addtmp");
}

Value* LessThenExprAST::codegen() const {
  Value *l = _nodes[0]->codegen();
  Value *d = _nodes[1]->codegen();
  if (!l || !d)
    return NULL;
  return Builder.CreateICmpULT(l, d, "lttmp");
}

Value* WhileExprAST::codegen() const {
  Function *F = Builder.GetInsertBlock()->getParent();
  BasicBlock *Loop1BB = BasicBlock::Create(TheContext, "loop1", F);
  BasicBlock *Loop2BB = BasicBlock::Create(TheContext, "loop2", F);
  BasicBlock *AfterLoopBB = BasicBlock::Create(TheContext, "afterloop", F);
  Builder.CreateBr(Loop1BB);
  
  Builder.SetInsertPoint(Loop1BB);
  Value* CondVal = _nodes[0]->codegen();
  if (!CondVal)
    return NULL;  
  Builder.CreateCondBr(CondVal, Loop2BB, AfterLoopBB);
  Loop1BB = Builder.GetInsertBlock();

  Builder.SetInsertPoint(Loop2BB);
  Value* Tmp = _nodes[1]->codegen();
  if (Tmp == NULL)
    return NULL;
  Builder.CreateBr(Loop1BB);
  Loop2BB = Builder.GetInsertBlock();

  Builder.SetInsertPoint(AfterLoopBB);
  return ConstantInt::get(TheContext, APInt(32, 0));
}

FunctionAST::~FunctionAST() {
  delete _e;
}

Function* FunctionAST::codegen() const {
  Function* F = TheModule->getFunction(_f);
  if (F) {
    cerr << "Funkcija " << _f << " je vec definisana" << endl;
    exit(EXIT_FAILURE);
  }

  vector<Type*> d(_args.size(), Type::getInt32Ty(TheContext));
  FunctionType* FT = FunctionType::get(Type::getInt32Ty(TheContext), d, false);
  F = Function::Create(FT, Function::ExternalLinkage, _f, TheModule);
  unsigned i = 0;
  for (auto &Arg : F->args())
    Arg.setName(_args[i++]);
  
  if (!F) {
    cerr << "Nemoguce generisanje koda za funkciju " << _f << endl;
    exit(EXIT_FAILURE);
  }

  BasicBlock *BB = BasicBlock::Create(TheContext, "entry", F);
  Builder.SetInsertPoint(BB);

  NamedValues.clear();
  for (auto &Arg : F->args()) {
    AllocaInst* Alloca = CreateEntryBlockAlloca(F, Arg.getName());
    NamedValues[Arg.getName()] = Alloca;
    Builder.CreateStore(&Arg, Alloca);
  }

  Value* RetVal = _e->codegen();
  if (_f == "main") {
    RetVal = ConstantInt::get(TheContext, APInt(32, 0));
  }
  if (RetVal) {
    Builder.CreateRet(RetVal);

    verifyFunction(*F);

    //TheFPM->run(*F);
    return F;
  }
  else {
    F->eraseFromParent();
    return NULL;
  }
}


void InitializeModuleAndPassManager(void) {
  TheModule = new Module("my_module", TheContext);

  // Create a new pass manager attached to it.
  TheFPM = new llvm::legacy::FunctionPassManager(TheModule);

  // Do simple "peephole" optimizations and bit-twiddling optzns.
  TheFPM->add(createInstructionCombiningPass());
  // Reassociate expressions.
  TheFPM->add(createReassociatePass());
  // Eliminate Common SubExpressions.
  TheFPM->add(createNewGVNPass());
  // Simplify the control flow graph (deleting unreachable blocks, etc).
  TheFPM->add(createCFGSimplificationPass());
  TheFPM->add(createPromoteMemoryToRegisterPass());

  TheFPM->doInitialization();
}

AllocaInst *CreateEntryBlockAlloca(Function *TheFunction, const string &VarName) {
  IRBuilder<> TmpB(&TheFunction->getEntryBlock(), TheFunction->getEntryBlock().begin());
  return TmpB.CreateAlloca(Type::getInt32Ty(TheContext), 0, VarName.c_str());
}
