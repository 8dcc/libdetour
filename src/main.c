/**
 * @file      main.c
 * @brief     Example of the detour library
 * @author    8dcc
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "detour.h"

/* Here we asociate the return type and arguments for the function "foo". This
 * macro will create the type "foo_type_t", that will be used by the
 * CALL_ORIGINAL() macro from the hook. */
DECL_DETOUR_TYPE(double, foo_type, double, double);

/* We declare the detour data struct global so it's accesible from our hook */
detour_data_t detour_data;

/* This would be the target function we want to hook */
double foo(double a, double b) {
    printf("foo: %.1f\n", a + b);
    return a - b;
}

/* This would be our hook, the function that we want to be called at the
 * begining of foo(). We don't need to return anything here. */
void hook(double a, double b) {
    printf("hook: %.1f, %.1f\n", a, b);

    /* We call the original function at the end of our hook, with its type. We
     * use GET_ORIGINAL instead of CALL_ORIGINAL because we care about the
     * returned value of foo(). Use CALL_ORIGNAL if you dont care about it. */
    double ret_value;
    GET_ORIGINAL(detour_data, ret_value, foo_type, a, b);

    printf("hook: returned value is %.1f\n", ret_value);
}

int main() {
    void* orig_ptr = &foo;
    void* hook_ptr = &hook;

    /* Initialize detour_data struct */
    detour_init(&detour_data, orig_ptr, hook_ptr);

    /* Detour hook the original function */
    detour_add(&detour_data);

    /* Call the original function to test */
    foo(5.0, 2.0);

    /* Once we are done, we can remove the hook */
    detour_del(&detour_data);

    /* Hook is not called anymore */
    foo(11.0, 3.0);

    return 0;
}
