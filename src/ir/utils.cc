#include "ir/utils.h"
#include "ir/node.h"
#include "base/io.h"

namespace yalx {

namespace ir {

PrintingContext *PrintingContext::OfIndent(base::PrintingWriter *printer) {
    printer->Indent(indent());
    return this;
}

base::PrintingWriter *PrintingContext::OfName(const Value *val, base::PrintingWriter *printer) {
    if (val->name()) {
        printer->Write("%")->Write(val->name()->data());
    } else {
        printer->Print("%%%d", Id(val));
    }
    return printer;
}

base::PrintingWriter *PrintingContext::OfName(const BasicBlock *val, base::PrintingWriter *printer) {
    if (val->name()) {
        printer->Write(val->name()->data())->Write(":");
    } else {
        printer->Print("L%d:", val->id() < 0 ? Id(val) : val->id());
    }
    return printer;
}

PackageContext::PackageContext(base::Arena *arena)
: modules_(arena) {
}

void PackageContext::Associate(Module *module) { modules_[module->full_name()->ToSlice()] = module; }

} // namespace ir

} // namespace yalx
