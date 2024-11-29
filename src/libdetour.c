/*
 * libdetour.c - Simple C/C++ library for detour hooking in Linux and Windows.
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

/* NOTE: Remember to change this if you move the header */
#include "libdetour.h"

#include <stdint.h>
#include <stdbool.h>
#include <string.h> /* memcpy() */

/*----------------------------------------------------------------------------*/

/*
 * 32 bits:
 *   0:  b8 44 33 22 11          mov    eax, 0x11223344
 *   5:  ff e0                   jmp    eax
 *
 * 64 bits:
 *   0:  48 b8 88 77 66 55 44    movabs rax, 0x1122334455667788
 *   7:  33 22 11
 *   a:  ff e0                   jmp    rax
 */
#ifdef __i386__
static uint8_t jmp_bytes[] = { 0xB8, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xE0 };
#define JMP_BYTES_OFF 1 /* Offset inside the array where the ptr should go */
#else
static uint8_t jmp_bytes[] = { 0x48, 0xB8, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0xFF, 0xE0 };
#define JMP_BYTES_OFF 2 /* Offset inside the array where the ptr should go */
#endif

/*----------------------------------------------------------------------------*/

#if defined(__unix__)
#include <unistd.h>   /* sysconf() */
#include <sys/mman.h> /* mprotect() */

static bool protect_addr(void* ptr, bool enable_write) {
    /*
     * For more information, see:
     * https://8dcc.github.io/reversing/challenge10.html#c-translation
     */
    const long page_size      = sysconf(_SC_PAGESIZE);
    const long page_mask      = ~(page_size - 1);
    const uintptr_t next_page = ((uintptr_t)ptr + page_size - 1) & page_mask;
    void* prev_page           = (void*)(next_page - page_size);

    uint32_t new_flags = PROT_READ | PROT_EXEC;
    if (enable_write)
        new_flags |= PROT_WRITE;

    if (mprotect(prev_page, page_size, new_flags) == -1)
        return false;

    return true;
}
#elif defined(_WIN32) /* !defined(__unix__) */
#include <windows.h>  /* VirtualProtect() */

static bool protect_addr(void* ptr, bool enable_write) {
    DWORD old_flags;
    DWORD new_flags = enable_write ? PAGE_EXECUTE_READWRITE : PAGE_EXECUTE_READ;
    return VirtualProtect(ptr, sizeof(jmp_bytes), new_flags, &old_flags);
}
#else                 /* !defined(_WIN32) */
#error "libdetour: This systems is not supported"
#endif /* !defined(_WIN32) */

/*----------------------------------------------------------------------------*/

void detour_init(detour_ctx_t* ctx, void* orig, void* hook) {
    ctx->detoured = false;
    ctx->orig     = orig;
    ctx->hook     = hook;

    /*
     * Store the first N bytes of the original function, where N is the size of
     * the `jmp' instructions.
     */
    memcpy(ctx->saved_bytes, orig, sizeof(ctx->saved_bytes));
}

bool detour_enable(detour_ctx_t* ctx) {
    if (ctx->detoured)
        return true;

    /*
     * Enable write permissions on the specified address. The `protect_addr'
     * function is defined above as a static function, depending on the current
     * OS.
     */
    if (!protect_addr(ctx->orig, true))
        return false;

    /*
     * Write the address of the `hook' function to the array for the encoded
     * `jmp' instruction, at the `JMP_BYTES_OFF' offset (which changes depending
     * on the architecture at compile-time).
     */
    memcpy(&jmp_bytes[JMP_BYTES_OFF], &(ctx->hook), sizeof(void*));

    /*
     * Copy the whole (now filled) `jmp_bytes' array to the start of the body
     * the target function (`ctx->orig'). This new `jmp' instruction will be
     * responsible for the actual hook.
     */
    memcpy(ctx->orig, jmp_bytes, sizeof(jmp_bytes));

    /*
     * Restore the old protection for this address.
     */
    if (!protect_addr(ctx->orig, false))
        return false;

    ctx->detoured = true;
    return true;
}

bool detour_disable(detour_ctx_t* ctx) {
    if (!ctx->detoured)
        return true;

    /*
     * See `detour_enable'.
     */
    if (!protect_addr(ctx->orig, true))
        return false;

    /*
     * Restore the bytes we saved inside the `detour_ctx_t' structure in
     * `detour_init'.
     */
    memcpy(ctx->orig, ctx->saved_bytes, sizeof(ctx->saved_bytes));

    if (!protect_addr(ctx->orig, false))
        return false;

    ctx->detoured = false;
    return true;
}
