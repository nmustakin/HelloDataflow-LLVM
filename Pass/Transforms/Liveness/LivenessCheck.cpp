#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/IR/PassManager.h"

#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/CFG.h"
#include <set>
#include <string>
#include <unordered_map>
#include <sstream> 
#include <queue>

using namespace std;

using namespace llvm;

namespace {

void printSet(set<string>  s){

    set<string>::iterator itr;
    
  // Displaying set elements
    for (itr = s.begin(); itr != s.end(); itr++) 
    {
        errs() << *itr << " ";
    }
    errs() <<"\n";
        
}


typedef unordered_map<BasicBlock*, set<string> > blockset; 

// This method implements what the pass does
void visitor(Function &F){
    // Here goes what you want to do with a pass
    
	string func_name = "main";
	errs() << "Liveness: " << F.getName() << "\n\n";
	    
	    // Comment this line
        //if (F.getName() != func_name) return;
	
    blockset UEVar; 
    blockset VarKill;
    blockset LiveOut; 
  
    queue<BasicBlock*> worklist = queue<BasicBlock*>();
    
    for (auto& basic_block : F){
        
        VarKill[&basic_block] = set<string>();
        UEVar[&basic_block] = set<string>(); 
        LiveOut[&basic_block] = set<string>();

        worklist.push(&basic_block);

        StringRef bbName(basic_block.getName());
        
        //errs() << bbName  << ":\n";
            
        for (auto& inst : basic_block){

            if(inst.getOpcode() == Instruction::Store || ((StringRef)inst.getOpcodeName())== "icmp"){

                int pick=1;
                StringRef compName(inst.getOpcodeName());
                if(compName == "icmp"){
                    //errs() << "Op Code:" << inst.getOpcodeName()<<"\n";
                    pick = 2;
                }
                    
                for(int p =0; p<pick;p++){
                        
                    string operand = inst.getOperand(p)->getName().str();
                    //errs() << "Op Code:" << inst.getOpcodeName()<<  "\n";
                    if (llvm::ConstantInt* CI = dyn_cast<llvm::ConstantInt>(inst.getOperand(p))) { 
                        //errs() << to_string(CI->getSExtValue()) << "\n";
                    }
                    
                    else if(llvm::Instruction* instr = dyn_cast<Instruction>(inst.getOperand(p))){
                        //errs() << *instr << "\n";
                        if(instr->getOpcode() == Instruction::Load){
                            
                            operand=(*(instr->getOperand(0))).getName().str();
                            
                            if(VarKill[&basic_block].find(operand) == VarKill[&basic_block].end()){
                                
                                UEVar[&basic_block].insert(operand);
                            }
                        }
                        else if(instr->isBinaryOp()){
                        //auto* ptr = dyn_cast<User>(instr);
                        //errs() << "\tBinary" << *instr << "\n";

                            int i=0;
                            for (i=0; i<2; i++){
                                llvm::User* x = dyn_cast<User>(instr->getOperand(i));
                                if (llvm::ConstantInt* CI = dyn_cast<llvm::ConstantInt>(x)) {
                                
                                }
                                else{
                                    for (auto err = x->op_begin(); err != x->op_end(); ++err){
                                        if ((*err)->hasName()) {
                                            operand = ((*err)->getName()).str();
                                            //errs() << "\t Binary: " << operand << "\n";
                                            if(VarKill[&basic_block].find(operand) == VarKill[&basic_block].end()){
                                                
                                                UEVar[&basic_block].insert(operand);
                                            }
                                        
                                        }
                                    }
                                }  
                            }
                        }
                    
                    }
                }

                if(pick ==1 ){
                    string operand2= inst.getOperand(1)->getName().str();
                    VarKill[&basic_block].insert(operand2);
                }
            } // end if
            
        } // end for inst
        
        
        
    } // end for block

    

    while(!worklist.empty()){
        BasicBlock* top = worklist.front();
        worklist.pop();
        set<string> newLiveOut= set<string>();
        for (BasicBlock *Succ : successors(top)) {
            
            set<string> dest1 = set<string>();
            set<string> dest2 = set<string>();

            set_difference(LiveOut[Succ].begin(), LiveOut[Succ].end(), VarKill[Succ].begin(), VarKill[Succ].end(), inserter(dest1, dest1.end()));
            set_union(dest1.begin(), dest1.end(), UEVar[Succ].begin(), UEVar[Succ].end(), inserter(dest2, dest2.end()));
            set_union(newLiveOut.begin(), newLiveOut.end(), dest2.begin(), dest2.end(), inserter(newLiveOut,newLiveOut.end()));
        }
        
        if(newLiveOut!=LiveOut[top]){
            LiveOut[top]= newLiveOut;
            for (BasicBlock *Pred : predecessors(top)){
                worklist.push(Pred);
            }
        }
    }

    for (auto& basic_block : F){
        StringRef bbName(basic_block.getName());
        
        errs() << "-----"<<bbName  << "-----\n";
            
        errs() << "UEVAR: ";
        printSet(UEVar[&basic_block]); 
        errs() << "VARKILL: ";
        printSet(VarKill[&basic_block]);
        errs() << "LiveOut: ";
        printSet(LiveOut[&basic_block]);
        errs() <<"\n";

    }
    
}


// New PM implementation
struct LivenessCheckPass : public PassInfoMixin<LivenessCheckPass> {

  // The first argument of the run() function defines on what level
  // of granularity your pass will run (e.g. Module, Function).
  // The second argument is the corresponding AnalysisManager
  // (e.g ModuleAnalysisManager, FunctionAnalysisManager)
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
  	visitor(F);
	return PreservedAnalyses::all();

	
  }
  
    static bool isRequired() { return true; }
};
}



//-----------------------------------------------------------------------------
// New PM Registration
//-----------------------------------------------------------------------------
extern "C" ::llvm::PassPluginLibraryInfo LLVM_ATTRIBUTE_WEAK
llvmGetPassPluginInfo() {
  return {
    LLVM_PLUGIN_API_VERSION, "LivenessCheckPass", LLVM_VERSION_STRING,
    [](PassBuilder &PB) {
      PB.registerPipelineParsingCallback(
        [](StringRef Name, FunctionPassManager &FPM,
        ArrayRef<PassBuilder::PipelineElement>) {
          if(Name == "liveness-check"){
            FPM.addPass(LivenessCheckPass());
            return true;
          }
          return false;
        });
    }};
}
