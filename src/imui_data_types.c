#include "imui/imui.h"

#include "imui_internal.h"

#include <math.h>
#include <string.h>

static const ImuiHash s_hashDefaultSeed = 0xc6b568d8;

ImuiStringView imuiStringViewCreate( const char* str )
{
	if( str == NULL )
	{
		const ImuiStringView string = { NULL, 0u };
		return string;
	}

	ImuiStringView string;
	string.data		= str;
	string.length	= strlen( str );
	return string;
}

ImuiStringView imuiStringViewCreateLength( const char* str, size_t length )
{
	ImuiStringView string;
	string.data		= str;
	string.length	= length;
	return string;
}

ImuiStringView imuiStringViewCreateEmpty()
{
	ImuiStringView string;
	string.data		= NULL;
	string.length	= 0u;
	return string;
}

bool imuiStringViewIsEquals( ImuiStringView string1, ImuiStringView string2 )
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

ImuiHash imuiHashCreate( const void* data, size_t dataSize )
{
	return imuiHashCreateSeed( data, dataSize, s_hashDefaultSeed );
}

ImuiHash imuiHashCreateSeed( const void* data, size_t dataSize, ImuiHash seed )
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

ImuiHash imuiHashString( ImuiStringView string )
{
	return imuiHashCreate( string.data, string.length );
}

ImuiHash imuiHashStringSeed( ImuiStringView string, ImuiHash seed )
{
	return imuiHashCreateSeed( string.data, string.length, seed );
}

ImuiHash imuiHashMix( ImuiHash hash1, ImuiHash hash2 )
{
	return hash1 ^ hash2;
}

ImuiPos imuiPosCreate( float x, float y )
{
	ImuiPos result;
	result.x = x;
	result.y = y;
	return result;
}

ImuiPos imuiPosCreateZero()
{
	const ImuiPos result = { 0.0f, 0.0f };
	return result;
}

ImuiPos imuiPosAdd( ImuiPos pos, float x, float y )
{
	ImuiPos result;
	result.x = pos.x + x;
	result.y = pos.y + y;
	return result;
}

ImuiPos imuiPosAddPos( ImuiPos pos, ImuiPos add )
{
	ImuiPos result;
	result.x = pos.x + add.x;
	result.y = pos.y + add.y;
	return result;
}

ImuiPos imuiPosSub( ImuiPos pos, float x, float y )
{
	ImuiPos result;
	result.x = pos.x - x;
	result.y = pos.y - y;
	return result;
}

ImuiPos imuiPosSubPos( ImuiPos pos, ImuiPos sub )
{
	ImuiPos result;
	result.x = pos.x - sub.x;
	result.y = pos.y - sub.y;
	return result;
}

ImuiPos imuiPosScale( ImuiPos pos, float factor )
{
	ImuiPos result;
	result.x = pos.x * factor;
	result.y = pos.y * factor;
	return result;
}

ImuiPos imuiPosMin( ImuiPos a, ImuiPos b )
{
	ImuiPos result;
	result.x = IMUI_MIN( a.x, b.x );
	result.y = IMUI_MIN( a.y, b.y );
	return result;
}

ImuiPos imuiPosMax( ImuiPos a, ImuiPos b )
{
	ImuiPos result;
	result.x = IMUI_MAX( a.x, b.x );
	result.y = IMUI_MAX( a.y, b.y );
	return result;
}

ImuiSize imuiSizeCreate( float width, float height )
{
	ImuiSize result;
	result.width	= width;
	result.height	= height;
	return result;
}

ImuiSize imuiSizeCreateAll( float value )
{
	ImuiSize result;
	result.width	= value;
	result.height	= value;
	return result;
}

ImuiSize imuiSizeCreateOne()
{
	const ImuiSize result = { 1.0f, 1.0f };
	return result;
}

ImuiSize imuiSizeCreateZero()
{
	const ImuiSize result = { 0.0f, 0.0f };
	return result;
}

ImuiSize imuiSizeCreateSkin( const ImuiSkin* skin )
{
	IMUI_ASSERT( skin );

	ImuiSize result;
	result.width	= (float)skin->width;
	result.height	= (float)skin->height;
	return result;
}

ImuiSize imuiSizeCreateImage( const ImuiImage* image )
{
	IMUI_ASSERT( image );

	ImuiSize result;
	result.width	= (float)image->width;
	result.height	= (float)image->height;
	return result;
}

ImuiSize imuiSizeAdd( ImuiSize size, float width, float height )
{
	ImuiSize result;
	result.width	= size.width + width;
	result.height	= size.height + height;
	return result;
}

ImuiSize imuiSizeAddSize( ImuiSize size, ImuiSize add )
{
	ImuiSize result;
	result.width	= size.width + add.width;
	result.height	= size.height + add.height;
	return result;
}

ImuiSize imuiSizeSub( ImuiSize size, float width, float height )
{
	ImuiSize result;
	result.width	= size.width - width;
	result.height	= size.height - height;
	return result;
}

ImuiSize imuiSizeSubSize( ImuiSize size, ImuiSize sub )
{
	ImuiSize result;
	result.width	= size.width - sub.width;
	result.height	= size.height - sub.height;
	return result;
}

ImuiSize imuiSizeScale( ImuiSize size, float factor )
{
	ImuiSize result;
	result.width	= size.width * factor;
	result.height	= size.height * factor;
	return result;
}

ImuiSize imuiSizeShrinkBorder( ImuiSize size, ImuiBorder border )
{
	ImuiSize result;
	result.width	= size.width - (border.left + border.right);
	result.height	= size.height - (border.top + border.bottom);
	return result;
}

ImuiSize imuiSizeExpandBorder( ImuiSize size, ImuiBorder border )
{
	ImuiSize result;
	result.width	= size.width + border.left + border.right;
	result.height	= size.height + border.top + border.bottom;
	return result;
}

ImuiSize imuiSizeLerp( ImuiSize a, ImuiSize b, float t )
{
	ImuiSize result;
	result.width	= a.width + ((b.width - a.width) * t);
	result.height	= a.height + ((b.height - a.height) * t);
	return result;
}

ImuiSize imuiSizeLerp2( ImuiSize a, ImuiSize b, float widthT, float heightT )
{
	ImuiSize result;
	result.width	= a.width + ((b.width - a.width) * widthT);
	result.height	= a.height + ((b.height - a.height) * heightT);
	return result;
}

ImuiSize imuiSizeMin( ImuiSize a, ImuiSize b )
{
	ImuiSize result;
	result.width	= IMUI_MIN( a.width, b.width );
	result.height	= IMUI_MIN( a.height, b.height );
	return result;
}

ImuiSize imuiSizeMax( ImuiSize a, ImuiSize b )
{
	ImuiSize result;
	result.width	= IMUI_MAX( a.width, b.width );
	result.height	= IMUI_MAX( a.height, b.height );
	return result;
}

ImuiSize imuiSizeFloor( ImuiSize size )
{
	ImuiSize result;
	result.width	= floorf( size.width );
	result.height	= floorf( size.height );
	return result;
}

ImuiSize imuiSizeCeil( ImuiSize size )
{
	ImuiSize result;
	result.width	= ceilf( size.width );
	result.height	= ceilf( size.height );
	return result;
}

ImuiPos imuiSizeToPos( ImuiSize size )
{
	ImuiPos result;
	result.x = size.width;
	result.y = size.height;
	return result;
}

ImuiBorder imuiBorderCreate( float top, float left, float bottom, float right )
{
	ImuiBorder result;
	result.top		= top;
	result.left		= left;
	result.bottom	= bottom;
	result.right	= right;
	return result;
}

ImuiBorder imuiBorderCreateAll( float all )
{
	ImuiBorder result;
	result.top		= all;
	result.left		= all;
	result.bottom	= all;
	result.right	= all;
	return result;
}

ImuiBorder imuiBorderCreateZero()
{
	const ImuiBorder result = { 0.0f, 0.0f, 0.0f, 0.0f };
	return result;
}

ImuiBorder imuiBorderCreateHorizontalVertical( float horizontal, float vertical )
{
	ImuiBorder result;
	result.top		= vertical;
	result.left		= horizontal;
	result.bottom	= vertical;
	result.right	= horizontal;
	return result;
}

ImuiBorder imuiBorderScale( ImuiBorder border, float factor )
{
	ImuiBorder result = border;
	result.top		*= factor;
	result.left		*= factor;
	result.bottom	*= factor;
	result.right	*= factor;
	return result;
}

ImuiSize imuiBorderGetMinSize( ImuiBorder border )
{
	ImuiSize result;
	result.width	= border.left + border.right;
	result.height	= border.top + border.bottom;
	return result;
}

ImuiRect imuiRectCreate( float x, float y, float width, float height )
{
	ImuiRect result;
	result.pos.x		= x;
	result.pos.y		= y;
	result.size.width	= width;
	result.size.height	= height;
	return result;
}

ImuiRect imuiRectCreatePos( ImuiPos pos, float width, float height )
{
	ImuiRect result;
	result.pos			= pos;
	result.size.width	= width;
	result.size.height	= height;
	return result;
}

ImuiRect imuiRectCreateSize( float x, float y, ImuiSize size )
{
	ImuiRect result;
	result.pos.x		= x;
	result.pos.y		= y;
	result.size			= size;
	return result;
}

ImuiRect imuiRectCreatePosSize( ImuiPos pos, ImuiSize size )
{
	ImuiRect result;
	result.pos			= pos;
	result.size			= size;
	return result;
}

ImuiRect imuiRectCreateMinMax( float minX, float minY, float maxX, float maxY )
{
	ImuiRect result;
	result.pos.x		= minX;
	result.pos.y		= minY;
	result.size.width	= maxX - minX;
	result.size.height	= maxY - minY;
	return result;
}

ImuiRect imuiRectCreateMinMaxPos( ImuiPos tl, ImuiPos br )
{
	ImuiRect result;
	result.pos			= tl;
	result.size.width	= br.x - tl.x;
	result.size.height	= br.y - tl.y;
	return result;
}

ImuiRect imuiRectCreateCenter( float x, float y, float width, float height )
{
	const float halfWidth	= width * 0.5f;
	const float halfHeight	= height * 0.5f;

	ImuiRect result;
	result.pos.x		= x - halfWidth;
	result.pos.y		= y - halfHeight;
	result.size.width	= width;
	result.size.height	= height;
	return result;
}

ImuiRect imuiRectCreateCenterPos( ImuiPos pos, float width, float height )
{
	return imuiRectCreateCenter( pos.x, pos.y, width, height );
}

ImuiRect imuiRectCreateCenterSize( float x, float y, ImuiSize size )
{
	return imuiRectCreateCenter( x, y, size.width, size.height );
}

ImuiRect imuiRectCreateCenterPosSize( ImuiPos pos, ImuiSize size )
{
	return imuiRectCreateCenter( pos.x, pos.y, size.width, size.height );
}

ImuiRect imuiRectCreateZero()
{
	const ImuiRect result = { { 0.0f, 0.0f }, { 0.0f, 0.0f } };
	return result;
}

ImuiRect imuiRectShrinkBorder( ImuiRect rect, ImuiBorder border )
{
	ImuiRect result;
	result.pos.x		= rect.pos.x + border.left;
	result.pos.y		= rect.pos.y + border.top;
	result.size.width	= IMUI_MAX( 0.0f, rect.size.width - border.left - border.right );
	result.size.height	= IMUI_MAX( 0.0f, rect.size.height - border.top - border.bottom );
	return result;
}

ImuiRect imuiRectIntersection( ImuiRect rect1, ImuiRect rect2 )
{
	const ImuiPos rect1br	= imuiRectGetBottomRight( rect1 );
	const ImuiPos rect2br	= imuiRectGetBottomRight( rect2 );
	if( rect2br.x >= rect1.pos.x &&
		rect2br.y >= rect1.pos.y &&
		rect2.pos.x <= rect1br.x &&
		rect2.pos.y <= rect1br.y )
	{
		return imuiRectCreateMinMax(
			IMUI_MAX( rect1.pos.x, rect2.pos.x ),
			IMUI_MAX( rect1.pos.y, rect2.pos.y ),
			IMUI_MIN( rect1br.x, rect2br.x ),
			IMUI_MIN( rect1br.y, rect2br.y )
		);
	}

	return imuiRectCreateZero();
}

bool imuiRectIncludesPos( ImuiRect rect, ImuiPos pos )
{
	return pos.x >= rect.pos.x &&
		pos.y >= rect.pos.y &&
		pos.x <= rect.pos.x + rect.size.width &&
		pos.y <= rect.pos.y + rect.size.height;
}

bool imuiRectIntersectsRect( ImuiRect rect1, ImuiRect rect2 )
{
	return rect2.pos.y < rect1.pos.y + rect1.size.height &&
		rect2.pos.y + rect2.size.height > rect1.pos.y &&
		rect2.pos.x < rect1.pos.x + rect1.size.width &&
		rect2.pos.x + rect2.size.width > rect1.pos.x;
}

ImuiPos imuiRectGetTopLeft( ImuiRect rect )
{
	return rect.pos;
}

ImuiPos imuiRectGetTopRight( ImuiRect rect )
{
	return imuiPosAdd( rect.pos, rect.size.width, 0.0f );
}

ImuiPos imuiRectGetBottomLeft( ImuiRect rect )
{
	return imuiPosAdd( rect.pos, 0.0f, rect.size.height );
}

ImuiPos imuiRectGetBottomRight( ImuiRect rect )
{
	return imuiPosAdd( rect.pos, rect.size.width, rect.size.height );
}

ImuiPos imuiRectGetCenter( ImuiRect rect )
{
	return imuiPosAdd( rect.pos, rect.size.width * 0.5f, rect.size.height * 0.5f );
}

float imuiRectGetRight( ImuiRect rect )
{
	return rect.pos.x + rect.size.width;
}

float imuiRectGetBottom( ImuiRect rect )
{
	return rect.pos.y + rect.size.height;
}

ImuiColor imuiColorCreate( uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha )
{
	ImuiColor result;
	result.red		= red;
	result.green	= green;
	result.blue		= blue;
	result.alpha	= alpha;
	return result;
}

ImuiColor imuiColorCreateFloat( float red, float green, float blue, float alpha )
{
	ImuiColor result;
	result.red		= (uint8_t)((red * 255.0f) + 0.5f);
	result.green	= (uint8_t)((green * 255.0f) + 0.5f);
	result.blue		= (uint8_t)((blue * 255.0f) + 0.5f);
	result.alpha	= (uint8_t)((alpha * 255.0f) + 0.5f);
	return result;
}

ImuiColor imuiColorCreateBlack()
{
	return imuiColorCreate( 0u, 0u, 0u, 0xffu );
}

ImuiColor imuiColorCreateBlackA( uint8_t alpha )
{
	return imuiColorCreate( 0u, 0u, 0u, alpha );
}

ImuiColor imuiColorCreateWhite()
{
	return imuiColorCreate( 0xffu, 0xffu, 0xffu, 0xffu );
}

ImuiColor imuiColorCreateWhiteA( uint8_t alpha )
{
	return imuiColorCreate( 0xffu, 0xffu, 0xffu, alpha );
}

ImuiColor imuiColorCreateGray( uint8_t gray )
{
	return imuiColorCreate( gray, gray, gray, 0xffu );
}

ImuiColor imuiColorCreateGrayA( uint8_t gray, uint8_t alpha )
{
	return imuiColorCreate( gray, gray, gray, alpha );
}

ImuiColor imuiColorCreateTransparentBlack()
{
	return imuiColorCreate( 0u, 0u, 0u, 0u );
}
