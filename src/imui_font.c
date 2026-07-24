#include "imui_font.h"

#include "imui_internal.h"
#include "imui_memory.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include <string.h>

struct ImuiFontTrueTypeData
{
	ImuiAllocator*		allocator;

	const void*			data;
	uintsize			dataSize;
	bool				ownsData;

	stbtt_fontinfo		font;

	uint32*				codepoints;
	uintsize			codepointCount;
	uintsize			codepointCapacity;
};

struct ImuiFontTrueTypeImage
{
	ImuiAllocator*		allocator;
	ImuiFontParameters	parameters;
};

typedef struct ImuiFontCodepointMapEntry
{
	const ImuiFontCodepoint*	codepoints;
} ImuiFontCodepointMapEntry;

static ImuiHash imuiFontCodepointHash( const void* entry )
{
	const ImuiFontCodepoint* codepoint = *(const ImuiFontCodepoint**)entry;
	return codepoint->codepoint;
}

static bool imuiFontCodepointIsKeyEquals( const void* lhs, const void* rhs )
{
	const ImuiFontCodepoint* lhsCp = *(const ImuiFontCodepoint**)lhs;
	const ImuiFontCodepoint* rhsCp = *(const ImuiFontCodepoint**)rhs;
	return lhsCp->codepoint == rhsCp->codepoint;
}

ImuiFont* imuiFontCreate( ImuiContext* imui, const ImuiFontParameters* parameters )
{
	ImuiFont* font = IMUI_MEMORY_NEW_ZERO( &imui->allocator, ImuiFont );
	if( !font )
	{
		return NULL;
	}

	font->image				= parameters->image;
	font->codepoints		= IMUI_MEMORY_ARRAY_NEW( &imui->allocator, ImuiFontCodepoint, parameters->codepointCount );
	font->codepointCount	= parameters->codepointCount;
	font->fontSize			= parameters->fontSize;
	font->lineGap			= parameters->lineGap;

	if( !font->codepoints )
	{
		imuiFontDestroy( imui, font );
		return NULL;
	}

	memcpy( font->codepoints, parameters->codepoints, sizeof( *parameters->codepoints ) * parameters->codepointCount );

	if( !imuiHashMapConstructStaticPointer( &font->codepointMap, &imui->allocator, font->codepoints, sizeof( *font->codepoints ), parameters->codepointCount, imuiFontCodepointHash, imuiFontCodepointIsKeyEquals ) )
	{
		imuiFontDestroy( imui, font );
		return NULL;
	}

	return font;
}

ImuiFont* imuiFontCreateTrueType( ImuiContext* imui, ImuiFontTrueTypeImage* ttfImage, ImuiImage image )
{
	ttfImage->parameters.image = image;

	ImuiFont* font = imuiFontCreate( imui, &ttfImage->parameters );

	imuiFontTrueTypeImageDestroy( ttfImage );

	return font;
}

void imuiFontDestroy( ImuiContext* imui, ImuiFont* font )
{
	imuiHashMapDestruct( &font->codepointMap );
	imuiMemoryFree( &imui->allocator, font->codepoints );
	imuiMemoryFree( &imui->allocator, font );
}

ImuiFontTrueTypeData* imuiFontTrueTypeDataCreate( ImuiContext* imui, const void* data, size_t dataSize )
{
	ImuiFontTrueTypeData* ttf = IMUI_MEMORY_NEW_ZERO( &imui->allocator, ImuiFontTrueTypeData );
	if( !ttf )
	{
		return NULL;
	}

	ttf->allocator	= &imui->allocator;
	ttf->data		= data;
	ttf->dataSize	= dataSize;

	if( !stbtt_InitFont( &ttf->font, (const uint8*)ttf->data, 0 ) )
	{
		imuiFontTrueTypeDataDestroy( ttf );
		return NULL;
	}

	return ttf;
}

ImuiFontTrueTypeData* imuiFontTrueTypeDataCreateCopy( ImuiContext* imui, const void* data, size_t dataSize )
{
	void* dataCopy = imuiMemoryAlloc( &imui->allocator, dataSize );
	if( !dataCopy )
	{
		return NULL;
	}

	memcpy( dataCopy, data, dataSize );

	ImuiFontTrueTypeData* ttf = imuiFontTrueTypeDataCreate( imui, dataCopy, dataSize );
	if( !ttf )
	{
		imuiMemoryFree( &imui->allocator, dataCopy );
		return NULL;
	}

	ttf->ownsData = true;

	return ttf;
}

void imuiFontTrueTypeDataDestroy( ImuiFontTrueTypeData* ttf )
{
	if( ttf->ownsData )
	{
		imuiMemoryFree( ttf->allocator, ttf->data );
	}

	imuiMemoryFree( ttf->allocator, ttf->codepoints );
	imuiMemoryFree( ttf->allocator, ttf );
}

bool imuiFontTrueTypeDataAddCodepoints( ImuiFontTrueTypeData* ttf, const uint32_t* codepoints, size_t codepointCount )
{
	if( !IMUI_MEMORY_ARRAY_CHECK_CAPACITY( ttf->allocator, ttf->codepoints, ttf->codepointCapacity, ttf->codepointCount + codepointCount ) )
	{
		return false;
	}

	memcpy( ttf->codepoints + ttf->codepointCount, codepoints, sizeof( *codepoints ) * codepointCount );
	ttf->codepointCount += codepointCount;

	return true;
}

bool imuiFontTrueTypeDataAddCodepointRange( ImuiFontTrueTypeData* ttf, uint32_t firstCodepoint, uint32_t lastCodepoint )
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

void imuiFontTrueTypeDataCalculateMinTextureSizeInternal( ImuiFontTrueTypeData* ttf, float fontSizeInPixel, uint32_t* targetWidth, uint32_t* targetHeight, int padding )
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

		const uint32 cpWidth = (x1 - x0) + padding * 2;
		const uint32 cpHeight = (y1 - y0) + padding * 2;

		x += cpWidth + 2u;
		lineHeight = IMUI_MAX( lineHeight, cpHeight + 2u );
	}

	const float minSizeBase			= sqrtf( (float)x * lineHeight );
	const float minSizeLines		= ceilf( minSizeBase / lineHeight ) + 1u;
	const uint32 minSize			= (uint32)minSizeLines * lineHeight;

	*targetWidth = minSize;
	*targetHeight = minSize;
}

void imuiFontTrueTypeDataCalculateMinTextureSize( ImuiFontTrueTypeData* ttf, float fontSizeInPixel, uint32_t* targetWidth, uint32_t* targetHeight )
{
	imuiFontTrueTypeDataCalculateMinTextureSizeInternal( ttf, fontSizeInPixel, targetWidth, targetHeight, 0 );
}

void imuiFontTrueTypeDataCalculateMinSDFTextureSize(ImuiFontTrueTypeData* ttf, float fontSizeInPixel, uint32_t* targetWidth, uint32_t* targetHeight, float sdfSpread)
{
	const int padding = IMUI_MIN( 16, IMUI_MAX( 4, (int)ceilf( fontSizeInPixel * sdfSpread ) ) );
	imuiFontTrueTypeDataCalculateMinTextureSizeInternal( ttf, fontSizeInPixel, targetWidth, targetHeight, padding );
}

bool imuiFontTrueTypeDataAddCodepointBitmapToTexture( ImuiFontTrueTypeData* ttf, ImuiFontCodepoint* targetCodepoint, uint8* data, uint32* x, uint32* y, float ascent, uint32 lineHeight, float scaleX, float scaleY, int codepoint, uint32_t width, uint32_t height )
{
	bool reachedAtlasXLimit = false;
	int advance;
	int lsb;
	stbtt_GetCodepointHMetrics( &ttf->font, codepoint, &advance, &lsb );

	int x0;
	int y0;
	int x1;
	int y1;
	stbtt_GetCodepointBitmapBox( &ttf->font, codepoint, scaleX, scaleY, &x0, &y0, &x1, &y1 );

	const uint32 cpWidth = x1 - x0;
	const uint32 cpHeight = y1 - y0;
	if( *x + cpWidth + 2u >= width )
	{
		*x = 1u;
		*y += lineHeight + 2u;
		reachedAtlasXLimit = true;
	}

	uint8* cpData = &data[ *x + (*y * width) ];
	stbtt_MakeCodepointBitmap( &ttf->font, cpData, cpWidth, cpHeight, width, scaleX, scaleY, codepoint );

	targetCodepoint->codepoint = codepoint;
	targetCodepoint->width = (float)cpWidth;
	targetCodepoint->height = (float)cpHeight;
	targetCodepoint->advance = scaleX * advance;
	targetCodepoint->ascentOffset = ascent + y0;
	targetCodepoint->xOffset = 0;
	targetCodepoint->uv.u0 = (float)*x / width;
	targetCodepoint->uv.v0 = (float)*y / height;
	targetCodepoint->uv.u1 = (float)(*x + cpWidth) / width;
	targetCodepoint->uv.v1 = (float)(*y + cpHeight) / height;

	*x += cpWidth + 2u;

	return reachedAtlasXLimit;
}

bool imuiFontTrueTypeDataAddCodepointSDFToTexture(ImuiFontTrueTypeData* ttf, ImuiFontCodepoint* targetCodepoint, uint8* data, uint32* x, uint32* y, float ascent, uint32 lineHeight, float scale, int padding, int codepoint, uint32_t width, uint32_t height)
{
	bool reachedAtlasXLimit = false;
	int advance;
	int lsb;
	stbtt_GetCodepointHMetrics( &ttf->font, codepoint, &advance, &lsb );
	
	const unsigned char onedgeValue = 128;
	const float pixelDistScale = 64.0f / (float)padding;

	int cpWidth = 0;
	int cpHeight = 0;
	int xoff = 0;
	int yoff = 0;

	uint8* sdf = stbtt_GetCodepointSDF( &ttf->font, scale, codepoint, padding, onedgeValue, pixelDistScale, &cpWidth, &cpHeight, &xoff, &yoff );

	if (sdf == NULL)
	{
		// no SDF available, could be space bar codepoint 32?
		int x0;
		int y0;
		int x1;
		int y1;
		stbtt_GetCodepointBitmapBox( &ttf->font, codepoint, scale, scale, &x0, &y0, &x1, &y1 );

		cpWidth = x1 - x0;
		cpHeight = y1 - y0;
	}

	if (*x + cpWidth + 2u >= width)
	{
		*x = 1u;
		*y += lineHeight + 2u;
		reachedAtlasXLimit = true;
	}

	IMUI_ASSERT( *y + cpHeight < height );

	targetCodepoint->codepoint = codepoint;
	targetCodepoint->width = (float)cpWidth;
	targetCodepoint->height = (float)cpHeight;
	targetCodepoint->advance = scale * advance;
	targetCodepoint->xOffset = (float)xoff;
	targetCodepoint->ascentOffset = ascent + yoff;
	targetCodepoint->uv.u0 = (float)*x / width;
	targetCodepoint->uv.v0 = (float)*y / height;
	targetCodepoint->uv.u1 = (float)(*x + cpWidth) / width;
	targetCodepoint->uv.v1 = (float)(*y + cpHeight) / height;

	uint8* cpData = &data[*x + (*y * width)];

	for( uint32 row = 0; row < (uint32)cpHeight; ++row )
	{
		memcpy( cpData + row * width, sdf + row * cpWidth, cpWidth );
	}

	stbtt_FreeSDF( sdf, NULL );

	*x += (uint32)cpWidth + 2u;

	return reachedAtlasXLimit;
}

ImuiFontTrueTypeImage* imuiFontTrueTypeDataGenerateTextureDataInternal(ImuiFontTrueTypeData* ttf, float fontSizeInPixel, void* targetData, size_t targetDataSize, uint32_t width, uint32_t height, float sdfSpread)
{
	if( targetDataSize < width * height )
	{
		// too small
		return NULL;
	}

	ImuiFontTrueTypeImage* image = IMUI_MEMORY_NEW_ZERO( ttf->allocator, ImuiFontTrueTypeImage );
	ImuiFontCodepoint* codepoints = IMUI_MEMORY_ARRAY_NEW( ttf->allocator, ImuiFontCodepoint, ttf->codepointCount );
	if( !image || !codepoints )
	{
		imuiMemoryFree( ttf->allocator, image );
		imuiMemoryFree( ttf->allocator, codepoints );
		return NULL;
	}

	image->allocator					= ttf->allocator;
	image->parameters.codepoints		= codepoints;
	image->parameters.codepointCount	= ttf->codepointCount;

	memset( targetData, 0, width * height );

	const float scale = stbtt_ScaleForPixelHeight( &ttf->font, fontSizeInPixel );

	float ascent;
	//float descent;
	float lineGap;
	{
		int ascentI;
		int descentI;
		int lineGapI;
		stbtt_GetFontVMetrics( &ttf->font, &ascentI, &descentI, &lineGapI );
		ascent = scale * ascentI;
		//descent = scale * descentI;
		lineGap = scale * lineGapI;
	}

	uint32 x = 1u;
	uint32 y = 1u;
	uint32 lineHeight = 0u;
	uint8* data = (uint8*)targetData;

	if( sdfSpread > 0.0f )
	{
		const int padding = IMUI_MIN(16, IMUI_MAX(4, (int)ceilf( fontSizeInPixel * sdfSpread ) ) );
		for( uintsize i = 0; i < ttf->codepointCount; ++i )
		{
			if( imuiFontTrueTypeDataAddCodepointSDFToTexture( ttf, &codepoints[ i ], data, &x, &y, ascent, lineHeight, scale, padding, (int)ttf->codepoints[ i ], width, height ) )
			{
				lineHeight = 0u;
			}
			lineHeight = IMUI_MAX( lineHeight, (uint32_t)codepoints[ i ].height );
		}
	}
	else
	{
		for( uintsize i = 0; i < ttf->codepointCount; ++i )
		{
			if( imuiFontTrueTypeDataAddCodepointBitmapToTexture(ttf, &codepoints[ i ], data, &x, &y, ascent, lineHeight, scale, scale, (int)ttf->codepoints[ i ], width, height ) )
			{
				lineHeight = 0u;
			}
			lineHeight = IMUI_MAX( lineHeight, (uint32_t)codepoints[ i ].height );
		}
	}

	image->parameters.fontSize    = fontSizeInPixel;
    image->parameters.lineGap    = lineGap;

	return image;
}

ImuiFontTrueTypeImage* imuiFontTrueTypeDataGenerateTextureData( ImuiFontTrueTypeData* ttf, float fontSizeInPixel, void* targetData, size_t targetDataSize, uint32_t width, uint32_t height )
{
	return imuiFontTrueTypeDataGenerateTextureDataInternal( ttf, fontSizeInPixel, targetData, targetDataSize, width, height, /*sdfSpread =*/ 0.0f );
}

ImuiFontTrueTypeImage* imuiFontTrueTypeDataGenerateSDFTextureData( ImuiFontTrueTypeData* ttf, float fontSizeInPixel, void* targetData, size_t targetDataSize, uint32_t width, uint32_t height, float sdfSpread )
{
	// sdfSpread is a fraction of the font size, values around [0.1f, 0.3f] work
	// If using SDFs you need a larger target image than for pure bitmap font, so use imuiFontTrueTypeDataCalculateMinSDFTextureSize to compute that.
	return imuiFontTrueTypeDataGenerateTextureDataInternal( ttf, fontSizeInPixel, targetData, targetDataSize, width, height, sdfSpread );
}

void imuiFontTrueTypeImageGetCodepoints( ImuiFontTrueTypeImage* ttfImage, const ImuiFontCodepoint** codepoints, size_t* codepointCount )
{
	*codepoints		= ttfImage->parameters.codepoints;
	*codepointCount	= ttfImage->parameters.codepointCount;
}

void imuiFontTrueTypeImageDestroy( ImuiFontTrueTypeImage* ttfImage )
{
	imuiMemoryFree( ttfImage->allocator, ttfImage->parameters.codepoints );
	imuiMemoryFree( ttfImage->allocator, ttfImage );
}
