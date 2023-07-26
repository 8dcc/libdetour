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
#define DECL_DETOUR_TYPE(funcRet, newTypeName, ...) \
    typedef funcRet (*newTypeName##_t)(__VA_ARGS__);

/* Reset original bytes, call original, detour again.
 * Keep in mind that:
 *   - The returned value of the original function is not stored. If the
 *     function is not void, and you care about the return value, use
 *     GET_ORIGINAL() instead.
 *   - detourData is NOT a pointer, it expects the full struct
 *   - funcType should be the same name passed to DECL_DETOUR_TYPE, without the
 *     ending added by the macro ("_t") */
#define CALL_ORIGINAL(detourData, funcType, ...)      \
    {                                                 \
        detour_del(&detourData);                      \
        ((funcType##_t)detourData.orig)(__VA_ARGS__); \
        detour_add(&detourData);                      \
    }

/* Same as CALL_ORIGINAL, but accepts an extra parameter for storing the
 * returned value of the original function */
#define GET_ORIGINAL(detourData, returnVar, funcType, ...)        \
    {                                                             \
        detour_del(&detourData);                                  \
        returnVar = ((funcType##_t)detourData.orig)(__VA_ARGS__); \
        detour_add(&detourData);                                  \
    }

#endif /* DETOUR_H_ */
