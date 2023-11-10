#pragma once
#ifndef YALX_RUNTIME_PLATFORM_SIGNAL_H
#define YALX_RUNTIME_PLATFORM_SIGNAL_H

#ifdef __cplusplus
extern "C" {
#endif

void yalx_install_signals_handler();
void yalx_uninstall_signals_handler();

#ifdef __cplusplus
}
#endif

#endif // YALX_RUNTIME_PLATFORM_SIGNAL_H
