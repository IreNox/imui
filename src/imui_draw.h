#pragma once

#include "imui/imui.h"

#include "imui_types.h"

typedef enum ImUiDrawElementType
{
	ImUiDrawElementType_Line,
	ImUiDrawElementType_Rectangle,
	ImUiDrawElementType_Skin,
	ImUiDrawElementType_Text,
} ImUiDrawElementType;

struct ImUiDrawElementDataLine
{
	ImUiPos			p0;
	ImUiPos			p1;
	ImUiColor		color;
};

struct ImUiDrawElementDataRectangle
{
	ImUiRect		rect;
	ImUiColor		color;
	ImUiTexCoord	uv;
};

struct ImUiDrawElementDataSkin
{
	ImUiRect		rect;
	ImUiBorder		border;
	ImUiTexCoord	uv;
	ImUiSize		texSize;
	ImUiColor		color;
};

struct ImUiDrawElementDataText
{
	ImUiPos			pos;
	ImUiColor		color;
	ImUiTextLayout*	layout;
};

typedef union ImUiDrawElementData ImUiDrawElementData;
union ImUiDrawElementData
{
	struct ImUiDrawElementDataLine		line;
	struct ImUiDrawElementDataRectangle	rect;
	struct ImUiDrawElementDataSkin		skin;
	struct ImUiDrawElementDataText		text;
};

typedef struct ImUiDrawElement ImUiDrawElement;
struct ImUiDrawElement
{
	ImUiDrawElementType	type;
	ImUiDrawElementData	data;
	void*				texture;
	ImUiRect			clipRect;
};

typedef struct ImUiDrawWindowData ImUiDrawWindowData;
struct ImUiDrawWindowData
{
	ImUiHash			id;
	bool				used;

	ImUiDrawElement*	elements;
	uintsize			elementCapacity;
	uintsize			elementCount;
};

typedef struct ImUiDrawSurfaceData ImUiDrawSurfaceData;
struct ImUiDrawSurfaceData
{
	ImUiSurface*		surface;
	bool				used;

	ImUiDrawCommand*	commands;
	uintsize			commandCapacity;
	uintsize			commandCount;

	uint8*				vertexData;
	uintsize			vertexDataCapacity;
	uintsize			vertexCount;

	uint32*				indices;
	uintsize			indicesCapacity;
	uintsize			indexCount;

	ImUiDrawData		data;
};

struct ImUiDraw
{
	ImUiAllocator*			allocator;
	ImUiVertexFormat		vertexFormat;
	uintsize				vertexSize;
	ImUiVertexType			vertexType;
	ImUiDrawTopology		triangleTopology;

	ImUiDrawWindowData*		windows;
	uintsize				windowCapacity;
	uintsize				windowCount;

	ImUiDrawSurfaceData*	surfaces;
	uintsize				surfaceCapacity;
	uintsize				surfaceCount;
};

bool				ImUiDrawConstruct( ImUiDraw* draw, ImUiAllocator* allocator, const ImUiVertexFormat* vertexFormat, ImUiVertexType vertexType );
void				ImUiDrawDestruct( ImUiDraw* draw );

uintsize			ImUiDrawRegisterWindow( ImUiDraw* draw, ImUiHash id );
void				ImUiDrawEndFrame( ImUiDraw* draw );

const ImUiDrawData*	ImUiDrawGenerateSurfaceData( ImUiDraw* draw, ImUiSurface* surface );

// more in draw section of imui.h
