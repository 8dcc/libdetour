/**
 * @file      main.c
 * @brief     Example of the detour library
 * @author    8dcc
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "libdetour.h"

/* Here we asociate the return type and arguments for the function `foo'. This
 * macro will create a `typedef' that will be used internally by the macros. */
LIBDETOUR_DECL_TYPE(double, foo, double, double);

/* We declare the libdetour context global so it's accesible from our hook */
libdetour_ctx_t detour_ctx;

/* Target function we want to hook */
double foo(double a, double b) {
    printf("foo: %.1f + %.1f = %.1f\n", a, b, a + b);
    return a + b;
}

/* Our hook, the function that we want to be called instead of foo() */
double hook(double a, double b) {
    printf("hook: got values %.1f and %.1f\n", a, b);

    printf("hook: calling original with custom values...\n");

    /* Call the original function, ignoring the return value */
    LIBDETOUR_ORIG_CALL(&detour_ctx, foo, 9.5, 1.5);

    printf("hook: calling with original values...\n");

    /* Call the original function, storing the return value.
     * Note that the 3rd parameter is the one used for LIBDETOUR_DECL_TYPE */
    double my_var;
    LIBDETOUR_ORIG_GET(&detour_ctx, my_var, foo, a, b);

    printf("hook: original returned %.1f\n", my_var);

    printf("hook: returning custom value...\n");
    return 420.0;
}

int main(void) {
    double returned;
    void* orig_ptr = (void*)&foo;
    void* hook_ptr = (void*)&hook;

    /* Initialize context */
    libdetour_init(&detour_ctx, orig_ptr, hook_ptr);

    /* Hook the original function */
    libdetour_add(&detour_ctx);

    printf("main: hooked, calling foo...\n");

    /* Call the original function to test */
    returned = foo(5.0, 2.0);
    printf("main: hooked foo returned %.1f\n\n", returned);

    /* Once we are done, we can remove the hook */
    libdetour_del(&detour_ctx);

    printf("main: unhooked, calling again...\n");

    /* Hook is not called anymore */
    returned = foo(11.0, 3.0);
    printf("main: unhooked foo returned %.1f\n", returned);

    return 0;
}
