#pragma once

#include "imui_types.h"

typedef enum ImUiDrawElementType ImUiDrawElementType;
enum ImUiDrawElementType
{
	ImUiDrawElementType_Line,
	ImUiDrawElementType_Rectangle,
	ImUiDrawElementType_Skin,
};

struct ImUiDrawElementDataLine
{
	ImUiPosition	p0;
	ImUiPosition	p1;
	ImUiColor		color;
};

struct ImUiDrawElementDataRectangle
{
	ImUiRectangle			rect;
	ImUiColor				color;
	ImUiTextureCooridinate	uv;
};

typedef union ImUiDrawElementData ImUiDrawElementData;
union ImUiDrawElementData
{
	struct ImUiDrawElementDataLine		line;
	struct ImUiDrawElementDataRectangle	rectangle;
};

typedef struct ImUiDrawElement ImUiDrawElement;
struct ImUiDrawElement
{
	ImUiDrawElementType	type;
	ImUiDrawElementData	data;
	void*				texture;
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
	uint8				lastTopology;
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
	bool					useIndexBuffer;

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
