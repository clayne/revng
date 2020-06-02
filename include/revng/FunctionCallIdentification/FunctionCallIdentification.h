#ifndef FUNCTIONCALLIDENTIFICATION_H
#define FUNCTIONCALLIDENTIFICATION_H

//
// This file is distributed under the MIT License. See LICENSE.md for details.
//

// LLVM includes
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Pass.h"
#include "llvm/Support/Casting.h"

// Local libraries includes
#include "revng/BasicAnalyses/CustomCFG.h"
#include "revng/BasicAnalyses/GeneratedCodeBasicInfo.h"
#include "revng/Support/IRHelpers.h"

/// \brief Identify function call instructions
///
/// This pass parses the generated IR looking for terminator instructions which
/// look like function calls, i.e., they store the next program counter (the
/// return address) to a CSV (the link register) or in memory (the stack).
///
/// The pass also injects a call to the "funcion_call" function before
/// terminator instructions identified. The first argument represents the callee
/// basic block, the second the return basic block and the third the return
/// address.
class FunctionCallIdentification : public llvm::ModulePass {
public:
  static char ID;

public:
  FunctionCallIdentification() : llvm::ModulePass(ID) {}

  void getAnalysisUsage(llvm::AnalysisUsage &AU) const override {
    AU.setPreservesAll();
    AU.addRequired<GeneratedCodeBasicInfo>();
  }

  bool runOnModule(llvm::Module &M) override;

  llvm::CallInst *getCall(llvm::BasicBlock *BB) const {
    return getCall(BB->getTerminator());
  }

  llvm::CallInst *findPostDominatedCall(llvm::BasicBlock *BB) {
    using namespace llvm;

    do {
      for (Instruction &I : make_range(BB->rbegin(), BB->rend())) {
        if (getCallee(&I) == FunctionCall) {
          return cast<CallInst>(&I);
        }
      }

      BB = BB->getSinglePredecessor();
    } while (BB != nullptr);

    return nullptr;
  }

  /// \brief Return true if \p T is a function call in the input assembly
  llvm::CallInst *getCall(llvm::Instruction *T) const {
    revng_assert(T != nullptr);
    revng_assert(T->isTerminator());
    llvm::Instruction *Previous = getPrevious(T);
    while (Previous != nullptr && isMarker(Previous)) {
      auto *Call = llvm::cast<llvm::CallInst>(Previous);
      if (Call->getCalledFunction() == FunctionCall)
        return Call;

      Previous = getPrevious(Previous);
    }

    return nullptr;
  }

  /// \brief Return true if \p T is a function call in the input assembly
  bool isCall(llvm::Instruction *I) const { return getCall(I) != nullptr; }

  bool isCall(llvm::BasicBlock *BB) const {
    return isCall(BB->getTerminator());
  }

  llvm::BasicBlock *getFallthrough(llvm::BasicBlock *BB) const {
    return getFallthrough(BB->getTerminator());
  }

  llvm::BasicBlock *getFallthrough(llvm::Instruction *T) const {
    revng_assert(T != nullptr);
    revng_assert(T->isTerminator());
    llvm::Instruction *Previous = getPrevious(T);
    while (Previous != nullptr && isMarker(Previous)) {
      auto *Call = llvm::cast<llvm::CallInst>(Previous);
      if (Call->getCalledFunction() == FunctionCall) {
        auto *Fallthrough = llvm::cast<llvm::BlockAddress>(Call->getOperand(1));
        return Fallthrough->getBasicBlock();
      }

      Previous = getPrevious(Previous);
    }

    revng_abort();
  }

  bool isFallthrough(MetaAddress Address) const {
    return FallthroughAddresses.count(Address) != 0;
  }

  bool isFallthrough(llvm::BasicBlock *BB) const {
    return isFallthrough(getBasicBlockPC(BB));
  }

  bool isFallthrough(llvm::Instruction *I) const {
    revng_assert(I->isTerminator());
    return isFallthrough(I->getParent());
  }

  const CustomCFG &cfg() const { return FilteredCFG; }

private:
  void buildFilteredCFG(llvm::Function &F);

private:
  llvm::Function *FunctionCall;
  std::set<MetaAddress> FallthroughAddresses;
  CustomCFG FilteredCFG;
};

#endif // FUNCTIONCALLIDENTIFICATION_H
