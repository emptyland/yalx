#include "runtime/runtime.h"
#include "runtime/thread.h"
#include "runtime/locks.h"
#include "runtime/scheduler.h"
#include "runtime/process.h"
#include "runtime/checking.h"
#include "runtime/hash-table.h"
#include "runtime/heap/heap.h"
#include "runtime/heap/object-visitor.h"
#include "runtime/object/yalx-string.h"
#include "runtime/object/number.h"
#include "runtime/object/throwable.h"
#include "runtime/object/arrays.h"
#include "runtime/object/type.h"
#include <unistd.h>
#if defined(YALX_OS_DARWIN)
#include <sys/sysctl.h>
#endif
#if defined(YALX_OS_LINUX)
#include <sys/sysinfo.h>
#endif
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdatomic.h>
#include <inttypes.h>
//#define _XOPEN_SOURCE
//#include <ucontext.h>

#ifdef __cplusplus
#error No C++
#endif

int ncpus = 0;

int os_page_size = 0;

size_t pointer_size_in_bytes = sizeof(void *);
int pointer_shift_in_bytes = 0;
size_t pointer_size_in_bits = sizeof(void *) * 8;
int pointer_shift_in_bits = 0;
int pointer_mask_in_bits = 0;

const struct yalx_str yalx_version = YALX_STR("yalx-lang v0.0.0");

struct processor *procs = NULL;

int nprocs = 0;

struct machine m0;
struct coroutine c0;

yalx_tls_t tls_mach;

struct stack_pool stack_pool;


// External symbol from generated code
// 1347046214, 1465142347, 1195658056
extern uint32_t yalx_magic_number1;
extern uint32_t yalx_magic_number2;
extern uint32_t yalx_magic_number3;

#define PKG_RECORD_VAL_SIZE (pointer_size_in_bytes * 2)
//
// const str
//
// global var slots
//
static struct hash_table pkg_init_records;
static struct yalx_mutex pkg_init_mutex;

struct lksz_header {
    int number_of_strings;
    const char *const sz[1];
};

struct kstr_header {
    int number_of_strings;
    struct yalx_value_str *ks[1];
};

#if defined(YALX_OS_LINUX)
void *const YALX_DL_HANDLE = RTLD_LOCAL;
#endif
#if defined(YALX_OS_DARWIN)
void *const YALX_DL_HANDLE = RTLD_MAIN_ONLY;
#endif

#ifndef NDEBUG

struct dev_struct_field {
    ptrdiff_t       offset;
    struct yalx_str name;
}; // struct dev_struct_field

#define DECLARE_FIELD(type, name) {offsetof(struct type, name), YALX_STR(#name)}
#define DECLARE_END() {0, {NULL, 0}}

static const struct dev_struct_field scheduler_fields[] = {
    DECLARE_FIELD(scheduler, mutex),
    DECLARE_FIELD(scheduler, next_coid),
    DECLARE_END()
};

static const struct dev_struct_field coroutine_fields[] = {
    DECLARE_FIELD(coroutine, next),
    DECLARE_FIELD(coroutine, prev),
    DECLARE_FIELD(coroutine, id),
    DECLARE_FIELD(coroutine, state),
    DECLARE_FIELD(coroutine, stack),
    DECLARE_FIELD(coroutine, entry),
    DECLARE_FIELD(coroutine, c_sp),
    DECLARE_FIELD(coroutine, c_fp),
    DECLARE_FIELD(coroutine, n_pc),
    DECLARE_FIELD(coroutine, n_sp),
    DECLARE_FIELD(coroutine, n_fp),
    DECLARE_FIELD(coroutine, stub),
    DECLARE_END()
};

static const struct dev_struct_field machine_fields[] = {
    DECLARE_FIELD(machine, next),
    DECLARE_FIELD(machine, prev),
    DECLARE_FIELD(machine, running),
    DECLARE_END()
};

static const struct dev_struct_field stack_fields[] = {
    DECLARE_FIELD(stack, next),
    DECLARE_FIELD(stack, prev),
    DECLARE_FIELD(stack, core),
    DECLARE_FIELD(stack, top),
    DECLARE_FIELD(stack, bottom),
    DECLARE_FIELD(stack, size),
    DECLARE_END()
};

static const struct dev_struct_field string_fields[] = {
    DECLARE_FIELD(yalx_value_str, hash_code),
    DECLARE_FIELD(yalx_value_str, len),
    DECLARE_FIELD(yalx_value_str, bytes),
    DECLARE_END()
};

#undef DECLARE_FIELD



static void dev_print_fields(const struct dev_struct_field *field) {
    for (;field->name.z != NULL; field++) {
        printf("    %s %zd 0x%04zx\n", field->name.z, field->offset, field->offset);
    }
}

static void dev_print_struct_fields(void) {
    printf("magic numbers: %u, %u, %u\n", yalx_magic_number1, yalx_magic_number2, yalx_magic_number3);
    
    printf("struct scheduler:\n");
    dev_print_fields(scheduler_fields);
    printf("struct machine:\n");
    dev_print_fields(machine_fields);
    printf("struct coroutine:\n");
    dev_print_fields(coroutine_fields);
    printf("struct stack:\n");
    dev_print_fields(stack_fields);
    printf("struct string:\n");
    dev_print_fields(string_fields);
}

#else // #ifndef NDEBUG

static void dev_print_struct_fields() {}

#endif



struct class_load_entry {
    const char *name;
    const struct yalx_class **location;
} classes_loading_entries[] = {
    {THROWABLE_CLASS_NAME, &throwable_class},
    {EXCEPTION_CLASS_NAME, &exception_class},
    {BACKTRACE_FRAME_CLASS_NAME, &backtrace_frame_class},
    {BAD_CASTING_EXCEPTION_CLASS_NAME, &bad_casting_exception_class},
    {ARRAY_INDEX_OUT_OF_BOUNDS_EXCEPTION_CLASS_NAME, &array_index_out_of_bounds_exception_class},
    {NULL, NULL} // end of entries
};

int yalx_runtime_init(const struct yalx_runtime_options *options) {
    pointer_shift_in_bytes = yalx_log2(pointer_size_in_bytes);
    pointer_shift_in_bits = yalx_log2(pointer_size_in_bits);
    pointer_mask_in_bits = (int)((1U << pointer_shift_in_bits) - 1);

#if defined(YALX_OS_DARWIN)
    os_page_size = getpagesize();
    
    int mib[2] = { CTL_HW, HW_AVAILCPU };
    unsigned cpu_count;
    size_t size_of_cpu_count = sizeof(cpu_count);
    int status = sysctl(mib, 2, &cpu_count, &size_of_cpu_count, NULL, 0);
    if (status != 0) {
        return -1;
    }
    ncpus = (int)cpu_count;
#endif // defined(YALX_OS_DARWIN)

#if defined(YALX_OS_LINUX)
    os_page_size = getpagesize();

    ncpus = get_nprocs();
#endif // defined(YALX_OS_LINUX)

    yalx_os_threading_env_enter();
    yalx_mutex_init(&pkg_init_mutex);
    yalx_tls_alloc(&tls_mach);

    yalx_init_hash_table(&pkg_init_records, 1.2f);
    if (yalx_init_heap(options->gc, options->max_heap_in_bytes, &heap) < 0) {
        goto error;
    }
    
    yalx_init_stack_pool(&stack_pool, 4 * GB);
    
    yalx_init_machine(&m0, &procs[0]);
    yalx_os_thread_attach_self(&m0.thread);

    yalx_tls_set(tls_mach, &m0);
    
    struct stack *s0 = yalx_new_stack_from_pool(&m0.stack_pool, 1 * MB);
    if (!s0) {
        goto error;
    }
    coid_t coid;
    coid.value = 0;
    yalx_init_coroutine(coid, &c0, s0, (address_t)y2zmain_main);

    nprocs = ncpus;
    procs = malloc(nprocs * sizeof(struct processor));
    if (!procs) {
        goto error;
    }
    
    for (int i = 0; i < nprocs; i++) {
        procid_t id = {i};
        if (yalx_init_processor(id, &procs[i]) < 0) {
            free(procs);
            goto error;
        }
    }
    
    yalx_add_machine_to_processor(&procs[0], &m0);
    
    yalx_init_scheduler(&scheduler);
    

    // FIXME:
#if 0
    const struct yalx_class *ty = yalx_find_class(ANY_CLASS_NAME);
    if (!ty) {
        die("Any class not found");
        return -1;
    }
    memcpy(&builtin_classes[Type_any], ty, sizeof(*ty));
    
    ty = yalx_find_class(STRING_CLASS_NAME);
    if (!ty) {
        die("String class not found");
        return -1;
    }
    memcpy(&builtin_classes[Type_string], ty, sizeof(*ty));
    
    for (struct class_load_entry *entry = &classes_loading_entries[0]; entry->name != NULL; entry++) {
        ty = yalx_find_class(entry->name);
        if (!ty) {
            die("`%s' class not found", entry->name);
            return -1;
        }
        *entry->location = ty;
    }
#endif // 
    dev_print_struct_fields();
    return 0;
error:
    yalx_tls_free(tls_mach);
    yalx_mutex_final(&pkg_init_mutex);
    yalx_os_threading_env_exit();
    heap = NULL;
    return -1;
}

void yalx_runtime_eixt(void) {
    yalx_free_hash_table(&pkg_init_records);
    yalx_free_scheduler(&scheduler);
    yalx_free_heap(heap);

    yalx_tls_free(tls_mach);
    yalx_mutex_final(&pkg_init_mutex);
    yalx_os_threading_env_exit();
    heap = NULL;
    // TODO:

}

void yalx_global_visit_root(struct yalx_root_visitor *visitor) {
    yalx_mutex_lock(&pkg_init_mutex);

    const size_t n = 1u << pkg_init_records.capacity_shift;
    for (size_t i = 0; i < n; i++) {
        struct hash_table_slot *const slot = &pkg_init_records.slots[i];
        for (struct hash_table_slot *node = slot->next; node != slot; node = node->next) {
            hash_table_value_span_t span;
            span.value = &node->key[(ROUND_UP(node->key_size, 4))];
            span.size  = node->value_size;

            if (span.size < PKG_RECORD_VAL_SIZE) {
                continue; // No constants pool
            }

            struct kstr_header *const_str = (struct kstr_header *)(((void **)span.value)[0]);
            struct pkg_global_slots *global_slots = (struct pkg_global_slots *)(((void **)span.value)[1]);

            visitor->visit_pointers(visitor,
                                    (yalx_ref_t *) const_str->ks,
                                    (yalx_ref_t *) (const_str->ks + const_str->number_of_strings));

            USE(global_slots);
            DLOG(INFO, "Visit global slots has not support yet");
        }
    }

    yalx_mutex_unlock(&pkg_init_mutex);
}

int yalx_rt0(int argc, char *argv[]) {
    USE(argc);
    USE(argv);
    
    if (yalx_magic_number1 != YALX_MAGIC_NUMBER1) {
        die("bad magic number.");
        return -1;
    }
    if (yalx_magic_number2 != YALX_MAGIC_NUMBER2) {
        die("bad magic number.");
        return -1;
    }
    if (yalx_magic_number3 != YALX_MAGIC_NUMBER3) {
        die("bad magic number.");
        return -1;
    }
    
    DCHECK(thread_local_mach != NULL);
    DCHECK(c0.state != CO_RUNNING);
    c0.state = CO_RUNNING;
    m0.state = MACH_RUNNING;
    
    m0.running = &c0;
    // Jump in to yalx lang env:
    trampoline();
    
    m0.state = MACH_IDLE;
    c0.state = CO_DEAD;
    return 0;
}

void die(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    putc('\n', stderr);
    va_end(ap);
    exit(-1);
}

void *fill_memory_zag(void *chunk, size_t n, uint32_t zag) {
    if (!chunk) {
        return NULL;
    }
    uint32_t *p = (uint32_t *)chunk;
    for (int i = 0; i < n / sizeof(zag); i++) {
        *p++ = zag;
    }
    const size_t r = n % sizeof(zag);
    address_t x = ((address_t)chunk) + n - r;
    switch (r) {
        case 1:
            *x = ((address_t)&zag)[0];
            break;
        case 2:
            *x++ = ((address_t)&zag)[0];
            *x++ = ((address_t)&zag)[1];
            break;
        case 3:
            *x++ = ((address_t)&zag)[0];
            *x++ = ((address_t)&zag)[1];
            *x++ = ((address_t)&zag)[2];
            break;
            
        case 0:
        default:
            break;
    }
    return chunk;
}

void dbg_class_output(const struct yalx_class *klass) {
    for (int i = 0; i < klass->n_methods; i++) {
        printf("%s%s\n", klass->methods[i].name.z, klass->methods[i].prototype_desc.z);
    }
}

// reference by yalx.lang.debugOutput
void dbg_output(yalx_ref_t obj) {
    DCHECK(obj != NULL);
    struct yalx_class *klass = CLASS(obj);
    //dbg_class_output(klass);

    switch (yalx_builtin_type(klass)) {
        case Type_string: {
            struct yalx_value_str *str = (struct yalx_value_str *)obj;
            printf("DBG %s\n", str->bytes);
        } break;
            
        case Type_I8:
            printf("DBG %d\n", ((struct yalx_value_number_l *)obj)->box.i8);
            break;
            
        case Type_U8:
            printf("DBG %u\n", ((struct yalx_value_number_l *)obj)->box.u8);
            break;
            
        case Type_I16:
            printf("DBG %d\n", ((struct yalx_value_number_l *)obj)->box.i16);
            break;
            
        case Type_U16:
            printf("DBG %u\n", ((struct yalx_value_number_l *)obj)->box.u16);
            break;
            
        case Type_I32:
            printf("DBG %d\n", ((struct yalx_value_number_l *)obj)->box.i32);
            break;
            
        case Type_U32:
            printf("DBG %u\n", ((struct yalx_value_number_l *)obj)->box.u32);
            break;
            
        case Type_I64:
            printf("DBG %"PRId64"\n", ((struct yalx_value_number_w *)obj)->box.i64);
            break;
            
        case Type_U64:
            printf("DBG %"PRIu64"\n", ((struct yalx_value_number_w *)obj)->box.u64);
            break;
            
        case Type_F32:
            printf("DBG %f\n", ((struct yalx_value_number_l *)obj)->box.f32);
            break;

        case Type_F64:
            printf("DBG %f\n", ((struct yalx_value_number_w *)obj)->box.f64);
            break;
            
        case NOT_BUILTIN_TYPE:
        default:
            DCHECK("Noreachable");
            break;
    }
}


void *yalx_zalloc(size_t n) {
    void *chunk = malloc(n);
    if (!chunk) {
        return NULL;
    }
    memset(chunk, 0, n);
    return chunk;
}

char *yalx_symbol_mangle(const char *const plain_name, const char *postfix) {
    const size_t extra_len = !postfix ? 0 : strlen(postfix);
    char *symbol = NULL;
    size_t buf_size = 98 + extra_len;
    int mangled_size = 0;
    do {
        free(symbol);
        buf_size <<= 1;
        symbol = (char *)realloc(symbol, buf_size);
        mangled_size = yalx_name_symbolize(plain_name, symbol, buf_size);
    } while(mangled_size < 0);
    strncat(symbol, postfix, mangled_size);
    return symbol;
}

size_t yalx_symbol_demangle_on_place(char symbol[], size_t size) {
    char *p = &symbol[0];
    const char *const end = p + size;
    if (size > 2 && p[0] == '_' && p[1] == '_') {
        return size;
    }
    if (*p == '_') {
        size--;
        memmove(p, p + 1, size - 1); // remove prefix `_'
    }
    while (p < end) {
    retry:
        if (*p != '_') {
            p++;
            continue;
        }
        
        char *dash = p++;
        switch (*p) {
            case 'Z': {
                p++;
                char next = *p;
                switch (next) {
                    case 'd':
                        *dash = '.';
                        break;
                    case '4':
                        *dash = '$';
                        break;
                    case 'o':
                        *dash = ':';
                        break;
                    case 'p':
                        *dash = '/';
                        break;
                    default:
                        p++;
                        goto retry;
                }
                if (end - dash - 3 > 0) {
                    // _Zd
                    // .Zd
                    size -= 2;
                    memmove(dash + 1, dash + 3, end - dash - 3);
                }
            } break;
            case 'D': {
                p++;
                char next = *p;
                switch (next) {
                    case 'k':
                        *dash = '<';
                        break;
                    case 'l':
                        *dash = '>';
                        break;
                    default:
                        p++;
                        goto retry;
                }
                if (end - dash - 3 > 0) {
                    // _Zd
                    // .Zd
                    size -= 2;
                    memmove(dash + 1, dash + 3, end - dash - 3);
                }
            } break;
            default:
                p++;
                break;
        }
    }
    return size;
}

int yalx_name_symbolize(const char *const plain_name, char symbol[], size_t size) {
    char *p = symbol;
    const char *const limit = p + size;
    
    *p++ = '_';

    const char *z = plain_name;
    while (*z) {
        char c = *z++;
        const char *escape = NULL;
        size_t n = 0;
        switch (c) {
            case '.':
                escape = "_Zd";
                n = 3;
                break;
            case '$':
                escape = "_Z4";
                n = 3;
                break;
            case ':':
                escape = "_Zo";
                n = 3;
                break;
            case '<':
                escape = "_Dk";
                n = 3;
                break;
            case '>':
                escape = "_Dl";
                n = 3;
                break;
            case '/':
            case '\\':
                escape = "_Zp";
                n = 3;
                break;
            default:
                escape = &c;
                n = 1;
                break;
        }
        if (limit - p < n) {
            return -1;
        }
        memcpy(p, escape, n);
        p += n;
    }
    if (limit - p < 1) {
        return -1;
    }
    *p++ = '\0'; // term zero
    return (int)(p - symbol);
}

int yalx_enter_returning_scope(struct yalx_returning_vals *state, size_t reserved_size, address_t fun_addr) {
    struct coroutine *co = CURRENT_COROUTINE;
    DCHECK(co->returning_vals != state);
    state->prev = co->returning_vals;
    co->returning_vals = state;
    
    state->offset = 0;
    state->fun_addr = fun_addr;
    state->total_size = (u16_t)reserved_size;
    if (reserved_size > 16) {
        state->allocated = 1;
        state->buf = (address_t)malloc(reserved_size);
    } else {
        state->allocated = 0;
        state->buf = state->inline_buf;
    }
    dbg_init_zag(state->buf, state->total_size);
    return 0;
}

int yalx_exit_returning_scope(struct yalx_returning_vals *state) {
    struct coroutine *const co = CURRENT_COROUTINE;
    DCHECK(co->returning_vals == state);
    if (state->allocated) {
        free(state->buf);
    }
    co->returning_vals = state->prev;
    return 0;
}

int yalx_return_cstring(const char *const z, size_t n) {
    struct yalx_value_str **place = (struct yalx_value_str **)yalx_return_reserved(sizeof(struct yalx_value_str *));
    if (!place) {
        return -1;
    }
    *place = yalx_new_string(heap, z, n);
    return 0;
}

void *yalx_return_reserved(size_t n) {
    return yalx_return_reserved_do(CURRENT_COROUTINE->returning_vals, n);
}

//void *yalx_return_reserved_do(struct yalx_returning_vals *state, size_t n);

void *yalx_return_reserved_do(struct yalx_returning_vals *state, size_t n) {
    size_t size = ROUND_UP(n, STACK_SLOT_ALIGNMENT);
    if (state->offset + size > state->total_size) {
        return NULL;
    }
    address_t addr = (state->buf + state->total_size) - state->offset - n;
    //memcpy(addr, p, n);
    state->offset += (u16_t)size;
    return addr;
}

const struct yalx_class *yalx_find_class(const char *const plain_name) {
    DCHECK(plain_name != NULL);
    DCHECK(plain_name[0] != 0);
    const char *const symbol = yalx_symbol_mangle(plain_name, "$class");
#if defined(YALX_OS_DARWIN)
    const struct yalx_class *const clazz = (struct yalx_class *)dlsym(RTLD_MAIN_ONLY, symbol + 1);
#endif
#if defined(YALX_OS_LINUX)
    const struct yalx_class *const clazz = (struct yalx_class *)dlsym(RTLD_LOCAL, symbol + 1);
#endif
    //puts(symbol);
    free((void *)symbol);
    return clazz;
}

void pkg_init_once(void *init_fun, const char *const plain_name) {
    // $global_slots
    static const size_t kMaxPostfixSize = 16;
    char *symbol = NULL;
    char buf[16] = {0};
    yalx_mutex_lock(&pkg_init_mutex);

    struct hash_table_value_span pkg = yalx_get_string_key(&pkg_init_records, plain_name);
    if (pkg.value != NULL) {
        goto done; // Already init
    }
    
    size_t buf_size = 98;
    do {
        //free(symbol);
        buf_size <<= 1;
        symbol = (char *)realloc(symbol, buf_size);
    } while(yalx_name_symbolize(plain_name, symbol, buf_size) < 0);
    
    const size_t prefix_len = strlen(symbol);
    DCHECK(prefix_len > 1);
    if (prefix_len + kMaxPostfixSize > buf_size) {
        symbol = (char *)realloc(symbol, prefix_len + kMaxPostfixSize);
    }
    strncpy(symbol + prefix_len, "_Lksz", 6);
    
#if defined(YALX_OS_POSIX)
    struct lksz_header *lksz_addr = (struct lksz_header *)dlsym(YALX_DL_HANDLE, symbol + 1/*Skip prefx '_'*/);
#endif
    
    strncpy(symbol + prefix_len, "_Kstr", 6);

#if defined(YALX_OS_POSIX)
    struct kstr_header *kstr_addr = (struct kstr_header *)dlsym(YALX_DL_HANDLE, symbol + 1/*Skip prefx '_'*/);
#endif
    
    strncpy(symbol + prefix_len, "$global_slots", 14);
    
#if defined(YALX_OS_POSIX)
    struct pkg_global_slots *slots = (struct pkg_global_slots *)dlsym(YALX_DL_HANDLE, symbol + 1/*Skip prefx '_'*/);
#endif
    
    if (!lksz_addr || !kstr_addr) {
        yalx_hash_table_put(&pkg_init_records, plain_name, strlen(plain_name), 4);
        goto done; // No constant string will be initializing
    }
    assert(lksz_addr->number_of_strings == kstr_addr->number_of_strings);
    
    for (int i = 0; i < lksz_addr->number_of_strings; i++) {
        kstr_addr->ks[i] = yalx_new_string(heap, lksz_addr->sz[i], strlen(lksz_addr->sz[i]));
    }
    
    hash_table_value_span_t rs = yalx_hash_table_put(&pkg_init_records, plain_name, strlen(plain_name), PKG_RECORD_VAL_SIZE);
    //TODO: memcpy(rs.value, &slots, sizeof(slots));
    //*((struct pkg_global_slots **)rs.value) = slots;
    DCHECK(PKG_RECORD_VAL_SIZE == rs.size);
    ((void **)rs.value)[0] = kstr_addr;
    ((void **)rs.value)[1] = slots;

done:
    yalx_mutex_unlock(&pkg_init_mutex);
    //printf("pkg init...%s\n", plain_name);
    free(symbol);
    if (init_fun) {
        call0_returning_vals(buf, sizeof(buf), init_fun);
    }
}

int pkg_initialized_count(void) { return pkg_init_records.size; }

int pkg_has_initialized(const char *const plain_name) {
    yalx_mutex_lock(&pkg_init_mutex);
    hash_table_value_span_t span = yalx_get_string_key(&pkg_init_records, plain_name);
    yalx_mutex_unlock(&pkg_init_mutex);
    return span.value != NULL;
}

const struct pkg_global_slots *pkg_get_global_slots(const char *const plain_name) {
    const struct pkg_global_slots *rs = NULL;
    yalx_mutex_lock(&pkg_init_mutex);
    hash_table_value_span_t span = yalx_get_string_key(&pkg_init_records, plain_name);
    if (span.value == NULL) {
        rs = NULL;
    } else {
        rs = *(struct pkg_global_slots **)span.value;
    }
    yalx_mutex_unlock(&pkg_init_mutex);
    return rs;
}

void put_field(struct yalx_value_any **address, struct yalx_value_any *field) {
    DCHECK(address != NULL);
    post_write_barrier(heap, address, field);
    *address = field;
}

void put_field_chunk(struct yalx_value_any *const host, address_t address, address_t incoming) {
    const struct yalx_class *const klass = CLASS(host);
    DCHECK(address > (address_t)host);
    
    struct yalx_class_field *field = NULL;
    const ptrdiff_t offset_of_head = address - (address_t)host;
    for (int i = 0; i < klass->n_fields; i++) {
        if (klass->fields[i].offset_of_head == offset_of_head) {
            field = &klass->fields[i];
            break;
        }
    }
    DCHECK(field != NULL);
    post_typing_write_barrier_if_needed(heap, field->type, address, incoming);
    memcpy(address, incoming, field->type->instance_size);
}

void put_field_chunk_by_index(struct yalx_value_any *const host, const int index_of_field, address_t incoming) {
    const struct yalx_class *const klass = CLASS(host);
    DCHECK(index_of_field >= 0);
    DCHECK(index_of_field < klass->n_fields);
    
    const struct yalx_class_field *const field = &klass->fields[index_of_field];
    address_t location = (address_t)host + field->offset_of_head;
    post_typing_write_barrier_if_needed(heap, field->type, location, incoming);
    memcpy(location, incoming, field->type->instance_size);
}

static const uintptr_t kPendingMask = 1;
static const uintptr_t kCreatedMask = ~kPendingMask;

static inline int need_init(struct yalx_value_any *_Atomic *address) {
    struct yalx_value_any *expected = NULL;
    if (atomic_compare_exchange_strong(address, &expected, (yalx_ref_t)kPendingMask)) {
        return 1;
    }
    while ((uintptr_t)atomic_load_explicit(address, memory_order_acquire) == kPendingMask) {
        sched_yield();
    }
    return 0;
}

struct yalx_value_any *lazy_load_object(struct yalx_value_any *_Atomic *address, const struct yalx_class *clazz) {
    DCHECK(address != NULL);
    DCHECK(clazz != NULL);
    
    if (!((uintptr_t)atomic_load_explicit(address, memory_order_acquire) & kCreatedMask) && need_init(address)) {
        struct allocate_result rs = yalx_heap_allocate(heap, clazz, clazz->instance_size, 0);
        assert(rs.object != NULL);

        // call constructor:
        char buf[16] = {0};
        call1_returning_vals(buf, sizeof(buf), clazz->ctor->entry, (intptr_t)rs.object);
        atomic_store_explicit(address, rs.object, memory_order_release);
    }
    return atomic_load_explicit(address, memory_order_relaxed);
}

void associate_stub_returning_vals(struct yalx_returning_vals *state,
                                   address_t returning_addr,
                                   size_t reserved_size,
                                   address_t fun_addr) {
    struct coroutine *const co = CURRENT_COROUTINE;
    DCHECK(co->returning_vals != state);
    state->prev = co->returning_vals;
    co->returning_vals = state;
    
    state->offset = 0;
    state->allocated = 0;
    state->total_size = (u16_t)reserved_size;
    state->buf = returning_addr;
    state->fun_addr = fun_addr;
    dbg_init_zag(state->buf, state->total_size);
}

void *reserve_handle_returning_vals(u32_t size) {
    DCHECK(thread_local_mach != NULL);
    struct coroutine *const co = CURRENT_COROUTINE;
    DCHECK(size == co->returning_vals->total_size);
    return co->returning_vals->buf;
}

struct yalx_value_any *heap_alloc(const struct yalx_class *const clazz) {
    struct allocate_result result = yalx_heap_allocate(heap, clazz, clazz->instance_size, 0);
    if (result.status != ALLOCATE_OK) {
        DCHECK(!"TODO: throw Exception");
        return NULL; // TODO: throw Exception
    }
    return result.object;
}

struct yalx_value_array_header *array_alloc(const struct yalx_class *const element_ty, void *const input, int nitems) {
    DCHECK(element_ty != NULL);
    DCHECK(input != NULL);
    DCHECK(nitems >= 0);
    
    const u32_t dims = *(u32_t *)input;
    const u32_t *caps = (u32_t *)input + 1;
    void *const elements = (u32_t *)input + dims + 1;

    enum yalx_builtin_type maybe_builtin_ty = yalx_builtin_type(element_ty);
    switch (maybe_builtin_ty) {
        case Type_any:
        case Type_string:
        case Type_array:
        case Type_multi_dims_array:
            return yalx_new_refs_array_with_data(heap, element_ty, dims, caps, (yalx_ref_t *)elements, nitems);
            
        case NOT_BUILTIN_TYPE: // It's not user definition types
            break;
            
        default: {
            if (element_ty->reference_size < STACK_SLOT_ALIGNMENT) {
                address_t d = (address_t)elements;
                address_t s = (address_t)elements;
                for (int i = 0; i < nitems; i++) {
                    memmove(d, s, element_ty->reference_size);
                    d += element_ty->reference_size;
                    s += STACK_SLOT_ALIGNMENT;
                }
            }
            DCHECK(element_ty->constraint == K_PRIMITIVE);
            return yalx_new_vals_array_with_data(heap, element_ty, dims, caps, elements, nitems);
        } break;
    }
    if (element_ty->constraint == K_CLASS || element_ty->compact_enum) {
        return yalx_new_refs_array_with_data(heap, element_ty, dims, caps, (yalx_ref_t *)elements, nitems);
    }
    DCHECK(element_ty->constraint == K_STRUCT);
    return yalx_new_vals_array_with_data(heap, element_ty, dims, caps, elements, nitems);
}

struct yalx_value_array_header *array_fill(const struct yalx_class *const element_ty, const void *params) {
    DCHECK(element_ty != NULL);
    const u32_t dims = *(const u32_t *)params;
    const u32_t *caps = (const u32_t *)params + 1;
    const void *filling = (const void *)(caps + dims);
  
    DCHECK(dims >= 1);
    struct yalx_value_array_header *rs = NULL;
    address_t data = NULL;
    if (dims == 1) {
        struct yalx_value_array *ar = yalx_new_array(heap, element_ty, caps[0]);
        data = ar->data;
        rs = (struct yalx_value_array_header *)ar;
    } else {
        struct yalx_value_multi_dims_array *ar = yalx_new_multi_dims_array(heap, element_ty, dims, caps);
        data = yalx_multi_dims_array_data(ar);
        rs = (struct yalx_value_array_header *)ar;
    }
    
    if (element_ty->constraint == K_STRUCT) {
        
        for (address_t p = data; p < data + rs->len * element_ty->instance_size; p += element_ty->instance_size) {
            memcpy(p, filling, element_ty->instance_size);
            init_typing_write_barrier_if_needed(heap, element_ty, p);
        }
    } else if (element_ty->compact_enum || yalx_is_ref_type(element_ty)) {
        for (yalx_ref_t *p = (yalx_ref_t *)data; p < (yalx_ref_t *)data + rs->len; p++) {
            yalx_ref_t obj = *(yalx_ref_t *)filling;
            
            if (yalx_is_array(obj)) {
                obj = (yalx_ref_t)yalx_array_clone(heap, (struct yalx_value_array_header *)obj);
            }
            
            *p = obj;
            init_write_barrier(heap, p);
        }
    } else {
        DCHECK(element_ty->constraint == K_PRIMITIVE);
        for (address_t p = data; p < data + rs->len * element_ty->instance_size; p += element_ty->instance_size) {
            switch (element_ty->instance_size) {
                case 1:
                    *(uint8_t *)p = *(uint8_t *)filling;
                    break;
                case 2:
                    *(uint16_t *)p = *(uint16_t *)filling;
                    break;
                case 4:
                    *(uint32_t *)p = *(uint32_t *)filling;
                    break;
                case 8:
                    *(uint64_t *)p = *(uint64_t *)filling;
                    break;
                default:
                    DCHECK(!"bad instance size");
                    break;
            }
        }
    }
    return rs;
}

void *array_location_at1(struct yalx_value_array_header *const array, const i32_t index) {
    DCHECK(array != NULL);
    const struct yalx_class *const klass = CLASS(array);
    DCHECK(klass == array_class);
    
    if (index < 0 || index >= array->len) {
        throw_array_index_out_of_bounds_exception(array, 0, index);
    }

    struct yalx_value_array *ar = (struct yalx_value_array *)array;
    if (yalx_is_ref_type(ar->item)) {
        return ar->data + index * ar->item->reference_size;
    } else {
        return ar->data + index * ar->item->instance_size;
    }
}

void *array_location_at2(struct yalx_value_multi_dims_array *const array, const i32_t d0, const i32_t d1) {
    DCHECK(array != NULL);
    const struct yalx_class *const klass = CLASS(array);
    DCHECK(klass == multi_dims_array_class);
    DCHECK(array->rank == 2);
    
    if (d0 < 0 || d0 >= array->caps[0]) {
        throw_array_index_out_of_bounds_exception((const struct yalx_value_array_header *)array, 0, d0);
    }
    if (d1 < 0 || d1 >= array->caps[1]) {
        throw_array_index_out_of_bounds_exception((const struct yalx_value_array_header *)array, 1, d1);
    }
    return yalx_array_location2(array, d0, d1);
}

void *array_location_at3(struct yalx_value_multi_dims_array *const array, const i32_t d0, const i32_t d1,
                         const i32_t d2) {
    DCHECK(array != NULL);
    const struct yalx_class *const klass = CLASS(array);
    DCHECK(klass == multi_dims_array_class);
    DCHECK(array->rank == 3);
    
    if (d0 < 0 || d0 >= array->caps[0]) {
        throw_array_index_out_of_bounds_exception((const struct yalx_value_array_header *)array, 0, d0);
    }
    if (d1 < 0 || d1 >= array->caps[1]) {
        throw_array_index_out_of_bounds_exception((const struct yalx_value_array_header *)array, 1, d1);
    }
    if (d2 < 0 || d2 >= array->caps[2]) {
        throw_array_index_out_of_bounds_exception((const struct yalx_value_array_header *)array, 2, d2);
    }
    return yalx_array_location3(array, d0, d1, d2);
}

void *array_location_at(struct yalx_value_array_header *const array, const i32_t *indices) {
    DCHECK(array != NULL);
    const struct yalx_class *const klass = CLASS(array);
    DCHECK(klass == multi_dims_array_class || klass == array_class);
    
    if (klass == array_class) {
        return array_location_at1(array, indices[0]);
    }

    struct yalx_value_multi_dims_array *ar = (struct yalx_value_multi_dims_array *)array;
    for (int i = 0; i < ar->rank; i++) {
        if (indices[i] < 0 || indices[i] >= ar->caps[i]) {
            throw_array_index_out_of_bounds_exception(array, 2, indices[i]);
        }
    }
    return yalx_array_location_more((struct yalx_value_multi_dims_array *)array, indices);
}

void array_set_ref1(struct yalx_value_array_header *const array, const i32_t index, yalx_ref_t value) {
    DCHECK(array != NULL);
    DCHECK(CLASS(array) == array_class);
    DCHECK(yalx_is_ref_type(array->item));
    
    if (index < 0 || index >= array->len) {
        throw_array_index_out_of_bounds_exception(array, 0, index);
    }

    struct yalx_value_array *ar = (struct yalx_value_array *)array;
    put_field(((yalx_ref_t *)ar->data) + index, value);
}

void array_set_ref2(struct yalx_value_array_header *const array, const i32_t d0, const i32_t d1, yalx_ref_t value) {
    DCHECK(array != NULL);
    DCHECK(CLASS(array) == multi_dims_array_class);
    DCHECK(yalx_is_ref_type(array->item) || array->item->compact_enum);
    
    struct yalx_value_multi_dims_array *ar = (struct yalx_value_multi_dims_array *)array;
    DCHECK(ar->rank == 2);
    if (d0 < 0 || d0 >= ar->caps[0]) {
        throw_array_index_out_of_bounds_exception(array, 0, d0);
    }
    if (d1 < 0 || d1 >= ar->caps[1]) {
        throw_array_index_out_of_bounds_exception(array, 1, d1);
    }
    
    put_field((yalx_ref_t *)yalx_array_location2(ar, d0, d1), value);
}

void array_set_ref3(struct yalx_value_array_header *const array, const i32_t d0, const i32_t d1, const i32_t d2,
                    yalx_ref_t value) {
    DCHECK(array != NULL);
    DCHECK(CLASS(array) == multi_dims_array_class);
    DCHECK(yalx_is_ref_type(array->item));
    
    struct yalx_value_multi_dims_array *ar = (struct yalx_value_multi_dims_array *)array;
    DCHECK(ar->rank == 3);
    if (d0 < 0 || d0 >= ar->caps[0]) {
        throw_array_index_out_of_bounds_exception(array, 0, d0);
    }
    if (d1 < 0 || d1 >= ar->caps[1]) {
        throw_array_index_out_of_bounds_exception(array, 1, d1);
    }
    if (d2 < 0 || d2 >= ar->caps[2]) {
        throw_array_index_out_of_bounds_exception(array, 2, d2);
    }
    
    put_field((yalx_ref_t *)yalx_array_location3(ar, d0, d1, d2), value);
}

void array_set_ref(struct yalx_value_array_header *const array, const i32_t *const indices, yalx_ref_t value) {
    DCHECK(array != NULL);
    DCHECK(yalx_is_array((yalx_ref_t)array));
    DCHECK(yalx_is_ref_type(array->item));
    
    const struct yalx_class *const klass = CLASS(array);
    if (klass == array_class) {
        array_set_ref1(array, indices[0], value);
        return;
    }
    
    DCHECK(klass == multi_dims_array_class);
    struct yalx_value_multi_dims_array *ar = (struct yalx_value_multi_dims_array *)array;
    for (int i = 0; i < ar->rank; i++) {
        if (indices[i] < 0 || indices[i] >= ar->caps[i]) {
            throw_array_index_out_of_bounds_exception(array, i, indices[i]);
        }
    }
    
    put_field((yalx_ref_t *)yalx_array_location_more(ar, indices), value);
}

void array_set_chunk1(struct yalx_value_array_header *const array, const i32_t index, address_t chunk) {
    DCHECK(array != NULL);
    DCHECK(chunk != NULL);
    DCHECK(CLASS(array) == array_class);
    DCHECK(array->item->constraint == K_STRUCT || array->item->constraint == K_ENUM);
    
    struct yalx_value_array *ar = (struct yalx_value_array *)array;
    ptrdiff_t offset = index * ar->item->instance_size;
    post_typing_write_barrier_if_needed(heap, ar->item, ar->data + offset, chunk);
    memcpy(ar->data + offset, chunk, ar->item->instance_size);
}

void array_set_chunk2(struct yalx_value_array_header *const array, const i32_t d0, const i32_t d1, address_t chunk) {
    DCHECK(array != NULL);
    DCHECK(chunk != NULL);
    DCHECK(CLASS(array) == multi_dims_array_class);
    DCHECK(array->item->constraint == K_STRUCT || array->item->constraint == K_ENUM);
    
    struct yalx_value_multi_dims_array *ar = (struct yalx_value_multi_dims_array *)array;
    DCHECK(ar->rank == 2);
    if (d0 < 0 || d0 >= ar->caps[0]) {
        throw_array_index_out_of_bounds_exception(array, 0, d0);
    }
    if (d1 < 0 || d1 >= ar->caps[1]) {
        throw_array_index_out_of_bounds_exception(array, 1, d1);
    }

    address_t dest = yalx_array_location2(ar, d0, d1);
    post_typing_write_barrier_if_needed(heap, ar->item, dest, chunk);
    memcpy(dest, chunk, ar->item->instance_size);
}

void array_set_chunk3(struct yalx_value_array_header *const array, const i32_t d0, const i32_t d1, const i32_t d2,
                      address_t chunk) {
    DCHECK(array != NULL);
    DCHECK(chunk != NULL);
    DCHECK(CLASS(array) == multi_dims_array_class);
    DCHECK(array->item->constraint == K_STRUCT || array->item->constraint == K_ENUM);
    
    struct yalx_value_multi_dims_array *ar = (struct yalx_value_multi_dims_array *)array;
    DCHECK(ar->rank == 3);
    if (d0 < 0 || d0 >= ar->caps[0]) {
        throw_array_index_out_of_bounds_exception(array, 0, d0);
    }
    if (d1 < 0 || d1 >= ar->caps[1]) {
        throw_array_index_out_of_bounds_exception(array, 1, d1);
    }
    if (d2 < 0 || d2 >= ar->caps[2]) {
        throw_array_index_out_of_bounds_exception(array, 2, d2);
    }
    
    address_t dest = yalx_array_location3(ar, d0, d1, d2);
    post_typing_write_barrier_if_needed(heap, ar->item, dest, chunk);
    memcpy(dest, chunk, ar->item->instance_size);
}

void array_set_chunk(struct yalx_value_array_header *const array, const i32_t *const indices, address_t chunk) {
    DCHECK(array != NULL);
    DCHECK(yalx_is_array((yalx_ref_t)array));
    DCHECK(array->item->constraint == K_STRUCT || array->item->constraint == K_ENUM);
    
    const struct yalx_class *const klass = CLASS(array);
    if (klass == array_class) {
        array_set_chunk1(array, indices[0], chunk);
        return;
    }
    
    DCHECK(klass == multi_dims_array_class);
    struct yalx_value_multi_dims_array *ar = (struct yalx_value_multi_dims_array *)array;
    for (int i = 0; i < ar->rank; i++) {
        if (indices[i] < 0 || indices[i] >= ar->caps[i]) {
            throw_array_index_out_of_bounds_exception(array, i, indices[i]);
        }
    }
    
    address_t dest = yalx_array_location_more(ar, indices);
    post_typing_write_barrier_if_needed(heap, ar->item, dest, chunk);
    memcpy(dest, chunk, ar->item->instance_size);
}

struct coroutine *current_root(void) { return CURRENT_COROUTINE; }


u8_t is_instance_of(struct yalx_value_any *const host, const struct yalx_class *const for_test) {
    DCHECK(host != NULL);
    DCHECK(for_test != NULL);
    const struct yalx_class *const ty = CLASS(host);
    for (const struct yalx_class *it = ty; it != NULL; it = it->super) {
        if (for_test == it) {
            return 1;
        }
    }
    return 0;
}

struct yalx_value_any *ref_asserted_to(struct yalx_value_any *const from, const struct yalx_class *const clazz) {
    if (!from) {
        return NULL;
    }
    if (!is_instance_of(from, clazz)) {
        throw_bad_casting_exception(CLASS(from), clazz);
    }
    //printf("%s->%s\n", CLASS(from)->location.z, clazz->location.z);
    return from;
}

struct yalx_value_any *closure(const struct yalx_class *const klass, address_t buf, size_t size) {
    struct yalx_value_any *fun = heap_alloc(klass);
    DCHECK(klass->n_methods == 1);
    DCHECK(!strcmp("apply", klass->methods[0].name.z));
    
    address_t apply = klass->methods[0].entry;
    *((address_t *)((address_t)fun + klass->fields[0].offset_of_head)) = apply;
    
    address_t p = buf;
    for (int i = 1; i < klass->n_fields; i++) {
        struct yalx_class *ty = klass->fields[i].type;
        address_t dest = (address_t)fun + klass->fields[i].offset_of_head;
        switch (ty->constraint) {
            case K_PRIMITIVE:
                memcpy(dest, p, ty->instance_size);
                p += ROUND_UP(ty->instance_size, STACK_SLOT_ALIGNMENT);
                break;
            case K_ENUM:
                if (ty->compact_enum) {
                    memcpy(dest, p, ty->reference_size);
                    init_write_barrier(heap, (yalx_ref_t *)dest);
                    p += ROUND_UP(ty->reference_size, STACK_SLOT_ALIGNMENT);
                } else {
                    memcpy(dest, p, ty->instance_size);
                    init_typing_write_barrier_if_needed(heap, ty, dest);
                    p += ROUND_UP(ty->instance_size, STACK_SLOT_ALIGNMENT);
                }
                break;
            case K_CLASS:
                memcpy(dest, p, ty->reference_size);
                init_write_barrier(heap, (yalx_ref_t *)dest);
                p += ROUND_UP(ty->reference_size, STACK_SLOT_ALIGNMENT);
                break;
            case K_STRUCT:
                memcpy(dest, p, ty->instance_size);
                if (ty->refs_mark_len > 0) {
                    init_typing_write_barrier_if_needed(heap, ty, dest);
                }
                p += ROUND_UP(ty->instance_size, STACK_SLOT_ALIGNMENT);
                break;
            default:
                DCHECK(!"Unreachable");
                break;
        }
    }
    DCHECK(p - buf == size);
    return fun;
}

struct yalx_value_str *concat(struct yalx_value_str **parts, size_t n) {
    struct yalx_value_str *rs = yalx_build_string(heap, parts, n);
    if (!rs) {
        DCHECK(!"TODO");
    }
    return rs;
}

struct yalx_value_str *concat2(struct yalx_value_str *part0, struct yalx_value_str *part1) {
    struct yalx_value_str *parts[] = {part0, part1};
    struct yalx_value_str *rs = yalx_build_string(heap, parts, arraysize(parts));
    if (!rs) {
        DCHECK(!"TODO");
    }
    return rs;
}

struct yalx_value_str *concat3(struct yalx_value_str *part0, struct yalx_value_str *part1, struct yalx_value_str *part2) {
    struct yalx_value_str *parts[] = {part0, part1, part2};
    struct yalx_value_str *rs = yalx_build_string(heap, parts, arraysize(parts));
    if (!rs) {
        DCHECK(!"TODO");
    }
    return rs;
}

struct yalx_value_str *concat4(struct yalx_value_str *part0, struct yalx_value_str *part1, struct yalx_value_str *part2,
                               struct yalx_value_str *part3) {
    struct yalx_value_str *parts[] = {part0, part1, part2, part3};
    struct yalx_value_str *rs = yalx_build_string(heap, parts, arraysize(parts));
    if (!rs) {
        DCHECK(!"TODO");
    }
    return rs;
}

//----------------------------------------------------------------------------------------------------------------------
// native fun's stubs:
//----------------------------------------------------------------------------------------------------------------------

void yalx_Zplang_Zolang_Zdprintln_stub(yalx_str_handle txt) {
    DCHECK(txt != NULL);
    const u32_t hash_code = yalx_str_hash_code(txt);
    if (hash_code == 634532469 || hash_code == 1342438586 || hash_code == 2593250737) {
        static const char *quote = "<👍>";
        fwrite(quote, 1, strlen(quote), stdout);
    }
    fwrite(yalx_str_bytes(txt), 1, yalx_str_len(txt), stdout);
    fputc('\n', stdout);
}



