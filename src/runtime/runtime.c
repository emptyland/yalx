#include "runtime/runtime.h"
#include "runtime/scheduler.h"
#include "runtime/process.h"
#include "runtime/checking.h"
#include "runtime/hash-table.h"
#include "runtime/heap/heap.h"
#include "runtime/object/yalx-string.h"
#include "runtime/object/number.h"
#include "runtime/object/throwable.h"
#include "runtime/object/arrays.h"
#include "runtime/object/type.h"
#include <unistd.h>
#if defined(YALX_OS_DARWIN)
#include <sys/sysctl.h>
#endif
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdatomic.h>
//#define _XOPEN_SOURCE
//#include <ucontext.h>

int ncpus = 0;

int os_page_size = 0;

const struct yalx_str yalx_version = YALX_STR("yalx-lang v0.0.0");

struct processor *procs = NULL;

int nprocs = 0;

struct machine m0;
struct coroutine c0;

_Thread_local struct machine *thread_local_mach;

struct stack_pool stack_pool;


// External symbol from generated code
// 1347046214, 1465142347, 1195658056
extern uint32_t yalx_magic_number1;
extern uint32_t yalx_magic_number2;
extern uint32_t yalx_magic_number3;

static struct hash_table pkg_init_records;
static pthread_mutex_t pkg_init_mutex = PTHREAD_MUTEX_INITIALIZER;

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
        printf("    %s %d 0x%04x\n", field->name.z, field->offset, field->offset);
    }
}

static void dev_print_struct_fields() {
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

extern struct yalx_class builtin_classes[MAX_BUILTIN_TYPES];

struct class_load_entry {
    const char *name;
    struct yalx_class **location;
} classes_loading_entries[] = {
    {THROWABLE_CLASS_NAME, &throwable_class},
    {EXCEPTION_CLASS_NAME, &exception_class},
    {BACKTRACE_FRAME_CLASS_NAME, &backtrace_frame_class},
    {BAD_CASTING_EXCEPTION_CLASS_NAME, &bad_casting_exception_class},
    {ARRAY_INDEX_OUT_OF_BOUNDS_EXCEPTION_CLASS_NAME, &array_index_out_of_bounds_exception_class},
    {NULL, NULL} // end of entries
};

int yalx_runtime_init() {
    
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
    yalx_init_hash_table(&pkg_init_records, 1.2);
    if (yalx_init_heap(&heap, GC_LXR) < 0) {
        return -1;
    }
    
    yalx_init_stack_pool(&stack_pool, 10 * MB);
    
    yalx_init_machine(&m0, &procs[0]);
    m0.thread = pthread_self();
    
    thread_local_mach = &m0;
    
    struct stack *s0 = yalx_new_stack_from_pool(&m0.stack_pool, 1 * MB);
    if (!s0) {
        return -1;
    }
    coid_t coid;
    coid.value = 0;
    yalx_init_coroutine(coid, &c0, s0, (address_t)y2zmain_main);

    nprocs = ncpus;
    procs = malloc(nprocs * sizeof(struct processor));
    if (!procs) {
        return -1;
    }
    
    for (int i = 0; i < nprocs; i++) {
        procid_t id = {i};
        if (yalx_init_processor(id, &procs[i]) < 0) {
            return -1;
        }
    }
    
    yalx_add_machine_to_processor(&procs[0], &m0);
    
    yalx_init_scheduler(&scheduler);
    

    // FIXME:
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

    dev_print_struct_fields();
    return 0;
}

void yalx_runtime_eixt(void) {
    yalx_free_hash_table(&pkg_init_records);
    yalx_free_scheduler(&scheduler);
    // TODO:
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
            printf("DBG %ld\n", ((struct yalx_value_number_w *)obj)->box.i64);
            break;
            
        case Type_U64:
            printf("DBG %lu\n", ((struct yalx_value_number_w *)obj)->box.u64);
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
    const extra_len = !postfix ? 0 : strlen(postfix);
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
}

int yalx_exit_returning_scope(struct yalx_returning_vals *state) {
    struct coroutine *const co = CURRENT_COROUTINE;
    DCHECK(co->returning_vals == state);
    if (state->allocated) {
        free(state->buf);
    }
    co->returning_vals = state->prev;
}

int yalx_return_cstring(const char *const z, size_t n) {
    struct yalx_value_str **place = (struct yalx_value_str **)yalx_return_reserved(sizeof(struct yalx_value_str *));
    if (!place) {
        return -1;
    }
    *place = yalx_new_string(&heap, z, n);
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
    const struct yalx_class *const clazz = (struct yalx_class *)dlsym(RTLD_MAIN_ONLY, symbol + 1);
    //puts(symbol);
    free(symbol);
    return clazz;
}

struct lksz_header {
    int number_of_strings;
    const char *const sz[1];
};

struct kstr_header {
    int number_of_strings;
    struct yalx_value_str *ks[1];
};

void pkg_init_once(void *init_fun, const char *const plain_name) {
    // $global_slots
    static const size_t kMaxPostfixSize = 16;
    char *symbol = NULL;
    char buf[16] = {0};
    pthread_mutex_lock(&pkg_init_mutex);

    struct hash_table_value_span pkg = yalx_get_string_key(&pkg_init_records, plain_name);
    if (pkg.value != NULL) {
        goto done; // Already init
    }
    
    size_t buf_size = 98;
    do {
        free(symbol);
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
    struct lksz_header *lksz_addr = (struct lksz_header *)dlsym(RTLD_MAIN_ONLY, symbol + 1/*Skip prefx '_'*/);
#endif
    
    strncpy(symbol + prefix_len, "_Kstr", 6);

#if defined(YALX_OS_POSIX)
    struct kstr_header *kstr_addr = (struct kstr_header *)dlsym(RTLD_MAIN_ONLY, symbol + 1/*Skip prefx '_'*/);
#endif
    
    strncpy(symbol + prefix_len, "$global_slots", 14);
    
#if defined(YALX_OS_POSIX)
    struct pkg_global_slots *slots = (struct pkg_global_slots *)dlsym(RTLD_MAIN_ONLY, symbol + 1/*Skip prefx '_'*/);
#endif
    
    if (!lksz_addr || !kstr_addr) {
        yalx_hash_table_put(&pkg_init_records, plain_name, strlen(plain_name), 4);
        goto done; // No constant string will be initializing
    }
    assert(lksz_addr->number_of_strings == kstr_addr->number_of_strings);
    
    for (int i = 0; i < lksz_addr->number_of_strings; i++) {
        kstr_addr->ks[i] = yalx_new_string(&heap, lksz_addr->sz[i], strlen(lksz_addr->sz[i]));
    }
    
    hash_table_value_span_t rs = yalx_hash_table_put(&pkg_init_records, plain_name, strlen(plain_name), sizeof(slots));
    //TODO: memcpy(rs.value, &slots, sizeof(slots));
    *((struct pkg_global_slots **)rs.value) = slots;
done:
    pthread_mutex_unlock(&pkg_init_mutex);
    printf("pkg init...%s\n", plain_name);
    free(symbol);
    if (init_fun) {
        call0_returning_vals(buf, sizeof(buf), init_fun);
    }
}

int pkg_initialized_count() { return pkg_init_records.size; }

int pkg_has_initialized(const char *const plain_name) {
    pthread_mutex_lock(&pkg_init_mutex);
    hash_table_value_span_t span = yalx_get_string_key(&pkg_init_records, plain_name);
    pthread_mutex_unlock(&pkg_init_mutex);
    return span.value != NULL;
}

const struct pkg_global_slots *pkg_get_global_slots(const char *const plain_name) {
    const struct pkg_global_slots *rs = NULL;
    pthread_mutex_lock(&pkg_init_mutex);
    hash_table_value_span_t span = yalx_get_string_key(&pkg_init_records, plain_name);
    if (span.value == NULL) {
        rs = NULL;
    } else {
        rs = *(struct pkg_global_slots **)span.value;
    }
    pthread_mutex_unlock(&pkg_init_mutex);
    return rs;
}

void put_field(struct yalx_value_any **address, struct yalx_value_any *field) {
    DCHECK(address != NULL);
    post_write_barrier(&heap, address, field);
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
    post_typing_write_barrier_if_needed(&heap, field->type, address, incoming);
    memcpy(address, incoming, field->type->instance_size);
}

void put_field_chunk_by_index(struct yalx_value_any *const host, const int index_of_field, address_t incoming) {
    const struct yalx_class *const klass = CLASS(host);
    DCHECK(index_of_field >= 0);
    DCHECK(index_of_field < klass->n_fields);
    
    const struct yalx_class_field *const field = &klass->fields[index_of_field];
    address_t location = (address_t)host + field->offset_of_head;
    post_typing_write_barrier_if_needed(&heap, field->type, location, incoming);
    memcpy(location, incoming, field->type->instance_size);
}

static const uintptr_t kPendingMask = 1;
static const uintptr_t kCreatedMask = ~kPendingMask;

static inline int need_init(struct yalx_value_any *_Atomic *address) {
    struct yalx_value_any *expected = NULL;
    if (atomic_compare_exchange_strong(address, &expected, kPendingMask)) {
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
        struct allocate_result rs = yalx_heap_allocate(&heap, clazz, clazz->instance_size, 0);
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
    struct allocate_result result = yalx_heap_allocate(&heap, clazz, clazz->instance_size, 0);
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
            return yalx_new_refs_array_with_data(&heap, element_ty, dims, caps, (yalx_ref_t *)elements, nitems);
            
        case NOT_BUILTIN_TYPE: // It's not user definition types
            break;
            
        default:
            DCHECK(element_ty->constraint == K_PRIMITIVE);
            return yalx_new_vals_array_with_data(&heap, element_ty, dims, caps, elements, nitems);
    }
    if (element_ty->constraint == K_CLASS) {
        return yalx_new_refs_array_with_data(&heap, element_ty, dims, caps, (yalx_ref_t *)elements, nitems);
    }
    DCHECK(element_ty->constraint == K_STRUCT);
    return yalx_new_vals_array_with_data(&heap, element_ty, dims, caps, elements, nitems);
}

struct yalx_value_array_header *array_fill_w32(const struct yalx_class *const element_ty, u32_t value, int nitems) {
    DCHECK(element_ty->constraint == K_PRIMITIVE);
    DCHECK(element_ty->reference_size == 4);
    DCHECK(element_ty->instance_size == 4);
    struct yalx_value_array *ar = yalx_new_array(&heap, element_ty, nitems);
    for (int i = 0; i < nitems; i++) {
        ((u32_t *)ar->data)[i] = value;
    }
    return (struct yalx_value_array_header *)ar;
}

struct yalx_value_array_header *array_fill_refs(const struct yalx_class *const element_ty, yalx_ref_t value,
                                                int nitems) {
    DCHECK(element_ty->reference_size == sizeof(yalx_ref_t));
    struct yalx_value_array *ar = yalx_new_array(&heap, element_ty, nitems);
    for (int i = 0; i < nitems; i++) {
        ((yalx_ref_t *)ar->data)[i] = value;
        init_write_barrier(&heap, ((yalx_ref_t *)ar->data) + i);
    }
    return (struct yalx_value_array_header *)ar;
}

struct yalx_value_array_header *array_fill_chunks(const struct yalx_class *const element_ty, address_t value,
                                                  int nitems) {
    DCHECK(element_ty != NULL);
    struct yalx_value_array *ar = yalx_new_array(&heap, element_ty, nitems);
    for (int i = 0; i < nitems; i++) {
        address_t location = ar->data + i * element_ty->instance_size;
        memcpy(location, value, element_ty->instance_size);
        init_typing_write_barrier_if_needed(&heap, element_ty, location);
    }
    return (struct yalx_value_array_header *)ar;
}

struct yalx_value_array_header *array_fill_dims(const struct yalx_class *const element_ty, yalx_ref_t value,
                                                int nitems) {
    DCHECK(!value || yalx_is_array(value));
    struct yalx_value_array *ar = yalx_new_array(&heap, element_ty, nitems);
    for (int i = 0; i < nitems; i++) {
        if (value == NULL) {
            ((yalx_ref_t *)ar->data)[i] = value;
            init_write_barrier(&heap, ((yalx_ref_t *)ar->data) + i);
            continue;
        }
        
        yalx_ref_t copied = NULL;
        DCHECK(!"TODO");
//        if (CLASS(value) == array_class) {
//            struct yalx_value_array *origin = (struct yalx_value_typed_array *)value;
//            copied = yalx_new_typed_array_with_data(&heap, origin->item, origin->data, origin->len);
//        } else {
//            DCHECK(CLASS(value) == multi_dims_array_class);
//            struct yalx_value_dims_array *origin = (struct yalx_value_dims_array *)value;
//            copied = yalx_new_dims_array_with_data(&heap, origin->item, origin->arrays, origin->len);
//        }
        ((yalx_ref_t *)ar->data)[i] = copied;
        init_write_barrier(&heap, ((yalx_ref_t *)ar->data) + i);
    }
    return (struct yalx_value_array_header *)ar;
}

void *array_location_at(struct yalx_value_array_header *const array, const i32_t index) {
    DCHECK(array != NULL);
    DCHECK(yalx_is_array(array));
    
    if (index < 0 || index >= array->len) {
        throw_array_index_out_of_bounds_exception(array, index);
    }
    
    const struct yalx_class *const klass = CLASS(array);
    DCHECK(klass == array_class);
    struct yalx_value_array *ar = (struct yalx_value_array *)array;
    if (yalx_is_ref_type(ar->item)) {
        return ar->data + index * ar->item->reference_size;
    } else {
        return ar->data + index * ar->item->instance_size;
    }
}

void array_set_ref(struct yalx_value_array_header *const array, const i32_t index, yalx_ref_t value) {
    DCHECK(array != NULL);
    DCHECK(CLASS(array) == array_class);
    DCHECK(yalx_is_ref_type(array->item));
    
    if (index < 0 || index >= array->len) {
        throw_array_index_out_of_bounds_exception(array, index);
    }
    
    const struct yalx_class *const klass = CLASS(array);
    struct yalx_value_array *ar = (struct yalx_value_array *)array;
    put_field(((yalx_ref_t *)ar->data) + index, value);
}

void array_set_chunk(struct yalx_value_array_header *const array, const i32_t index, address_t chunk) {
    DCHECK(array != NULL);
    DCHECK(chunk != NULL);
    DCHECK(CLASS(array) == array_class);
    DCHECK(!yalx_is_ref_type(array->item));
    
    struct yalx_value_array *ar = (struct yalx_value_array *)array;
    ptrdiff_t offset = index * ar->item->instance_size;
    post_typing_write_barrier_if_needed(&heap, ar->item, ar->data + offset, chunk);
    memcpy(ar->data + offset, chunk, ar->item->instance_size);
}

struct coroutine *current_root() { return CURRENT_COROUTINE; }


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



