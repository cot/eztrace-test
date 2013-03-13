#ifndef INSTRUMENT_H
#define INSTRUMENT_H
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void record_event0(uint64_t code);
void record_event1(uint64_t code, uint64_t arg1);
void record_event2(uint64_t code, uint64_t arg1, uint64_t arg2);
void record_event3(uint64_t code, uint64_t arg1, uint64_t arg2, uint64_t arg3);
void record_event4(uint64_t code, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4);
void record_event5(uint64_t code, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5);

void __init (void);
void __conclude (void);

#ifdef __cplusplus
};
#endif
#endif
