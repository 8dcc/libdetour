/**
 * @file      detour.h
 * @brief     Detour hooking library header
 * @author    8dcc
 *
 * https://github.com/8dcc/detour-lib
 */

#ifndef DETOUR_H_
#define DETOUR_H_

#ifdef __i386__
typedef uint32_t detour_ptr_t;
#define JMP_SZ_ 7 /* Size of jmp instructions in 32bit */
#else
typedef uint64_t detour_ptr_t;
#define JMP_SZ_ 12 /* Size of jmp instructions in 64bit */
#endif

typedef struct {
    bool detoured;
    void* orig;
    void* hook;
    uint8_t jmp_bytes[JMP_SZ_];
    uint8_t saved_bytes[JMP_SZ_];
} detour_data_t;

/*----------------------------------------------------------------------------*/

void detour_init(detour_data_t* data, void* orig, void* hook);
bool detour_add(detour_data_t* d);
bool detour_del(detour_data_t* d);

/*----------------------------------------------------------------------------*/

/* Declare the type for the original function */
#define DECL_DETOUR_TYPE(funcRet, funcName, ...) \
    typedef funcRet (*funcName##_t)(__VA_ARGS__);

/* Reset original bytes, call original, detour again. detourData is NOT a ptr */
#define CALL_ORIGINAL(detourData, funcName, ...)      \
    {                                                 \
        detour_del(&detourData);                      \
        ((funcName##_t)detourData.orig)(__VA_ARGS__); \
        detour_add(&detourData);                      \
    }

#endif /* DETOUR_H_ */
