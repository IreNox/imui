#pragma once

#include <imui/imui.h>

#include "imui_helpers.h"

typedef struct ImUiTextLayoutCache
{
	ImUiAllocator*			allocator;

	ImUiHashMap				layoutMap;

	ImUiTextLayout*			firstLayout;
	ImUiTextLayout*			firstUnusedLayout;

	uint32					frameIndex;
} ImUiTextLayoutCache;

typedef struct ImUiTextGlyph
{
	uint32					charIndex;
	uint32					codepoint;
	ImUiPos					pos;
	ImUiSize				size;
	ImUiTexCoord			uv;
} ImUiTextGlyph;

typedef struct ImUiTextLayoutParameters
{
	ImUiFont*				font;
	ImUiStringView			text;
} ImUiTextLayoutParameters;

struct ImUiTextLayout
{
	ImUiFont*				font;
	ImUiStringView			text;

	ImUiTextLayout*			prevLayout;
	ImUiTextLayout*			nextLayout;

	const ImUiTextGlyph*	glyphs;
	uintsize				glyphCount;

	uint32					frameIndex;

	ImUiSize				size;
};

bool						ImUiTextLayoutCacheConstruct( ImUiTextLayoutCache* cache, ImUiAllocator* allocator );
void						ImUiTextLayoutCacheDestruct( ImUiTextLayoutCache* cache );

void						ImUiTextLayoutCacheEndFrame( ImUiTextLayoutCache* cachce );

ImUiTextLayout*				ImUiTextLayoutCacheCreateLayout( ImUiTextLayoutCache* cache, const ImUiTextLayoutParameters* parameters );
ImUiSize					ImUiTextLayoutCacheMesureTextSize( ImUiTextLayoutCache* cache, const ImUiTextLayoutParameters* parameters );
