#include "Optimizer.h"

// New PassBuilder
#include "llvm/Passes/PassBuilder.h"

// New Passes
#include "llvm/Passes/StandardInstrumentations.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Scalar/Reassociate.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"
#include "llvm/Transforms/Utils/Mem2Reg.h"

// P5 passes
#include "llvm/Transforms/Scalar/LICM.h"
#include "llvm/Transforms/Scalar/LoopDeletion.h"

// New
#include "llvm/Transforms/Scalar/LoopUnrollPass.h"
#include "llvm/Transforms/IPO/MergeFunctions.h"
#include "llvm/Transforms/Scalar/DCE.h"
#include "llvm/Transforms/Vectorize/SLPVectorizer.h"
#include "llvm/Transforms/IPO/Inliner.h"


#include "llvm/Passes/OptimizationLevel.h"
#include "llvm/Transforms/Scalar/TailRecursionElimination.h"
#include "llvm/Transforms/Scalar/EarlyCSE.h"
#include "llvm/Transforms/Scalar/CorrelatedValuePropagation.h"
// For logging
#include "loguru.hpp"

namespace { // Anonymous namespace for local function
	    
bool contains(Optimization o, llvm::cl::list<Optimization> &l) {
  for (unsigned i = 0; i != l.size(); ++i) {
    if (o == l[i]) return true;
  }
  return false;
}

}

//  NOTE:
//  We must use llvm Adaptors to run per-loop passes in the function pass
//  manager. In LLVM14+, The hierarchy for code sections is : Module -> (CGSCC)*
//  -> Functions -> Loops
//
//  [*] is optional.
//
//  eg: To run a loop pass on a module ->
//  ModulePassManager.add(functionAdaptor(LoopAdaptor(llvm::LoopPass())))

void Optimizer::optimize(llvm::Module *theModule, 
		llvm::cl::list<Optimization> &enabledOpts) {
  LOG_S(1) << "Optimizing program " << theModule->getName().str();

  // New pass builder

  llvm::PassBuilder passBuilder;

  // Setting-up Analysis Managers

  llvm::FunctionAnalysisManager functionAnalysisManager;
  llvm::ModuleAnalysisManager moduleAnalysisManager;
  llvm::LoopAnalysisManager loopAnalysisManager;
  llvm::CGSCCAnalysisManager cgsccAnalysisManager;

  // Registering the analysis managers with the pass builder

  passBuilder.registerModuleAnalyses(moduleAnalysisManager);
  passBuilder.registerCGSCCAnalyses(cgsccAnalysisManager);
  passBuilder.registerFunctionAnalyses(functionAnalysisManager);
  passBuilder.registerLoopAnalyses(loopAnalysisManager);
  // Cross Register Proxies
  passBuilder.crossRegisterProxies(loopAnalysisManager, functionAnalysisManager,
                                   cgsccAnalysisManager, moduleAnalysisManager);

  // Initiating Function and Module level PassManagers

  llvm::ModulePassManager modulePassManager;
  llvm::FunctionPassManager functionPassManager;
  llvm::LoopPassManager loopPassManagerWithMSSA;
  llvm::LoopPassManager loopPassManager;

  // Adding passes to the pipeline

  functionPassManager.addPass(llvm::PromotePass()); // New Reg2Mem
  functionPassManager.addPass(llvm::InstCombinePass());
  // Reassociate expressions.
  functionPassManager.addPass(llvm::ReassociatePass());
  // Eliminate Common SubExpressions.
  functionPassManager.addPass(llvm::GVNPass());
  // Simplify the control flow graph (deleting unreachable blocks, etc).
  functionPassManager.addPass(llvm::SimplifyCFGPass());
  
  // Dead Code Elimination (DCE): Removes code that does not affect the program's observable behavior.
  if (contains(dce, enabledOpts)) {
    functionPassManager.addPass(llvm::DCEPass()); 
  }  
  // Loop Unrolling: Expands the loop body multiple times,
  // reducing the loop overhead and increasing parallelism.
  if (contains(lu, enabledOpts)) {
    functionPassManager.addPass(llvm::LoopUnrollPass()); 
  } 
  // Merge function Pass
  if (contains(mfp, enabledOpts)){
    modulePassManager.addPass(llvm::MergeFunctionsPass());
  }

  // Inline pass
  if (contains(ilp, enabledOpts)){
    modulePassManager.addPass(llvm::ModuleInlinerPass());
  }

  // Early Common Subexpression Elimination (EarlyCSE): 
  // Identifies and eliminates redundant computations performed multiple times within the same block.
  if (contains(ecse, enabledOpts)) {
    functionPassManager.addPass(llvm::EarlyCSEPass()); 
  } 

  // Superword Level Parallelism Vectorizer (SLPVectorizer):
  // Combines several independent scalar operations into a single
  // vector operation to improve execution efficiency.
  // if (contains(slpv, enabledOpts)) {
  //   functionPassManager.addPass(llvm::SLPVectorizerPass()); 
  // } 

  // // Tail Call Elimination: Optimizes tail-recursive calls
  // // to be executed as a loop, reducing function call overhead and stack usage.
  // if (contains(tce, enabledOpts)) {
  //   functionPassManager.addPass(llvm::TailCallElimPass()); 
  // } 
  
  // // Correlated Value Propagation: Propagates information about
  // // values in conditional branches to optimize the branches and other instructions.
  // if (contains(cvp, enabledOpts)) {
  //   functionPassManager.addPass(llvm::CorrelatedValuePropagationPass()); 
  // }

  // Add loop pass managers with and w/out MemorySSA
  // functionPassManager.addPass(
  //     createFunctionToLoopPassAdaptor(std::move(loopPassManagerWithMSSA),true));

  // functionPassManager.addPass(
  //     createFunctionToLoopPassAdaptor(std::move(loopPassManager)));

  // Passing the function pass manager to the modulePassManager using a function
  // adaptor, then passing theModule to the ModulePassManager along with
  // ModuleAnalysisManager.

  modulePassManager.addPass(
      createModuleToFunctionPassAdaptor(std::move(functionPassManager), true));
  modulePassManager.run(*theModule, moduleAnalysisManager);
}