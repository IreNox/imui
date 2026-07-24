#pragma once

#include <imui/imui.h>

#include "imui_helpers.h"

typedef struct ImuiTextLayoutCache
{
	ImuiAllocator*			allocator;

	ImuiHashMap				layoutMap;

	ImuiTextLayout*			firstLayout;
	ImuiTextLayout*			firstUnusedLayout;

	uint32					frameIndex;
} ImuiTextLayoutCache;

typedef struct ImuiTextGlyph
{
	uint32					charIndex;
	uint32					codepoint;
	ImuiPos					pos;
	ImuiSize				size;
	ImuiTexCoord			uv;
} ImuiTextGlyph;

typedef struct ImuiTextLayoutParameters
{
	ImuiFont*				font;
	ImuiStringView			text;
} ImuiTextLayoutParameters;

struct ImuiTextLayout
{
	ImuiFont*				font;
	ImuiStringView			text;

	ImuiTextLayout*			prevLayout;
	ImuiTextLayout*			nextLayout;

	const ImuiTextGlyph*	glyphs;
	uintsize				glyphCount;

	uint32					frameIndex;

	ImuiSize				size;
};

bool						imuiTextLayoutCacheConstruct( ImuiTextLayoutCache* cache, ImuiAllocator* allocator );
void						imuiTextLayoutCacheDestruct( ImuiTextLayoutCache* cache );

void						imuiTextLayoutCacheEndFrame( ImuiTextLayoutCache* cachce );

ImuiTextLayout*				imuiTextLayoutCacheCreateLayout( ImuiTextLayoutCache* cache, const ImuiTextLayoutParameters* parameters );
ImuiSize					imuiTextLayoutCacheMesureTextSize( ImuiTextLayoutCache* cache, const ImuiTextLayoutParameters* parameters );
