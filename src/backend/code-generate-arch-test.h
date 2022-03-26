#pragma once
#ifndef YALX_BACKEND_CODE_GENERATE_ARCH_H_
#define YALX_BACKEND_CODE_GENERATE_ARCH_H_

#include "runtime/runtime.h"

extern "C" {
void call_returning_vals(void *returnning_vals, size_t size_in_bytes, void *yalx_fun);
void main_Zomain_Zd_Z4init();
void main_Zomain_Zdissue1();
void main_Zomain_Zdissue5();
void main_Zomain_Zdfoo();
void main_Zomain_Zdmain_had();
void main_Zomain_Zdissue6_had(i32_t a, i32_t b);
void main_Zomain_Zdissue11_had();

void issue00_Zoissue00_Zd_Z4init();
void issue00_Zoissue00_Zdissue1_had();

void issue02_Zoissue02_Zd_Z4init();
void issue02_Zoissue02_Zdissue1_had();
void issue02_Zoissue02_Zdissue2_had();
void issue02_Zoissue02_Zdissue3_had();



} // extern "C"

#endif // YALX_BACKEND_CODE_GENERATE_ARCH_H_
