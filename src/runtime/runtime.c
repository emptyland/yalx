#include "runtime/runtime.h"
#include "runtime/scheduler.h"
#include "runtime/process.h"
#include "runtime/checking.h"
#include "runtime/hash-table.h"
#include "runtime/heap/heap.h"
#include "runtime/object/yalx-string.h"
#include "runtime/object/number.h"
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
    if (yalx_init_heap(&heap) < 0) {
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
    
    if (yalx_magic_number1 != 1347046214) {
        die("bad magic number.");
        return -1;
    }
    if (yalx_magic_number2 != 1465142347) {
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

struct lksz_header {
    int number_of_strings;
    const char *const sz[1];
};

struct kstr_header {
    int number_of_strings;
    struct yalx_value_str *ks[1];
};

void pkg_init_once(void *init_fun, const char *const plain_name) {
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
    assert(prefix_len > 1);
    if (prefix_len + 6 > buf_size) {
        symbol = (char *)realloc(symbol, prefix_len + 6);
    }
    strncpy(symbol + prefix_len, "_Lksz", 6);
    
#if defined(YALX_OS_POSIX)
    struct lksz_header *lksz_addr = (struct lksz_header *)dlsym(RTLD_MAIN_ONLY, symbol + 1/*Skip prefx '_'*/);
#endif
    
    strncpy(symbol + prefix_len, "_Kstr", 6);

#if defined(YALX_OS_POSIX)
    struct kstr_header *kstr_addr = (struct kstr_header *)dlsym(RTLD_MAIN_ONLY, symbol + 1/*Skip prefx '_'*/);
#endif
    
    if (!lksz_addr || !kstr_addr) {
        yalx_hash_table_put(&pkg_init_records, plain_name, strlen(plain_name), 4);
        goto done; // No constant string will be initializing
    }
    assert(lksz_addr->number_of_strings == kstr_addr->number_of_strings);
    
    for (int i = 0; i < lksz_addr->number_of_strings; i++) {
        kstr_addr->ks[i] = yalx_new_string(&heap, lksz_addr->sz[i], strlen(lksz_addr->sz[i]));
    }
    
    yalx_hash_table_put(&pkg_init_records, plain_name, strlen(plain_name), 4);
done:
    pthread_mutex_unlock(&pkg_init_mutex);
    printf("pkg init...%s\n", plain_name);
    free(symbol);
    call_returning_vals(buf, sizeof(buf), init_fun);
}

int pkg_initialized_count() { return pkg_init_records.size; }

int pkg_has_initialized(const char *const plain_name) {
    pthread_mutex_lock(&pkg_init_mutex);
    hash_table_value_span_t span = yalx_get_string_key(&pkg_init_records, plain_name);
    pthread_mutex_unlock(&pkg_init_mutex);
    return span.value != NULL;
}
