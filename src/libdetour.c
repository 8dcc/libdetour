/**
 * @file   libdetour.c
 * @brief  Detour hooking library source
 * @author 8dcc
 *
 * https://github.com/8dcc/libdetour
 */

/* NOTE: Remember to change this if you move the header */
#include "libdetour.h"

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

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
static uint8_t def_jmp_bytes[] = { 0xB8, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xE0 };
#define JMP_BYTES_OFF 1 /* Offset inside the array where the ptr should go */
#else
static uint8_t def_jmp_bytes[] = { 0x48, 0xB8, 0x00, 0x00, 0x00, 0x00,
                                   0x00, 0x00, 0x00, 0x00, 0xFF, 0xE0 };
#define JMP_BYTES_OFF 2 /* Offset inside the array where the ptr should go */
#endif

/*----------------------------------------------------------------------------*/

#ifdef __unix__
#include <unistd.h>   /* sysconf() */
#include <sys/mman.h> /* mprotect() */

static bool protect_addr(void* ptr, bool enable_write) {
    long page_size      = sysconf(_SC_PAGESIZE);
    long page_mask      = ~(page_size - 1);
    uintptr_t next_page = ((uintptr_t)ptr + page_size - 1) & page_mask;
    uintptr_t prev_page = next_page - page_size;
    void* page          = (void*)prev_page;

    uint32_t new_flags = PROT_READ | PROT_EXEC;
    if (enable_write)
        new_flags |= PROT_WRITE;

    if (mprotect(page, page_size, new_flags) == -1)
        return false;

    return true;
}
#elif defined _WIN32
#include <Windows.h> /* VirtualProtect() */

static bool protect_addr(void* ptr, bool enable_write) {
    DWORD old_flags;
    DWORD new_flags = enable_write ? PAGE_EXECUTE_READWRITE : PAGE_EXECUTE_READ;
    return VirtualProtect(ptr, sizeof(def_jmp_bytes), new_flags, &old_flags);
}
#else
#error "libdetour: This systems is not supported"
#endif

/*----------------------------------------------------------------------------*/

void libdetour_init(libdetour_ctx_t* ctx, void* orig, void* hook) {
    ctx->detoured = false;
    ctx->orig     = orig;
    ctx->hook     = hook;

    /* Store the first N bytes of the original function, where N is the size of
     * the jmp instructions */
    memcpy(ctx->saved_bytes, orig, sizeof(ctx->saved_bytes));

    /* Default jmp bytes */
    memcpy(ctx->jmp_bytes, &def_jmp_bytes, sizeof(def_jmp_bytes));

    /* JMP_BYTES_OFF is defined bellow def_jmp_bytes, and it changes depending
     * on the arch.
     * We use "&hook" and not "hook" because we want the address of
     * the func, not the first bytes of it like before. */
    memcpy(&ctx->jmp_bytes[JMP_BYTES_OFF], &hook, sizeof(void*));
}

bool libdetour_add(libdetour_ctx_t* ctx) {
    /* Already detoured, nothing to do */
    if (ctx->detoured)
        return true;

    /* See util.c */
    if (!protect_addr(ctx->orig, true))
        return false;

    /* Copy our jmp instruction with our hook address to the orig */
    memcpy(ctx->orig, ctx->jmp_bytes, sizeof(ctx->jmp_bytes));

    /* Restore old protection */
    if (!protect_addr(ctx->orig, false))
        return false;

    ctx->detoured = true;
    return true;
}

bool libdetour_del(libdetour_ctx_t* ctx) {
    /* Not detoured, nothing to do */
    if (!ctx->detoured)
        return true;

    /* See util.c */
    if (!protect_addr(ctx->orig, true))
        return false;

    /* Restore the bytes that were at the start of orig (we saved on init) */
    memcpy(ctx->orig, ctx->saved_bytes, sizeof(ctx->saved_bytes));

    /* Restore old protection */
    if (!protect_addr(ctx->orig, false))
        return false;

    ctx->detoured = false;
    return true;
}
