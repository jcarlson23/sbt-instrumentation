#ifndef POINTS_TO_PLUGIN_H
#define POINTS_TO_PLUGIN_H

#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Constants.h>
#include <tuple>
#include "instr_plugin.hpp"
#include "llvm/analysis/PointsTo/PointsTo.h"
#include "analysis/PointsTo/PointsToWithInvalidate.h"
#include "call_graph.hpp"


using dg::analysis::pta::PSNode;
class PointsToPlugin : public InstrPlugin
{
    private:
        std::unique_ptr<dg::LLVMPointerAnalysis> PTA;
        CallGraph cg;

        std::string isNull(llvm::Value* a);
        std::string isValidPointer(llvm::Value* a, llvm::Value *len);
        std::string pointsTo(llvm::Value* a, llvm::Value *b);
        std::string hasKnownSize(llvm::Value* a);

    public:
        bool supports(const std::string& query) override;
        std::string query(const std::string& query,
                const std::vector<llvm::Value *>& operands) {
            if (query == "isValidPointer") {
                assert(operands.size() == 2 && "Wrong number of operands");
                return isValidPointer(operands[0], operands[1]);
            } else if (query == "pointsTo") {
                assert(operands.size() == 2 && "Wrong number of operands");
                return pointsTo(operands[0], operands[1]);
            } else if (query == "hasKnownSize") {
                assert(operands.size() == 1 && "Wrong number of operands");
                return hasKnownSize(operands[0]);
            } else if (query == "isNull") {
                assert(operands.size() == 1 && "Wrong number of operands");
                return isNull(operands[0]);
            } else {
                return "unsupported query";
            }
        }

        virtual std::tuple<llvm::Value*, uint64_t, uint64_t> getPointerInfo(llvm::Value* a);
        virtual void getReachableFunctions(std::set<const llvm::Function*>& reachableFunctions, const llvm::Function* a);

        PointsToPlugin(llvm::Module* module) : InstrPlugin("PointsTo") {
            llvm::errs() << "Running points-to analysis...\n";
            PTA = std::unique_ptr<dg::LLVMPointerAnalysis>(new dg::LLVMPointerAnalysis(module));
            PTA->run<dg::analysis::pta::PointsToWithInvalidate>();
            llvm::errs() << "Building call-graph...\n";
            cg.buildCallGraph(*module, PTA);
            llvm::errs() << "PT plugin done.\n";
        }
};

#endif

