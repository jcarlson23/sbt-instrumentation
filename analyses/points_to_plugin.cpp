#include "points_to_plugin.hpp"
#include <set>

using dg::analysis::pta::PSNode;

std::string PointsToPlugin::isNull(llvm::Value* a) {
    if (!a->getType()->isPointerTy())
        // null must be a pointer
        return "false";

    // need to have the PTA
    assert(PTA);
    PSNode *psnode = PTA->getPointsTo(a);
    if (!psnode || psnode->pointsTo.empty()) {
        llvm::errs() << "No points-to for " << *a << "\n";
        // we know nothing, it may be null
        return "unknown";
    }

    for (const auto& ptr : psnode->pointsTo) {
        // unknown pointer can be null too
        if (ptr.isNull() || ptr.isUnknown())
            return "true";
    }

    // a can not be null
    return "false";
}

std::string PointsToPlugin::hasKnownSize(llvm::Value* a) {
    // need to have the PTA
    assert(PTA);
    PSNode *psnode = PTA->getPointsTo(a);
    if (!psnode || psnode->pointsTo.size()!=1) {
        // we know nothing about the allocated size
        return "unknown";
    }

    const auto& ptr = *(psnode->pointsTo.begin());

    if (!ptr.target->getUserData<llvm::Value>())
        return "false";

    if (const llvm::AllocaInst *AI = llvm::dyn_cast<llvm::AllocaInst>(ptr.target->getUserData<llvm::Value>())) {
        if (llvm::ConstantInt *C = llvm::dyn_cast<llvm::ConstantInt>(AI->getOperand(0))) {
            if (llvm::Instruction *I = llvm::dyn_cast<llvm::Instruction>(a)) {
                if (AI->getFunction() == I->getFunction()) {
                    return "true";
                }
            }
        }
    }

    return "false";
}

std::pair<llvm::Value*, uint64_t> PointsToPlugin::getPointerInfo(llvm::Value* a) {
    // need to have the PTA
    assert(PTA);
    PSNode *psnode = PTA->getPointsTo(a);
    if (!psnode || psnode->pointsTo.size()!=1) {
        // we know nothing about the allocated size
        return std::make_pair(nullptr, 0);
    }

    const auto& ptr = *(psnode->pointsTo.begin());

    llvm::Value* node = ptr.target->getUserData<llvm::Value>();
    if (!node){
        return std::make_pair(nullptr, 0);}

    if (const llvm::AllocaInst *AI = llvm::dyn_cast<llvm::AllocaInst>(node)) {
        if (llvm::ConstantInt *C = llvm::dyn_cast<llvm::ConstantInt>(AI->getOperand(0))) {
            llvm::DataLayout* DL = new llvm::DataLayout(AI->getModule());
            llvm::Type* Ty = AI->getAllocatedType();
            if(!Ty->isSized()) {
                delete DL;
                return std::make_pair(nullptr, 0);
            }
            return std::make_pair(node, DL->getTypeAllocSize(Ty) * C->getZExtValue());
        }
    }
    return std::make_pair(nullptr, 0);
}


std::string PointsToPlugin::isValidPointer(llvm::Value* a, llvm::Value *len) {
    if (!a->getType()->isPointerTy())
        // null must be a pointer
        return "false";

    uint64_t size = 0;
    if (llvm::ConstantInt *C = llvm::dyn_cast<llvm::ConstantInt>(len)) {
        size = C->getLimitedValue();
        // if the size cannot be expressed as an uint64_t,
        // say we do not know
        if (size == ~((uint64_t) 0))
            return "unknown";

        // the offset is concrete number, fall-through
    } else {
        // we do not know anything with variable length
        return "unknown";
    }

    assert(size > 0 && size < ~((uint64_t) 0));

    // need to have the PTA
    assert(PTA);
    PSNode *psnode = PTA->getPointsTo(a);
    if (!psnode || psnode->pointsTo.empty()) {
        llvm::errs() << "No points-to for " << *a << "\n";
        // we know nothing, it may be invalid
        return "unknown";
    }

    for (const auto& ptr : psnode->pointsTo) {
        // unknown pointer and null are invalid
        if (ptr.isNull() || ptr.isUnknown())
            return "false";

        // the memory this pointer points-to was invalidated
        if (ptr.isInvalidated())
            return "false";

        // if the offset is unknown, than the pointer
        // may point after the end of allocated memory
        if (ptr.offset.isUnknown())
            return "unknown";

        // if the offset + size > the size of allocated memory,
        // then this can be invalid operation. Check it so that
        // we won't overflow, that is, first ensure that psnode->size <= size
        // and than use this fact and equality with ptr.offset + size > psnode->size)
        if (size > ptr.target->getSize()
                || *ptr.offset > ptr.target->getSize() - size) {
            return "false";
        }

        if (llvm::Instruction *I = llvm::dyn_cast<llvm::Instruction>(a)) {
            if(llvm::Instruction *Iptr = llvm::dyn_cast<llvm::Instruction>(ptr.target->getUserData<llvm::Value>())) {
                llvm::Function *F = I->getParent()->getParent();
                if(Iptr->getParent()->getParent() != F || cg.isRecursive(F))
                    return "false";
            } else {
                llvm::errs() << "In bound pointer for non-allocated memory: " << *a << "\n";
            }
        } else {
            llvm::errs() << "In bound pointer for non-allocated memory: " << *a << "\n";
        }
    }

    // this pointer is valid
    return "true";
}

std::string PointsToPlugin::pointsTo(llvm::Value* a, llvm::Value* b) {
    if(PTA) {
        PSNode *psnode = PTA->getPointsTo(a);
        if (!psnode) return "unknown";
        for (auto& ptr : psnode->pointsTo) {
            llvm::Value *llvmVal = ptr.target->getUserData<llvm::Value>();
            if(llvmVal == b) return "true";
        }
    } else {
        return "true"; // a may point to b
    }

    return "false";
}

void PointsToPlugin::getReachableFunctions(std::set<const llvm::Function*>& reachableFunctions, const llvm::Function* from) {
    cg.getReachableFunctions(reachableFunctions, from);
}

extern "C" InstrPlugin* create_object(llvm::Module* module) {
        return new PointsToPlugin(module);
}
