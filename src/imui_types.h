#pragma once

#include <float.h>
#include <stddef.h>
#include <stdint.h>

#ifdef _DEBUG
#	include <assert.h>
#endif

#ifdef _MSC_VER
#	include <intrin.h>
#endif

#include <limits.h>

typedef uint8_t		uint8;
typedef uint16_t	uint16;
typedef uint32_t	uint32;
typedef uint64_t	uint64;

typedef int8_t		sint8;
typedef int16_t		sint16;
typedef int32_t		sint32;
typedef int64_t		sint64;

typedef size_t		uintsize;
typedef ptrdiff_t 	sintsize;

typedef uint8_t		byte;

typedef struct ImUiStringView
{
	const char*		data;
	uintsize		length;
} ImUiStringView;

#ifndef IMUI_DEFAULT_ARRAY_CAPACITY
#	define IMUI_DEFAULT_ARRAY_CAPACITY				16u
#endif
#ifndef IMUI_DEFAULT_WIDGET_CHUNK_SIZE
#	define IMUI_DEFAULT_WIDGET_CHUNK_SIZE			256u
#endif
#ifndef IMUI_DEFAULT_STRING_POOL_CHUNK_SIZE
#	define IMUI_DEFAULT_STRING_POOL_CHUNK_SIZE		4096u
#endif

#define IMUI_FLOAT_INF (FLT_MAX / 100.0f)
#define IMUI_SIZE_MAX ((uintsize)-1)

#ifdef _DEBUG
#	define IMUI_ASSERT( exp ) assert( exp )
#else
#	define IMUI_ASSERT( exp )
#endif

#define IMUI_MIN( a, b ) ((a) < (b) ? (a) : (b))
#define IMUI_MAX( a, b ) ((a) > (b) ? (a) : (b))

#define IMUI_ARRAY_COUNT( arr ) (sizeof( arr ) / sizeof( *(arr) ))

#if defined( __GNUC__ ) || defined( __clang__ )
#	define IMUI_OFFSETOF( type, member )		__builtin_offsetof( type, member )
#	define IMUI_COUNT_LEADING_ZEROS32( val )	__builtin_clz( val )
#	define IMUI_COUNT_LEADING_ZEROS64( val )	__builtin_clzl( val )
#else
#	define IMUI_OFFSETOF( type, member )		((uintsize)(&((type*)0)->member))
#	define IMUI_COUNT_LEADING_ZEROS32( val )	__lzcnt( val )
#	define IMUI_COUNT_LEADING_ZEROS64( val )	__lzcnt64( val )
#endif

#if defined(_M_X64) || defined(__amd64__)
#	define IMUI_NEXT_POWER_OF_TWO( val )	(1ll << (64 - IMUI_COUNT_LEADING_ZEROS64( val -1 )))
#else
#	define IMUI_NEXT_POWER_OF_TWO( val )	(1 << (32 - IMUI_COUNT_LEADING_ZEROS32( val -1 )))
#endif
