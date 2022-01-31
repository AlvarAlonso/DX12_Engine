#pragma once

// Unsigned int types
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;

// Signed int types
typedef signed char int8;
typedef signed short int16;
typedef signed int int32;
typedef signed long long int64;

// Floating point types
typedef float float32;
typedef double float64;

#define STATIC_ASSERT static_assert

// Ensure all types are of the correct type
STATIC_ASSERT(sizeof(uint8) == 1, "Expected uint8 to be 1 byte.");
STATIC_ASSERT(sizeof(uint16) == 2, "Expected uint16 to be 2 byte.");
STATIC_ASSERT(sizeof(uint32) == 4, "Expected uint32 to be 4 byte.");
STATIC_ASSERT(sizeof(uint64) == 8, "Expected uint64 to be 8 byte.");

STATIC_ASSERT(sizeof(int8) == 1, "Expected int8 to be 1 byte.");
STATIC_ASSERT(sizeof(int16) == 2, "Expected int16 to be 2 byte.");
STATIC_ASSERT(sizeof(int32) == 4, "Expected int32 to be 4 byte.");
STATIC_ASSERT(sizeof(int64) == 8, "Expected int64 to be 8 byte.");

STATIC_ASSERT(sizeof(float32) == 4, "Expected float32 to be 4 byte.");
STATIC_ASSERT(sizeof(float64) == 8, "Expected float64 to be 8 byte.");

STATIC_ASSERT(sizeof(bool) == 1, "Expected bool to be 8 byte.");

#define BIT(x) (1 << x)

// Pass function X with the event as parameter in arg position _1
#define BIND_EVENT_FUNC(x) std::bind(&x, this, std::placeholders::_1)