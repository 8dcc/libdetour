/**
 * @file   detour.c
 * @brief  Detour hooking library source
 * @author 8dcc
 *
 * https://github.com/8dcc/detour-lib
 */

#ifndef __unix__
#error "detour-lib: Non-unix systems are not supported"
#endif

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>   /* sysconf() */
#include <sys/mman.h> /* mprotect() */

/* NOTE: Remember to change this if you move the header */
#include "detour.h"

#define PAGE_MASK          (~(PAGE_SIZE - 1))
#define PAGE_ALIGN(X)      (((uintptr_t)(X) + PAGE_SIZE - 1) & PAGE_MASK)
#define PAGE_ALIGN_DOWN(X) (PAGE_ALIGN(X) - PAGE_SIZE)

static bool protect_addr(void* ptr, int new_flags) {
    int PAGE_SIZE = sysconf(_SC_PAGESIZE);
    void* page    = (void*)PAGE_ALIGN_DOWN(ptr);

    if (mprotect(page, PAGE_SIZE, new_flags) == -1)
        return false;

    return true;
}

/*
 * 64 bits:
 *   0:  48 b8 88 77 66 55 44    movabs rax, 0x1122334455667788
 *   7:  33 22 11
 *   a:  ff e0                   jmp    rax
 *
 * 32 bits:
 *   0:  b8 44 33 22 11          mov    eax, 0x11223344
 *   5:  ff e0                   jmp    eax
 */
#ifdef __i386__
static uint8_t def_jmp_bytes[] = { 0xB8, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xE0 };
#define JMP_BYTES_OFF 1 /* Offset inside the array where the ptr should go */
#else
static uint8_t def_jmp_bytes[] = { 0x48, 0xB8, 0x00, 0x00, 0x00, 0x00,
                                   0x00, 0x00, 0x00, 0x00, 0xFF, 0xE0 };
#define JMP_BYTES_OFF 2 /* Offset inside the array where the ptr should go */
#endif

void detour_init(detour_ctx_t* ctx, void* orig, void* hook) {
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

bool detour_add(detour_ctx_t* ctx) {
    /* Already detoured, nothing to do */
    if (ctx->detoured)
        return true;

    /* See util.c */
    if (!protect_addr(ctx->orig, PROT_READ | PROT_WRITE | PROT_EXEC))
        return false;

    /* Copy our jmp instruction with our hook address to the orig */
    memcpy(ctx->orig, ctx->jmp_bytes, sizeof(ctx->jmp_bytes));

    /* Restore old protection */
    if (!protect_addr(ctx->orig, PROT_READ | PROT_EXEC))
        return false;

    ctx->detoured = true;
    return true;
}

bool detour_del(detour_ctx_t* ctx) {
    /* Not detoured, nothing to do */
    if (!ctx->detoured)
        return true;

    /* See util.c */
    if (!protect_addr(ctx->orig, PROT_READ | PROT_WRITE | PROT_EXEC))
        return false;

    /* Restore the bytes that were at the start of orig (we saved on init) */
    memcpy(ctx->orig, ctx->saved_bytes, sizeof(ctx->saved_bytes));

    /* Restore old protection */
    if (!protect_addr(ctx->orig, PROT_READ | PROT_EXEC))
        return false;

    ctx->detoured = false;
    return true;
}
