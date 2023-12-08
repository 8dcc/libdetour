**Simple C library for detour hooking in linux.**


# Table of Contents

1.  [Description](#org04ad219)
2.  [Building](#org3f0b275)
3.  [Usage](#org7a323ba)


<a id="org04ad219"></a>

# Description

This is a linux library, but it should be easy to port to windows. As far as I
know, you should only need to change the [protect<sub>addr</sub>](https://github.com/8dcc/detour-lib/blob/f171e7fcb0e10eeb04c942c6f004a2fea75c7b2c/src/detour.c#L18-L30) function.

It supports x64 and x86.

If you want to use this library, simply copy the detour source and headers to
your project, include the header in your source files and compile the detour
source with the rest of your code. Please see [src/main.c](https://github.com/8dcc/detour-lib/blob/main/src/main.c) and for an [Usage](#org7a323ba)
example on how to use it.

This library was originally made for [8dcc/hl-cheat](https://github.com/8dcc/hl-cheat), and was inspired by
[this OOP abomination](https://guidedhacking.com/threads/simple-linux-windows-detour-class.10580/) ([mirror](https://gist.github.com/8dcc/d0cbef32cd46ab9c73c6f830fa71d999)).


<a id="org3f0b275"></a>

# Building

    $ git clone https://github.com/8dcc/detour-lib
    $ cd detour-lib
    $ make
    ...


<a id="org7a323ba"></a>

# Usage

First, you will need to specify the type and arguments of the original function
with the `DETOUR_DECL_TYPE` macro. You will also need to declare a `detour_ctx_t`
context struct:

    /* int orig(double a, double b); */
    DETOUR_DECL_TYPE(int, orig, double, double);
    
    detour_ctx_t detour_ctx;

This macro will `typedef` a type needed internally by the library, so make sure
you call it globally. The context struct be accesible when calling the original
function (e.g. from your hook), so keep that in mind as well.

Then, initialize the context struct by calling `detour_init` with a pointer to the
original function and a pointer to your hook function:

    void* orig_ptr = &orig; /* orig(...) */
    void* hook_ptr = &hook; /* hook(...) */
    
    /* Initialize the detour context */
    detour_init(&detour_ctx, orig_ptr, hook_ptr);
    
    /* Hook the original function */
    detour_add(&detour_ctx);

If you want to call the original function from your hook, you can use one of the
following macros:

-   `DETOUR_ORIG_CALL`: Calls the original function, ignores the returned value.
-   `DETOUR_ORIG_GET`: Takes an extra parameter used for storing the return value.

    double hook(double a, double b) {
        /* Call original ignoring return */
        DETOUR_ORIG_CALL(&detour_ctx, orig, a, b);
    
        /* Store return value in variable */
        int result;
        DETOUR_ORIG_GET(&detour_ctx, result, orig, a, b);
    
        /* Our hook can overwrite the return value */
        return 123;
    }

Once we are done, we can call `detour_del` to remove the hook:

    /* Remove hook */
    detour_del(&detour_ctx);

If we call `orig()` again, our hook function will not be called.

For a full working example, see [src/main.c](https://github.com/8dcc/detour-lib/blob/main/src/main.c). You can also run `make all` or `make
all-32bit`, and try executing `detour-test.out`.

