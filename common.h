#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

typedef uint8_t   u8;
typedef uint16_t  u16;
typedef uint32_t  u32;
typedef uint64_t  u64;

typedef int8_t    i8;
typedef int16_t   i16;
typedef int32_t   i32;
typedef int64_t   i64;

typedef float     f32;
typedef double    f64;

typedef u8 	  b8;

typedef i8 status;
#define SENTINEL	 0x69
#define STATUS_SENTINEL  ((status)SENTINEL)
#define STATUS_OK        ((status)0x00)
#define STATUS_ERR_NULL  ((status)0x10)
#define STATUS_ERR_OOB   ((status)0x20)
#define STATUS_ERR_IO    ((status)0x30)
#define STATUS_ERR_MEM   ((status)0x40)

 #define DEBUG(fmt, ...) fprintf(stderr, "[DEBUG %s:%d in %s()] " fmt "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define ARR_GET(arr, i, len) (assert((i) < (len)), (arr)[(i)])
#define ARR_SET(arr, i, len, val) (assert((i) < (len)), (arr)[(i)] = (val))
#define ARR2D_GET(arr, i, ilen, j, jlen) (assert((i) < (ilen) && (j) < (jlen)), (arr)[(i)][(j)])
#define ARR2D_SET(arr, i, ilen, j, jlen, val) (assert((i) < (ilen) && (j) < (jlen)), (arr)[(i)][(j)] = (val))
#define LOG(fmt, ...) fprintf(stderr, "[LOG] %s:%d in %s()" fmt "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define CHECK(expr) do { if (!(expr)) { \
	fprintf(stderr, "[CHECK] %s at %s:%d in %s()\n", "!"#expr, __FILE__, __LINE__, __func__); exit(1); }} while(0)
#define TRY(call) do { if ((call)!=STATUS_OK) { \
	fprintf(stderr, "[TRY] %s at %s:%d in %s()\n", #call"!=STATUS_OK", __FILE__, __LINE__, __func__); exit(1); }} while(0)
#define EXPECT(val, good) do { if ((val)!=(good)) { \
	fprintf(stderr, "[EXPECT] %s at %s:%d in %s()\n", #val"!="#good, __FILE__, __LINE__, __func__); exit(1); }} while(0)

#endif // COMMON_H
