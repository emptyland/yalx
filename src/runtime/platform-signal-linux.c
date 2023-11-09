#include "runtime/checking.h"
#include <signal.h>
#include <errno.h>
#include <stddef.h>

extern int yalx_handle_signal(int sig, siginfo_t *info, void *uv, int abort_if_unrecognized);

static void signal_handler(int sig, siginfo_t* info, void* uc) {
    GUARANTEE(info != NULL && uc != NULL, "Should be old kernel");
    int saved_err_no = errno;
    yalx_handle_signal(sig, info, uc, 1);
    errno = saved_err_no;
}

static void set_signal_handler(int sig, int install) {
    struct sigaction old_act;
    sigaction(sig, NULL, &old_act);

    void *old_handler = old_act.sa_sigaction ? (void *)old_act.sa_sigaction : (void *)old_act.sa_handler;
    if (old_handler != SIG_DFL && old_handler != SIG_IGN && old_handler != signal_handler) {
        if (!install) {
            return;
        }
        LOG(FATAL, "Bad signal handler");
    }

    struct sigaction sig_act;
    sigfillset(&sig_act.sa_mask);
    sig_act.sa_handler = SIG_DFL;
    sig_act.sa_flags = SA_SIGINFO|SA_RESTART;
    if (install) {
        sig_act.sa_sigaction = signal_handler;
    }

    int rs = sigaction(sig, &sig_act, &old_act);
    GUARANTEE(rs == 0, "sigaction fail!");

    void *prev_handler = old_act.sa_sigaction ? (void *)old_act.sa_sigaction : (void *)old_act.sa_handler;
    DCHECK(prev_handler == old_handler);
}

void yalx_install_signals_handler() {
    set_signal_handler(SIGSEGV, 1);
    set_signal_handler(SIGPIPE, 1);
    set_signal_handler(SIGBUS, 1);
    set_signal_handler(SIGILL, 1);
    set_signal_handler(SIGFPE, 1);
    set_signal_handler(SIGXFSZ, 1);
}