#include "external_types_plugin.hpp"
#include <llvm/IR/LegacyPassManager.h>

using namespace llvm;

bool ExternalTypesPlugin::processFunctions() {
  bool status = false;
  for (const llvm::Function &F : *mod) {
    if ( F.isDeclaration() ) {

      const AttributeList &Attrs = F.getAttributes();
      
      SmallVector<std::pair<unsigned, MDNode*>, 4> MDs;
      F.getAllMetadata(MDs);
      // we can now print metadata, if we want..

      auto linkage = F.getLinkage();

      auto  visibility = F.getVisibility();
      
      // get the return Type of the function
      llvm::Type * retType = F.getReturnType();

      // get the function type
      FunctionType* FT = F.getFunctionType();
      
      // get the argument information to the function
      for (unsigned I=0, E=FT->getNumParams(); I != E; ++I) {
	// from here we get the param type
	llvm::Type * pType = FT->getParamType(I);
	llvm::AttributeSet ArgAttrs = Attrs.getParamAttributes(I);
	// this goes to TypePrinter
      }
      
    }
  }

  status = true;
  
  return status;
}
