#ifndef DETOUR_H_
#define DETOUR_H_

#define JMP_SZ 7

typedef struct {
    bool detoured;
    void* orig;
    void* hook;
    uint8_t jmp_bytes[JMP_SZ];
    uint8_t saved_bytes[JMP_SZ];
} detour_data_t;

/*----------------------------------------------------------------------------*/

void detour_init(detour_data_t* data, void* orig, void* hook);
bool detour_add(detour_data_t* d);
bool detour_del(detour_data_t* d);

/*----------------------------------------------------------------------------*/

/* Declare the type for the original function */
#define DECL_DETOUR_TYPE(funcRet, funcName, ...) \
    typedef funcRet (*funcName##_t)(__VA_ARGS__);

/* Reset original bytes, call original, detour again. detourData is NOT a ptr */
#define CALL_ORIGINAL(detourData, funcName, ...)      \
    {                                                 \
        detour_del(&detourData);                      \
        ((funcName##_t)detourData.orig)(__VA_ARGS__); \
        detour_add(&detourData);                      \
    }

#endif    // DETOUR_H_
