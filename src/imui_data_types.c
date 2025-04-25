#include "imui/imui.h"

#include "imui_internal.h"

#include <math.h>
#include <string.h>

static const ImUiHash s_hashDefaultSeed = 0xc6b568d8;

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

ImUiHash ImUiHashCreate( const void* data, size_t dataSize )
{
	return ImUiHashCreateSeed( data, dataSize, s_hashDefaultSeed );
}

ImUiHash ImUiHashCreateSeed( const void* data, size_t dataSize, ImUiHash seed )
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

ImUiHash ImUiHashString( ImUiStringView string )
{
	return ImUiHashCreate( string.data, string.length );
}

ImUiHash ImUiHashStringSeed( ImUiStringView string, ImUiHash seed )
{
	return ImUiHashCreateSeed( string.data, string.length, seed );
}

ImUiHash ImUiHashMix( ImUiHash hash1, ImUiHash hash2 )
{
	return hash1 ^ hash2;
}

ImUiPos ImUiPosCreate( float x, float y )
{
	ImUiPos result;
	result.x = x;
	result.y = y;
	return result;
}

ImUiPos ImUiPosCreateZero()
{
	const ImUiPos result = { 0.0f, 0.0f };
	return result;
}

ImUiPos ImUiPosAdd( ImUiPos pos, float x, float y )
{
	ImUiPos result;
	result.x = pos.x + x;
	result.y = pos.y + y;
	return result;
}

ImUiPos ImUiPosAddPos( ImUiPos pos, ImUiPos add )
{
	ImUiPos result;
	result.x = pos.x + add.x;
	result.y = pos.y + add.y;
	return result;
}

ImUiPos ImUiPosSub( ImUiPos pos, float x, float y )
{
	ImUiPos result;
	result.x = pos.x - x;
	result.y = pos.y - y;
	return result;
}

ImUiPos ImUiPosSubPos( ImUiPos pos, ImUiPos sub )
{
	ImUiPos result;
	result.x = pos.x - sub.x;
	result.y = pos.y - sub.y;
	return result;
}

ImUiPos ImUiPosScale( ImUiPos pos, float factor )
{
	ImUiPos result;
	result.x = pos.x * factor;
	result.y = pos.y * factor;
	return result;
}

ImUiPos ImUiPosMin( ImUiPos a, ImUiPos b )
{
	ImUiPos result;
	result.x = IMUI_MIN( a.x, b.x );
	result.y = IMUI_MIN( a.y, b.y );
	return result;
}

ImUiPos ImUiPosMax( ImUiPos a, ImUiPos b )
{
	ImUiPos result;
	result.x = IMUI_MAX( a.x, b.x );
	result.y = IMUI_MAX( a.y, b.y );
	return result;
}

ImUiSize ImUiSizeCreate( float width, float height )
{
	ImUiSize result;
	result.width	= width;
	result.height	= height;
	return result;
}

ImUiSize ImUiSizeCreateAll( float value )
{
	ImUiSize result;
	result.width	= value;
	result.height	= value;
	return result;
}

ImUiSize ImUiSizeCreateOne()
{
	const ImUiSize result = { 1.0f, 1.0f };
	return result;
}

ImUiSize ImUiSizeCreateZero()
{
	const ImUiSize result = { 0.0f, 0.0f };
	return result;
}

ImUiSize ImUiSizeCreateSkin( const ImUiSkin* skin )
{
	IMUI_ASSERT( skin );

	ImUiSize result;
	result.width	= (float)skin->width;
	result.height	= (float)skin->height;
	return result;
}

ImUiSize ImUiSizeCreateImage( const ImUiImage* image )
{
	IMUI_ASSERT( image );

	ImUiSize result;
	result.width	= (float)image->width;
	result.height	= (float)image->height;
	return result;
}

ImUiSize ImUiSizeAdd( ImUiSize size, float width, float height )
{
	ImUiSize result;
	result.width	= size.width + width;
	result.height	= size.height + height;
	return result;
}

ImUiSize ImUiSizeAddSize( ImUiSize size, ImUiSize add )
{
	ImUiSize result;
	result.width	= size.width + add.width;
	result.height	= size.height + add.height;
	return result;
}

ImUiSize ImUiSizeSub( ImUiSize size, float width, float height )
{
	ImUiSize result;
	result.width	= size.width - width;
	result.height	= size.height - height;
	return result;
}

ImUiSize ImUiSizeSubSize( ImUiSize size, ImUiSize sub )
{
	ImUiSize result;
	result.width	= size.width - sub.width;
	result.height	= size.height - sub.height;
	return result;
}

ImUiSize ImUiSizeScale( ImUiSize size, float factor )
{
	ImUiSize result;
	result.width	= size.width * factor;
	result.height	= size.height * factor;
	return result;
}

ImUiSize ImUiSizeShrinkBorder( ImUiSize size, ImUiBorder border )
{
	ImUiSize result;
	result.width	= size.width - (border.left + border.right);
	result.height	= size.height - (border.top + border.bottom);
	return result;
}

ImUiSize ImUiSizeExpandBorder( ImUiSize size, ImUiBorder border )
{
	ImUiSize result;
	result.width	= size.width + border.left + border.right;
	result.height	= size.height + border.top + border.bottom;
	return result;
}

ImUiSize ImUiSizeLerp( ImUiSize a, ImUiSize b, float t )
{
	ImUiSize result;
	result.width	= a.width + ((b.width - a.width) * t);
	result.height	= a.height + ((b.height - a.height) * t);
	return result;
}

ImUiSize ImUiSizeLerp2( ImUiSize a, ImUiSize b, float widthT, float heightT )
{
	ImUiSize result;
	result.width	= a.width + ((b.width - a.width) * widthT);
	result.height	= a.height + ((b.height - a.height) * heightT);
	return result;
}

ImUiSize ImUiSizeMin( ImUiSize a, ImUiSize b )
{
	ImUiSize result;
	result.width	= IMUI_MIN( a.width, b.width );
	result.height	= IMUI_MIN( a.height, b.height );
	return result;
}

ImUiSize ImUiSizeMax( ImUiSize a, ImUiSize b )
{
	ImUiSize result;
	result.width	= IMUI_MAX( a.width, b.width );
	result.height	= IMUI_MAX( a.height, b.height );
	return result;
}

ImUiSize ImUiSizeFloor( ImUiSize size )
{
	ImUiSize result;
	result.width	= floorf( size.width );
	result.height	= floorf( size.height );
	return result;
}

ImUiSize ImUiSizeCeil( ImUiSize size )
{
	ImUiSize result;
	result.width	= ceilf( size.width );
	result.height	= ceilf( size.height );
	return result;
}

ImUiPos ImUiSizeToPos( ImUiSize size )
{
	ImUiPos result;
	result.x = size.width;
	result.y = size.height;
	return result;
}

ImUiBorder ImUiBorderCreate( float top, float left, float bottom, float right )
{
	ImUiBorder result;
	result.top		= top;
	result.left		= left;
	result.bottom	= bottom;
	result.right	= right;
	return result;
}

ImUiBorder ImUiBorderCreateAll( float all )
{
	ImUiBorder result;
	result.top		= all;
	result.left		= all;
	result.bottom	= all;
	result.right	= all;
	return result;
}

ImUiBorder ImUiBorderCreateZero()
{
	const ImUiBorder result = { 0.0f, 0.0f, 0.0f, 0.0f };
	return result;
}

ImUiBorder ImUiBorderCreateHorizontalVertical( float horizontal, float vertical )
{
	ImUiBorder result;
	result.top		= vertical;
	result.left		= horizontal;
	result.bottom	= vertical;
	result.right	= horizontal;
	return result;
}

ImUiBorder ImUiBorderScale( ImUiBorder border, float factor )
{
	ImUiBorder result = border;
	result.top		*= factor;
	result.left		*= factor;
	result.bottom	*= factor;
	result.right	*= factor;
	return result;
}

ImUiSize ImUiBorderGetMinSize( ImUiBorder border )
{
	ImUiSize result;
	result.width	= border.left + border.right;
	result.height	= border.top + border.bottom;
	return result;
}

ImUiRect ImUiRectCreate( float x, float y, float width, float height )
{
	ImUiRect result;
	result.pos.x		= x;
	result.pos.y		= y;
	result.size.width	= width;
	result.size.height	= height;
	return result;
}

ImUiRect ImUiRectCreatePos( ImUiPos pos, float width, float height )
{
	ImUiRect result;
	result.pos			= pos;
	result.size.width	= width;
	result.size.height	= height;
	return result;
}

ImUiRect ImUiRectCreateSize( float x, float y, ImUiSize size )
{
	ImUiRect result;
	result.pos.x		= x;
	result.pos.y		= y;
	result.size			= size;
	return result;
}

ImUiRect ImUiRectCreatePosSize( ImUiPos pos, ImUiSize size )
{
	ImUiRect result;
	result.pos			= pos;
	result.size			= size;
	return result;
}

ImUiRect ImUiRectCreateMinMax( float minX, float minY, float maxX, float maxY )
{
	ImUiRect result;
	result.pos.x		= minX;
	result.pos.y		= minY;
	result.size.width	= maxX - minX;
	result.size.height	= maxY - minY;
	return result;
}

ImUiRect ImUiRectCreateMinMaxPos( ImUiPos tl, ImUiPos br )
{
	ImUiRect result;
	result.pos			= tl;
	result.size.width	= br.x - tl.x;
	result.size.height	= br.y - tl.y;
	return result;
}

ImUiRect ImUiRectCreateCenter( float x, float y, float width, float height )
{
	const float halfWidth	= width * 0.5f;
	const float halfHeight	= height * 0.5f;

	ImUiRect result;
	result.pos.x		= x - halfWidth;
	result.pos.y		= y - halfHeight;
	result.size.width	= width;
	result.size.height	= height;
	return result;
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
	const ImUiRect result = { { 0.0f, 0.0f }, { 0.0f, 0.0f } };
	return result;
}

ImUiRect ImUiRectShrinkBorder( ImUiRect rect, ImUiBorder border )
{
	ImUiRect result;
	result.pos.x		= rect.pos.x + border.left;
	result.pos.y		= rect.pos.y + border.top;
	result.size.width	= IMUI_MAX( 0.0f, rect.size.width - border.left - border.right );
	result.size.height	= IMUI_MAX( 0.0f, rect.size.height - border.top - border.bottom );
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
	ImUiColor result;
	result.red		= red;
	result.green	= green;
	result.blue		= blue;
	result.alpha	= alpha;
	return result;
}

ImUiColor ImUiColorCreateFloat( float red, float green, float blue, float alpha )
{
	ImUiColor result;
	result.red		= (uint8_t)((red * 255.0f) + 0.5f);
	result.green	= (uint8_t)((green * 255.0f) + 0.5f);
	result.blue		= (uint8_t)((blue * 255.0f) + 0.5f);
	result.alpha	= (uint8_t)((alpha * 255.0f) + 0.5f);
	return result;
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
