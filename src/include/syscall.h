#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum {
    KR_RECV,
    KR_SEND,
    KR_READ,
    KR_WRITE,
} KR_Type;

//////////////////////////////////
//// Syscall argument structs ////
//////////////////////////////////

// RECV //
typedef struct {
    const char* path;
    void* buf;
    size_t size;
    bool nonblock;
} KR_RECV_Request;
typedef struct {
    uint32_t error;
    size_t received;
    bool closed;
} KR_RECV_Response;

// SEND //
typedef struct {
    const char* path;
    void* buf;
    size_t size;
} KR_SEND_Request;
typedef struct {
    uint32_t error;
} KR_SEND_Response;

// READ //
typedef struct {
    const char* path;
    void* buf;
    size_t size;
} KR_READ_Request;
typedef struct {
    uint32_t error;
} KR_READ_Response;

// WRITE //
typedef struct {
    const char* path;
    void* buf;
    size_t size;
} KR_WRITE_Request;
typedef struct {
    uint32_t error;
} KR_WRITE_Response;

//////////////////////////////////
/////// Main syscall struct //////
//////////////////////////////////
typedef struct {
    KR_Type type;
    union {
        KR_RECV_Request recv;
        KR_SEND_Request send;
        KR_READ_Request read;
        KR_WRITE_Request write;
    } request;
    union {
        KR_RECV_Response recv;
        KR_SEND_Response send;
        KR_READ_Response read;
        KR_WRITE_Response write;
    } response;
} KR;
void syscall(KR* request);
#endif // SYSCALL_H
