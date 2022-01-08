#pragma once
#ifndef YALX_IR_UTILS_H_
#define YALX_IR_UTILS_H_

#include "base/base.h"
#include <unordered_map>

namespace yalx {
namespace base {
class PrintingWriter;
} // namespace base
namespace ir {

class BasicBlock;
class Value;

class PrintingContext final {
public:
    explicit PrintingContext(int indent): indent_(indent) {}
    DEF_VAL_GETTER(int, indent);
    
    PrintingContext *OfIndent(base::PrintingWriter *printer);
    base::PrintingWriter *OfName(const Value *val, base::PrintingWriter *printer);
    base::PrintingWriter *OfName(const BasicBlock *val, base::PrintingWriter *printer);
    
    int Id(const Value *val) {
        if (auto iter = value_ids_.find(val); iter == value_ids_.end()) {
            const int id = next_value_id_++;
            value_ids_[val] = id;
            return id;
        } else {
            return iter->second;
        }
    }
    
    int Id(const BasicBlock *blk) {
        if (auto iter = block_ids_.find(blk); iter == block_ids_.end()) {
            const int id = next_block_id_++;
            block_ids_[blk] = id;
            return id;
        } else {
            return iter->second;
        }
    }
private:
    const int indent_;
    int next_value_id_ = 0;
    std::unordered_map<const Value *, int> value_ids_;
    int next_block_id_ = 0;
    std::unordered_map<const BasicBlock *, int> block_ids_;
}; // class PrintingContext

} // namespace ir

} // namespace yalx

#endif // YALX_IR_UTILS_H_
