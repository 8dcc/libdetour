/*
 * Copyright 2024 8dcc
 *
 * This file is part of libdetour.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <stdio.h>

#include "libdetour.h"

/*
 * Declare an internal libdetour type for the function `foo', used by the
 * `DETOUR_ORIG_CALL' and `DETOUR_ORIG_GET' macros. You don't need to declare
 * this type if you don't plan on using those macros.
 */
DETOUR_DECL_TYPE(double, foo, double, double);

/*
 * Since we are going to initialize the `detour_ctx_t' structure from `main',
 * but we also want to access it from `my_hook' (for calling the original
 * function), we declare it globally.
 *
 * Again, it's not necessary to make it global, it just depends on what you want
 * to do with it.
 */
detour_ctx_t detour_ctx;

/*
 * The `foo' function represents the target function we want to hook.
 */
double foo(double a, double b) {
    printf("foo: %.1f + %.1f = %.1f\n", a, b, a + b);
    return a + b;
}

/*
 * This `my_hook' function represents the function that we want to hook to
 * `foo'. That is, the function that should be called whenever a call to `foo'
 * is made.
 */
double my_hook(double a, double b) {
    printf("my_hook: got values %.1f and %.1f\n", a, b);

    /*
     * Call the original function, ignoring the return value
     */
    printf("my_hook: calling original with custom values...\n");
    DETOUR_ORIG_CALL(&detour_ctx, foo, 9.5, 1.5);

    /*
     * Call the original function, storing the return value in `my_var'.
     *
     * Note that the 3rd parameter of `DETOUR_ORIG_GET' matches the one that we
     * used as the second argument for `DETOUR_DECL_TYPE' above.
     */
    printf("my_hook: calling with original values...\n");
    double my_var;
    DETOUR_ORIG_GET(&detour_ctx, my_var, foo, a, b);
    printf("my_hook: original returned %.1f\n", my_var);

    printf("my_hook: returning custom value...\n");
    return 420.0;
}

int main(void) {
    double returned;
    void* orig_ptr = (void*)&foo;
    void* hook_ptr = (void*)&my_hook;

    /*
     * Initialize the detour context we declared globally, and enable it.
     */
    detour_init(&detour_ctx, orig_ptr, hook_ptr);
    assert(detour_enable(&detour_ctx) == true);

    /*
     * Call the original function, and print what was returned.
     */
    printf("main: hooked, calling `foo'...\n");
    returned = foo(5.0, 2.0);
    printf("main: hooked `foo' returned %.1f\n\n", returned);

    /*
     * Disable the hook, and make sure that the behavior of `foo' is restored.
     */
    assert(detour_disable(&detour_ctx) == true);
    printf("main: unhooked, calling `foo' again...\n");
    returned = foo(11.0, 3.0);
    printf("main: unhooked `foo' returned %.1f\n", returned);

    return 0;
}
