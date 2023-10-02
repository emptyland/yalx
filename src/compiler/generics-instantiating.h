#pragma once
#ifndef YALX_COMPILER_GENERICS_INSTANTIATING_H_
#define YALX_COMPILER_GENERICS_INSTANTIATING_H_

#include "compiler/node.h"
#include "base/status.h"
#include "base/arena.h"
#include <functional>

namespace yalx::cpl {

    class SyntaxFeedback;

    class GenericsInstantiating final {
    public:
        //using Resolver = std::function<Statement *(std::string_view, std::string_view)>;
        class Resolver {
        public:
            Resolver() = default;

            virtual Statement *Find(std::string_view, std::string_view) = 0;

            virtual Statement *FindOrInsert(std::string_view, std::string_view, Statement *) = 0;

            virtual void Enter(Statement *) = 0;

            virtual void Exit(Statement *) = 0;
            DISALLOW_IMPLICIT_CONSTRUCTORS(Resolver);
        }; // class Resolver

        static base::Status Instantiate(const String *actual_name,
                                        Statement *def,
                                        base::Arena *arena,
                                        SyntaxFeedback *feedback,
                                        Resolver *resolver,
                                        size_t argc,
                                        Type **args,
                                        Statement **inst);

        DISALLOW_ALL_CONSTRUCTORS(GenericsInstantiating);
    }; // class GenericsInstantiating

} // namespace yalx

#endif // YALX_COMPILER_GENERICS_INSTANTIATING_H_
