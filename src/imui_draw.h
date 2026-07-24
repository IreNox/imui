#pragma once

#include "imui/imui.h"

#include "imui_types.h"

typedef struct ImuiDrawSurfaceData ImuiDrawSurfaceData;
typedef struct ImuiDrawWindowData ImuiDrawWindowData;

typedef enum ImuiDrawElementType
{
	ImuiDrawElementType_Line,
	ImuiDrawElementType_Triangle,
	ImuiDrawElementType_Rect,
	ImuiDrawElementType_RectPartial,
	ImuiDrawElementType_Skin,
	ImuiDrawElementType_SkinPartial,
	ImuiDrawElementType_Text,
	ImuiDrawElementType_TextOffset,
} ImuiDrawElementType;

typedef struct ImuiDrawElementDataPrimitive
{
	ImuiPos				p0;
	ImuiPos				p1;
	ImuiPos				p2;
	ImuiColor			color;
} ImuiDrawElementDataPrimitive;

typedef struct ImuiDrawElementDataRect
{
	ImuiRect			relativRect;
	ImuiColor			color;
	ImuiTexCoord		uv;
} ImuiDrawElementDataRect;

// must have the same layout as imuiDrawElementDataRect
typedef struct ImuiDrawElementDataSkin
{
	ImuiRect			relativRect;
	ImuiColor			color;
	ImuiTexCoord		uv;
	ImuiBorder			border;
	ImuiSize			texSize;
} ImuiDrawElementDataSkin;

typedef struct ImuiDrawElementDataText
{
	ImuiPos				relativPos;
	ImuiColor			color;
	ImuiTextLayout*		layout;
	float				size;
} ImuiDrawElementDataText;

typedef union ImuiDrawElementData
{
	ImuiDrawElementDataPrimitive	primitive;
	ImuiDrawElementDataRect			rect;
	ImuiDrawElementDataSkin			skin;
	ImuiDrawElementDataText			text;
} ImuiDrawElementData;

typedef struct ImuiDrawElement
{
	ImuiDrawElementType		type;
	ImuiDrawElementData		data;
	ImuiWidget*				widget;
	uint64_t				textureHandle;
} ImuiDrawElement;

struct ImuiDraw
{
	ImuiAllocator*			allocator;
	ImuiVertexFormat		vertexFormat;
	uintsize				vertexSize;
	ImuiVertexType			vertexType;
	ImuiDrawTopology		triangleTopology;

	ImuiDrawSurfaceData*	surfaces;
	uintsize				surfaceCapacity;
	uintsize				surfaceCount;
	ImuiDrawWindowData*		windows;
	uintsize				windowCapacity;
	uintsize				windowCount;
};

bool				imuiDrawConstruct( ImuiDraw* draw, ImuiAllocator* allocator, const ImuiVertexFormat* vertexFormat, ImuiVertexType vertexType );
void				imuiDrawDestruct( ImuiDraw* draw );

uintsize			imuiDrawRegisterSurface( ImuiDraw* draw, ImuiStringView name, ImuiSize size );
uintsize			imuiDrawRegisterWindow( ImuiDraw* draw, ImuiStringView name, uintsize surfaceIndex, uint32 zOrder );
void				imuiDrawSurfaceEnd( ImuiDraw* draw, uintsize surfaceIndex );
void				imuiDrawEndFrame( ImuiDraw* draw );

ImuiDrawElement*	imuiDrawPushElement( ImuiWidget* widget, ImuiDrawElementType type, uint64_t textureHandle );
ImuiDrawElement*	imuiDrawPushElementText( ImuiWidget* widget, ImuiDrawElementType type, ImuiTextLayout* layout );

void				imuiDrawGetSurfaceMaxBufferSizes( ImuiDraw* draw, uintsize surfaceIndex, size_t* outVertexDataSize, size_t* outIndexDataSize );
const ImuiDrawData*	imuiDrawGenerateSurfaceData( ImuiDraw* draw, uintsize surfaceIndex, void* outVertexData, size_t* inOutVertexDataSize, void* outIndexData, size_t* inOutIndexDataSize );
