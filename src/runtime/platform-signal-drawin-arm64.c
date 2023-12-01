#include "runtime/process.h"
#include "runtime/mm-thread.h"
#include "runtime/thread.h"
#include "runtime/checking.h"
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/sysctl.h>

#if __DARWIN_UNIX03 && (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5)
  // 10.5 UNIX03 member name prefixes
  #define DU3_PREFIX(s, m) __ ## s.__ ## m
#else
  #define DU3_PREFIX(s, m) s ## . ## m
#endif

// Implement in file: stubs-drawin-arm64.s
extern void handle_c_polling_page_entry(void);

static address_t ucontext_get_pc(ucontext_t *uv) {
    return (address_t)uv->uc_mcontext->DU3_PREFIX(ss, pc);
}

static void ucontext_set_pc(ucontext_t *uv, address_t pc) {
    uv->uc_mcontext->DU3_PREFIX(ss, pc) = (register_t)pc;
}

int yalx_handle_signal(int sig, siginfo_t *info, void *uv, int abort_if_unrecognized) {
    struct yalx_os_thread *thread = yalx_os_thread_self_or_null();
    ucontext_t *ucontext = (ucontext_t *)uv;

    if (sig == SIGPIPE || sig == SIGXFSZ) {
        return 1; // Ignore
    }

    address_t stub = NULL;
    address_t pc = NULL;

    if (info && uv && thread) {
        pc = ucontext_get_pc(ucontext);

        if (sig == SIGSEGV) {
            // TODO: Maybe Stack overflow:
            USE(thread);
        }
    }


    if (sig == SIGBUS) { // Hardware fault
        // TODO:
        DLOG(ERROR, "Signal handle hardware fault");
        return 1;
    }

    struct machine *m = thread_local_mach;
    if (m != NULL) {
        DCHECK(info != NULL);
        if (sig == SIGSEGV && mm_is_polling_page(info->si_addr)) {
            // Handle polling page
            if (m->dummy.run) {
                stub = (address_t)handle_c_polling_page_entry;
            } else {
                NOT_IMPL(); // TODO:
            }
        }
    }

    if (stub) {
        if (m) { // Save pc for restore
            m->saved_exception_pc = pc;
        }
        ucontext_set_pc(ucontext, stub);
        return 1;
    }

    if (!abort_if_unrecognized) {
        // caller wants another chance, so give it to him
        return 0;
    }

    // unmask current signal
    sigset_t new_set;
    sigemptyset(&new_set);
    sigaddset(&new_set, sig);
    sigprocmask(SIG_UNBLOCK, &new_set, NULL);

    if (thread) {
        LOG(FATAL, "\nThread start point[%s:%d] \nsignal handle: %d, \nis polling page: %d\nIn machine: %p",
            thread->start_point.file,
            thread->start_point.line,
            sig,
            mm_is_polling_page(info->si_addr),
            m);
    } else {
        LOG(FATAL, "signal handle: %d", sig);
    }

    UNREACHABLE();
    return 0;
}
