/*
 * libdetour.h - Simple C/C++ library for detour hooking in Linux and Windows.
 * See: https://github.com/8dcc/libdetour
 *
 * Copyright (C) 2024 8dcc
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of  MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LIBDETOUR_H_
#define LIBDETOUR_H_ 1

#include <stdint.h>
#include <stdbool.h>

#if defined(__i386__)
#define LIBDETOUR_JMP_SZ_ 7  /* Size of `jmp' instructions in 32-bit */
#elif defined(__x86_64__)    /* !defined(__i386__) */
#define LIBDETOUR_JMP_SZ_ 12 /* Size of `jmp' instructions in 64-bit */
#else                        /* !defined(__x86_64__) */
#error "libdetour: The current architecture is not supported"
#endif /* !defined(__x86_64__) */

typedef struct {
    bool detoured;
    void* orig;
    void* hook;
    uint8_t saved_bytes[LIBDETOUR_JMP_SZ_];
} detour_ctx_t;

/*----------------------------------------------------------------------------*/

/*
 * Initialize the specified `detour_ctx_t' structure with the specified original
 * and hook functions. Although the structure is initialized, the `hook'
 * function is not yet hooked to `orig'; use `detour_add' for that.
 *
 * Naturally, the `orig' function should not be hooked when calling
 * `detour_init'.
 */
void detour_init(detour_ctx_t* ctx, void* orig, void* hook);

/*
 * Enable the detour hook associated to the specified `detour_ctx_t' structure.
 * The function returns `true' if the function was hooked successfully, or
 * `false' otherwise.
 */
bool detour_enable(detour_ctx_t* ctx);

/*
 * Disable the detour hook associated to the specified `detour_ctx_t' structure.
 * The function returns `true' if the function was unhooked successfully, or
 * `false' otherwise.
 */
bool detour_disable(detour_ctx_t* ctx);

/*----------------------------------------------------------------------------*/

/*
 * Declare the prototype of the original function, used when calling the
 * original.
 */
#define DETOUR_DECL_TYPE(FUNCRET, FUNCNAME, ...)                               \
    typedef FUNCRET (*libdetour_##FUNCNAME##_t)(__VA_ARGS__)

/*
 * Call the original function named FUNCNAME, using its associated
 * `detour_ctx_t' structure. It does this by unhooking the original, calling it,
 * and hooking again.
 *
 * The returned value of the original function is not stored. If the function
 * does not return void, and you care about the return value, use
 * `DETOUR_ORIG_GET' instead.
 *
 * The FUNCNAME argument should be the same symbol passed to `DETOUR_DECL_TYPE',
 * without prefixes or suffixes.
 */
#define DETOUR_ORIG_CALL(CTX_PTR, FUNCNAME, ...)                               \
    do {                                                                       \
        detour_disable(CTX_PTR);                                               \
        ((libdetour_##FUNCNAME##_t)((CTX_PTR)->orig))(__VA_ARGS__);            \
        detour_enable(CTX_PTR);                                                \
    } while (0)

/*
 * Same as `DETOUR_ORIG_CALL', but accepts an extra parameter for storing the
 * returned value of the original function.
 */
#define DETOUR_ORIG_GET(CTX_PTR, OUT_VAR, FUNCNAME, ...)                       \
    do {                                                                       \
        detour_disable(CTX_PTR);                                               \
        OUT_VAR = ((libdetour_##FUNCNAME##_t)((CTX_PTR)->orig))(__VA_ARGS__);  \
        detour_enable(CTX_PTR);                                                \
    } while (0)

#endif /* LIBDETOUR_H_ */
