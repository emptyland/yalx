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
    std::string base_lib;
    std::string project_dir;
    int optimization = 0;
};

namespace {

const Option options_conf[] = {
    {
        offsetof(Options, base_lib),
        "string",
        "base-lib",
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

base::Status ParseOption(const Option *conf, const std::string_view value, Options *options) {
    switch (conf->type[0]) {
        case 's': { // string
            *OffsetOf<std::string>(options, conf->offset) = value;
        } break;
        case 'i': { // int
            *OffsetOf<int>(options, conf->offset) = ::atoi(value.data());
        } break;
        case 'b': { // bool
            if (value.empty()) {
                *OffsetOf<bool>(options, conf->offset) = true;
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
                *OffsetOf<bool>(options, conf->offset) = incoming;
            }
        } break;
        default:
            UNREACHABLE();
            break;
    }
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
        
        if (auto rs = ParseOption(iter->second, value, options); rs.fail()) {
            return rs;
        }
    }
    return base::Status::OK();
}

} // namespace


// yalx --dir=./ --base-lib=/usr/bin --optimization=0
/*static*/ int Compiler::Main(int argc, char *argv[]) {
    Options options;
    if (auto rs = ParseOptions(argc, argv, options_conf, &options); rs.fail()) {
        printf("%s\n", rs.ToString().c_str());
        return -1;
    }
    
    
    return 0;
}

static base::Status Build(const std::string &project_dir,
                          const std::string &base_lib,
                          int optimization) {
    
    
    return base::Status::OK();
}


} // namespace cpl

} // namespace yalx
