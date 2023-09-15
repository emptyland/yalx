#pragma once
#ifndef YALX_COMPILER_ASSEMBLY_GENERATING_DELEGATE_H_
#define YALX_COMPILER_ASSEMBLY_GENERATING_DELEGATE_H_

#include "base/status.h"
#include "base/base.h"
#include <string_view>

namespace yalx {
    namespace base {
        class PrintingWriter;
    } // namespace base
    namespace cpl {

        class AssemblyGeneratingDelegate {
        public:
            virtual ~AssemblyGeneratingDelegate() {}

            virtual bool Expired(std::string_view) = 0;

            virtual base::Status NewOutputPrinter(std::string_view, std::shared_ptr<base::PrintingWriter> *) = 0;

            DISALLOW_IMPLICIT_CONSTRUCTORS(AssemblyGeneratingDelegate);
        }; // class AssemblyGeneratingDelegate

    } // namespace cpl

} // namespace yalx

#endif // YALX_COMPILER_ASSEMBLY_GENERATING_DELEGATE_H_
