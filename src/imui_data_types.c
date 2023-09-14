#include "imui/imui.h"

#include "imui_internal.h"

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

	return memcmp( string1.data, string2.data, string1.length ) == 0u;
}

ImUiHash ImUiHashCreate( const void* data, size_t dataSize, ImUiHash seed )
{
	// Murmur3
	uint32 hash = seed;
	uint32_t dataPart;
	const uint8* bytes = (const uint8*)data;
	for( size_t i = dataSize >> 2; i; --i )
	{
		memcpy( &dataPart, bytes, sizeof( uint32 ) );
		bytes += sizeof( uint32 );

		uint32 scramble = dataPart * 0xcc9e2d51;
		scramble = (scramble << 15) | (scramble >> 17);
		scramble *= 0x1b873593;

		hash ^= scramble;
		hash = (hash << 13) | (hash >> 19);
		hash = hash * 5 + 0xe6546b64;
	}

	dataPart = 0;
	for( size_t i = dataSize & 3; i; --i )
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

ImUiAlignment ImUiAlignmentCreate( ImUiHorizontalAlignment horizintal, ImUiVerticalAlignment vertical )
{
	const ImUiAlignment align = { horizintal, vertical };
	return align;
}

ImUiPosition ImUiPositionCreate( float x, float y )
{
	const ImUiPosition pos = { x, y };
	return pos;
}

ImUiPosition ImUiPositionAdd( ImUiPosition pos, float x, float y )
{
	const ImUiPosition result = { pos.x + x, pos.y + y };
	return result;
}

ImUiPosition ImUiPositionAddPos( ImUiPosition pos, ImUiPosition add )
{
	const ImUiPosition result = { pos.x + add.x, pos.y + add.y };
	return result;
}

ImUiSize ImUiSizeCreate( float width, float height )
{
	const ImUiSize size = { width, height };
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

ImUiSize ImUiSizeCreateAll( float value )
{
	const ImUiSize size = { value, value };
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

ImUiSize ImUiSizeShrinkThickness( ImUiSize size, ImUiThickness thickness )
{
	const ImUiSize result =
	{
		size.width - (thickness.left + thickness.right),
		size.height - (thickness.top + thickness.bottom)
	};
	return result;
}

ImUiSize ImUiSizeExpandThickness( ImUiSize size, ImUiThickness thickness )
{
	const ImUiSize result =
	{
		size.width + thickness.left + thickness.right,
		size.height + thickness.top + thickness.bottom
	};
	return result;
}

ImUiThickness ImUiThicknessCreate( float top, float left, float bottom, float right )
{
	const ImUiThickness thickness = { top, left, bottom, right };
	return thickness;
}

ImUiThickness ImUiThicknessCreateAll( float all )
{
	const ImUiThickness thickness = { all, all, all, all };
	return thickness;
}

ImUiThickness ImUiThicknessCreateVerticalHorizontal( float vertical, float horizontal )
{
	const ImUiThickness thickness = { vertical, horizontal, vertical, horizontal };
	return thickness;
}

ImUiRectangle ImUiRectangleCreate( float x, float y, float width, float height )
{
	const ImUiRectangle rect = { { x, y }, { width, height } };
	return rect;
}

ImUiRectangle ImUiRectangleCreatePos( ImUiPosition pos, float width, float height )
{
	const ImUiRectangle rect = { pos, { width, height } };
	return rect;
}

ImUiRectangle ImUiRectangleCreateSize( float x, float y, ImUiSize size )
{
	const ImUiRectangle rect = { { x, y }, size };
	return rect;
}

ImUiRectangle ImUiRectangleCreatePosSize( ImUiPosition pos, ImUiSize size )
{
	const ImUiRectangle rect = { pos, size };
	return rect;
}

ImUiRectangle ImUiRectangleCreateCenter( float x, float y, float width, float height )
{
	const float halfWidth	= width * 0.5f;
	const float halfHeight	= height * 0.5f;
	const ImUiRectangle rect = { { x - halfWidth, y - halfHeight }, { width, height } };
	return rect;
}

ImUiRectangle ImUiRectangleCreateCenterPos( ImUiPosition pos, float width, float height )
{
	return ImUiRectangleCreateCenter( pos.x, pos.y, width, height );
}

ImUiRectangle ImUiRectangleCreateCenterSize( float x, float y, ImUiSize size )
{
	return ImUiRectangleCreateCenter( x, y, size.width, size.height );
}

ImUiRectangle ImUiRectangleCreateCenterPosSize( ImUiPosition pos, ImUiSize size )
{
	return ImUiRectangleCreateCenter( pos.x, pos.y, size.width, size.height );
}

ImUiRectangle ImUiRectangleShrinkThickness( ImUiRectangle rect, ImUiThickness thickness )
{
	const ImUiRectangle result =
	{
		{ rect.position.x + thickness.left, rect.position.y + thickness.top},
		{ rect.size.width - thickness.left - thickness.right, rect.size.height - thickness.top - thickness.bottom }
	};
	return result;
}

bool ImUiRectangleIncludesPosition( ImUiRectangle rect, ImUiPosition position )
{
	return rect.position.x >= position.x &&
		rect.position.y >= position.y &&
		rect.position.x + rect.size.width <= position.x &&
		rect.position.y + rect.size.height <= position.y;
}

bool ImUiRectangleIntersectsRectangle( ImUiRectangle rect1, ImUiRectangle rect2 )
{
	return rect2.position.y < rect1.position.y + rect1.size.height &&
		rect2.position.y + rect2.size.height > rect1.position.y &&
		rect2.position.x < rect1.position.x + rect1.size.width &&
		rect2.position.x + rect2.size.width > rect1.position.x;
}

ImUiPosition ImUiRectangleGetTopLeft( ImUiRectangle rect )
{
	return rect.position;
}

ImUiPosition ImUiRectangleGetTopRight( ImUiRectangle rect )
{
	return ImUiPositionAdd( rect.position, rect.size.width, 0.0f );
}

ImUiPosition ImUiRectangleGetBottomLeft( ImUiRectangle rect )
{
	return ImUiPositionAdd( rect.position, 0.0f, rect.size.height );
}

ImUiPosition ImUiRectangleGetBottomRight( ImUiRectangle rect )
{
	return ImUiPositionAdd( rect.position, rect.size.width, rect.size.height );
}

ImUiColor ImUiColorCreate( float red, float green, float blue, float alpha )
{
	const ImUiColor color = { red, green, blue, alpha };
	return color;
}

ImUiColor ImUiColorCreateBlack( float alpha )
{
	return ImUiColorCreate( 0.0f, 0.0f, 0.0f, alpha );
}

ImUiColor ImUiColorCreateWhite( float alpha )
{
	return ImUiColorCreate( 1.0f, 1.0f, 1.0f, alpha );
}

ImUiColor ImUiColorCreateTransparentBlack()
{
	return ImUiColorCreate( 0.0f, 0.0f, 0.0f, 0.0f );
}
