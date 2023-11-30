#pragma once
#ifndef YALX_RUNTIME_PLATFORM_SIGNAL_H
#define YALX_RUNTIME_PLATFORM_SIGNAL_H

#ifdef __cplusplus
extern "C" {
#endif

void yalx_install_signals_handler(void);
void yalx_uninstall_signals_handler(void);

#ifdef __cplusplus
}
#endif

#endif // YALX_RUNTIME_PLATFORM_SIGNAL_H
