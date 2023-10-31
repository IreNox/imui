#include "imui/imui.h"

#include "imui_internal.h"

#include <math.h>
#include <string.h>

ImUiStringView ImUiStringViewCreate( const char* str )
{
	if( str == NULL )
	{
		const ImUiStringView string = { NULL, 0u };
		return string;
	}

	ImUiStringView string;
	string.data		= str;
	string.length	= strlen( str );
	return string;
}

ImUiStringView ImUiStringViewCreateLength( const char* str, size_t length )
{
	ImUiStringView string;
	string.data		= str;
	string.length	= length;
	return string;
}

ImUiStringView ImUiStringViewCreateEmpty()
{
	ImUiStringView string;
	string.data		= NULL;
	string.length	= 0u;
	return string;
}

bool ImUiStringViewIsEquals( ImUiStringView string1, ImUiStringView string2 )
{
	if( string1.length != string2.length )
	{
		return false;
	}

		if( string1.length == 0u )
	{
		return true;
	}

	if( string1.data[ 0u ] != string2.data[ 0u ] )
	{
		return false;
	}

	return memcmp( string1.data, string2.data, string1.length ) == 0u;
}

ImUiHash ImUiHashCreate( const void* data, size_t dataSize, ImUiHash seed )
{
	// Murmur3
	uint32 hash = seed;
	uint32 dataPart;
	const uint8* bytes = (const uint8*)data;
	for( uintsize i = dataSize >> 2; i; --i )
	{
		dataPart = *(uint32*)bytes;
		bytes += sizeof( uint32 );

		uint32 scramble = dataPart * 0xcc9e2d51;
		scramble = (scramble << 15) | (scramble >> 17);
		scramble *= 0x1b873593;

		hash ^= scramble;
		hash = (hash << 13) | (hash >> 19);
		hash = hash * 5 + 0xe6546b64;
	}

	dataPart = 0;
	for( uintsize i = dataSize & 3; i; --i )
	{
		dataPart <<= 8;
		dataPart |= bytes[ i - 1 ];
	}

	uint32 scramble = dataPart * 0xcc9e2d51;
	scramble = (scramble << 15) | (scramble >> 17);
	scramble *= 0x1b873593;

	hash ^= scramble;
	hash ^= dataSize;
	hash ^= hash >> 16u;
	hash *= 0x85ebca6b;
	hash ^= hash >> 13u;
	hash *= 0xc2b2ae35;
	hash ^= hash >> 16u;

	return hash;
}

ImUiHash ImUiHashString( ImUiStringView string, ImUiHash seed )
{
	return ImUiHashCreate( string.data, string.length, seed );
}

ImUiHash ImUiHashMix( ImUiHash hash1, ImUiHash hash2 )
{
	return hash1 ^ hash2;
}

ImUiPos ImUiPosCreate( float x, float y )
{
	const ImUiPos pos = { x, y };
	return pos;
}

ImUiPos ImUiPosCreateZero()
{
	const ImUiPos pos = { 0.0f, 0.0f };
	return pos;
}

ImUiPos ImUiPosAdd( ImUiPos pos, float x, float y )
{
	const ImUiPos result = { pos.x + x, pos.y + y };
	return result;
}

ImUiPos ImUiPosAddPos( ImUiPos pos, ImUiPos add )
{
	const ImUiPos result = { pos.x + add.x, pos.y + add.y };
	return result;
}

ImUiPos ImUiPosSub( ImUiPos pos, float x, float y )
{
	const ImUiPos result = { pos.x - x, pos.y - y };
	return result;
}

ImUiPos ImUiPosSubPos( ImUiPos pos, ImUiPos sub )
{
	const ImUiPos result = { pos.x - sub.x, pos.y - sub.y };
	return result;
}

ImUiPos ImUiPosScale( ImUiPos pos, float factor )
{
	const ImUiPos result = { pos.x * factor, pos.y * factor };
	return result;
}

ImUiPos ImUiPosMin( ImUiPos a, ImUiPos b )
{
	const ImUiPos result =
	{
		IMUI_MIN( a.x, b.x ),
		IMUI_MIN( a.y, b.y )
	};
	return result;
}

ImUiPos ImUiPosMax( ImUiPos a, ImUiPos b )
{
	const ImUiPos result =
	{
		IMUI_MAX( a.x, b.x ),
		IMUI_MAX( a.y, b.y )
	};
	return result;
}

ImUiSize ImUiSizeCreate( float width, float height )
{
	const ImUiSize size = { width, height };
	return size;
}

ImUiSize ImUiSizeCreateAll( float value )
{
	const ImUiSize size ={ value, value };
	return size;
}

ImUiSize ImUiSizeCreateOne()
{
	const ImUiSize size = { 1.0f, 1.0f };
	return size;
}

ImUiSize ImUiSizeCreateZero()
{
	const ImUiSize size = { 0.0f, 0.0f };
	return size;
}

ImUiSize ImUiSizeCreateSkin( const ImUiSkin* skin )
{
	IMUI_ASSERT( skin );
	const ImUiSize size = { (float)skin->width, (float)skin->height };
	return size;
}

ImUiSize ImUiSizeCreateImage( const ImUiImage* image )
{
	IMUI_ASSERT( image );
	const ImUiSize size = { (float)image->width, (float)image->height };
	return size;
}

ImUiSize ImUiSizeAdd( ImUiSize size, float width, float height )
{
	const ImUiSize result = { size.width + width, size.height + height };
	return result;
}

ImUiSize ImUiSizeAddSize( ImUiSize size, ImUiSize add )
{
	const ImUiSize result = { size.width + add.width, size.height + add.height };
	return result;
}

ImUiSize ImUiSizeSub( ImUiSize size, float width, float height )
{
	const ImUiSize result = { size.width - width, size.height - height };
	return result;
}

ImUiSize ImUiSizeSubSize( ImUiSize size, ImUiSize sub )
{
	const ImUiSize result = { size.width - sub.width, size.height - sub.height };
	return result;
}

ImUiSize ImUiSizeScale( ImUiSize size, float factor )
{
	const ImUiSize result = { size.width * factor, size.height * factor };
	return result;
}

ImUiSize ImUiSizeShrinkBorder( ImUiSize size, ImUiBorder border )
{
	const ImUiSize result =
	{
		size.width - (border.left + border.right),
		size.height - (border.top + border.bottom)
	};
	return result;
}

ImUiSize ImUiSizeExpandBorder( ImUiSize size, ImUiBorder border )
{
	const ImUiSize result =
	{
		size.width + border.left + border.right,
		size.height + border.top + border.bottom
	};
	return result;
}

ImUiSize ImUiSizeLerp( ImUiSize a, ImUiSize b, float t )
{
	const ImUiSize result =
	{
		a.width + ((b.width - a.width) * t),
		a.height + ((b.height - a.height) * t)
	};
	return result;
}

ImUiSize ImUiSizeLerp2( ImUiSize a, ImUiSize b, float widthT, float heightT )
{
	const ImUiSize result =
	{
		a.width + ((b.width - a.width) * widthT),
		a.height + ((b.height - a.height) * heightT)
	};
	return result;
}

ImUiSize ImUiSizeMin( ImUiSize a, ImUiSize b )
{
	const ImUiSize result =
	{
		IMUI_MIN( a.width, b.width ),
		IMUI_MIN( a.height, b.height )
	};
	return result;
}

ImUiSize ImUiSizeMax( ImUiSize a, ImUiSize b )
{
	const ImUiSize result =
	{
		IMUI_MAX( a.width, b.width ),
		IMUI_MAX( a.height, b.height )
	};
	return result;
}

ImUiSize ImUiSizeFloor( ImUiSize size )
{
	const ImUiSize result =
	{
		floorf( size.width ),
		floorf( size.height )
	};
	return result;
}

ImUiSize ImUiSizeCeil( ImUiSize size )
{
	const ImUiSize result =
	{
		ceilf( size.width ),
		ceilf( size.height )
	};
	return result;
}

ImUiPos ImUiSizeToPos( ImUiSize size )
{
	const ImUiPos result =
	{
		size.width,
		size.height
	};
	return result;
}

ImUiBorder ImUiBorderCreate( float top, float left, float bottom, float right )
{
	const ImUiBorder result = { top, left, bottom, right };
	return result;
}

ImUiBorder ImUiBorderCreateAll( float all )
{
	const ImUiBorder result = { all, all, all, all };
	return result;
}

ImUiBorder ImUiBorderCreateZero()
{
	const ImUiBorder result = { 0.0f, 0.0f, 0.0f, 0.0f };
	return result;
}

ImUiBorder ImUiBorderCreateHorizontalVertical( float horizontal, float vertical )
{
	const ImUiBorder result = { vertical, horizontal, vertical, horizontal };
	return result;
}

ImUiSize ImUiBorderGetMinSize( ImUiBorder border )
{
	const ImUiSize result = { border.left + border.right, border.top + border.bottom };
	return result;
}

ImUiRect ImUiRectCreate( float x, float y, float width, float height )
{
	const ImUiRect rect = { { x, y }, { width, height } };
	return rect;
}

ImUiRect ImUiRectCreatePos( ImUiPos pos, float width, float height )
{
	const ImUiRect rect = { pos, { width, height } };
	return rect;
}

ImUiRect ImUiRectCreateSize( float x, float y, ImUiSize size )
{
	const ImUiRect rect = { { x, y }, size };
	return rect;
}

ImUiRect ImUiRectCreatePosSize( ImUiPos pos, ImUiSize size )
{
	const ImUiRect rect = { pos, size };
	return rect;
}

ImUiRect ImUiRectCreateMinMax( float minX, float minY, float maxX, float maxY )
{
	const ImUiRect rect = { { minX, minY }, { maxX - minX , maxY - minY } };
	return rect;
}

ImUiRect ImUiRectCreateMinMaxPos( ImUiPos tl, ImUiPos br )
{
	const ImUiRect rect ={ tl, { br.x - tl.x , br.y - tl.y } };
	return rect;
}

ImUiRect ImUiRectCreateCenter( float x, float y, float width, float height )
{
	const float halfWidth	= width * 0.5f;
	const float halfHeight	= height * 0.5f;
	const ImUiRect rect = { { x - halfWidth, y - halfHeight }, { width, height } };
	return rect;
}

ImUiRect ImUiRectCreateCenterPos( ImUiPos pos, float width, float height )
{
	return ImUiRectCreateCenter( pos.x, pos.y, width, height );
}

ImUiRect ImUiRectCreateCenterSize( float x, float y, ImUiSize size )
{
	return ImUiRectCreateCenter( x, y, size.width, size.height );
}

ImUiRect ImUiRectCreateCenterPosSize( ImUiPos pos, ImUiSize size )
{
	return ImUiRectCreateCenter( pos.x, pos.y, size.width, size.height );
}

ImUiRect ImUiRectCreateZero()
{
	const ImUiRect rect ={ { 0.0f, 0.0f }, { 0.0f, 0.0f } };
	return rect;
}

ImUiRect ImUiRectShrinkBorder( ImUiRect rect, ImUiBorder border )
{
	const ImUiRect result =
	{
		{ rect.pos.x + border.left, rect.pos.y + border.top },
		{ IMUI_MAX( 0.0f, rect.size.width - border.left - border.right ), IMUI_MAX( 0.0f, rect.size.height - border.top - border.bottom ) }
	};
	return result;
}

ImUiRect ImUiRectIntersection( ImUiRect rect1, ImUiRect rect2 )
{
	const ImUiPos rect1br	= ImUiRectGetBottomRight( rect1 );
	const ImUiPos rect2br	= ImUiRectGetBottomRight( rect2 );
	if( rect2br.x >= rect1.pos.x &&
		rect2br.y >= rect1.pos.y &&
		rect2.pos.x <= rect1br.x &&
		rect2.pos.y <= rect1br.y )
	{
		return ImUiRectCreateMinMax(
			IMUI_MAX( rect1.pos.x, rect2.pos.x ),
			IMUI_MAX( rect1.pos.y, rect2.pos.y ),
			IMUI_MIN( rect1br.x, rect2br.x ),
			IMUI_MIN( rect1br.y, rect2br.y )
		);
	}

	return ImUiRectCreateZero();
}

bool ImUiRectIncludesPos( ImUiRect rect, ImUiPos pos )
{
	return pos.x >= rect.pos.x &&
		pos.y >= rect.pos.y &&
		pos.x <= rect.pos.x + rect.size.width &&
		pos.y <= rect.pos.y + rect.size.height;
}

bool ImUiRectIntersectsRect( ImUiRect rect1, ImUiRect rect2 )
{
	return rect2.pos.y < rect1.pos.y + rect1.size.height &&
		rect2.pos.y + rect2.size.height > rect1.pos.y &&
		rect2.pos.x < rect1.pos.x + rect1.size.width &&
		rect2.pos.x + rect2.size.width > rect1.pos.x;
}

ImUiPos ImUiRectGetTopLeft( ImUiRect rect )
{
	return rect.pos;
}

ImUiPos ImUiRectGetTopRight( ImUiRect rect )
{
	return ImUiPosAdd( rect.pos, rect.size.width, 0.0f );
}

ImUiPos ImUiRectGetBottomLeft( ImUiRect rect )
{
	return ImUiPosAdd( rect.pos, 0.0f, rect.size.height );
}

ImUiPos ImUiRectGetBottomRight( ImUiRect rect )
{
	return ImUiPosAdd( rect.pos, rect.size.width, rect.size.height );
}

ImUiPos ImUiRectGetCenter( ImUiRect rect )
{
	return ImUiPosAdd( rect.pos, rect.size.width * 0.5f, rect.size.height * 0.5f );
}

float ImUiRectGetRight( ImUiRect rect )
{
	return rect.pos.x + rect.size.width;
}

float ImUiRectGetBottom( ImUiRect rect )
{
	return rect.pos.y + rect.size.height;
}

ImUiColor ImUiColorCreate( uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha )
{
	const ImUiColor color = { red, green, blue, alpha };
	return color;
}

ImUiColor ImUiColorCreateFloat( float red, float green, float blue, float alpha )
{
	const ImUiColor color = {
		(uint8_t)((red * 255.0f) + 0.5f),
		(uint8_t)((green * 255.0f) + 0.5f),
		(uint8_t)((blue * 255.0f) + 0.5f),
		(uint8_t)((alpha * 255.0f) + 0.5f)
	};
	return color;
}

ImUiColor ImUiColorCreateBlack()
{
	return ImUiColorCreate( 0u, 0u, 0u, 0xffu );
}

ImUiColor ImUiColorCreateBlackA( uint8_t alpha )
{
	return ImUiColorCreate( 0u, 0u, 0u, alpha );
}

ImUiColor ImUiColorCreateWhite()
{
	return ImUiColorCreate( 0xffu, 0xffu, 0xffu, 0xffu );
}

ImUiColor ImUiColorCreateWhiteA( uint8_t alpha )
{
	return ImUiColorCreate( 0xffu, 0xffu, 0xffu, alpha );
}

ImUiColor ImUiColorCreateGray( uint8_t gray )
{
	return ImUiColorCreate( gray, gray, gray, 0xffu );
}

ImUiColor ImUiColorCreateGrayA( uint8_t gray, uint8_t alpha )
{
	return ImUiColorCreate( gray, gray, gray, alpha );
}

ImUiColor ImUiColorCreateTransparentBlack()
{
	return ImUiColorCreate( 0u, 0u, 0u, 0u );
}
