#include "imui_text.h"

#include "imui_font.h"
#include "imui_internal.h"
#include "imui_memory.h"

#include <math.h>
#include <string.h>

static ImUiTextLayout* ImUiTextLayoutCreateNew( ImUiTextLayoutCache* cache, const ImUiTextLayoutParameters* parameters, ImUiTextLayout** mapLayout );

static ImUiHash ImUiTextLayoutCacheHash( const void* entry )
{
	const ImUiTextLayout* layout = *(const ImUiTextLayout**)entry;

	const ImUiHash fontHash = ImUiHashCreate( &layout->font, sizeof( &layout->font ) );
	return ImUiHashStringSeed( layout->text, fontHash );
}

static bool ImUiTextLayoutCacheIsKeyEquals( const void* lhs, const void* rhs )
{
	const ImUiTextLayout* lhsLayout = *(const ImUiTextLayout**)lhs;
	const ImUiTextLayout* rhsLayout = *(const ImUiTextLayout**)rhs;

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

bool ImUiTextLayoutCacheConstruct( ImUiTextLayoutCache* cache, ImUiAllocator* allocator )
{
	cache->allocator = allocator;

	if( !ImUiHashMapConstructSize( &cache->layoutMap, allocator, sizeof( ImUiTextLayout* ), ImUiTextLayoutCacheHash, ImUiTextLayoutCacheIsKeyEquals, 64u ) )
	{
		ImUiTextLayoutCacheDestruct( cache );
		return false;
	}

	return true;
}

void ImUiTextLayoutCacheDestruct( ImUiTextLayoutCache* cache )
{
	ImUiTextLayout* layout = cache->firstLayout;
	ImUiTextLayout* nextLayout = NULL;
	while( layout )
	{
		nextLayout = layout->nextLayout;
		ImUiMemoryFree( cache->allocator, layout );
		layout = nextLayout;
	}

	layout = cache->firstUnusedLayout;
	while( layout )
	{
		nextLayout = layout->nextLayout;
		ImUiMemoryFree( cache->allocator, layout );
		layout = nextLayout;
	}

	ImUiHashMapDestruct( &cache->layoutMap );
}

void ImUiTextLayoutCacheEndFrame( ImUiTextLayoutCache* cache )
{
	ImUiTextLayout* unusedLayout = cache->firstUnusedLayout;
	ImUiTextLayout* nextUnusedLayout = NULL;
	while( unusedLayout )
	{
		nextUnusedLayout = unusedLayout->nextLayout;

		const bool removed = ImUiHashMapRemove( &cache->layoutMap, &unusedLayout );
		(void)removed;
		IMUI_ASSERT( removed );
		ImUiMemoryFree( cache->allocator, unusedLayout );
		unusedLayout = nextUnusedLayout;
	}
	cache->firstUnusedLayout	= cache->firstLayout;
	cache->firstLayout			= NULL;

	cache->frameIndex++;
}

ImUiTextLayout* ImUiTextLayoutCreate( ImUiContext* imui, ImUiFont* font, const char* text )
{
	return ImUiTextLayoutCreateLength( imui, font, text, strlen( text ) );
}

ImUiTextLayout* ImUiTextLayoutCreateLength( ImUiContext* imui, ImUiFont* font, const char* text, size_t length )
{
	if( !font )
	{
		return NULL;
	}

	ImUiTextLayoutParameters parameters;
	parameters.font			= font;
	parameters.text.data	= text;
	parameters.text.length	= length;

	return ImUiTextLayoutCacheCreateLayout( &imui->layoutCache, &parameters );
}

ImUiTextLayout* ImUiTextLayoutCreateWidget( ImUiWidget* widget, ImUiFont* font, const char* text )
{
	return ImUiTextLayoutCreateWidgetLength( widget, font, text, strlen( text ) );
}

ImUiTextLayout* ImUiTextLayoutCreateWidgetLength( ImUiWidget* widget, ImUiFont* font, const char* text, size_t length )
{
	return ImUiTextLayoutCreateLength( widget->window->context, font, text, length );
}

ImUiTextLayout* ImUiTextLayoutCacheCreateLayout( ImUiTextLayoutCache* cache, const ImUiTextLayoutParameters* parameters )
{
	if( parameters->text.length == 0u )
	{
		return NULL;
	}

	bool isNew = false;
	ImUiTextLayout** mapLayout = (ImUiTextLayout**)ImUiHashMapInsertNew( &cache->layoutMap, &parameters, &isNew );
	if( !mapLayout )
	{
		return NULL;
	}

	if( !isNew )
	{
		ImUiTextLayout* layout = *mapLayout;
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

	return ImUiTextLayoutCreateNew( cache, parameters, mapLayout );
}

static ImUiTextLayout* ImUiTextLayoutCreateNew( ImUiTextLayoutCache* cache, const ImUiTextLayoutParameters* parameters, ImUiTextLayout** mapLayout )
{
	uint32 glyphCount = 0u;
	for( uintsize i = 0; i < parameters->text.length; )
	{
		const char c = parameters->text.data[ i ];
		if( c == '\n' )
		{
			i++;
			continue;
		}

		uint32 codepointByteCount = IMUI_COUNT_LEADING_ZEROS( ~((uint32)c << 24u) );
		if( codepointByteCount > 4 )
		{
			// invalid character
			codepointByteCount = 1;
		}

		i += IMUI_MAX( codepointByteCount, 1 );
		glyphCount++;
	}

	const uintsize memorySize = sizeof( ImUiTextLayout ) + parameters->text.length + 1u + (sizeof( ImUiTextGlyph ) * glyphCount);
	ImUiTextLayout* layout = (ImUiTextLayout*)ImUiMemoryAlloc( cache->allocator, memorySize );
	if( !layout )
	{
		ImUiHashMapRemove( &cache->layoutMap, mapLayout );
		return NULL;
	}

	ImUiTextGlyph* glyphs = (ImUiTextGlyph*)&layout[ 1u ];
	char* textData = (char*)&glyphs[ glyphCount ];
	memcpy( textData, parameters->text.data, parameters->text.length + 1u );

	uintsize lineCount = 1u;
	uintsize glyphIndex = 0u;
	float x = 0.0f;
	float y = 0.0f;
	for( uintsize i = 0; i < parameters->text.length; ++i )
	{
		ImUiTextGlyph* glyph = &glyphs[ glyphIndex ];

		const char c = parameters->text.data[ i ];
		if( c == '\n' )
		{
			x = 0.0f;
			y += parameters->font->fontSize; // parameters->font->lineGap * parameters->font->fontSize
			y = ceilf( y );
			lineCount++;
			continue;
		}

		const uint32 codepointByteCount = IMUI_COUNT_LEADING_ZEROS( ~((uint32)c << 24u) );

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
		ImUiFontCodepoint** mapCodepoint = (ImUiFontCodepoint**)ImUiHashMapFind( &parameters->font->codepointMap, &mapCodepointKey );
		if( !mapCodepoint )
		{
			codepoint = 0xfffd; // invalid codepoint
			mapCodepoint = (ImUiFontCodepoint**)ImUiHashMapFind( &parameters->font->codepointMap, &mapCodepointKey );
			if( !mapCodepoint )
			{

			}
		}

		ImUiFontCodepoint* codepointInfo;
		if( mapCodepoint )
		{
			codepointInfo = *mapCodepoint;
		}
		else
		{
			// TODO: what to do here?
			x += parameters->font->lineGap;
			layout->glyphCount--;
			continue;
		}

		glyph->codepoint	= codepoint;
		glyph->pos			= ImUiPosCreate( x, y + codepointInfo->ascentOffset );
		glyph->size			= ImUiSizeCreate( (float)codepointInfo->width, (float)codepointInfo->height );
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
	layout->size		= ImUiSizeCreate( ceilf( x ), lineCount * parameters->font->fontSize );
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

ImUiSize ImUiTextLayoutCacheMesureTextSize( ImUiTextLayoutCache* cache, const ImUiTextLayoutParameters* parameters )
{
	ImUiTextLayout* layout = ImUiTextLayoutCacheCreateLayout( cache, parameters );
	if( !layout )
	{
		return ImUiSizeCreateZero();
	}

	return ImUiTextLayoutGetSize( layout );
}

size_t ImUiTextLayoutGetGlyphCount( const ImUiTextLayout* layout )
{
	if( !layout )
	{
		return 0u;
	}

	return layout->glyphCount;
}

size_t ImUiTextLayoutFindGlyphIndex( const ImUiTextLayout* layout, ImUiPos pos, float scale )
{
	if( !layout )
	{
		return 0u;
	}

	for( uintsize i = 0u; i < layout->glyphCount; ++i )
	{
		const ImUiTextGlyph* glyph = &layout->glyphs[ i ];
		const float glyphCenterX = (glyph->pos.x * scale) + (glyph->size.width * 0.5f * scale);
		if( glyphCenterX > pos.x )
		{
			return i;
		}
	}

	return layout->glyphCount;
}

ImUiSize ImUiTextLayoutGetSize( const ImUiTextLayout* layout )
{
	if( !layout )
	{
		return ImUiSizeCreateZero();
	}

	return layout->size;
}

ImUiPos ImUiTextLayoutGetGlyphPos( const ImUiTextLayout* layout, size_t glyphIndex, float scale )
{
	if( !layout )
	{
		return ImUiPosCreateZero();
	}
	else if( glyphIndex >= layout->glyphCount  )
	{
		return ImUiPosCreate( layout->size.width * scale, 0.0f );
	}

	return ImUiPosScale( layout->glyphs[ glyphIndex ].pos, scale );
}
