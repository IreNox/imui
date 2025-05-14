#pragma once

#include <imui/imui.h>

#include "imui_helpers.h"

typedef struct ImUiFont ImUiFont;

typedef struct ImUiTextLayoutCache ImUiTextLayoutCache;
struct ImUiTextLayoutCache
{
	ImUiAllocator*			allocator;

	ImUiHashMap				layoutMap;

	ImUiTextLayout*			firstLayout;
	ImUiTextLayout*			firstUnusedLayout;

	uint32					frameIndex;
};

typedef struct ImUiTextGlyph ImUiTextGlyph;
struct ImUiTextGlyph
{
	uint32					charIndex;
	uint32					codepoint;
	ImUiPos					pos;
	ImUiSize				size;
	ImUiTexCoord			uv;
};

typedef struct ImUiTextLayoutParameters ImUiTextLayoutParameters;
struct ImUiTextLayoutParameters
{
	ImUiFont*				font;
	ImUiStringView			text;
};

typedef struct ImUiTextLayout ImUiTextLayout;
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
