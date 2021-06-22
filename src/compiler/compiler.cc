#include "compiler/compiler.h"
#include "base/checking.h"
#include <map>

namespace yalx {
    
namespace cpl {

struct Option {
    ptrdiff_t offset;
    const char *type;
    const char *name;
    const char *comment;
};

struct Options {
    std::string base_dir;
    std::string project_dir;
    int optimization = 0;
};

namespace {

const Option options_conf[] = {
    {
        offsetof(Options, base_dir),
        "string",
        "base-dir",
        ""
    }, {
        offsetof(Options, project_dir),
        "string",
        "dir",
        ""
    }, {
        offsetof(Options, optimization),
        "int",
        "optimization",
        "[0, 3]"
    }, { // End of configuration
        0,
        nullptr,
        nullptr,
        nullptr
    }
}; // namespace

template<class T>
inline T *OffsetOf(Options *options, ptrdiff_t offset) {
    return reinterpret_cast<T *>(reinterpret_cast<Address>(options) + offset);
}

base::Status ParseOptions(int argc, char *argv[], const Option options_conf[], Options *options) {
    std::map<std::string_view, const Option *> mapped_conf;
    for (const Option *o = options_conf; o->name != nullptr; o++) {
        mapped_conf[o->name] = o;
    }
    
    // pattern: --name[=value]
    for (int i = 1; i < argc; i++) {
        if (::strstr(argv[i], "--") != argv[i]) {
            continue;
        }
        
        const char *p = argv[i] + 2; // skip "--"
        while (*p) {
            if (*p == '=') {
                break;
            }
            p++;
        }
        std::string_view name(argv[i] + 2, p - argv[i] + 2);
        auto iter = mapped_conf.find(name);
        if (iter == mapped_conf.end()) {
            continue;
        }
        
        std::string_view value;
        if (*p == '=') {
            value = std::string_view(p + 1);
        }
        
        switch (iter->second->type[0]) {
            case 's': { // string
                *OffsetOf<std::string>(options, iter->second->offset) = value;
            } break;
            case 'i': { // int
                *OffsetOf<int>(options, iter->second->offset) = ::atoi(value.data());
            } break;
            case 'b': { // bool
                if (value.empty()) {
                    *OffsetOf<bool>(options, iter->second->offset) = true;
                } else {
                    bool incoming = false;
                    if (value == "true") {
                        incoming == true;
                    } else if (value == "false") {
                        incoming == false;
                    } else if (value == "1") {
                        incoming == true;
                    } else if (value == "0") {
                        incoming == false;
                    }
                    *OffsetOf<bool>(options, iter->second->offset) = incoming;
                }
            } break;
            default:
                UNREACHABLE();
                break;
        }
    }
    
    return base::Status::OK();
}

} // namespace


// yalx --dir=./ --base-dir=/usr/bin --optimization=0
/*static*/ int Compiler::Main(int argc, char *argv[]) {
    
}


} // namespace cpl

} // namespace yalx
