#include "compiler/compiler.h"
#include "compiler/parser.h"
#include "compiler/syntax-feedback.h"
#include "compiler/ast.h"
#include "base/checking.h"
#include "base/arena.h"
#include "base/env.h"
#include "base/io.h"
#include <map>
#include <filesystem>
#include <vector>

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
    bool dont_generate_code = false;
    bool just_checking = false;
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
    }, {
        offsetof(Options, dont_generate_code),
        "bool",
        "dont-generate-code",
        "Do not generated binary code, just checking syntax."
    }, {
        offsetof(Options, just_checking),
        "bool",
        "just-checking",
        "Do not generated binary code, just checking syntax."
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
    
    if (auto rs = Build(options.project_dir, options.base_lib, options.optimization); rs.fail()) {
        printf("%s\n", rs.ToString().c_str());
        return -1;
    }
    return 0;
}

namespace fs = std::filesystem;

base::Status Compiler::Build(const std::string &project_dir,
                             const std::string &base_lib,
                             int optimization) {
    fs::path dir = project_dir.empty() ? fs::current_path() : fs::path(project_dir);
    if (base_lib.empty()) {
        return ERR_INVALID_ARGUMENT("Base library dir has not specified.");
    }
    fs::path base_dir(base_lib);
    if (!fs::exists(base_dir) || !fs::is_directory(base_dir)) {
        return ERR_CORRUPTION("Base library is not a directory.");
    }
    if (!fs::exists(dir) || !fs::is_directory(dir)) {
        return ERR_CORRUPTION("Project dir is not a directory.");
    }
    
    std::vector<std::string> source_files;
    for(auto& entry: fs::recursive_directory_iterator(dir)) {
        if (fs::is_directory(entry.path())) {
            continue;
        }
        const auto name = entry.path().string();
        if (name.rfind(kSourceExtendedName) == name.size() - strlen(kSourceExtendedName)) {
            source_files.push_back(name);
        }
    }
    
    for(auto& entry: fs::recursive_directory_iterator(base_dir)) {
        if (fs::is_directory(entry.path())) {
            continue;
        }
        const auto name = entry.path().string();
        if (name.rfind(kSourceExtendedName) == name.size() - strlen(kSourceExtendedName)) {
            source_files.push_back(name);
        }
    }
    
    base::Arena arena;
    
    
    return base::Status::OK();
}


base::Status Compiler::ParseAllSourceFiles(const std::vector<std::string> &files) {
    
    return base::Status::OK();
}

static Package *FindPackageByPath(const std::vector<std::string> &search_paths,
                                  std::string_view key,
                                  base::ArenaMap<std::string_view, Package *> *all) {
    for (auto path : search_paths) {
        path.append("/").append(key);
        if (auto iter = all->find(path); iter != all->end()) {
            return iter->second;
        }
    }
    return nullptr;
}

static base::Status RecursiveFindEntries(const std::vector<std::string> &search_paths,
                                  std::set<Package *> *unique_pkgs,
                                  SyntaxFeedback *error_feedback,
                                  Package *entry,
                                  base::ArenaVector<Package *> *entries,
                                  base::ArenaMap<std::string_view, Package *> *all) {
    for (auto file_unit : entry->source_files()) {
        for (auto import : file_unit->imports()) {
            if (!import->original_package_name()) {
                auto pkg = FindPackageByPath(search_paths, import->package_path()->ToSlice(), all);
                import->set_original_package_name(DCHECK_NOTNULL(pkg)->name());
            }
        }
    }
    
    if (entry->dependences_size() == 0) {
        auto iter = std::find(entries->begin(), entries->end(), entry);
        if (iter == entries->end()) {
            entries->push_back(entry);
        }
        return base::Status::OK();
    }

    unique_pkgs->insert(entry);
    for (auto [_, import] : entry->imports()) {
        if (unique_pkgs->find(import.pkg) != unique_pkgs->end()) {
            error_feedback->set_file_name(import.file_unit->file_name()->ToString());
            error_feedback->Printf(import.entry->source_position(), "Import dependence-ring package");
            return ERR_CORRUPTION("Import ring package");
        }
        
        if (auto rs = RecursiveFindEntries(search_paths, unique_pkgs, error_feedback, import.pkg, entries, all);
            rs.fail()) {
            return rs;
        }
    }
    
    return base::Status::OK();
}

base::Status Compiler::FindAndParseProjectSourceFiles(const std::string &project_dir,
                                                      const std::string &base_lib,
                                                      base::Arena *arena,
                                                      SyntaxFeedback *error_feedback,
                                                      Package **main_pkg,
                                                      base::ArenaVector<Package *> *entries,
                                                      base::ArenaMap<std::string_view, Package *> *all) {
    if (auto rs = FindAndParseMainSourceFiles(project_dir, arena, error_feedback, main_pkg); rs.fail()) {
        return rs;
    }
    
    std::string path(project_dir);
    std::vector<std::string> search_paths;
    path.append("/").append(kSourceDirName);
    search_paths.push_back(path);
    path.assign(project_dir);
    path.append("/").append(kPackageDirName);
    search_paths.push_back(path);
    search_paths.push_back(base_lib);
    
    if (auto rs = FindAndParseAllDependencesSourceFiles(search_paths, arena, error_feedback, *main_pkg, all);
        rs.fail()) {
        return rs;
    }
    
    std::set<Package *> unique_pkgs;
    return RecursiveFindEntries(search_paths, &unique_pkgs, error_feedback, *main_pkg, entries, all);
    //return base::Status::OK();
}

base::Status Compiler::FindAndParseMainSourceFiles(const std::string &project_dir,
                                                   base::Arena *arena,
                                                   SyntaxFeedback *error_feedback,
                                                   Package **receiver) {
    fs::path path(project_dir);
    path.append(kSourceDirName).append(kMainPkgName);
    if (!fs::exists(path) || !fs::is_directory(path)) {
        return ERR_NOT_FOUND();
    }
    
    return ParsePackageSourceFiles(path.string(), kMainPkgName, arena, error_feedback, receiver);
}

std::string FindInSearchPaths(const std::vector<std::string> &search_paths, std::string_view import_path) {
    for (auto item : search_paths) {
        fs::path path(item);
        path.append(import_path);

        if (fs::is_directory(path)) {
            return path.string();
        }
    }
    return "";
}

base::Status Compiler::FindAndParseAllDependencesSourceFiles(const std::vector<std::string> &search_paths,
                                                             base::Arena *arena,
                                                             SyntaxFeedback *error_feedback,
                                                             Package *root,
                                                             base::ArenaMap<std::string_view, Package *> *all) {
    for (auto [import_path, import] : root->imports()) {
        auto pkg_path = FindInSearchPaths(search_paths, import_path);
        if (pkg_path.empty()) {
            error_feedback->set_file_name(import.file_unit->file_name()->ToString());
            error_feedback->Printf(import.entry->source_position(), "Import \"%s\" path not found", import_path.data());
            return ERR_CORRUPTION("import path not found");
        }
        
        Package *pkg = nullptr;
        if (auto iter = all->find(pkg_path); iter != all->end()) {
            pkg = iter->second;
        } else {
            if (auto rs = ParsePackageSourceFiles(pkg_path, import_path, arena, error_feedback, &pkg); rs.fail()) {
                return rs;
            }
            (*all)[pkg->full_path()->ToSlice()] = pkg;
            if (auto rs = FindAndParseAllDependencesSourceFiles(search_paths, arena, error_feedback, pkg, all);
                rs.fail()) {
                return rs;
            }
        }
        root->import(import_path)->pkg = pkg;
        root->dependences_.push_back(pkg);
        pkg->references_.push_back(root);
    }
    
    return base::Status::OK();
}

base::Status Compiler::ParsePackageSourceFiles(std::string_view pkg_dir,
                                               std::string_view import_path,
                                               base::Arena *arena,
                                               SyntaxFeedback *error_feedback,
                                               Package **receiver) {
    fs::path path(pkg_dir);
    if (!fs::exists(path) || !fs::is_directory(path)) {
        return ERR_NOT_FOUND();
    }
    
    base::ArenaVector<FileUnit *> files(arena);
    Parser parser(arena, error_feedback);
    const String *pkg_name = String::New(arena, path.filename().string());
    FileUnit::ImportEntry *default_import = nullptr;
    for(auto& entry: fs::recursive_directory_iterator(path)) {
        if (fs::is_directory(entry.path())) {
            continue;
        }
        const auto name = entry.path().filename().string();
        if (name.rfind(kSourceExtendedName) == name.size() - strlen(kSourceExtendedName)) {
            const auto file_path = entry.path().string();
            std::unique_ptr<base::SequentialFile> file;
            if (auto rs = base::Env::NewSequentialFile(file_path, &file); rs.fail()) {
                return rs;
            }
            parser.SwitchInputFile(entry.path().string(), file.get());
            bool ok = true;
            auto file_unit = parser.Parse(&ok);
            if (!ok) {
                return ERR_CORRUPTION("Syntax error");
            }
            files.push_back(file_unit);
            
            if (!DCHECK_NOTNULL(file_unit->package_name())->Equal(pkg_name)) {
                error_feedback->set_file_name(file_unit->file_name()->ToString());
                error_feedback->Printf(file_unit->source_position(), "Different package name: %s, need: main",
                                       file_unit->package_name()->data());
                return ERR_CORRUPTION("Syntax error");
            }
            
            // Default import:
            // import "yalx/lang" as *
            if (import_path.compare(kDefaultImport) != 0) { // Exclude yalx/lang package self
                if (!default_import) {
                    default_import = new (arena) FileUnit::ImportEntry(String::New(arena, "lang"),
                                                                       String::New(arena, kDefaultImport),
                                                                       String::New(arena, "*"), {1,1,1,1});
                }
                file_unit->mutable_imports()->push_back(default_import);
            }
            pkg_name = file_unit->package_name();
        }
    }
    
    auto pkg_id = String::New(arena, path.string().append(":").append(pkg_name->data()));
    auto pkg_path = String::New(arena, import_path);
    auto pkg_full_path = String::New(arena, path.string());
    *receiver = new (arena) Package(arena, pkg_id, pkg_path, pkg_full_path, pkg_name);
    
    for (auto file_unit : files) {
        for (auto stmt : file_unit->statements()) {
            if (Declaration::Is(stmt)) {
                static_cast<Declaration *>(stmt)->set_package(*receiver);
                if (auto decl = static_cast<Declaration *>(stmt); decl != nullptr) {
                    for (size_t i = 0; i < decl->ItemSize(); i++) {
                        decl->AtItem(i)->set_package(*receiver);
                    }
                }
            }
            if (Definition::Is(stmt)) {
                static_cast<Definition *>(stmt)->set_package(*receiver);
            }
        }
    }
    *(*receiver)->mutable_source_files() = std::move(files);
    (*receiver)->Prepare();
    
    return base::Status::OK();
}

} // namespace cpl

} // namespace yalx
