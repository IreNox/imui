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

typedef uint8_t		uint8;
typedef uint16_t	uint16;
typedef uint32_t	uint32;
typedef uint64_t	uint64;

typedef int8_t		sint8;
typedef int16_t		sint16;
typedef int32_t		sint32;
typedef int64_t		sint64;

typedef size_t		uintsize;

typedef uint8_t		byte;

typedef struct ImUiStringView
{
	const char*		data;
	size_t			length;
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
#	define IMUI_OFFSETOF( type, member )	__builtin_offsetof( type, member )
#	define IMUI_COUNT_LEADING_ZEROS( val )	__builtin_clz( val )
#else
#	define IMUI_OFFSETOF( type, member )	((size_t)(&((type*)0)->member))
#	define IMUI_COUNT_LEADING_ZEROS( val )	__lzcnt( val )
#endif
