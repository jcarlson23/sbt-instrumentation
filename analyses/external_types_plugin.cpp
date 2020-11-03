#include "external_types_plugin.hpp"
#include <llvm/IR/LegacyPassManager.h>

using namespace llvm;

bool ExternalTypesPlugin::processFunctions() {
  bool status = false;
  for (const llvm::Function &F : *mod) {
    if ( F.hasName() ) {

      
    }
  }

  status = true;
  
  return status;
}
