
#include <ASTDeclNode.h>

#include "AST.h"
#include "InternalError.h"
#include "SemanticAnalysis.h"

#include "llvm/ADT/STLExtras.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Support/TypeSize.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Utils.h"

#include "loguru.hpp"

using namespace llvm;

/*
 * Code Generation Routines from TIP Tree Representation
 *
 * These code generation routines are based on the LLVM Kaleidoscope
 * Tutorial for C++ Chapters 3 and 7
 *     https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/LangImpl03.html
 *     https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/LangImpl07.html
 * We thank the authors for making these example available.
 *
 * Unlike the tutorial, these routines rely on following LLVM passes
 * to transform the code to SSA form, via the PromoteMemToReg pass,
 * and especially the CFGSimplification pass, which removes nops and
 * dead blocks.
 *
 * The approach taken in these codegen routines is to rely on the fact
 * that the program is type correct, which is checked in a previous pass.
 * This allows the code generator to infer the type of values accessed
 * in memory based on the nature of the expression, e.g., if we are generating
 * code for "*p" then "p" must be a pointer.  This approach is facilitated
 * by choosing a uniform representation of all types in memory, Int64, and
 * then as needed inserting type conversions into the generated code to convert
 * them to the appropriate type for an operator.
 *
 * This results in some suboptimal code, but we rely on the powerful LLVM
 * optimization passes to clean most of it up.
 */

namespace {
/*
 * These structures records lots of information that spans the codegen()
 * routines.  For example, the current insertion, entry, return blocks
 * in the current function.
 */
LLVMContext TheContext;
IRBuilder<> Builder(TheContext);

/*
 * Functions are represented with indices into a table.
 * This permits function values to be passed, i.e, as Int64 indices.
 */
std::map<std::string, int> functionIndex;

std::map<std::string, std::vector<std::string>> functionFormalNames;

/*
 * This structure stores the mapping from names in a function scope
 * to their LLVM values.  The structure is built when entering a
 * scope and cleared when exiting a scope.
 */
std::map<std::string, AllocaInst *> NamedValues;

/**
 * The UberRecord is a the type of all records
 *  It has a field for every named field in the program
 */
llvm::StructType *uberRecordType;

/**
 * The type for pointers to UberRecordType
 */
llvm::PointerType *ptrToUberRecordType;

// Maps field names to their index in the UberRecor
std::map<std::basic_string<char>, int> fieldIndex;

// Vector of fields in an uber record
std::vector<std::basic_string<char>> fieldVector;

// Permits getFunction to access the current module being compiled
std::shared_ptr<Module> CurrentModule;

/*
 * We use calls to llvm intrinsics for several purposes.  To construct a "nop",
 * using an LLVM internal intrinsic, to perform TIP specific IO, and
 * to allocate heap memory.
 */
llvm::Function *nop = nullptr;
llvm::Function *inputIntrinsic = nullptr;
llvm::Function *outputIntrinsic = nullptr;
llvm::Function *errorIntrinsic = nullptr;
llvm::Function *callocFun = nullptr;

// A counter to create shared labels
int labelNum = 0;

// Indicate whether the expression code gen is for an L-value
bool lValueGen = false;

// Indicate whether the expression code gen is for an alloc'd value
bool allocFlag = false;

/*
 * The global function dispatch table is created in a shallow pass over
 * the function signatures, stored here, and then referenced in generating
 * function applications.
 */
GlobalVariable *tipFTable = nullptr;

// The number of TIP program parameters
int numTIPArgs = 0;

/*
 * The global argument count and array are used to communicate command
 * line inputs to the TIP main function.
 */
GlobalVariable *tipNumInputs = nullptr;
GlobalVariable *tipInputArray = nullptr;

/*
 * Some constants are used repeatedly in code generation.  We define them
 * hear to eliminate redundancy.
 */
Constant *zeroV = ConstantInt::get(Type::getInt64Ty(TheContext), 0);
Constant *oneV = ConstantInt::get(Type::getInt64Ty(TheContext), 1);

/*
 * Create LLVM Function in Module associated with current program.
 * This function declares the function, but it does not generate code.
 * This is a key element of the shallow pass that builds the function
 * dispatch table.
 */
llvm::Function *getFunction(std::string Name) {
  // Lookup the symbol to access the formal parameter list
  auto formals = functionFormalNames[Name];

  /*
   * Main is handled specially.  It is declared as "_tip_main" with
   * no arguments - any arguments are converted to locals with special
   * initializaton in Function::codegen().
   */
  if (Name == "main") {
    if (auto *M = CurrentModule->getFunction("_tip_main")) {
      return M;
    }

    // initialize the number of TIP program args for initializing globals
    numTIPArgs = formals.size();

    // Declare "_tip_main()"
    auto *M = llvm::Function::Create(
        FunctionType::get(Type::getInt64Ty(TheContext), false),
        llvm::Function::ExternalLinkage, "_tip_" + Name, CurrentModule.get());
    return M;
  } else {
    // check if function is in the current module
    if (auto *F = CurrentModule->getFunction(Name)) {
      return F;
    }

    // function not found, so create it

    std::vector<Type *> FormalTypes(formals.size(),
                                    Type::getInt64Ty(TheContext));

    // Use type factory to create function from formal type to int
    auto *FT =
        FunctionType::get(Type::getInt64Ty(TheContext), FormalTypes, false);

    auto *F = llvm::Function::Create(FT, llvm::Function::InternalLinkage, Name,
                                     CurrentModule.get());

    // assign names to args for readability of generated code
    unsigned i = 0;
    for (auto &param : F->args()) {
      param.setName(formals[i++]);
    }

    return F;
  }
}

/*
 * Create an alloca instruction in the entry block of the function.
 * This is used for mutable variables, including arguments to functions.
 */
AllocaInst *CreateEntryBlockAlloca(llvm::Function *TheFunction,
                                   const std::string &VarName) {
  IRBuilder<> tmp(&TheFunction->getEntryBlock(),
                  TheFunction->getEntryBlock().begin());
  return tmp.CreateAlloca(Type::getInt64Ty(TheContext), 0, VarName);
}

} // namespace

/********************* codegen() routines ************************/

std::shared_ptr<llvm::Module> ASTProgram::codegen(SemanticAnalysis *analysis,
                                                  std::string programName) {
  LOG_S(1) << "Generating code for program " << programName;

  // Create module to hold generated code
  auto TheModule = std::make_shared<Module>(programName, TheContext);

// Set the default target triple for this platform
llvm:
  Triple targetTriple(llvm::sys::getProcessTriple());
  TheModule->setTargetTriple(targetTriple.str());

  // Initialize nop declaration
  nop = Intrinsic::getDeclaration(TheModule.get(), Intrinsic::donothing);

  labelNum = 0;

  // Transfer the module for access by shared codegen routines
  CurrentModule = std::move(TheModule);

  /*
   * This shallow pass over the function declarations builds the
   * function symbol table, creates the function declarations, and
   * builds the function dispatch table.
   */
  {
    /*
     * First create the local function symbol table which stores
     * the function index and formal parameters
     */
    int funIndex = 0;
    for (auto const &fn : getFunctions()) {
      functionIndex[fn->getName()] = funIndex++;

      auto formals = fn->getFormals();
      std::vector<std::string> names;
      std::transform(formals.begin(), formals.end(), std::back_inserter(names),
                     [](auto &d) { return d->getName(); });
      functionFormalNames[fn->getName()] = names;
    }

    /*
     * Create the llvm functions.
     * Store as a vector of constants, which works because Function
     * is a subtype of Constant, to workaround the inability of the
     * compiler to find a conversion from Function to Constant
     * below in creating the ftableInit.
     */
    std::vector<llvm::Constant *> programFunctions;
    for (auto const &fn : getFunctions()) {
      programFunctions.push_back(getFunction(fn->getName()));
    }

    /*
     * Create a generic function pointer type "() -> Int64" that is
     * used to declare the global function dispatch table and to
     * bitcast the declared functions prior to inserting them.
     */
    auto *genFunPtrType = PointerType::get(
        FunctionType::get(Type::getInt64Ty(TheContext), None, false), 0);

    // Create and record the function dispatch table
    auto *ftableType = ArrayType::get(genFunPtrType, funIndex);

    // Cast TIP functions to the generic function pointer type for initializer
    std::vector<Constant *> castProgramFunctions;
    for (auto const &pf : programFunctions) {
      castProgramFunctions.push_back(
          ConstantExpr::getPointerCast(pf, genFunPtrType));
    };

    /*
     * Create initializer for function table using generic function type
     * and set the initial value.
     */
    auto *ftableInit = ConstantArray::get(ftableType, castProgramFunctions);

    // Create the global function dispatch table
    tipFTable = new GlobalVariable(*CurrentModule, ftableType, true,
                                   llvm::GlobalValue::InternalLinkage,
                                   ftableInit, "_tip_ftable");
  }

  /*
   * Generate globals that establish the parameter passing structures from the
   * rtlib main() and define a "_tip_main" if one is not already defind.
   */
  {
    /*
     * If there is no "main(...)" defined in this TIP program we
     * create main that calls the "_tip_main_undefined()" rtlib function.
     *
     * For this function we perform all code generation here and
     * we never visit it during the codegen() traversals - since
     * the function doesn't exist in the TIP program.
     */
    auto fidx = functionIndex.find("main");
    if (fidx == functionIndex.end()) {
      auto *M = llvm::Function::Create(
          FunctionType::get(Type::getInt64Ty(TheContext), false),
          llvm::Function::ExternalLinkage, "_tip_main", CurrentModule.get());
      BasicBlock *BB = BasicBlock::Create(TheContext, "entry", M);
      Builder.SetInsertPoint(BB);

      auto *undef = llvm::Function::Create(
          FunctionType::get(Type::getVoidTy(TheContext), false),
          llvm::Function::ExternalLinkage, "_tip_main_undefined",
          CurrentModule.get());
      Builder.CreateCall(undef);
      Builder.CreateRet(zeroV);
    }

    // create global _tip_num_inputs with init of numTIPArgs
    tipNumInputs = new GlobalVariable(
        *CurrentModule, Type::getInt64Ty(TheContext), true,
        llvm::GlobalValue::ExternalLinkage,
        ConstantInt::get(Type::getInt64Ty(TheContext), numTIPArgs),
        "_tip_num_inputs");

    // create global _tip_input_array with up to numTIPArgs of Int64
    auto *inputArrayType =
        ArrayType::get(Type::getInt64Ty(TheContext), numTIPArgs);
    std::vector<Constant *> zeros(numTIPArgs, zeroV);
    tipInputArray = new GlobalVariable(
        *CurrentModule, inputArrayType, false, llvm::GlobalValue::CommonLinkage,
        ConstantArray::get(inputArrayType, zeros), "_tip_input_array");
  }

  // declare the calloc function
  // the calloc function takes in two ints: the number of items and the size of
  // the items
  std::vector<Type *> twoInt(2, Type::getInt64Ty(TheContext));
  auto *FT = FunctionType::get(Type::getInt8PtrTy(TheContext), twoInt, false);
  callocFun = llvm::Function::Create(FT, llvm::Function::ExternalLinkage,
                                     "calloc", CurrentModule.get());
  callocFun->addFnAttr(llvm::Attribute::NoUnwind);

  callocFun->setAttributes(callocFun->getAttributes().addAttributeAtIndex(
      callocFun->getContext(), 0, llvm::Attribute::NoAlias));

  /* We create a single unified record structure that is capable of representing
   * all records in a TIP program.  While wasteful of memory, this approach is
   * compatible with the limited type checking provided for records in TIP.
   *
   * We refer to this single unified record structure as the "uber record"
   */
  std::vector<Type *> member_values;
  int index = 0;
  for (auto field : analysis->getSymbolTable()->getFields()) {
    member_values.push_back(IntegerType::getInt64Ty((TheContext)));
    fieldVector.push_back(field);
    fieldIndex[field] = index;
    index++;
  }
  uberRecordType = StructType::create(TheContext, member_values, "uberRecord");
  ptrToUberRecordType = PointerType::get(uberRecordType, 0);

  // Code is generated into the module by the other routines
  for (auto const &fn : getFunctions()) {
    fn->codegen();
  }

  TheModule = std::move(CurrentModule);

  verifyModule(*TheModule);

  return TheModule;
}

llvm::Value *ASTFunction::codegen() {
  LOG_S(1) << "Generating code for " << *this;

  llvm::Function *TheFunction = getFunction(getName());
  if (TheFunction == nullptr) {
    throw InternalError("failed to declare the function" + // LCOV_EXCL_LINE
                        getName());                        // LCOV_EXCL_LINE
  }

  // create basic block to hold body of function definition
  BasicBlock *BB = BasicBlock::Create(TheContext, "entry", TheFunction);
  Builder.SetInsertPoint(BB);

  // keep scope separate from prior definitions
  NamedValues.clear();

  /*
   * Add arguments to the symbol table
   *   - for main function, we initialize allocas with array loads
   *   - for other functions, we initialize allocas with the arg values
   */
  if (getName() == "main") {
    int argIdx = 0;
    // Note that the args are not in the LLVM function decl, so we use the AST
    // formals
    for (auto &argName : functionFormalNames[getName()]) {
      // Create an alloca for this argument and store its value
      AllocaInst *argAlloc = CreateEntryBlockAlloca(TheFunction, argName);

      // Emit the GEP instruction to index into input array
      std::vector<Value *> indices;
      indices.push_back(zeroV);
      indices.push_back(ConstantInt::get(Type::getInt64Ty(TheContext), argIdx));
      auto *gep = Builder.CreateInBoundsGEP(tipInputArray->getValueType(),
                                            tipInputArray, indices, "inputidx");

      // Load the value and store it into the arg's alloca
      auto *inVal =
          Builder.CreateLoad(gep->getType()->getPointerElementType(), gep,
                             "tipinput" + std::to_string(argIdx++));
      Builder.CreateStore(inVal, argAlloc);

      // Record name binding to alloca
      NamedValues[argName] = argAlloc;
    }
  } else {
    for (auto &arg : TheFunction->args()) {
      // Create an alloca for this argument and store its value
      AllocaInst *argAlloc =
          CreateEntryBlockAlloca(TheFunction, arg.getName().str());
      Builder.CreateStore(&arg, argAlloc);

      // Record name binding to alloca
      NamedValues[arg.getName().str()] = argAlloc;
    }
  }

  // add local declarations to the symbol table
  for (auto const &decl : getDeclarations()) {
    if (decl->codegen() == nullptr) {
      TheFunction->eraseFromParent();                    // LCOV_EXCL_LINE
      throw InternalError(                               // LCOV_EXCL_LINE
          "failed to generate bitcode for the function " // LCOV_EXCL_LINE
          "declarations");                               // LCOV_EXCL_LINE
    }
  }

  for (auto &stmt : getStmts()) {
    if (stmt->codegen() == nullptr) {
      TheFunction->eraseFromParent();                    // LCOV_EXCL_LINE
      throw InternalError(                               // LCOV_EXCL_LINE
          "failed to generate bitcode for the function " // LCOV_EXCL_LINE
          "statement");                                  // LCOV_EXCL_LINE
    }
  }

  verifyFunction(*TheFunction);
  return TheFunction;
} // LCOV_EXCL_LINE

llvm::Value *ASTNumberExpr::codegen() {
  LOG_S(1) << "Generating code for " << *this;

  return ConstantInt::get(Type::getInt64Ty(TheContext), getValue());
} // LCOV_EXCL_LINE

llvm::Value *ASTBinaryExpr::codegen() {
  LOG_S(1) << "Generating code for " << *this;

  Value *L = getLeft()->codegen();
  Value *R = getRight()->codegen();
  if (L == nullptr || R == nullptr) {
    throw InternalError("null binary operand");
  }

  if (getOp() == "+") {
    return Builder.CreateAdd(L, R, "addtmp");
  } else if (getOp() == "-") {
    return Builder.CreateSub(L, R, "subtmp");
  } else if (getOp() == "*") {
    return Builder.CreateMul(L, R, "multmp");
  } else if (getOp() == "/") {
    return Builder.CreateSDiv(L, R, "divtmp");
  } else if (getOp() == ">") {
    auto *cmp = Builder.CreateICmpSGT(L, R, "_gttmp");
    return Builder.CreateIntCast(cmp, IntegerType::getInt64Ty(TheContext),
                                 false, "gttmp");
  } else if (getOp() == "==") {
    auto *cmp = Builder.CreateICmpEQ(L, R, "_eqtmp");
    return Builder.CreateIntCast(cmp, IntegerType::getInt64Ty(TheContext),
                                 false, "eqtmp");
  } else if (getOp() == "!=") {
    auto *cmp = Builder.CreateICmpNE(L, R, "_neqtmp");
    return Builder.CreateIntCast(cmp, IntegerType::getInt64Ty(TheContext),
                                 false, "neqtmp");
  } 
  // EXTENSIONS
  else if (getOp() == "and") { // bool
    auto *cmp = Builder.CreateAnd(L, R, "_andtmp");
    return Builder.CreateIntCast(cmp, IntegerType::getInt64Ty(TheContext),
                                 false, "andtmp");
  } else if (getOp() == "or") { // bool 
    auto *cmp = Builder.CreateOr(L, R, "_ortmp");
    return Builder.CreateIntCast(cmp, IntegerType::getInt64Ty(TheContext),
                                 false, "ortmp");
  } else if (getOp() == ">=") { // bool
    auto *cmp = Builder.CreateICmpSGE(L, R, "_gtetmp");
    return Builder.CreateIntCast(cmp, IntegerType::getInt64Ty(TheContext),
                                 false, "gtetmp");
  } else if (getOp() == "<") { // bool
    auto *cmp = Builder.CreateICmpSLT(L, R, "_lttmp");
    return Builder.CreateIntCast(cmp, IntegerType::getInt64Ty(TheContext),
                                 false, "lttmp");
  } else if (getOp() == "<=") { // bool
    auto *cmp = Builder.CreateICmpSLE(L, R, "_ltetmp");
    return Builder.CreateIntCast(cmp, IntegerType::getInt64Ty(TheContext),
                                 false, "ltetmp");
  } else if (getOp() == "%") { // Int
      return Builder.CreateSRem(L, R, "modtmp");
  } else {
    throw InternalError("Invalid binary operator: " + OP);
  }
}

/*
 * First lookup the variable in the symbol table for names and
 * if that fails, then look in the symbol table for functions.
 *
 * This relies on the fact that TIP programs have been checked to
 * ensure that names obey the scope rules.
 */
llvm::Value *ASTVariableExpr::codegen() {
  LOG_S(1) << "Generating code for " << *this;

  auto nv = NamedValues.find(getName());
  if (nv != NamedValues.end()) {
    if (lValueGen) {
      return NamedValues[nv->first];
    } else {
      return Builder.CreateLoad(nv->second->getAllocatedType(), nv->second,
                                getName().c_str());
    }
  }

  auto fidx = functionIndex.find(getName());
  if (fidx == functionIndex.end()) {
    throw InternalError("Unknown variable name: " + getName());
  }

  return ConstantInt::get(Type::getInt64Ty(TheContext), fidx->second);
}

llvm::Value *ASTInputExpr::codegen() {
  LOG_S(1) << "Generating code for " << *this;

  if (inputIntrinsic == nullptr) {
    auto *FT = FunctionType::get(Type::getInt64Ty(TheContext), false);
    inputIntrinsic = llvm::Function::Create(FT, llvm::Function::ExternalLinkage,
                                            "_tip_input", CurrentModule.get());
  }
  return Builder.CreateCall(inputIntrinsic);
} // LCOV_EXCL_LINE

/*
 * Function application in TIP can either be through explicitly named
 * functions or through expressions that evaluate to a function reference.
 * We consolidate these two cases by binding function names to values
 * and then using those values, which may flow through the program as function
 * references, to index into a function dispatch table to invoke the function.
 *
 * The function name values and table are setup in a shallow-pass over
 * functions performed during codegen for the Program.
 */
llvm::Value *ASTFunAppExpr::codegen() {
  LOG_S(1) << "Generating code for " << *this;

  /*
   * Evaluate the function expression - it will resolve to an integer value
   * whether it is a function literal or an expression.
   */
  auto *funVal = getFunction()->codegen();
  if (funVal == nullptr) {
    throw InternalError("failed to generate bitcode for the function");
  }

  /*
   * Emit the GEP instruction to compute the address of LLVM function
   * pointer to be called.
   */
  std::vector<Value *> indices;
  indices.push_back(zeroV);
  indices.push_back(funVal);

  auto *gep = Builder.CreateInBoundsGEP(tipFTable->getValueType(), tipFTable,
                                        indices, "ftableidx");

  // Load the function pointer
  auto *genericFunPtr = Builder.CreateLoad(
      gep->getType()->getPointerElementType(), gep, "genfptr");

  /*
   * Compute the specific function pointer type based on the actual parameter
   * list.
   *
   * TBD: Currently the type is Int64^N -> Int64, * where N is the length of the
   * list
   *
   * Once type information is available we will need to iterate the actuals
   * and construct the per actual vector of types.
   */
  std::vector<Type *> actualTypes(getActuals().size(),
                                  Type::getInt64Ty(TheContext));
  auto *funType =
      FunctionType::get(Type::getInt64Ty(TheContext), actualTypes, false);
  auto *funPtrType = PointerType::get(funType, 0);

  // Bitcast the function pointer to the call-site determined function type
  auto *castFunPtr =
      Builder.CreatePointerCast(genericFunPtr, funPtrType, "castfptr");

  // Compute the actual parameters
  std::vector<Value *> argsV;
  for (auto const &arg : getActuals()) {
    Value *argVal = arg->codegen();
    if (argVal == nullptr) {
      throw InternalError(                                // LCOV_EXCL_LINE
          "failed to generate bitcode for the argument"); // LCOV_EXCL_LINE
    }
    argsV.push_back(argVal);
  }

  return Builder.CreateCall(funType, castFunPtr, argsV, "calltmp");
}

/* 'alloc' Allocate expression
 * Generates a pointer to the allocs arguments (ints, records, ...)
 */
llvm::Value *ASTAllocExpr::codegen() {
  LOG_S(1) << "Generating code for " << *this;

  allocFlag = true;
  Value *argVal = getInitializer()->codegen();
  allocFlag = false;
  if (argVal == nullptr) {
    throw InternalError("failed to generate bitcode for the initializer of the "
                        "alloc expression");
  }

  // Allocate an int pointer with calloc
  std::vector<Value *> twoArg;
  twoArg.push_back(ConstantInt::get(Type::getInt64Ty(TheContext), 1));
  twoArg.push_back(ConstantInt::get(Type::getInt64Ty(TheContext), 8));
  auto *allocInst = Builder.CreateCall(callocFun, twoArg, "allocPtr");
  auto *castPtr = Builder.CreatePointerCast(
      allocInst, Type::getInt64PtrTy(TheContext), "castPtr");
  // Initialize with argument
  auto *initializingStore = Builder.CreateStore(argVal, castPtr);

  return Builder.CreatePtrToInt(castPtr, Type::getInt64Ty(TheContext),
                                "allocIntVal");
}

llvm::Value *ASTNullExpr::codegen() {
  auto *nullPtr = ConstantPointerNull::get(Type::getInt64PtrTy(TheContext));
  return Builder.CreatePtrToInt(nullPtr, Type::getInt64Ty(TheContext),
                                "nullPtrIntVal");
}

/* '&' address of expression
 *
 * The argument must be capable of generating an l-value.
 * This is checked in the weeding pass.
 *
 */
llvm::Value *ASTRefExpr::codegen() {
  LOG_S(1) << "Generating code for " << *this;

  lValueGen = true;
  Value *lValue = getVar()->codegen();
  lValueGen = false;

  if (lValue == nullptr) {
    throw InternalError("could not generate l-value for address of");
  }

  return Builder.CreatePtrToInt(lValue, Type::getInt64Ty(TheContext),
                                "addrOfPtr");
} // LCOV_EXCL_LINE

/* '*' dereference expression
 *
 * The argument is assumed to be a reference expression, but
 * our code generation strategy stores everything as an integer.
 * Consequently, we convert the value with "inttoptr" before loading
 * the value at the pointed-to memory location.
 */
llvm::Value *ASTDeRefExpr::codegen() {
  LOG_S(1) << "Generating code for " << *this;

  bool isLValue = lValueGen;

  if (isLValue) {
    // This flag is reset here so that sub-expressions are treated as r-values
    lValueGen = false;
  }

  Value *argVal = getPtr()->codegen();
  if (argVal == nullptr) {
    throw InternalError("failed to generate bitcode for the pointer");
  }

  // compute the address
  Value *address = Builder.CreateIntToPtr(
      argVal, Type::getInt64PtrTy(TheContext), "ptrIntVal");

  if (isLValue) {
    // For an l-value, return the address
    return address;
  } else {
    // For an r-value, return the value at the address
    return Builder.CreateLoad(address->getType()->getPointerElementType(),
                              address, "valueAt");
  }
}

/* {field1 : val1, ..., fieldN : valN} record expression
 *
 * Builds an instance of the UberRecord using the declared fields
 */
llvm::Value *ASTRecordExpr::codegen() {
  LOG_S(1) << "Generating code for " << *this;

  // If this is an alloc, we calloc the record
  if (allocFlag) {
    // Allocate the a pointer to an uber record
    auto *allocaRecord = Builder.CreateAlloca(ptrToUberRecordType);

    // Use Builder to create the calloc call using pre-defined callocFun
    auto sizeOfUberRecord = CurrentModule->getDataLayout()
                                .getStructLayout(uberRecordType)
                                ->getSizeInBytes();
    std::vector<Value *> callocArgs;
    callocArgs.push_back(oneV);
    callocArgs.push_back(
        ConstantInt::get(Type::getInt64Ty(TheContext), sizeOfUberRecord));
    auto *calloc = Builder.CreateCall(callocFun, callocArgs, "callocedPtr");

    // Bitcast the calloc call to theStruct Type
    auto recordPtr =
        Builder.CreatePointerCast(calloc, ptrToUberRecordType, "recordCalloc");

    // Store the ptr to the record in the record alloc
    Builder.CreateStore(recordPtr, allocaRecord);

    // Load allocaRecord
    auto loadInst = Builder.CreateLoad(ptrToUberRecordType, allocaRecord);

    // For each field, generate GEP for location of field in the uberRecord
    // Generate the code for the field and store it in the GEP
    for (auto const &field : getFields()) {
      auto *gep = Builder.CreateStructGEP(uberRecordType, loadInst,
                                          fieldIndex[field->getField()],
                                          field->getField());
      auto value = field->codegen();
      Builder.CreateStore(value, gep);
    }

    // Return int64 pointer to the pointer to the record
    return Builder.CreatePtrToInt(recordPtr, Type::getInt64Ty(TheContext),
                                  "recordPtr");
  } else {
    // Allocate a the space for a uber record
    auto *allocaRecord = Builder.CreateAlloca(uberRecordType);

    // Codegen the fields present in this record and store them in the
    // appropriate location We do not give a value to fields that are not
    // explictly set. Thus, accessing them is undefined behavior
    for (auto const &field : getFields()) {
      auto *gep = Builder.CreateStructGEP(
          allocaRecord->getAllocatedType(), allocaRecord,
          fieldIndex[field->getField()], field->getField());
      auto value = field->codegen();
      Builder.CreateStore(value, gep);
    }
    // Return int64 pointer to the record since all variables are pointers to
    // ints
    return Builder.CreatePtrToInt(allocaRecord, Type::getInt64Ty(TheContext),
                                  "record");
  }
}

/* field : val field expression
 *
 * Expression for generating the code for the value of a field
 */
llvm::Value *ASTFieldExpr::codegen() {
  LOG_S(1) << "Generating code for " << *this;

  return this->getInitializer()->codegen();
} // LCOV_EXCL_LINE

/* record.field Access Expression
 *
 * In an l-value context this returns the location of the field being accessed
 * In an r-value context this returns the value of the field being accessed
 */
llvm::Value *ASTAccessExpr::codegen() {
  LOG_S(1) << "Generating code for " << *this;

  bool isLValue = lValueGen;

  if (isLValue) {
    // This flag is reset here so that sub-expressions are treated as r-values
    lValueGen = false;
  }

  // Get current field and check if it exists
  auto currField = this->getField();
  if (fieldIndex.count(currField) == 0) {
    throw InternalError("This field doesn't exist");
  }

  // Generate record instruction address
  Value *recordVal = this->getRecord()->codegen();
  Value *recordAddress = Builder.CreateIntToPtr(recordVal, ptrToUberRecordType);

  // Generate the field index
  auto index = fieldIndex[currField];

  // Generate the location of the field
  auto *gep =
      Builder.CreateStructGEP(uberRecordType, recordAddress, index, currField);

  // If LHS, return location of field
  if (isLValue) {
    return gep;
  }

  // Load value at GEP and return it
  auto fieldLoad = Builder.CreateLoad(IntegerType::getInt64Ty(TheContext), gep);
  return Builder.CreatePtrToInt(fieldLoad, Type::getInt64Ty(TheContext),
                                "fieldAccess");
}

llvm::Value *ASTDeclNode::codegen() {
  throw InternalError("Declarations do not emit code");
}

llvm::Value *ASTDeclStmt::codegen() {
  LOG_S(1) << "Generating code for " << *this;

  // The LLVM builder records the function we are currently generating
  llvm::Function *TheFunction = Builder.GetInsertBlock()->getParent();

  AllocaInst *localAlloca = nullptr;

  // Register all variables and emit their initializer.
  for (auto l : getVars()) {
    localAlloca = CreateEntryBlockAlloca(TheFunction, l->getName());

    // Initialize all locals to "0"
    Builder.CreateStore(zeroV, localAlloca);

    // Remember this binding.
    NamedValues[l->getName()] = localAlloca;
  }

  // Return the body computation.
  return localAlloca;
} // LCOV_EXCL_LINE

llvm::Value *ASTAssignStmt::codegen() {
  LOG_S(1) << "Generating code for " << *this;

  // trigger code generation for l-value expressions
  lValueGen = true;
  Value *lValue = getLHS()->codegen();
  lValueGen = false;

  if (lValue == nullptr) {
    throw InternalError(
        "failed to generate bitcode for the lhs of the assignment");
  }

  Value *rValue = getRHS()->codegen();
  if (rValue == nullptr) {
    throw InternalError(
        "failed to generate bitcode for the rhs of the assignment");
  }

  return Builder.CreateStore(rValue, lValue);
} // LCOV_EXCL_LINE

llvm::Value *ASTBlockStmt::codegen() {
  LOG_S(1) << "Generating code for " << *this;

  Value *lastStmt = nullptr;

  for (auto const &s : getStmts()) {
    lastStmt = s->codegen();
  }

  // If the block was empty return a nop
  return (lastStmt == nullptr) ? Builder.CreateCall(nop) : lastStmt;
} // LCOV_EXCL_LINE

/*
 * The code generated for an WhileStmt looks like this:
 *
 *       <COND> == 0		this is called the "header" block
 *   true   /  ^  \   false
 *         v  /    \
 *      <BODY>     /
 *                /
 *               v
 *              nop        	this is called the "exit" block
 *
 * Much of the code involves setting up the different blocks, establishing
 * the insertion point, and then letting other codegen functions write
 * code at that insertion point.  A key difference is that the condition
 * is generated into a basic block since it will be branched to after the
 * body executes.
 */
llvm::Value *ASTWhileStmt::codegen() {
  LOG_S(1) << "Generating code for " << *this;

  llvm::Function *TheFunction = Builder.GetInsertBlock()->getParent();

  /*
   * Create blocks for the loop header, body, and exit; HeaderBB is first
   * so it is added to the function in the constructor.
   *
   * Blocks don't need to be contiguous or ordered in
   * any particular way because we will explicitly branch between them.
   * This can be optimized by later passes.
   */
  labelNum++; // create shared labels for these BBs

  BasicBlock *HeaderBB = BasicBlock::Create(
      TheContext, "header" + std::to_string(labelNum), TheFunction);
  BasicBlock *BodyBB =
      BasicBlock::Create(TheContext, "body" + std::to_string(labelNum));
  BasicBlock *ExitBB =
      BasicBlock::Create(TheContext, "exit" + std::to_string(labelNum));

  // Add an explicit branch from the current BB to the header
  Builder.CreateBr(HeaderBB);

  // Emit loop header
  {
    Builder.SetInsertPoint(HeaderBB);

    Value *CondV = getCondition()->codegen();
    if (CondV == nullptr) {
      throw InternalError(                                   // LCOV_EXCL_LINE
          "failed to generate bitcode for the conditional"); // LCOV_EXCL_LINE
    }

    // Convert condition to a bool by comparing non-equal to 0.
    CondV = Builder.CreateICmpNE(CondV, ConstantInt::get(CondV->getType(), 0),
                                 "loopcond");

    Builder.CreateCondBr(CondV, BodyBB, ExitBB);
  }

  // Emit loop body
  {
    TheFunction->getBasicBlockList().push_back(BodyBB);
    Builder.SetInsertPoint(BodyBB);

    Value *BodyV = getBody()->codegen();
    if (BodyV == nullptr) {
      throw InternalError(                                 // LCOV_EXCL_LINE
          "failed to generate bitcode for the loop body"); // LCOV_EXCL_LINE
    }

    Builder.CreateBr(HeaderBB);
  }

  // Emit loop exit block.
  TheFunction->getBasicBlockList().push_back(ExitBB);
  Builder.SetInsertPoint(ExitBB);
  return Builder.CreateCall(nop);
} // LCOV_EXCL_LINE

/*
 * The code generated for an IfStmt looks like this:
 *
 *       <COND> == 0
 *   true   /     \   false
 *         v       v
 *      <THEN>   <ELSE>  if defined, otherwise use a nop
 *          \     /
 *           v   v
 *            nop        this is called the merge basic block
 *
 * Much of the code involves setting up the different blocks, establishing
 * the insertion point, and then letting other codegen functions write
 * code at that insertion point.
 */
llvm::Value *ASTIfStmt::codegen() {
  LOG_S(1) << "Generating code for " << *this;

  Value *CondV = getCondition()->codegen();
  if (CondV == nullptr) {
    throw InternalError(
        "failed to generate bitcode for the condition of the if statement");
  }

  // Convert condition to a bool by comparing non-equal to 0.
  CondV = Builder.CreateICmpNE(CondV, ConstantInt::get(CondV->getType(), 0),
                               "ifcond");

  llvm::Function *TheFunction = Builder.GetInsertBlock()->getParent();

  /*
   * Create blocks for the then and else cases.  The then block is first so
   * it is inserted in the function in the constructor. The rest of the blocks
   * need to be inserted explicitly into the functions basic block list
   * (via a push_back() call).
   *
   * Blocks don't need to be contiguous or ordered in
   * any particular way because we will explicitly branch between them.
   * This can be optimized to fall through behavior by later passes.
   */
  labelNum++; // create shared labels for these BBs
  BasicBlock *ThenBB = BasicBlock::Create(
      TheContext, "then" + std::to_string(labelNum), TheFunction);
  BasicBlock *ElseBB =
      BasicBlock::Create(TheContext, "else" + std::to_string(labelNum));
  BasicBlock *MergeBB =
      BasicBlock::Create(TheContext, "ifmerge" + std::to_string(labelNum));

  Builder.CreateCondBr(CondV, ThenBB, ElseBB);

  // Emit then block.
  {
    Builder.SetInsertPoint(ThenBB);

    Value *ThenV = getThen()->codegen();
    if (ThenV == nullptr) {
      throw InternalError(                                  // LCOV_EXCL_LINE
          "failed to generate bitcode for the then block"); // LCOV_EXCL_LINE
    }

    Builder.CreateBr(MergeBB);
  }

  // Emit else block.
  {
    TheFunction->getBasicBlockList().push_back(ElseBB);
    Builder.SetInsertPoint(ElseBB);

    // if there is no ELSE then exist emit a "nop"
    Value *ElseV = nullptr;
    if (getElse() != nullptr) {
      ElseV = getElse()->codegen();
      if (ElseV == nullptr) {
        throw InternalError(                                  // LCOV_EXCL_LINE
            "failed to generate bitcode for the else block"); // LCOV_EXCL_LINE
      }
    } else {
      Builder.CreateCall(nop);
    }

    Builder.CreateBr(MergeBB);
  }

  // Emit merge block.
  TheFunction->getBasicBlockList().push_back(MergeBB);
  Builder.SetInsertPoint(MergeBB);
  return Builder.CreateCall(nop);
} // LCOV_EXCL_LINE

llvm::Value *ASTOutputStmt::codegen() {
  LOG_S(1) << "Generating code for " << *this;

  if (outputIntrinsic == nullptr) {
    std::vector<Type *> oneInt(1, Type::getInt64Ty(TheContext));
    auto *FT = FunctionType::get(Type::getInt64Ty(TheContext), oneInt, false);
    outputIntrinsic =
        llvm::Function::Create(FT, llvm::Function::ExternalLinkage,
                               "_tip_output", CurrentModule.get());
  }

  Value *argVal = getArg()->codegen();
  if (argVal == nullptr) {
    throw InternalError(
        "failed to generate bitcode for the argument of the output statement");
  }

  std::vector<Value *> ArgsV(1, argVal);

  return Builder.CreateCall(outputIntrinsic, ArgsV);
}

llvm::Value *ASTErrorStmt::codegen() {
  LOG_S(1) << "Generating code for " << *this;

  if (errorIntrinsic == nullptr) {
    std::vector<Type *> oneInt(1, Type::getInt64Ty(TheContext));
    auto *FT = FunctionType::get(Type::getInt64Ty(TheContext), oneInt, false);
    errorIntrinsic = llvm::Function::Create(FT, llvm::Function::ExternalLinkage,
                                            "_tip_error", CurrentModule.get());
  }

  Value *argVal = getArg()->codegen();
  if (argVal == nullptr) {
    throw InternalError(
        "failed to generate bitcode for the argument of the error statement");
  }

  std::vector<Value *> ArgsV(1, argVal);

  return Builder.CreateCall(errorIntrinsic, ArgsV);
}

llvm::Value *ASTReturnStmt::codegen() {
  LOG_S(1) << "Generating code for " << *this;

  Value *argVal = getArg()->codegen();
  return Builder.CreateRet(argVal);
} // LCOV_EXCL_LINE

// SIP EXTENSIONS


/*
  Generates the LLVM for accessing an element of an array
  - Generate Value for the array and the index expression.
  - Check for null values for the array and the index, throwing an error if any are null.
  - Convert the array's address to a pointer type for indexing.
  - Load the length of the array and performing bounds checking on the index to ensure it is within the valid range of the array. 
      check if the index is negative or greater than or equal to the array length.
  - If out-of-bounds index, the function branches to an error handling block.
  - If the index is within bounds, the function calculates the address of the desired array element and either:
    - Returns a pointer to the array element.
    - Loads and returns the value of the array element.
*/
llvm::Value *ASTArrayAccessExpr::codegen() {
  // Done
  LOG_S(1) << "Generating code for " << *this;
  
  bool isLValue = lValueGen;

  if (isLValue) {
    // This flag is reset here so that sub-expressions are treated as r-values
    lValueGen = false;
  }
  // Get array 
  Value *array = getArray()->codegen();
  if (array == nullptr) {
    throw InternalError("failed to generate bitcode for the array of the array access expression");
  }
  Value *arrayAddr = Builder.CreateIntToPtr(array, Type::getInt64PtrTy(TheContext), "arrayAddr");
  Value *arrayPtr = Builder.CreateIntToPtr(arrayAddr, Type::getInt64PtrTy(TheContext), "arrayPtr");

  // get array index
  Value *index = getIndex()->codegen();
  if (index == nullptr) {
    throw InternalError("failed to generate bitcode for the index of the array access expression");
  }

  // Array length
  Value *arrayLength = Builder.CreateLoad(Type::getInt64Ty(TheContext), arrayAddr, "arrayLength");


  // Create Error Intrinsic
  if (errorIntrinsic == nullptr) {
    std::vector<Type *> oneInt(1, Type::getInt64Ty(TheContext));
    auto *FT = FunctionType::get(Type::getInt64Ty(TheContext), oneInt, false);
    errorIntrinsic = llvm::Function::Create(FT, llvm::Function::ExternalLinkage,
                                            "_tip_error", CurrentModule.get());
  }
 
  // Prepare basic blocks for branching
  Function* TheFunction = Builder.GetInsertBlock()->getParent();
  BasicBlock* ErrorBB = BasicBlock::Create(TheContext, "error", TheFunction);
  BasicBlock* AccessBB = BasicBlock::Create(TheContext, "access", TheFunction); // Corrected to AccessBB
  
  // Check index bounds
  Value *isIndexNegative = Builder.CreateICmpSLT(index, llvm::ConstantInt::get(TheContext, llvm::APInt(64, 0)), "isneg");
  Value *isIndexOutOfBounds = Builder.CreateICmpSGE(index, arrayLength, "isoutofbounds");
  
  // Use logical OR to check if either bound condition is true, then branch to error.
  Value* indexOutOfBound = Builder.CreateOr(isIndexNegative, isIndexOutOfBounds, "indexOutOfBound");
  Builder.CreateCondBr(indexOutOfBound, ErrorBB, AccessBB);
  
  // Emit code for ErrorBB
  Builder.SetInsertPoint(ErrorBB);
  Value *error = Builder.CreateCall(errorIntrinsic, {index});
  // After calling the error function, the code will return
  Builder.CreateRet(error);
  
  // Emit code for the AccessBB
  Builder.SetInsertPoint(AccessBB);
  Value *idxPlusOne = Builder.CreateAdd(index, oneV, "idxPlusOne");
  Value *elemPtr = Builder.CreateGEP(arrayPtr->getType()->getPointerElementType(), arrayPtr, idxPlusOne, "elemPtr");
  
  if (lValueGen) {
    // If it's an L-value, return the pointer to the element.
    return elemPtr;
  } else {
    // If it's an R-value, load and return the element's value.
    return Builder.CreateLoad(elemPtr->getType()->getPointerElementType(), elemPtr, "loadElem");
  }
}


/*
  Generates LLVM code for creating an array
  - It retrieves the elements of the array and determines the size of the array.
  - Allocates memory for the array using 'calloc', with space for the array length plus the elements.
  - The first element of the allocated space is used to store the length of the array.
  - Iterates over the array elements, generating code for
     each element and storing them in the allocated array space.
  - The array elements start from the second position in the allocated space (index 1), 
      as the first position (index 0) is reserved for the array length.
  - After storing all elements, it returns the pointer to the allocated array, cast to a 64-bit integer.
*/
llvm::Value *ASTArrayExpr::codegen() {
  // DONE
  LOG_S(1) << "Generating code for " << *this;

  auto elements = getElements();
  int loopSize = ELEMENTS.size();
  Value* arrLen = ConstantInt::get(Type::getInt64Ty(TheContext), loopSize);

  //Allocate an int pointer with calloc
  std::vector<Value *> twoArg;
  twoArg.push_back(ConstantInt::get(Type::getInt64Ty(TheContext), loopSize+1));
  twoArg.push_back(ConstantInt::get(Type::getInt64Ty(TheContext), 8));
  auto *allocInst = Builder.CreateCall(callocFun, twoArg, "allocPtr");
  auto *arrayPtr = Builder.CreatePointerCast(allocInst, Type::getInt64PtrTy(TheContext), "arrayPtr");

  Builder.CreateStore(arrLen, arrayPtr);
  
  for (int i = 0; i < loopSize; i++) {
    Value* counter = ConstantInt::get(Type::getInt64Ty(TheContext), i+1);
    Value* elementValue = ELEMENTS.at(i)->codegen();
    Value* gep = Builder.CreateGEP(arrayPtr->getType()->getPointerElementType(), arrayPtr, counter, "elementPtr");
    Builder.CreateStore(elementValue, gep);
  }

  return Builder.CreatePtrToInt(arrayPtr, Type::getInt64Ty(TheContext));
}


/*
  Generates LLVM code for obtaining the length of an array.
  - The function first generates the LLVM Value for the array using the `getArray()->codegen()` method.
  - It checks if the returned array Value is null, throwing an InternalError if so.
  - The array's address is then loaded from memory. 
  - Loads the first element, which is the length of the array.
  - The length of the array is returned as a 64-bit integer.
*/
llvm::Value *ASTArrayLengthExpr::codegen() {
  // DONE
  LOG_S(1) << "Generating code for " << *this;

  // * Get the array.
  lValueGen = true;
  llvm::Value *array = getArray()->codegen();
  lValueGen = false;

  if (array == nullptr) {
    throw InternalError("failed to generate bitcode for the array of the "
                        "array length expression");
  }
  // Load addr of array from its memory home
  Value *arrayAddr = Builder.CreateIntToPtr(array, Type::getInt64PtrTy(TheContext), "arrayAddr");

  // Assuming 'array' is a pointer to the start of the array where the first element is the length
  llvm::Value *arrayLength = Builder.CreateLoad(Type::getInt64Ty(TheContext), arrayAddr, "arrayLength");

  return arrayLength;
}

/*
  Generates LLVM code for creating an arrayofexpr.
  - Generates size and check if null, throwing an error if it's null.
  - Generates value and value if null, throwing an error if it's null.
  - Allocates memory for the array using 'calloc', which is sized based on the size expression plus one.
  - The first element of the array is used to store its size.
  - Loop to intialize each value in array
  - Return a pointer to the array, cast to a 64-bit integer.
*/
llvm::Value *ASTArrayOfExpr::codegen() {
  // Done
  // Check if we have exactly two elements
  if (ELEMENTS.size() != 2) {
    throw InternalError("Array of expression requires exactly two elements.");
  }
  // Generate code for the size and the value
  Value* sizeExpr = ELEMENTS[0]->codegen();
  if (sizeExpr == nullptr) {
        throw InternalError("Error generating code for array sizeExpr");
  }
  Value* sizeExprValue = Builder.CreateAdd(sizeExpr, oneV, "sizePlusOne");
  Value* valueExpr = ELEMENTS[1]->codegen();
  // Allocate memory for the array
  std::vector<llvm::Value*> callocArgs = {
    sizeExprValue, // Number of elements in the array
    llvm::ConstantInt::get(Type::getInt64Ty(TheContext), 8) // Size of each element (8 bytes for int64)
  };
  auto *allocInst = Builder.CreateCall(callocFun, callocArgs, "allocPtr");
  auto *arrayPtr = Builder.CreatePointerCast(allocInst, Type::getInt64PtrTy(TheContext), "arrayPtr");
  Builder.CreateStore(sizeExpr, arrayPtr);
  
  // Create a loop to initialize each element of the array with valueExpr
  Function* TheFunction = Builder.GetInsertBlock()->getParent();
  BasicBlock* preheaderBlock = Builder.GetInsertBlock();
  BasicBlock* loopBlock = BasicBlock::Create(TheContext, "loop", TheFunction);
  BasicBlock* afterLoopBlock = BasicBlock::Create(TheContext, "afterLoop", TheFunction);
  Builder.CreateBr(loopBlock);

  // Start insertion in loopBlock
  Builder.SetInsertPoint(loopBlock);

  // Create the PHI node for the loop variable, initialize with 1 to skip size storage
  llvm::PHINode* loopVar = Builder.CreatePHI(Type::getInt64Ty(TheContext), 2);
  loopVar->addIncoming(ConstantInt::get(Type::getInt64Ty(TheContext), 1), preheaderBlock);

  // Calculate the address for the current element in the array
  llvm::Value* elementPtr = Builder.CreateGEP(arrayPtr->getType()->getPointerElementType(),arrayPtr, loopVar, "elementPtr");
  Builder.CreateStore(valueExpr, elementPtr);

  // Increment the loop variable
  llvm::Value* nextVal = Builder.CreateAdd(loopVar, oneV, "nextVal");
  loopVar->addIncoming(nextVal, loopBlock);

  // Create the end condition for the loop
  llvm::Value* endCond = Builder.CreateICmpEQ(nextVal, sizeExprValue, "loopcond");
  Builder.CreateCondBr(endCond, afterLoopBlock, loopBlock);

  // Insert the after loop block
  Builder.SetInsertPoint(afterLoopBlock);

  // The arrayPtr is a pointer to the first element of the array
  return Builder.CreatePtrToInt(arrayPtr, Type::getInt64Ty(TheContext));
}

/* 
  Generates LLVM code for creating boolean expression
  - the following boolean convention is set as: "true" returns 1, "false" returns 0
 */ 
llvm::Value *ASTBooleanExpr::codegen() {
  // Done
  LOG_S(1) << "Generating code for " << *this;
  if (getValue()) {
    return oneV;
  }else if(!getValue()){
    return zeroV;
  }else{
     throw InternalError("failed to generate bitcode for bool");
  }
}

/*
 * The code generated for an forIteratorStmt looks like this:
 *       <INIT> initializes the array
 *          v
 *       <HEADER>		
 *          /  ^  \ out of range
 *         v  /    \
 *      <BODY>     /
 *                /
 *               v
 *              nop        	this is called the "exit" block
 */
llvm::Value *ASTForIteratorStmt::codegen() {
  LOG_S(1) << "Generating code for " << *this;

  llvm::Function *TheFunction = Builder.GetInsertBlock()->getParent();

  labelNum++; // create shared labels for these BBs

  BasicBlock *InitBB = BasicBlock::Create(
      TheContext, "init" + std::to_string(labelNum), TheFunction);
  BasicBlock *HeaderBB = BasicBlock::Create(
      TheContext, "header" + std::to_string(labelNum));
  BasicBlock *BodyBB =
      BasicBlock::Create(TheContext, "body" + std::to_string(labelNum));
  BasicBlock *ExitBB =
      BasicBlock::Create(TheContext, "exit" + std::to_string(labelNum));

  // Add an explicit branch from the current BB to the Init block
  Builder.CreateBr(InitBB);

  // Emit loop init
  Builder.SetInsertPoint(InitBB);
  // trigger code generation for Element expression l-value
  lValueGen = true;
  Value *ElemV = getElement()->codegen();
  lValueGen = false;
  if (ElemV == nullptr) {
    throw InternalError(                                 // LCOV_EXCL_LINE
        "failed to generate bitcode for the loop element"); // LCOV_EXCL_LINE
  }

  // trigger code generation for Array expression
  Value *ArrayV = getArray()->codegen();
  if (ArrayV == nullptr) {
    throw InternalError(                                 // LCOV_EXCL_LINE
        "failed to generate bitcode for the loop array"); // LCOV_EXCL_LINE
  }

  Value *ArrayAddr = Builder.CreateIntToPtr(ArrayV, Type::getInt64PtrTy(TheContext), "ArrayAddr");
  Value *ArrayPtr = Builder.CreateIntToPtr(ArrayAddr, Type::getInt64PtrTy(TheContext), "ArrayPtr");

  // Array Length
  Value *ArrayLength = Builder.CreateLoad(Type::getInt64Ty(TheContext), ArrayAddr, "ArrayLength"); 

  // Index starts from 1 for implementing reason
  Value *Index = ConstantInt::get(Type::getInt64Ty(TheContext), 1);

  Builder.CreateBr(HeaderBB);

  // Emit loop header
  TheFunction->getBasicBlockList().push_back(HeaderBB);
  Builder.SetInsertPoint(HeaderBB);

  Value *CondV = Builder.CreateICmpSLE(Index, ArrayLength, "inrange");

  Builder.CreateCondBr(CondV, BodyBB, ExitBB);

  // Emit loop body
  TheFunction->getBasicBlockList().push_back(BodyBB);
  Builder.SetInsertPoint(BodyBB);
    
  // Assign element of array
  Value *ElemPtr = Builder.CreateGEP(ArrayPtr->getType()->getPointerElementType(), ArrayPtr, {Index}, "ElemPtr");
  Builder.CreateLoad(ElemPtr->getType()->getPointerElementType(), ElemPtr, "LoadElement");
        
  // Update index
  Index = Builder.CreateAdd(Index, ConstantInt::get(Type::getInt64Ty(TheContext), 1), "addtmp");

  // Emit loop body
  Value *BodyV = getBody()->codegen();
  if (BodyV == nullptr) {
    throw InternalError(                                 // LCOV_EXCL_LINE
        "failed to generate bitcode for the loop body"); // LCOV_EXCL_LINE
  }

  Builder.CreateBr(HeaderBB);

  // Emit loop exit block.
  TheFunction->getBasicBlockList().push_back(ExitBB);
  Builder.SetInsertPoint(ExitBB);
  return Builder.CreateCall(nop);
}


/*
 * The code generated for an forRangeStmt looks like this:
 *        <INIT>           this is executed only once
 *           v
 *          <TEST>
 *   true   /  ^  \   false
 *         v   |   v
 *      <BODY> /  nop      this is called the "exit" block
 *         v  v
 *      <UPDATE>
 */
llvm::Value *ASTForRangeStmt::codegen() {

  LOG_S(1) << "Generating code for " << *this;

  llvm::Function *TheFunction = Builder.GetInsertBlock()->getParent();

  // Create blocks for the loop init, header, body, and exit;
  labelNum++; // create shared labels for these BBs

  BasicBlock *InitBB = BasicBlock::Create(
      TheContext, "init" + std::to_string(labelNum), TheFunction);
  BasicBlock *UpdateBB = BasicBlock::Create(
      TheContext, "update" + std::to_string(labelNum));
  BasicBlock *TestBB = BasicBlock::Create(
      TheContext, "test" + std::to_string(labelNum));
  BasicBlock *BodyBB =
      BasicBlock::Create(TheContext, "body" + std::to_string(labelNum));
  BasicBlock *ExitBB =
      BasicBlock::Create(TheContext, "exit" + std::to_string(labelNum));

  // Add an explicit branch from the current BB to the Init
  Builder.CreateBr(InitBB);

  // Emit loop header, including init, test, and update
  {
  // Init block
  Builder.SetInsertPoint(InitBB);

  // trigger code generation for counter expression l-value
  lValueGen = true;
  Value *CounterLV = getCounter()->codegen();
  lValueGen = false;
  if (CounterLV == nullptr) {
    throw InternalError(                                 // LCOV_EXCL_LINE
        "failed to generate bitcode for the loop body"); // LCOV_EXCL_LINE
  }

  // trigger code generation for begin expression
  Value *BeginV = getBegin()->codegen();
  if (BeginV == nullptr) {
    throw InternalError(                                 // LCOV_EXCL_LINE
        "failed to generate bitcode for the begin of the range"); //LOV_EXCL_LINE
  }

  // trigger code generation for end expression
  Value *EndV = getEnd()->codegen();
  if (EndV == nullptr) {
    throw InternalError(                                        // LCOV_EXCL_LINE
        "failed to generate bitcode for the end of the range"); // LCOV_EXCL_LINE
  }

  // trigger code generation for step expression
  Value *StepV;
  if (getStep() == nullptr){
      StepV = ConstantInt::get(Type::getInt64Ty(TheContext), 1);     
  }
  else{
      StepV = getStep()->codegen();
  }
  // Value *StepV = getStep()->codegen();

  if (StepV == nullptr) {
    throw InternalError(                                         // LCOV_EXCL_LINE
        "failed to generate bitcode for the step of the range"); // LCOV_EXCL_LINE
  }

  // assign BeginV to CounterLV
  Builder.CreateStore(BeginV, CounterLV);

  // Add an explicit branch from the init to the header
  Builder.CreateBr(TestBB);

  // Test block
  TheFunction->getBasicBlockList().push_back(TestBB);
  Builder.SetInsertPoint(TestBB);

  auto CounterRV =  Builder.CreateLoad(CounterLV->getType()->getPointerElementType(), CounterLV, "rv");
  // Convert condition to a bool by comparing counter with end;
  auto CondV = Builder.CreateICmpSLT(CounterRV, EndV, "forcond");

  Builder.CreateCondBr(CondV, BodyBB, ExitBB);

  // Update block
  TheFunction->getBasicBlockList().push_back(UpdateBB);
  Builder.SetInsertPoint(UpdateBB);

  Value *UpdateV = Builder.CreateAdd(CounterRV, StepV, "addtmp");   
  Builder.CreateStore(UpdateV, CounterLV);
    
  Builder.CreateBr(TestBB);
  }

  // Emit loop body
  {
    TheFunction->getBasicBlockList().push_back(BodyBB);
    Builder.SetInsertPoint(BodyBB);

    Value *BodyV = getBody()->codegen();
    if (BodyV == nullptr) {
      throw InternalError(                                 // LCOV_EXCL_LINE
          "failed to generate bitcode for the loop body"); // LCOV_EXCL_LINE
    }

    Builder.CreateBr(UpdateBB);
  }

  // Emit loop exit block.
  TheFunction->getBasicBlockList().push_back(ExitBB);
  Builder.SetInsertPoint(ExitBB);
  return Builder.CreateCall(nop);
}

/*
  Generating LLVM code for a negation expression
  - Generates negation expression using `getArg()->codegen()`.
  - It checks if the generated Value is null, throwing an InternalError if so.
  - Create an LLVM negation instruction (`CreateNeg`), which negates the value of the argument.
  - Return Negation
*/
llvm::Value *ASTNegExpr::codegen() {
  // Done
  LOG_S(1) << "Generating code for " << *this;
  Value* argValue = getArg()->codegen();
  if (argValue == nullptr) {
    throw InternalError("NULL operand");
  }

  // Create an LLVM negation instruction
  return Builder.CreateNeg(argValue, "negtmp");
}

/*
  Generating LLVM code for the not expression
  - Generates not expression using `getArg()->codegen()`.
  - It checks if the generated Value is null, throwing an InternalError if so.
  - Create an LLVM not instruction (`CreateNot`), which executes the logical not.
  - Return logical not
*/
llvm::Value *ASTNotExpr::codegen() {
  // Done
  LOG_S(1) << "Generating code for " << *this;
  Value* argValue = getArg()->codegen();
  if (argValue == nullptr) {
    throw InternalError("NULL operand");
  }

  return Builder.CreateXor(argValue, oneV, "notmp");
}

/*
  Generates LLVM IR code for postfix operations.
  - Sets the lValueGen flag to true to generate code for an l-value expression.
  - Generates the LLVM Value for the l-value expression using `getArg()->codegen()` 
    checks if this Value is null, throwing an InternalError if it is null.
  - The lValueGen flag is set to false, and the r-value of the expression is generated.
  - rValue is stored back into the l-value.
  - Creates LLVM instruction (CreateAdd for '++', CreateSub for '--') to modify the r-value.
  - UpdateV is stored in the l-value, completing the postfix operation.
*/
llvm::Value *ASTPostfixStmt::codegen() {
  // Done
  LOG_S(1) << "Generating code for " << *this;

  // trigger code generation for l-value expressions
  lValueGen = true;
  Value *lValue = getArg()->codegen();
  lValueGen = false;
  if (lValue == nullptr) {
      throw InternalError(                                 // LCOV_EXCL_LINE
      "failed to generate lvalue bitcode for the argument of the statement"); // LCOV_EXCL_LINE
  }

  Value *rValue = getArg()->codegen();
  if (rValue == nullptr) {
      throw InternalError(                                 // LCOV_EXCL_LINE
      "failed to generate rvalue bitcode for the argument of the statement"); // LCOV_EXCL_LINE
  }

  Value *UpdateV;
  if (getOp() == "++") {
    UpdateV = Builder.CreateAdd(rValue, oneV, "addtmp");
  } else if (getOp() == "--") {
      UpdateV = Builder.CreateSub(rValue, oneV, "subtmp");
  }

  return Builder.CreateStore(UpdateV, lValue);
}

/*
Comment Here
*/
llvm::Value *ASTTernaryExpr::codegen() {
  LOG_S(1) << "Generating code for " << *this;

  Value *CondV = getCondition()->codegen();
  if (CondV == nullptr) {
      throw InternalError(                                   // LCOV_EXCL_LINE
      "failed to generate bitcode for the conditional expression"); // LCOV_EXCL_LINE
  }

  // Convert condition to a bool by comparing non-equal to 0.
  CondV = Builder.CreateICmpNE(CondV, ConstantInt::get(CondV->getType(), 0), "ternarycond");
  
  Value *ThenV = getThen()->codegen();
  if (ThenV == nullptr) {
      throw InternalError(                                   // LCOV_EXCL_LINE
      "failed to generate bitcode for the then expression"); // LCOV_EXCL_LINE
  }
  Value *ElseV = getElse()->codegen();
  if (ElseV == nullptr) {
      throw InternalError(                                   // LCOV_EXCL_LINE
      "failed to generate bitcode for the else expression"); // LCOV_EXCL_LINE
  }

  return Builder.CreateSelect(CondV, ThenV, ElseV);
}
