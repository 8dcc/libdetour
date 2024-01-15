/**
 * @file   detour.h
 * @brief  Detour hooking library header
 * @author 8dcc
 *
 * https://github.com/8dcc/detour-lib
 */

#ifndef DETOUR_H_
#define DETOUR_H_ 1

#include <stdint.h>

#ifdef __i386__
#define JMP_SZ_ 7 /* Size of jmp instructions in 32bit */
#else
#define JMP_SZ_ 12 /* Size of jmp instructions in 64bit */
#endif

typedef struct {
    bool detoured;
    void* orig;
    void* hook;
    uint8_t jmp_bytes[JMP_SZ_];
    uint8_t saved_bytes[JMP_SZ_];
} detour_ctx_t;

/*----------------------------------------------------------------------------*/

void detour_init(detour_ctx_t* ctx, void* orig, void* hook);
bool detour_add(detour_ctx_t* ctx);
bool detour_del(detour_ctx_t* ctx);

/*----------------------------------------------------------------------------*/

/* Declare the prototype of the original function */
#define DETOUR_DECL_TYPE(FUNCRET, FUNCNAME, ...) \
    typedef FUNCRET (*detour_##FUNCNAME##_t)(__VA_ARGS__);

/* Remove detour hook, call original, detour again.
 * Keep in mind that:
 *   - The returned value of the original function is not stored. If the
 *     function is not void, and you care about the return value, use
 *     DETOUR_ORIG_GET() instead.
 *   - FUNCNAME should be the same name passed to DETOUR_DECL_TYPE, without the
 *     prefix or subfix added by the macro ("detour_X_t") */
#define DETOUR_ORIG_CALL(CTX_PTR, FUNCNAME, ...)                 \
    do {                                                         \
        detour_del(CTX_PTR);                                     \
        ((detour_##FUNCNAME##_t)((CTX_PTR)->orig))(__VA_ARGS__); \
        detour_add(CTX_PTR);                                     \
    } while (0)

/* Same as DETOUR_ORIG_CALL, but accepts an extra parameter for storing the
 * returned value of the original function */
#define DETOUR_ORIG_GET(CTX_PTR, OUT_VAR, FUNCNAME, ...)                   \
    do {                                                                   \
        detour_del(CTX_PTR);                                               \
        OUT_VAR = ((detour_##FUNCNAME##_t)((CTX_PTR)->orig))(__VA_ARGS__); \
        detour_add(CTX_PTR);                                               \
    } while (0)

#endif /* DETOUR_H_ */
