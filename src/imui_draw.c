#include "imui_draw.h"

#include "imui_memory.h"

#include <string.h>

static void					ImUiDrawFreeWindow( ImUiDraw* draw, ImUiDrawWindowData* window );
static void					ImUiDrawFreeSurface( ImUiDraw* draw, ImUiDrawSurfaceData* surface );
static ImUiDrawWindowData*	ImUiDrawGetWindow( ImUiDraw* draw, ImUiWidget* widget );
static ImUiDrawElement*		ImUiDrawPushElement( ImUiWidget* widget, ImUiDrawElementType type, void* texture );
static void					ImUiDrawSurfaceGenerateElementData( ImUiDraw* draw, ImUiDrawSurfaceData* surface, const ImUiDrawElement* element );
static ImUiDrawCommand*		ImUiDrawSurfaceGetElementCommand( ImUiDraw* draw, ImUiDrawSurfaceData* surface, const ImUiDrawElement* element );
static bool					ImUiDrawSurfacePreparePushVertices( ImUiDraw* draw, ImUiDrawSurfaceData* surface, uintsize vertexCount );
static uint32				ImUiDrawSurfacePushVertex( ImUiDraw* draw, ImUiDrawSurfaceData* surface, ImUiPosition position, ImUiColor color, ImUiPosition uv );
static void					ImUiDrawSurfacePushIndices( ImUiDraw* draw, ImUiDrawSurfaceData* surface, const uint32* indices, uintsize count );
static uintsize				ImUiVertexElementTypeGetSize( ImUiVertexType type );

bool ImUiDrawConstruct( ImUiDraw* draw, ImUiAllocator* allocator, const ImUiVertexFormat* vertexFormat, ImUiVertexType vertexType )
{
	draw->allocator		= allocator;
	draw->vertexType	= vertexType;

	switch( vertexType )
	{
	case ImUiVertexType_VertexList:			draw->triangleTopology = ImUiDrawTopology_TriangleList; break;
	case ImUiVertexType_VertexStrip:		draw->triangleTopology = ImUiDrawTopology_TriangleStrip; break;
	case ImUiVertexType_IndexedVertexList:	draw->triangleTopology = ImUiDrawTopology_IndexedTriangleList; draw->useIndexBuffer = true; break;
	case ImUiVertexType_IndexedVertexStrip:	draw->triangleTopology = ImUiDrawTopology_IndexedTriangleStrip; draw->useIndexBuffer = true; break;
	}

	const bool hasVertexFormat = vertexFormat->elementCount != 0u;
	const uintsize vertexElementCount = hasVertexFormat ? vertexFormat->elementCount : 3u;
	ImUiVertexElement* elements = IMUI_MEMORY_ARRAY_NEW( allocator, ImUiVertexElement, vertexFormat->elementCount );
	if( !elements  )
	{
		ImUiDrawDestruct( draw );
		return false;
	}

	if( hasVertexFormat )
	{
		memcpy( elements, vertexFormat->elements, sizeof( ImUiVertexElement ) * vertexFormat->elementCount );
	}
	else
	{
		elements[ 0u ].alignment	= 1u;
		elements[ 0u ].type			= ImUiVertexElementType_Float2;
		elements[ 0u ].semantic		= ImUiVertexElementSemantic_PositionScreenSpace;
		elements[ 1u ].alignment	= 1u;
		elements[ 1u ].type			= ImUiVertexElementType_Float2;
		elements[ 1u ].semantic		= ImUiVertexElementSemantic_TextureCoordinate;
		elements[ 2u ].alignment	= 1u;
		elements[ 2u ].type			= ImUiVertexElementType_Float4;
		elements[ 2u ].semantic		= ImUiVertexElementSemantic_Color;
	}

	draw->vertexFormat.elements		= elements;
	draw->vertexFormat.elementCount	= vertexElementCount;

	for( uintsize i = 0u; i < vertexElementCount; ++i )
	{
		const ImUiVertexElement* vertexElement = &elements[ i ];

		draw->vertexSize = (draw->vertexSize + vertexElement->alignment - 1u) & (0u - vertexElement->alignment);
		draw->vertexSize += ImUiVertexElementTypeGetSize( vertexElement->type );
	}

	return true;
}

void ImUiDrawDestruct( ImUiDraw* draw )
{
	for( uintsize i = 0; i < draw->windowCapacity; ++i )
	{
		ImUiDrawWindowData* window = &draw->windows[ i ];
		ImUiDrawFreeWindow( draw, window );
	}
	ImUiMemoryFree( draw->allocator, draw->windows );

	for( uintsize i = 0; i < draw->surfaceCapacity; ++i )
	{
		ImUiDrawSurfaceData* surface = &draw->surfaces[ i ];
		ImUiDrawFreeSurface( draw, surface );
	}
	ImUiMemoryFree( draw->allocator, draw->surfaces );

	ImUiMemoryFree( draw->allocator, draw->vertexFormat.elements );
}

uintsize ImUiDrawRegisterWindow( ImUiDraw* draw, ImUiHash id )
{
	for( uintsize i = 0; i < draw->windowCount; ++i )
	{
		ImUiDrawWindowData* window = &draw->windows[ i ];
		if( window->id != id )
		{
			continue;
		}

		window->used = true;
		return i;
	}

	if( !IMUI_MEMORY_CHECK_ARRAY_CAPACITY_ZERO( draw->allocator, draw->windows, draw->windowCapacity, draw->windowCount + 1u ) )
	{
		return IMUI_SIZE_MAX;
	}

	const uintsize windowIndex = draw->windowCount++;
	ImUiDrawWindowData* window = &draw->windows[ windowIndex ];
	window->id		= id;
	window->used	= true;

	return windowIndex;
}

void ImUiDrawEndFrame( ImUiDraw* draw )
{
	for( uintsize i = 0; i < draw->windowCount; ++i )
	{
		ImUiDrawWindowData* window = &draw->windows[ i ];

		if( !window->used )
		{
			ImUiDrawFreeWindow( draw, window );

			if( draw->windowCount > 1u )
			{
				*window = draw->windows[ draw->windowCount - 1u ];
			}
			draw->windowCount--;

			i--;
			continue;
		}

		window->used = false;
	}

	for( uintsize i = 0; i < draw->surfaceCount; ++i )
	{
		ImUiDrawSurfaceData* surface = &draw->surfaces[ i ];

		if( !surface->used )
		{
			ImUiDrawFreeSurface( draw, surface );

			if( draw->surfaceCount > 1u )
			{
				*surface = draw->surfaces[ draw->surfaceCount - 1u ];
			}
			draw->surfaceCount--;

			i--;
			continue;
		}

		surface->used = false;
	}
}

const ImUiDrawData* ImUiDrawGenerateSurfaceData( ImUiDraw* draw, ImUiSurface* surface )
{
	ImUiDrawSurfaceData* drawSurface = NULL;
	for( uintsize i = 0; i < draw->surfaceCount; ++i )
	{
		ImUiDrawSurfaceData* currentSurface = &draw->surfaces[ i ];
		if( currentSurface->surface != surface )
		{
			continue;
		}

		drawSurface = currentSurface;
		break;
	}

	if( !drawSurface )
	{
		if( !IMUI_MEMORY_CHECK_ARRAY_CAPACITY_ZERO( draw->allocator, draw->surfaces, draw->surfaceCapacity, draw->surfaceCount + 1u ) )
		{
			return NULL;
		}

		drawSurface = &draw->surfaces[ draw->surfaceCount++ ];
		drawSurface->surface = surface;
	}

	drawSurface->lastTopology	= ImUiDrawTopology_MAX;
	drawSurface->used			= true;

	for( uintsize windowIndex = 0u; windowIndex < surface->windowCount; ++windowIndex )
	{
		const uintsize drawIndex = surface->windows[ windowIndex ].drawIndex;
		const ImUiDrawWindowData* window = &draw->windows[ drawIndex ];

		for( uintsize elementIndex = 0u; elementIndex < window->elementCount; ++elementIndex )
		{
			const ImUiDrawElement* element = &window->elements[ elementIndex ];
			ImUiDrawSurfaceGenerateElementData( draw, drawSurface, element );
		}
	}

	ImUiDrawData* data = &drawSurface->data;
	data->commands			= drawSurface->commands;
	data->commandCount		= drawSurface->commandCount;
	data->vertexData		= drawSurface->vertexData;
	data->vertexDataSize	= draw->vertexSize * drawSurface->vertexCount;
	data->indexData			= drawSurface->indices;
	data->indexCount		= drawSurface->indexCount;

	return &drawSurface->data;
}

void ImUiDrawLine( ImUiWidget* widget, ImUiPosition p0, ImUiPosition p1, ImUiColor color )
{
	ImUiDrawElement* element = ImUiDrawPushElement( widget, ImUiDrawElementType_Line, NULL );
	struct ImUiDrawElementDataLine* lineData = &element->data.line;
	lineData->p0	= p0;
	lineData->p1	= p1;
	lineData->color	= color;
}

void ImUiDrawRectangleColor( ImUiWidget* widget, ImUiRectangle rect, ImUiColor color )
{
	ImUiDrawElement* element = ImUiDrawPushElement( widget, ImUiDrawElementType_Rectangle, NULL );
	struct ImUiDrawElementDataRectangle* rectData = &element->data.rectangle;
	rectData->rect		= rect;
	rectData->color		= color;
	memset( &rectData->uv, 0, sizeof( rectData->uv ) );
}

void ImUiDrawRectangleTexture( ImUiWidget* widget, ImUiRectangle rect, void* texture )
{
	ImUiDrawElement* element = ImUiDrawPushElement( widget, ImUiDrawElementType_Rectangle, texture );
	struct ImUiDrawElementDataRectangle* rectData = &element->data.rectangle;
	rectData->rect		= rect;
	rectData->uv.u0		= 0.0f;
	rectData->uv.v0		= 0.0f;
	rectData->uv.u1		= 1.0f;
	rectData->uv.v1		= 1.0f;
}

void ImUiDrawRectangleTextureUv( ImUiWidget* widget, ImUiRectangle rect, void* texture, ImUiTextureCooridinate uv )
{
	ImUiDrawElement* element = ImUiDrawPushElement( widget, ImUiDrawElementType_Rectangle, texture );
	struct ImUiDrawElementDataRectangle* rectData = &element->data.rectangle;
	rectData->rect		= rect;
	rectData->color		= ImUiColorCreateWhite( 1.0f );
	rectData->uv		= uv;
}

static void ImUiDrawFreeWindow( ImUiDraw* draw, ImUiDrawWindowData* window )
{
	ImUiMemoryFree( draw->allocator, window->elements );
	window->elements = NULL;
}

static void ImUiDrawFreeSurface( ImUiDraw* draw, ImUiDrawSurfaceData* surface )
{
	ImUiMemoryFree( draw->allocator, surface->commands );
	surface->commands = NULL;

	ImUiMemoryFree( draw->allocator, surface->vertexData );
	surface->vertexData = NULL;

	ImUiMemoryFree( draw->allocator, surface->indices );
	surface->indices = NULL;
}

static ImUiDrawWindowData* ImUiDrawGetWindow( ImUiDraw* draw, ImUiWidget* widget )
{
	return &draw->windows[ widget->window->drawIndex ];
}

static ImUiDrawElement* ImUiDrawPushElement( ImUiWidget* widget, ImUiDrawElementType type, void* texture )
{
	ImUiDraw* draw = &widget->window->surface->imui->draw;
	ImUiDrawWindowData* window = ImUiDrawGetWindow( draw, widget );

	if( !IMUI_MEMORY_CHECK_ARRAY_CAPACITY( draw->allocator, window->elements, window->elementCapacity, window->elementCount + 1u ) )
	{
		return NULL;
	}

	ImUiDrawElement* element = &window->elements[ window->elementCount++ ];
	element->type		= type;
	element->texture	= texture;

	return element;
}

static void ImUiDrawSurfaceGenerateElementData( ImUiDraw* draw, ImUiDrawSurfaceData* surface, const ImUiDrawElement* element )
{
	ImUiDrawCommand* command = ImUiDrawSurfaceGetElementCommand( draw, surface, element );

	uintsize elementCount = 0u;
	switch( element->type )
	{
	case ImUiDrawElementType_Line:
		{
			const struct ImUiDrawElementDataLine* lineData = &element->data.line;
			const ImUiPosition uv = { 0.0f, 0.0f };
			uint32 vertexIndices[ 2u ];

			ImUiDrawSurfacePreparePushVertices( draw, surface, 2u );
			vertexIndices[ 0u ] = ImUiDrawSurfacePushVertex( draw, surface, lineData->p0, lineData->color, uv );
			vertexIndices[ 1u ] = ImUiDrawSurfacePushVertex( draw, surface, lineData->p1, lineData->color, uv );

			if( draw->useIndexBuffer )
			{
				ImUiDrawSurfacePushIndices( draw, surface, vertexIndices, IMUI_ARRAY_COUNT( vertexIndices ) );
			}

			elementCount = 2u;
		}
		break;

	case ImUiDrawElementType_Rectangle:
		{
			const struct ImUiDrawElementDataRectangle* rectData = &element->data.rectangle;

			const ImUiPosition posTl = ImUiRectangleGetTopLeft( rectData->rect );
			const ImUiPosition posTr = ImUiRectangleGetTopRight( rectData->rect );
			const ImUiPosition posBl = ImUiRectangleGetBottomLeft( rectData->rect );
			const ImUiPosition posBr = ImUiRectangleGetBottomRight( rectData->rect );
			const ImUiPosition uvTl = ImUiPositionCreate( rectData->uv.u0, rectData->uv.v0 );
			const ImUiPosition uvTr = ImUiPositionCreate( rectData->uv.u1, rectData->uv.v0 );
			const ImUiPosition uvBl = ImUiPositionCreate( rectData->uv.u0, rectData->uv.v1 );
			const ImUiPosition uvBr = ImUiPositionCreate( rectData->uv.u1, rectData->uv.v1 );
			if( draw->useIndexBuffer )
			{
				ImUiDrawSurfacePreparePushVertices( draw, surface, 4u );
				const uint32 indexTl = ImUiDrawSurfacePushVertex( draw, surface, posTl, rectData->color, uvTl );
				const uint32 indexTr = ImUiDrawSurfacePushVertex( draw, surface, posTr, rectData->color, uvTr );
				const uint32 indexBl = ImUiDrawSurfacePushVertex( draw, surface, posBl, rectData->color, uvBl );
				const uint32 indexBr = ImUiDrawSurfacePushVertex( draw, surface, posBr, rectData->color, uvBr );

				const uint32 vertexIndices[ 6u ] =
				{
					indexTl, indexTr, indexBl,
					indexBl, indexTr, indexBr
				};
				ImUiDrawSurfacePushIndices( draw, surface, vertexIndices, IMUI_ARRAY_COUNT( vertexIndices ) );
			}
			else
			{
				ImUiDrawSurfacePreparePushVertices( draw, surface, 6u );
				ImUiDrawSurfacePushVertex( draw, surface, posTl, rectData->color, uvTl );
				ImUiDrawSurfacePushVertex( draw, surface, posTr, rectData->color, uvTr );
				ImUiDrawSurfacePushVertex( draw, surface, posBl, rectData->color, uvBl );
				ImUiDrawSurfacePushVertex( draw, surface, posBl, rectData->color, uvBl );
				ImUiDrawSurfacePushVertex( draw, surface, posTr, rectData->color, uvTr );
				ImUiDrawSurfacePushVertex( draw, surface, posBr, rectData->color, uvBr );
			}

			elementCount = 6u;
		}
		break;

	case ImUiDrawElementType_Skin:
		{
			// todo
		}
		break;
	}

	command->count += elementCount;
}

static ImUiDrawCommand* ImUiDrawSurfaceGetElementCommand( ImUiDraw* draw, ImUiDrawSurfaceData* surface, const ImUiDrawElement* element )
{
	const ImUiDrawTopology topology = element->type == ImUiDrawElementType_Line ? ImUiDrawTopology_LineList : draw->triangleTopology;
	if( topology == surface->lastTopology )
	{
		ImUiDrawCommand* command = &surface->commands[ surface->commandCount - 1u ];
		if( command->texture == element->texture )
		{
			return command;
		}
	}

	if( !IMUI_MEMORY_CHECK_ARRAY_CAPACITY_ZERO( draw->allocator, surface->commands, surface->commandCapacity, surface->commandCount + 1u ) )
	{
		return NULL;
	}

	ImUiDrawCommand* command = &surface->commands[ surface->commandCount++ ];
	command->topology	= topology;
	command->texture	= element->texture;
	command->offset		= draw->useIndexBuffer ? surface->indexCount : surface->vertexCount;
	command->count		= 0u;

	return command;
}

static bool ImUiDrawSurfacePreparePushVertices( ImUiDraw* draw, ImUiDrawSurfaceData* surface, uintsize vertexCount )
{
	const uintsize requiredSize = draw->vertexSize * (surface->vertexCount + vertexCount);
	return IMUI_MEMORY_CHECK_ARRAY_CAPACITY( draw->allocator, surface->vertexData, surface->vertexDataCapacity, requiredSize );
}

static uint32 ImUiDrawSurfacePushVertex( ImUiDraw* draw, ImUiDrawSurfaceData* surface, ImUiPosition position, ImUiColor color, ImUiPosition uv )
{
	const uint32 vertexIndex = (uint32)surface->vertexCount;
	surface->vertexCount++;

	uint32 vertexOffset = 0u;
	uint8* vertex = surface->vertexData + (draw->vertexSize * vertexIndex);
	IMUI_ASSERT( vertex + draw->vertexSize <= surface->vertexData + surface->vertexDataCapacity );

	for( uintsize i = 0u; i < draw->vertexFormat.elementCount; ++i )
	{
		const ImUiVertexElement* vertexElement = &draw->vertexFormat.elements[ i ];

		vertexOffset = (vertexOffset + vertexElement->alignment - 1u) & (0u - vertexElement->alignment);

		uint8* elementData = vertex + vertexOffset;
		switch( vertexElement->semantic )
		{
		case ImUiVertexElementSemantic_None:
			break;

		case ImUiVertexElementSemantic_PositionScreenSpace:
			{
				if( vertexElement->type >= ImUiVertexElementType_Float2 && vertexElement->type <= ImUiVertexElementType_Float4 )
				{
					float* floatData = (float*)elementData;
					floatData[ 0u ] = position.x;
					floatData[ 1u ] = position.y;

					if( vertexElement->type >= ImUiVertexElementType_Float3 )
					{
						floatData[ 2u ] = 0.0f;
					}

					if( vertexElement->type == ImUiVertexElementType_Float4 )
					{
						floatData[ 3u ] = 0.0f;
					}
				}
				else if( vertexElement->type >= ImUiVertexElementType_Int2 && vertexElement->type <= ImUiVertexElementType_Int4 )
				{
					sint32* intData = (sint32*)elementData;
					intData[ 0u ] = (sint32)position.x;
					intData[ 1u ] = (sint32)position.y;

					if( vertexElement->type >= ImUiVertexElementType_Int3 )
					{
						intData[ 2u ] = 0;
					}

					if( vertexElement->type >= ImUiVertexElementType_Int4 )
					{
						intData[ 3u ] = 0;
					}
				}
				else if( vertexElement->type >= ImUiVertexElementType_UInt2 && vertexElement->type <= ImUiVertexElementType_UInt4 )
				{
					uint32* uintData = (uint32*)elementData;
					uintData[ 0u ] = (uint32)position.x;
					uintData[ 1u ] = (uint32)position.y;

					if( vertexElement->type >= ImUiVertexElementType_UInt3 )
					{
						uintData[ 2u ] = 0u;
					}

					if( vertexElement->type == ImUiVertexElementType_UInt4 )
					{
						uintData[ 3u ] = 0u;
					}
				}
			}
			break;

		case ImUiVertexElementSemantic_PositionClipSpace:
			{
				ImUiPosition clipSpacePosition =
				{
					-1.0f + (position.x / surface->surface->size.width),
					 1.0f - (position.y / surface->surface->size.height),
				};
				if( vertexElement->type >= ImUiVertexElementType_Float2 && vertexElement->type <= ImUiVertexElementType_Float4 )
				{
					float* floatData = (float*)elementData;
					floatData[ 0u ] = clipSpacePosition.x;
					floatData[ 1u ] = clipSpacePosition.y;

					if( vertexElement->type >= ImUiVertexElementType_Float3 )
					{
						floatData[ 2u ] = 0.0f;
					}

					if( vertexElement->type == ImUiVertexElementType_Float4 )
					{
						floatData[ 3u ] = 0.0f;
					}
				}
				else if( vertexElement->type >= ImUiVertexElementType_Int2 && vertexElement->type <= ImUiVertexElementType_Int4 )
				{
					sint32* intData = (sint32*)elementData;
					intData[ 0u ] = (sint32)(clipSpacePosition.x * 2147483647.0f + 0.5f);
					intData[ 1u ] = (sint32)(clipSpacePosition.y * 2147483647.0f + 0.5f);

					if( vertexElement->type >= ImUiVertexElementType_Int3 )
					{
						intData[ 2u ] = 0;
					}

					if( vertexElement->type == ImUiVertexElementType_Int4 )
					{
						intData[ 3u ] = 0;
					}
				}
			}
			break;

		case ImUiVertexElementSemantic_TextureCoordinate:
			{
				if( vertexElement->type >= ImUiVertexElementType_Float2 && vertexElement->type <= ImUiVertexElementType_Float4 )
				{
					float* floatData = (float*)elementData;
					floatData[ 0u ] = uv.x;
					floatData[ 1u ] = uv.y;

					if( vertexElement->type >= ImUiVertexElementType_Float3 )
					{
						floatData[ 2u ] = 0.0f;
					}

					if( vertexElement->type == ImUiVertexElementType_Float4 )
					{
						floatData[ 3u ] = 0.0f;
					}
				}
				else if( vertexElement->type >= ImUiVertexElementType_Int2 && vertexElement->type <= ImUiVertexElementType_Int4 )
				{
					sint32* intData = (sint32*)elementData;
					intData[ 0u ] = (sint32)(uv.x * 2147483647.0f + 0.5f);
					intData[ 1u ] = (sint32)(uv.y * 2147483647.0f + 0.5f);

					if( vertexElement->type >= ImUiVertexElementType_Int3 )
					{
						intData[ 2u ] = 0;
					}

					if( vertexElement->type >= ImUiVertexElementType_Int4 )
					{
						intData[ 3u ] = 0;
					}
				}
				else if( vertexElement->type >= ImUiVertexElementType_UInt2 && vertexElement->type <= ImUiVertexElementType_UInt4 )
				{
					uint32* uintData = (uint32*)elementData;
					uintData[ 0u ] = (uint32)(uv.x * 4294967295.0f + 0.5f);
					uintData[ 1u ] = (uint32)(uv.y * 4294967295.0f + 0.5f);

					if( vertexElement->type >= ImUiVertexElementType_UInt3 )
					{
						uintData[ 2u ] = 0u;
					}

					if( vertexElement->type == ImUiVertexElementType_UInt4 )
					{
						uintData[ 3u ] = 0u;
					}
				}
			}
			break;

		case ImUiVertexElementSemantic_Color:
			{
				if( vertexElement->type >= ImUiVertexElementType_Float3 && vertexElement->type <= ImUiVertexElementType_Float4 )
				{
					float* floatData = (float*)elementData;
					floatData[ 0u ] = color.red;
					floatData[ 1u ] = color.green;
					floatData[ 2u ] = color.blue;

					if( vertexElement->type == ImUiVertexElementType_Float4 )
					{
						floatData[ 3u ] = color.alpha;
					}
				}
				else if( vertexElement->type >= ImUiVertexElementType_Int3 && vertexElement->type <= ImUiVertexElementType_Int4 )
				{
					sint32* intData = (sint32*)elementData;
					intData[ 0u ] = (sint32)(color.red * 2147483647.0f + 0.5f);
					intData[ 1u ] = (sint32)(color.green * 2147483647.0f + 0.5f);
					intData[ 2u ] = (sint32)(color.blue * 2147483647.0f + 0.5f);

					if( vertexElement->type == ImUiVertexElementType_Int4 )
					{
						intData[ 3u ] = (sint32)(color.alpha * 2147483647.0f + 0.5f);
					}
				}
				else if( vertexElement->type == ImUiVertexElementType_UInt  )
				{
					const uint32 uintR = (uint32)(color.red * 255.0f + 0.5f);
					const uint32 uintG = (uint32)(color.green * 255.0f + 0.5f);
					const uint32 uintB = (uint32)(color.blue * 255.0f + 0.5f);
					const uint32 uintA = (uint32)(color.alpha * 255.0f + 0.5f);

					*(uint32*)elementData = (uintR << 24u) | (uintG << 16u) | (uintB << 8u) | uintA;
				}
				else if( vertexElement->type >= ImUiVertexElementType_UInt3 && vertexElement->type <= ImUiVertexElementType_UInt4 )
				{
					uint32* uintData = (uint32*)elementData;
					uintData[ 0u ] = (uint32)(color.red * 4294967295.0f + 0.5f);
					uintData[ 1u ] = (uint32)(color.green * 4294967295.0f + 0.5f);
					uintData[ 2u ] = (uint32)(color.blue * 4294967295.0f + 0.5f);

					if( vertexElement->type == ImUiVertexElementType_UInt4 )
					{
						uintData[ 3u ] = (uint32)(color.alpha * 4294967295.0f + 0.5f);
					}
				}
			}
			break;
		}

		vertexOffset += ImUiVertexElementTypeGetSize( vertexElement->type );
	}

	return vertexIndex;
}

static void ImUiDrawSurfacePushIndices( ImUiDraw* draw, ImUiDrawSurfaceData* surface, const uint32* indices, uintsize count )
{
	const uintsize requiredSize = surface->indexCount + count;
	if( !IMUI_MEMORY_CHECK_ARRAY_CAPACITY( draw->allocator, surface->indices, surface->indicesCapacity, requiredSize ) )
	{
		return;
	}

	uint32* targetIndices = surface->indices + surface->indexCount;
	for( uintsize i = 0u; i < count; ++i )
	{
		targetIndices[ i ] = indices[ i ];
	}

	surface->indexCount += count;
}

static uintsize ImUiVertexElementTypeGetSize( ImUiVertexType type )
{
	switch( type )
	{
	case ImUiVertexElementType_Invalid:	return 0u;
	case ImUiVertexElementType_Float:	return 4u;
	case ImUiVertexElementType_Float2:	return 8u;
	case ImUiVertexElementType_Float3:	return 12u;
	case ImUiVertexElementType_Float4:	return 16u;
	case ImUiVertexElementType_Int:		return 4u;
	case ImUiVertexElementType_Int2:	return 8u;
	case ImUiVertexElementType_Int3:	return 12u;
	case ImUiVertexElementType_Int4:	return 16u;
	case ImUiVertexElementType_UInt:	return 4u;
	case ImUiVertexElementType_UInt2:	return 8u;
	case ImUiVertexElementType_UInt3:	return 12u;
	case ImUiVertexElementType_UInt4:	return 16u;
	}

	return 0u;
}