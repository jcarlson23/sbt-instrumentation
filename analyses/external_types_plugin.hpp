#ifndef EXTERNAL_TYPES_PLUGIN_H
#define EXTERNAL_TYPES_PLUGIN_H

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <llvm/IR/Module.h>
#include <llvm/Support/raw_ostream.h>
#include "dg/llvm/ValueRelations/ValueRelations.h"
#include "instr_plugin.hpp"

namespace llvm {
  class Module;
  class Value;
}

class ExternalTypesPlugin : public InstrPlugin
{


  bool externalSuccess;
  llvm::Module* mod;

  bool processFunctions();
  
public:
  ExternalTypesPlugin(llvm::Module* module) : InstrPlugin("External") {
    externalSuccess = false;
    // here we do our analysis of the module, looking for
    // any external types.
    mod = module;
  }
  
  bool supports(const std::string& query) override {
    return query == "isExternalType";
  }

  std::string query(const std::string& query,
		    const std::vector<llvm::Value *>& operands) {
    return "Not Implemented";
  }

};

#endif
