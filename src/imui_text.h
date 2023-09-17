#pragma once

#include <imui/imui.h>

#include "imui_helpers.h"

typedef struct ImUiFont ImUiFont;

typedef struct ImUiTextLayoutCache ImUiTextLayoutCache;
struct ImUiTextLayoutCache
{
	ImUiAllocator*		allocator;

	ImUiChunkedPool		layoutPool;
	ImUiHashMap			layoutMap;
};

typedef struct ImUiTextGlyph ImUiTextGlyph;
struct ImUiTextGlyph
{
	uint32					codepoint;
	ImUiPos			pos;
	ImUiSize				size;
	ImUiTexCoord	uv;
};

typedef struct ImUiTextLayoutParameters ImUiTextLayoutParameters;
struct ImUiTextLayoutParameters
{
	ImUiFont*			font;
	ImUiStringView		text;
};

typedef struct ImUiTextLayout ImUiTextLayout;
struct ImUiTextLayout
{
	ImUiFont*				font;
	ImUiStringView			text;

	const ImUiTextGlyph*	glyphs;
	uintsize				glyphCount;

	ImUiSize				size;
};

bool					ImUiTextLayoutCacheConstruct( ImUiTextLayoutCache* cache, ImUiAllocator* allocator );
void					ImUiTextLayoutCacheDestruct( ImUiTextLayoutCache* cache );

void					ImUiTextLayoutCacheFreeUnused( ImUiTextLayoutCache* cachce );

ImUiTextLayout*			ImUiTextLayoutCacheCreateLayout( ImUiTextLayoutCache* cache, const ImUiTextLayoutParameters* parameters );
ImUiSize				ImUiTextLayoutCacheMesureTextSize( ImUiTextLayoutCache* cache, const ImUiTextLayoutParameters* parameters );
