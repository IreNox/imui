#include "imui_text.h"

#include "imui_font.h"
#include "imui_internal.h"
#include "imui_memory.h"

#include <math.h>
#include <string.h>

static ImuiTextLayout*	imuiTextLayoutCreateNew( ImuiTextLayoutCache* cache, const ImuiTextLayoutParameters* parameters, ImuiTextLayout** mapLayout );

static ImuiHash ImuiTextLayoutCacheHash( const void* entry )
{
	const ImuiTextLayout* layout = *(const ImuiTextLayout**)entry;

	const ImuiHash fontHash = imuiHashCreate( &layout->font, sizeof( &layout->font ) );
	return imuiHashStringSeed( layout->text, fontHash );
}

static bool ImuiTextLayoutCacheIsKeyEquals( const void* lhs, const void* rhs )
{
	const ImuiTextLayout* lhsLayout = *(const ImuiTextLayout**)lhs;
	const ImuiTextLayout* rhsLayout = *(const ImuiTextLayout**)rhs;

	if( lhsLayout->font != rhsLayout->font )
	{
		return false;
	}

	if( lhsLayout->text.length != rhsLayout->text.length )
	{
		return false;
	}
	else if( lhsLayout->text.length == 0u )
	{
		return true;
	}
	else if( lhsLayout->text.data[ 0u ] != rhsLayout->text.data[ 0u ] )
	{
		return false;
	}

	return memcmp( lhsLayout->text.data, rhsLayout->text.data, lhsLayout->text.length ) == 0;
}

bool imuiTextLayoutCacheConstruct( ImuiTextLayoutCache* cache, ImuiAllocator* allocator )
{
	cache->allocator = allocator;

	if( !imuiHashMapConstructSize( &cache->layoutMap, allocator, sizeof( ImuiTextLayout* ), ImuiTextLayoutCacheHash, ImuiTextLayoutCacheIsKeyEquals, 64u ) )
	{
		imuiTextLayoutCacheDestruct( cache );
		return false;
	}

	return true;
}

static void imuiTextLayoutCacheDestruct( ImuiTextLayoutCache* cache )
{
	ImuiTextLayout* layout = cache->firstLayout;
	ImuiTextLayout* nextLayout = NULL;
	while( layout )
	{
		nextLayout = layout->nextLayout;
		imuiMemoryFree( cache->allocator, layout );
		layout = nextLayout;
	}

	layout = cache->firstUnusedLayout;
	while( layout )
	{
		nextLayout = layout->nextLayout;
		imuiMemoryFree( cache->allocator, layout );
		layout = nextLayout;
	}

	imuiHashMapDestruct( &cache->layoutMap );
}

void imuiTextLayoutCacheEndFrame( ImuiTextLayoutCache* cache )
{
	ImuiTextLayout* unusedLayout = cache->firstUnusedLayout;
	ImuiTextLayout* nextUnusedLayout = NULL;
	while( unusedLayout )
	{
		nextUnusedLayout = unusedLayout->nextLayout;

		const bool removed = imuiHashMapRemove( &cache->layoutMap, &unusedLayout );
		(void)removed;
		IMUI_ASSERT( removed );
		imuiMemoryFree( cache->allocator, unusedLayout );
		unusedLayout = nextUnusedLayout;
	}
	cache->firstUnusedLayout	= cache->firstLayout;
	cache->firstLayout			= NULL;

	cache->frameIndex++;
}

ImuiTextLayout* imuiTextLayoutCreate( ImuiContext* imui, ImuiFont* font, const char* text )
{
	return imuiTextLayoutCreateLength( imui, font, text, strlen( text ) );
}

ImuiTextLayout* imuiTextLayoutCreateLength( ImuiContext* imui, ImuiFont* font, const char* text, size_t length )
{
	if( !font )
	{
		return NULL;
	}

	ImuiTextLayoutParameters parameters;
	parameters.font			= font;
	parameters.text.data	= text;
	parameters.text.length	= length;

	return imuiTextLayoutCacheCreateLayout( &imui->layoutCache, &parameters );
}

ImuiTextLayout* imuiTextLayoutCreateWidget( ImuiWidget* widget, ImuiFont* font, const char* text )
{
	return imuiTextLayoutCreateWidgetLength( widget, font, text, strlen( text ) );
}

ImuiTextLayout* imuiTextLayoutCreateWidgetLength( ImuiWidget* widget, ImuiFont* font, const char* text, size_t length )
{
	return imuiTextLayoutCreateLength( widget->window->context, font, text, length );
}

ImuiTextLayout* imuiTextLayoutCacheCreateLayout( ImuiTextLayoutCache* cache, const ImuiTextLayoutParameters* parameters )
{
	if( parameters->text.length == 0u )
	{
		return NULL;
	}

	bool isNew = false;
	ImuiTextLayout** mapLayout = (ImuiTextLayout**)imuiHashMapInsertNew( &cache->layoutMap, &parameters, &isNew );
	if( !mapLayout )
	{
		return NULL;
	}

	if( !isNew )
	{
		ImuiTextLayout* layout = *mapLayout;
		if( layout->frameIndex != cache->frameIndex )
		{
			if( layout->prevLayout )
			{
				layout->prevLayout->nextLayout = layout->nextLayout;
			}
			else
			{
				IMUI_ASSERT( cache->firstUnusedLayout == layout );
				cache->firstUnusedLayout = layout->nextLayout;
			}

			if( layout->nextLayout )
			{
				layout->nextLayout->prevLayout = layout->prevLayout;
			}

			layout->nextLayout = cache->firstLayout;
			layout->prevLayout = NULL;

			if( cache->firstLayout )
			{
				cache->firstLayout->prevLayout = layout;
			}
			cache->firstLayout = layout;

			layout->frameIndex = cache->frameIndex;
		}

		return *mapLayout;
	}

	return imuiTextLayoutCreateNew( cache, parameters, mapLayout );
}

size_t imuiTextLayoutCalculateGlyphCount( const char* text, size_t length )
{
	size_t glyphCount = 0u;
	for( uintsize i = 0; i < length; )
	{
		const char c = text[ i ];
		if( c == '\n' )
		{
			i++;
			continue;
		}

		uint32 codepointByteCount = IMUI_COUNT_LEADING_ZEROS32( ~((uint32)c << 24u) );
		if( codepointByteCount > 4 )
		{
			// invalid character
			codepointByteCount = 1;
		}

		i += IMUI_MAX( codepointByteCount, 1 );
		glyphCount++;
	}

	return glyphCount;
}

ImuiSize imuiTextLayoutCalculateSize( ImuiContext* imui, ImuiFont* font, const char* text, size_t length )
{
	ImuiTextLayout* pLayout = imuiTextLayoutCreateLength( imui, font, text, length );
	if( !pLayout )
	{
		return imuiSizeCreateZero();
	}

	return pLayout->size;
}

static ImuiTextLayout* imuiTextLayoutCreateNew( ImuiTextLayoutCache* cache, const ImuiTextLayoutParameters* parameters, ImuiTextLayout** mapLayout )
{
	uintsize glyphCount = imuiTextLayoutCalculateGlyphCount( parameters->text.data, parameters->text.length );
	const uintsize memorySize = sizeof( ImuiTextLayout ) + parameters->text.length + 1u + (sizeof( ImuiTextGlyph ) * glyphCount);
	ImuiTextLayout* layout = (ImuiTextLayout*)imuiMemoryAlloc( cache->allocator, memorySize );
	if( !layout )
	{
		imuiHashMapRemove( &cache->layoutMap, mapLayout );
		return NULL;
	}

	ImuiTextGlyph* glyphs = (ImuiTextGlyph*)&layout[ 1u ];
	char* textData = (char*)&glyphs[ glyphCount ];
	memcpy( textData, parameters->text.data, parameters->text.length + 1u );

	uintsize lineCount = 1u;
	uintsize glyphIndex = 0u;
	float x = 0.0f;
	float y = 0.0f;
	for( uintsize i = 0; i < parameters->text.length; ++i )
	{
		ImuiTextGlyph* glyph = &glyphs[ glyphIndex ];

		glyph->charIndex = (uint32)i;

		const char c = parameters->text.data[ i ];
		if( c == '\n' )
		{
			x = 0.0f;
			y += parameters->font->fontSize; // parameters->font->lineGap * parameters->font->fontSize
			y = ceilf( y );
			lineCount++;
			continue;
		}

		const uint32 codepointByteCount = IMUI_COUNT_LEADING_ZEROS32( ~((uint32)c << 24u) );

		uint32 codepoint;
		if( codepointByteCount == 1 || codepointByteCount > 4 )
		{
			// invalid character
			codepoint = 0xfffd; // invalid codepoint
		}
		else
		{
			const uint32 codepointMask = (1u << (8 - codepointByteCount)) - 1u;
			codepoint = c & codepointMask;
			for( int remainingBytes = (int)codepointByteCount - 1; remainingBytes > 0; --remainingBytes )
			{
				i++;
				if( i >= parameters->text.length )
				{
					break;
				}
				const char c2 = parameters->text.data[ i ];

				codepoint <<= 6u;
				codepoint += (c2 & 0x3fu);
			}
		}

		uint32* mapCodepointKey = &codepoint;
		ImuiFontCodepoint** mapCodepoint = (ImuiFontCodepoint**)imuiHashMapFind( &parameters->font->codepointMap, &mapCodepointKey );
		if( !mapCodepoint )
		{
			codepoint = 0xfffd; // invalid codepoint
			mapCodepoint = (ImuiFontCodepoint**)imuiHashMapFind( &parameters->font->codepointMap, &mapCodepointKey );
			if( !mapCodepoint )
			{

			}
		}

		ImuiFontCodepoint* codepointInfo;
		if( mapCodepoint )
		{
			codepointInfo = *mapCodepoint;
		}
		else
		{
			// TODO: what to do here?
			x += parameters->font->lineGap;
			glyphCount--;
			continue;
		}

		glyph->codepoint	= codepoint;
		glyph->pos			= imuiPosCreate( x + codepointInfo->xOffset, y + codepointInfo->ascentOffset );
		glyph->size			= imuiSizeCreate( (float)codepointInfo->width, (float)codepointInfo->height );
		glyph->uv			= codepointInfo->uv;

		glyphIndex++;
		x += codepointInfo->advance;
		//height = IMUI_MAX( height, glyph->pos.y + glyph->size.height );
	}

	layout->font		= parameters->font;
	layout->text.data	= textData;
	layout->text.length	= parameters->text.length;
	layout->glyphs		= glyphs;
	layout->glyphCount	= glyphCount;
	layout->size		= imuiSizeCreate( ceilf( x ), lineCount * parameters->font->fontSize );
	layout->frameIndex	= cache->frameIndex;

	layout->prevLayout	= NULL;
	layout->nextLayout	= cache->firstLayout;

	if( cache->firstLayout )
	{
		cache->firstLayout->prevLayout = layout;
	}
	cache->firstLayout = layout;

	*mapLayout = layout;
	return layout;
}

ImuiSize imuiTextLayoutCacheMesureTextSize( ImuiTextLayoutCache* cache, const ImuiTextLayoutParameters* parameters )
{
	ImuiTextLayout* layout = imuiTextLayoutCacheCreateLayout( cache, parameters );
	if( !layout )
	{
		return imuiSizeCreateZero();
	}

	return imuiTextLayoutGetSize( layout );
}

size_t imuiTextLayoutGetGlyphCount( const ImuiTextLayout* layout )
{
	if( !layout )
	{
		return 0u;
	}

	return layout->glyphCount;
}

size_t imuiTextLayoutFindGlyphIndex( const ImuiTextLayout* layout, ImuiPos pos, float scale )
{
	if( !layout )
	{
		return 0u;
	}

	for( uintsize i = 0u; i < layout->glyphCount; ++i )
	{
		const ImuiTextGlyph* glyph = &layout->glyphs[ i ];
		const float glyphCenterX = (glyph->pos.x * scale) + (glyph->size.width * 0.5f * scale);
		if( glyphCenterX > pos.x )
		{
			return i;
		}
	}

	return layout->glyphCount;
}

size_t imuiTextLayoutGetGlyphCharIndex( const ImuiTextLayout* layout, size_t glyphIndex )
{
	if( !layout )
	{
		return 0u;
	}

	if( glyphIndex < layout->glyphCount )
	{
		const ImuiTextGlyph* glyph = &layout->glyphs[ glyphIndex ];
		return glyph->charIndex;
	}

	return layout->text.length;
}

ImuiSize imuiTextLayoutGetSize( const ImuiTextLayout* layout )
{
	if( !layout )
	{
		return imuiSizeCreateZero();
	}

	return layout->size;
}

ImuiPos imuiTextLayoutGetGlyphPos( const ImuiTextLayout* layout, size_t glyphIndex, float scale )
{
	if( !layout )
	{
		return imuiPosCreateZero();
	}
	else if( glyphIndex >= layout->glyphCount  )
	{
		return imuiPosCreate( layout->size.width * scale, 0.0f );
	}

	return imuiPosScale( layout->glyphs[ glyphIndex ].pos, scale );
}
