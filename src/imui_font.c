#include "imui_font.h"

#include "imui_memory.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include <string.h>

struct ImUiFontTrueTypeData
{
	ImUiAllocator*		allocator;

	const void*			data;
	uintsize			dataSize;
	bool				ownsData;

	stbtt_fontinfo		font;

	uint32*				codepoints;
	uintsize			codepointCount;
	uintsize			codepointCapacity;
};

struct ImUiFontTrueTypeImage
{
	ImUiAllocator*		allocator;
	ImUiFontParameters	parameters;
};

typedef struct ImUiFontCodepointMapEntry ImUiFontCodepointMapEntry;
struct ImUiFontCodepointMapEntry
{
	const ImUiFontCodepoint*	codepoints;
};

static ImUiHash ImUiFontCodepointHash( const void* entry )
{
	const ImUiFontCodepoint* codepoint = *(const ImUiFontCodepoint**)entry;
	return codepoint->codepoint;
}

static bool ImUiFontCodepointIsKeyEquals( const void* lhs, const void* rhs )
{
	const ImUiFontCodepoint* lhsCp = *(const ImUiFontCodepoint**)lhs;
	const ImUiFontCodepoint* rhsCp = *(const ImUiFontCodepoint**)rhs;
	return lhsCp->codepoint == rhsCp->codepoint;
}

ImUiFont* ImUiFontCreate( ImUiContext* imui, const ImUiFontParameters* parameters )
{
	ImUiFont* font = IMUI_MEMORY_NEW_ZERO( &imui->allocator, ImUiFont );
	if( !font )
	{
		return NULL;
	}

	font->texture			= parameters->texture;
	font->codepoints		= IMUI_MEMORY_ARRAY_NEW( &imui->allocator, ImUiFontCodepoint, parameters->codepointCount );
	font->codepointCount	= parameters->codepointCount;
	font->fontSize			= parameters->fontSize;
	font->lineGap			= parameters->lineGap;

	if( !font->codepoints )
	{
		ImUiFontDestroy( imui, font );
		return NULL;
	}

	memcpy( font->codepoints, parameters->codepoints, sizeof( *parameters->codepoints ) * parameters->codepointCount );

	if( !ImUiHashMapConstructStaticPointer( &font->codepointMap, &imui->allocator, font->codepoints, sizeof( *font->codepoints ), parameters->codepointCount, ImUiFontCodepointHash, ImUiFontCodepointIsKeyEquals ) )
	{
		ImUiFontDestroy( imui, font );
		return NULL;
	}

	return font;
}

ImUiFont* ImUiFontCreateTrueType( ImUiContext* imui, ImUiFontTrueTypeImage* ttfImage, ImUiTexture texture )
{
	ttfImage->parameters.texture = texture;

	ImUiFont* font = ImUiFontCreate( imui, &ttfImage->parameters );

	ImUiFontTrueTypeImageDestroy( ttfImage );

	return font;
}

void ImUiFontDestroy( ImUiContext* imui, ImUiFont* font )
{
	ImUiHashMapDestruct( &font->codepointMap );
	ImUiMemoryFree( &imui->allocator, font->codepoints );
	ImUiMemoryFree( &imui->allocator, font );
}

ImUiFontTrueTypeData* ImUiFontTrueTypeDataCreate( ImUiContext* imui, const void* data, size_t dataSize )
{
	ImUiFontTrueTypeData* ttf = IMUI_MEMORY_NEW_ZERO( &imui->allocator, ImUiFontTrueTypeData );
	if( !ttf )
	{
		return NULL;
	}

	ttf->allocator	= &imui->allocator;
	ttf->data		= data;
	ttf->dataSize	= dataSize;

	if( !stbtt_InitFont( &ttf->font, (const uint8*)ttf->data, 0 ) )
	{
		ImUiFontTrueTypeDataDestroy( ttf );
		return NULL;
	}

	return ttf;
}

ImUiFontTrueTypeData* ImUiFontTrueTypeDataCreateCopy( ImUiContext* imui, const void* data, size_t dataSize )
{
	void* dataCopy = ImUiMemoryAlloc( &imui->allocator, dataSize );
	if( !dataCopy )
	{
		return NULL;
	}

	memcpy( dataCopy, data, dataSize );

	ImUiFontTrueTypeData* ttf = ImUiFontTrueTypeDataCreate( imui, dataCopy, dataSize );
	if( !ttf )
	{
		ImUiMemoryFree( &imui->allocator, dataCopy );
		return NULL;
	}

	ttf->ownsData = true;

	return ttf;
}

void ImUiFontTrueTypeDataDestroy( ImUiFontTrueTypeData* ttf )
{
	if( ttf->ownsData )
	{
		ImUiMemoryFree( ttf->allocator, ttf->data );
	}

	ImUiMemoryFree( ttf->allocator, ttf->codepoints );
	ImUiMemoryFree( ttf->allocator, ttf );
}

bool ImUiFontTrueTypeDataAddCodepoints( ImUiFontTrueTypeData* ttf, const uint32_t* codepoints, size_t codepointCount )
{
	if( !IMUI_MEMORY_ARRAY_CHECK_CAPACITY( ttf->allocator, ttf->codepoints, ttf->codepointCapacity, ttf->codepointCount + codepointCount ) )
	{
		return false;
	}

	memcpy( ttf->codepoints + ttf->codepointCount, codepoints, sizeof( *codepoints ) * codepointCount );
	ttf->codepointCount += codepointCount;

	return true;
}

bool ImUiFontTrueTypeDataAddCodepointRange( ImUiFontTrueTypeData* ttf, uint32_t firstCodepoint, uint32_t lastCodepoint )
{
	const uintsize codepointCount = (lastCodepoint - firstCodepoint) + 1u;
	if( !IMUI_MEMORY_ARRAY_CHECK_CAPACITY( ttf->allocator, ttf->codepoints, ttf->codepointCapacity, ttf->codepointCount + codepointCount ) )
	{
		return false;
	}

	for( uint32 i = 0; i < codepointCount; ++i )
	{
		ttf->codepoints[ ttf->codepointCount++ ] = firstCodepoint + i;
	}

	return true;
}

void ImUiFontTrueTypeDataCalculateMinTextureSize( ImUiFontTrueTypeData* ttf, float fontSizeInPixel, uint32_t* targetWidth, uint32_t* targetHeight )
{
	const float scale = stbtt_ScaleForPixelHeight( &ttf->font, fontSizeInPixel );

	int ascent;
	stbtt_GetFontVMetrics( &ttf->font, &ascent, NULL, NULL );

	uint32 x = 0;
	uint32 lineHeight = 0;
	for( uintsize i = 0; i < ttf->codepointCount; ++i )
	{
		const int codepoint = (int)ttf->codepoints[ i ];

		//const float x_shift = xpos - (float)floor( xpos );

		int advance;
		int lsb;
		stbtt_GetCodepointHMetrics( &ttf->font, codepoint, &advance, &lsb );

		int x0;
		int y0;
		int x1;
		int y1;
		stbtt_GetCodepointBitmapBox( &ttf->font, codepoint, scale, scale, &x0, &y0, &x1, &y1 );

		const uint32 cpWidth = x1 - x0;
		const uint32 cpHeight = y1 - y0;

		x += cpWidth + 2u;
		lineHeight = IMUI_MAX( lineHeight, cpHeight + 2u );
	}

	const float minSizeBase			= sqrtf( (float)x * lineHeight );
	const float minSizeLines		= ceilf( minSizeBase / lineHeight ) + 1u;
	const uint32 minSize			= (uint32)minSizeLines * lineHeight;

	*targetWidth = minSize;
	*targetHeight = minSize;
}

ImUiFontTrueTypeImage* ImUiFontTrueTypeDataGenerateTextureData( ImUiFontTrueTypeData* ttf, float fontSizeInPixel, void* targetData, size_t targetDataSize, uint32_t width, uint32_t height )
{
	ImUiFontTrueTypeImage* image = IMUI_MEMORY_NEW_ZERO( ttf->allocator, ImUiFontTrueTypeImage );
	ImUiFontCodepoint* codepoints = IMUI_MEMORY_ARRAY_NEW( ttf->allocator, ImUiFontCodepoint, ttf->codepointCount );
	if( !image || !codepoints )
	{
		ImUiMemoryFree( ttf->allocator, image );
		ImUiMemoryFree( ttf->allocator, codepoints );
		return NULL;
	}

	image->allocator					= ttf->allocator;
	image->parameters.codepoints		= codepoints;
	image->parameters.codepointCount	= ttf->codepointCount;

	memset( targetData, 0, width * height );

	const float scale = stbtt_ScaleForPixelHeight( &ttf->font, fontSizeInPixel );

	float ascent;
	float descent;
	float lineGap;
	{
		int ascentI;
		int descentI;
		int lineGapI;
		stbtt_GetFontVMetrics( &ttf->font, &ascentI, &descentI, &lineGapI );
		ascent = scale * ascentI;
		descent = scale * descentI;
		lineGap = scale * lineGapI;
	}

	uint32 x = 1u;
	uint32 y = 1u;
	uint32 lineHeight = 0u;
	uint8* data = (uint8*)targetData;
	for( uintsize i = 0; i < ttf->codepointCount; ++i )
	{
		const int codepoint = (int)ttf->codepoints[ i ];
		ImUiFontCodepoint* targetCodepoint = &codepoints[ i ];

		int advance;
		int lsb;
		stbtt_GetCodepointHMetrics( &ttf->font, codepoint, &advance, &lsb );

		int x0;
		int y0;
		int x1;
		int y1;
		stbtt_GetCodepointBitmapBox( &ttf->font, codepoint, scale, scale, &x0, &y0, &x1, &y1 );

		const uint32 cpWidth = x1 - x0;
		const uint32 cpHeight = y1 - y0;
		if( x + cpWidth + 2u >= width )
		{
			x = 1u;
			y += lineHeight + 2u;
			lineHeight = 0u;
		}

		uint8* cpData = &data[ x + (y * width) ];
		stbtt_MakeCodepointBitmap( &ttf->font, cpData, cpWidth, cpHeight, width, scale, scale, codepoint );

		targetCodepoint->codepoint		= codepoint;
		targetCodepoint->width			= (float)cpWidth;
		targetCodepoint->height			= (float)cpHeight;
		targetCodepoint->advance		= scale * advance;
		targetCodepoint->ascentOffset	= ascent + y0;
		targetCodepoint->uv.u0			= (float)x / width;
		targetCodepoint->uv.v0			= (float)y / height;
		targetCodepoint->uv.u1			= (float)(x + cpWidth) / width;
		targetCodepoint->uv.v1			= (float)(y + cpHeight) / height;

		x += cpWidth + 2u;
		lineHeight = IMUI_MAX( lineHeight, cpHeight );
	}

	image->parameters.fontSize	= fontSizeInPixel;
	image->parameters.lineGap	= lineGap;

	return image;
}

void ImUiFontTrueTypeImageDestroy( ImUiFontTrueTypeImage* ttfImage )
{
	ImUiMemoryFree( ttfImage->allocator, ttfImage->parameters.codepoints );
	ImUiMemoryFree( ttfImage->allocator, ttfImage );
}
