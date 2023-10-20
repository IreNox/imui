#pragma once

#include "imui/imui.h"

#include "imui_types.h"

typedef struct ImUiDrawSurfaceData ImUiDrawSurfaceData;
typedef struct ImUiDrawWindowData ImUiDrawWindowData;

typedef enum ImUiDrawElementType
{
	ImUiDrawElementType_Line,
	ImUiDrawElementType_Triangle,
	ImUiDrawElementType_Rect,
	ImUiDrawElementType_RectPartial,
	ImUiDrawElementType_Skin,
	ImUiDrawElementType_SkinPartial,
	ImUiDrawElementType_Text,
	ImUiDrawElementType_TextOffset,
} ImUiDrawElementType;

typedef struct ImUiDrawElementDataPrimitive
{
	ImUiPos				p0;
	ImUiPos				p1;
	ImUiPos				p2;
	ImUiColor			color;
} ImUiDrawElementDataPrimitive;

typedef struct ImUiDrawElementDataRect
{
	ImUiRect			relativRect;
	ImUiColor			color;
	ImUiTexCoord		uv;
} ImUiDrawElementDataRect;

typedef struct ImUiDrawElementDataSkin
{
	ImUiRect			relativRect;
	ImUiBorder			border;
	ImUiTexCoord		uv;
	ImUiSize			texSize;
	ImUiColor			color;
} ImUiDrawElementDataSkin;

typedef struct ImUiDrawElementDataText
{
	ImUiPos				relativPos;
	ImUiColor			color;
	ImUiTextLayout*		layout;
} ImUiDrawElementDataText;

typedef union ImUiDrawElementData
{
	ImUiDrawElementDataPrimitive	primitive;
	ImUiDrawElementDataRect			rect;
	ImUiDrawElementDataSkin			skin;
	ImUiDrawElementDataText			text;
} ImUiDrawElementData;

typedef struct ImUiDrawElement
{
	ImUiDrawElementType		type;
	ImUiDrawElementData		data;
	ImUiWidget*				widget;
	void*					texture;
} ImUiDrawElement;

struct ImUiDraw
{
	ImUiAllocator*			allocator;
	ImUiVertexFormat		vertexFormat;
	uintsize				vertexSize;
	ImUiVertexType			vertexType;
	ImUiDrawTopology		triangleTopology;

	ImUiDrawSurfaceData*	surfaces;
	uintsize				surfaceCapacity;
	uintsize				surfaceCount;
	ImUiDrawWindowData*		windows;
	uintsize				windowCapacity;
	uintsize				windowCount;
};

bool				ImUiDrawConstruct( ImUiDraw* draw, ImUiAllocator* allocator, const ImUiVertexFormat* vertexFormat, ImUiVertexType vertexType );
void				ImUiDrawDestruct( ImUiDraw* draw );

uintsize			ImUiDrawRegisterSurface( ImUiDraw* draw, ImUiStringView name, ImUiSize size );
uintsize			ImUiDrawRegisterWindow( ImUiDraw* draw, ImUiStringView name, uintsize surfaceIndex, uint32 zOrder );
void				ImUiDrawSurfaceEnd( ImUiDraw* draw, uintsize surfaceIndex );
void				ImUiDrawEndFrame( ImUiDraw* draw );

ImUiDrawElement*	ImUiDrawPushElement( ImUiWidget* widget, ImUiDrawElementType type, void* texture );
ImUiDrawElement*	ImUiDrawPushElementText( ImUiWidget* widget, ImUiDrawElementType type, ImUiTextLayout* layout );

void				ImUiDrawGetSurfaceMaxBufferSizes( ImUiDraw* draw, uintsize surfaceIndex, size_t* outVertexDataSize, size_t* outIndexDataSize );
const ImUiDrawData*	ImUiDrawGenerateSurfaceData( ImUiDraw* draw, uintsize surfaceIndex, void* outVertexData, size_t* inOutVertexDataSize, void* outIndexData, size_t* inOutIndexDataSize );
