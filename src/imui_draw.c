#include "imui_draw.h"

#include "imui_font.h"
#include "imui_memory.h"
#include "imui_text.h"

#include <string.h>

typedef enum ImUiDrawSkinPointX ImUiDrawSkinPointX;
enum ImUiDrawSkinPointX
{
	ImUiDrawSkinPointX_Left,
	ImUiDrawSkinPointX_CenterLeft,
	ImUiDrawSkinPointX_CenterRight,
	ImUiDrawSkinPointX_Right,

	ImUiDrawSkinPointX_END = ImUiDrawSkinPointX_Right
};

typedef enum ImUiDrawSkinPointY ImUiDrawSkinPointY;
enum ImUiDrawSkinPointY
{
	ImUiDrawSkinPointY_Top,
	ImUiDrawSkinPointY_CenterTop,
	ImUiDrawSkinPointY_CenterBottom,
	ImUiDrawSkinPointY_Bottom,

	ImUiDrawSkinPointY_END = ImUiDrawSkinPointY_Bottom
};

static void					ImUiDrawFreeWindow( ImUiDraw* draw, ImUiDrawWindowData* window );
static void					ImUiDrawFreeSurface( ImUiDraw* draw, ImUiDrawSurfaceData* surface );
static ImUiDrawWindowData*	ImUiDrawGetWindow( ImUiDraw* draw, ImUiWidget* widget );
static ImUiDrawElement*		ImUiDrawPushElement( ImUiWidget* widget, ImUiDrawElementType type, void* texture );
static void					ImUiDrawSurfaceGenerateElementData( ImUiDraw* draw, ImUiDrawSurfaceData* surface, const ImUiDrawElement* element );
static ImUiDrawCommand*		ImUiDrawSurfaceGetElementCommand( ImUiDraw* draw, ImUiDrawSurfaceData* surface, const ImUiDrawElement* element );
static bool					ImUiDrawSurfacePreparePushVertices( ImUiDraw* draw, ImUiDrawSurfaceData* surface, uintsize vertexCount );
static bool					ImUiDrawSurfacePreparePushRects( ImUiDraw* draw, ImUiDrawSurfaceData* surface, uintsize rectCount );
static uint32				ImUiDrawSurfacePushVertex( ImUiDraw* draw, ImUiDrawSurfaceData* surface, float x, float y, float u, float v, ImUiColor color );
static void					ImUiDrawSurfacePushIndices( ImUiDraw* draw, ImUiDrawSurfaceData* surface, const uint32* indices, uintsize count );
static uintsize				ImUiDrawSurfacePushRect( ImUiDraw* draw, ImUiDrawSurfaceData* surface, ImUiPos posTl, ImUiPos posBr, ImUiTexCoord uv, ImUiColor color );
static uintsize				ImUiVertexElementTypeGetSize( ImUiVertexElementType type );

bool ImUiDrawConstruct( ImUiDraw* draw, ImUiAllocator* allocator, const ImUiVertexFormat* vertexFormat, ImUiVertexType vertexType )
{
	draw->allocator		= allocator;
	draw->vertexType	= vertexType;

	switch( vertexType )
	{
	case ImUiVertexType_VertexList:			draw->triangleTopology = ImUiDrawTopology_TriangleList; break;
	case ImUiVertexType_VertexStrip:		draw->triangleTopology = ImUiDrawTopology_TriangleStrip; break;
	case ImUiVertexType_IndexedVertexList:	draw->triangleTopology = ImUiDrawTopology_IndexedTriangleList; break;
	case ImUiVertexType_IndexedVertexStrip:	draw->triangleTopology = ImUiDrawTopology_IndexedTriangleStrip; break;
	}

	const bool hasVertexFormat = vertexFormat->elementCount != 0u;
	const uintsize vertexElementCount = hasVertexFormat ? vertexFormat->elementCount : 3u;
	ImUiVertexElement* elements = IMUI_MEMORY_ARRAY_NEW( allocator, ImUiVertexElement, vertexElementCount );
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
		elements[ 0u ].align	= 1u;
		elements[ 0u ].type			= ImUiVertexElementType_Float2;
		elements[ 0u ].semantic		= ImUiVertexElementSemantic_PositionScreenSpace;
		elements[ 1u ].align	= 1u;
		elements[ 1u ].type			= ImUiVertexElementType_Float2;
		elements[ 1u ].semantic		= ImUiVertexElementSemantic_TextureCoordinate;
		elements[ 2u ].align	= 1u;
		elements[ 2u ].type			= ImUiVertexElementType_Float4;
		elements[ 2u ].semantic		= ImUiVertexElementSemantic_Color;
	}

	draw->vertexFormat.elements		= elements;
	draw->vertexFormat.elementCount	= vertexElementCount;

	for( uintsize i = 0u; i < vertexElementCount; ++i )
	{
		const ImUiVertexElement* vertexElement = &elements[ i ];

		draw->vertexSize = (draw->vertexSize + vertexElement->align - 1u) & (0u - vertexElement->align);
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

	if( !IMUI_MEMORY_ARRAY_CHECK_CAPACITY_ZERO( draw->allocator, draw->windows, draw->windowCapacity, draw->windowCount + 1u ) )
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

			IMUI_MEMORY_ARRAY_REMOVE_UNSORTED_ZERO( draw->windows, draw->windowCount, i );
			i--;
			continue;
		}

		window->used			= false;
		window->elementCount	= 0u;
	}

	for( uintsize i = 0; i < draw->surfaceCount; ++i )
	{
		ImUiDrawSurfaceData* surface = &draw->surfaces[ i ];

		if( !surface->used )
		{
			ImUiDrawFreeSurface( draw, surface );

			IMUI_MEMORY_ARRAY_REMOVE_UNSORTED_ZERO( draw->surfaces, draw->surfaceCount, i );
			i--;
			continue;
		}

		surface->used			= false;
		surface->commandCount	= 0u;
		surface->vertexCount	= 0u;
		surface->indexCount		= 0u;
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
		if( !IMUI_MEMORY_ARRAY_CHECK_CAPACITY_ZERO( draw->allocator, draw->surfaces, draw->surfaceCapacity, draw->surfaceCount + 1u ) )
		{
			return NULL;
		}

		drawSurface = &draw->surfaces[ draw->surfaceCount++ ];
		drawSurface->surface = surface;
	}

	drawSurface->used = true;

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
	data->indexDataSize		= drawSurface->indexCount * sizeof( *drawSurface->indices );

	return &drawSurface->data;
}

void ImUiDrawLine( ImUiWidget* widget, ImUiPos p0, ImUiPos p1, ImUiColor color )
{
	ImUiDrawElement* element = ImUiDrawPushElement( widget, ImUiDrawElementType_Line, NULL );
	struct ImUiDrawElementDataLine* lineData = &element->data.line;
	lineData->p0	= p0;
	lineData->p1	= p1;
	lineData->color	= color;
}

void ImUiDrawWidgetColor( ImUiWidget* widget, ImUiColor color )
{
	ImUiDrawElement* element = ImUiDrawPushElement( widget, ImUiDrawElementType_Rectangle, NULL );
	struct ImUiDrawElementDataRectangle* rectData = &element->data.rect;
	rectData->rect		= widget->rect;
	rectData->color		= color;
	memset( &rectData->uv, 0, sizeof( rectData->uv ) );
}

void ImUiDrawWidgetTexture( ImUiWidget* widget, ImUiTexture texture )
{
	ImUiDrawElement* element = ImUiDrawPushElement( widget, ImUiDrawElementType_Rectangle, texture.data );
	struct ImUiDrawElementDataRectangle* rectData = &element->data.rect;
	rectData->rect		= widget->rect;
	rectData->color		= ImUiColorCreateWhite( 1.0f );
	rectData->uv.u0		= 0.0f;
	rectData->uv.v0		= 0.0f;
	rectData->uv.u1		= 1.0f;
	rectData->uv.v1		= 1.0f;
}

void ImUiDrawWidgetTextureColor( ImUiWidget* widget, ImUiTexture texture, ImUiColor color )
{
	ImUiDrawElement* element = ImUiDrawPushElement( widget, ImUiDrawElementType_Rectangle, texture.data );
	struct ImUiDrawElementDataRectangle* rectData = &element->data.rect;
	rectData->rect		= widget->rect;
	rectData->color		= color;
	rectData->uv.u0		= 0.0f;
	rectData->uv.v0		= 0.0f;
	rectData->uv.u1		= 1.0f;
	rectData->uv.v1		= 1.0f;
}

void ImUiDrawWidgetSkin( ImUiWidget* widget, ImUiSkin skin )
{
	ImUiDrawElement* element = ImUiDrawPushElement( widget, ImUiDrawElementType_Skin, skin.texture.data );
	struct ImUiDrawElementDataSkin* skinData = &element->data.skin;
	skinData->rect		= widget->rect;
	skinData->border	= skin.border;
	skinData->uv		= skin.uv;
	skinData->texSize	= skin.texture.size;
	skinData->color		= ImUiColorCreateWhite( 1.0f );
}

void ImUiDrawWidgetSkinColor( ImUiWidget* widget, ImUiSkin skin, ImUiColor color )
{
	ImUiDrawElement* element = ImUiDrawPushElement( widget, ImUiDrawElementType_Skin, skin.texture.data );
	struct ImUiDrawElementDataSkin* skinData = &element->data.skin;
	skinData->rect		= widget->rect;
	skinData->border	= skin.border;
	skinData->uv		= skin.uv;
	skinData->texSize	= skin.texture.size;
	skinData->color		= color;
}

void ImUiDrawWidgetText( ImUiWidget* widget, ImUiTextLayout* layout )
{
	ImUiDrawElement* element = ImUiDrawPushElement( widget, ImUiDrawElementType_Text, layout->font->texture.data );
	struct ImUiDrawElementDataText* textData = &element->data.text;
	textData->pos		= widget->rect.pos;
	textData->color		= ImUiColorCreateWhite( 1.0f );
	textData->layout	= layout;
}

void ImUiDrawWidgetTextColor( ImUiWidget* widget, ImUiTextLayout* layout, ImUiColor color )
{
	ImUiDrawElement* element = ImUiDrawPushElement( widget, ImUiDrawElementType_Text, layout->font->texture.data );
	struct ImUiDrawElementDataText* textData = &element->data.text;
	textData->pos		= widget->rect.pos;
	textData->color		= color;
	textData->layout	= layout;
}

void ImUiDrawRectColor( ImUiWidget* widget, ImUiRect rect, ImUiColor color )
{
	ImUiDrawElement* element = ImUiDrawPushElement( widget, ImUiDrawElementType_Rectangle, NULL );
	struct ImUiDrawElementDataRectangle* rectData = &element->data.rect;
	rectData->rect		= rect;
	rectData->color		= color;
	memset( &rectData->uv, 0, sizeof( rectData->uv ) );
}

void ImUiDrawRectTexture( ImUiWidget* widget, ImUiRect rect, ImUiTexture texture )
{
	ImUiDrawElement* element = ImUiDrawPushElement( widget, ImUiDrawElementType_Rectangle, texture.data );
	struct ImUiDrawElementDataRectangle* rectData = &element->data.rect;
	rectData->rect		= rect;
	rectData->color		= ImUiColorCreateWhite( 1.0f );
	rectData->uv.u0		= 0.0f;
	rectData->uv.v0		= 0.0f;
	rectData->uv.u1		= 1.0f;
	rectData->uv.v1		= 1.0f;
}

void ImUiDrawRectTextureUv( ImUiWidget* widget, ImUiRect rect, ImUiTexture texture, ImUiTexCoord uv )
{
	ImUiDrawElement* element = ImUiDrawPushElement( widget, ImUiDrawElementType_Rectangle, texture.data );
	struct ImUiDrawElementDataRectangle* rectData = &element->data.rect;
	rectData->rect		= rect;
	rectData->color		= ImUiColorCreateWhite( 1.0f );
	rectData->uv		= uv;
}

void ImUiDrawSkin( ImUiWidget* widget, ImUiRect rect, ImUiSkin skin )
{
	ImUiDrawElement* element = ImUiDrawPushElement( widget, ImUiDrawElementType_Skin, skin.texture.data );
	struct ImUiDrawElementDataSkin* skinData = &element->data.skin;
	skinData->rect		= rect;
	skinData->border	= skin.border;
	skinData->uv		= skin.uv;
	skinData->texSize	= skin.texture.size;
	skinData->color		= ImUiColorCreateWhite( 1.0f );
}

void ImUiDrawSkinColor( ImUiWidget* widget, ImUiRect rect, ImUiSkin skin, ImUiColor color )
{
	ImUiDrawElement* element = ImUiDrawPushElement( widget, ImUiDrawElementType_Skin, skin.texture.data );
	struct ImUiDrawElementDataSkin* skinData = &element->data.skin;
	skinData->rect		= rect;
	skinData->border	= skin.border;
	skinData->uv		= skin.uv;
	skinData->texSize	= skin.texture.size;
	skinData->color		= color;
}

void ImUiDrawText( ImUiWidget* widget, ImUiPos pos, ImUiTextLayout* layout )
{
	ImUiDrawElement* element = ImUiDrawPushElement( widget, ImUiDrawElementType_Text, layout->font->texture.data );
	struct ImUiDrawElementDataText* textData = &element->data.text;
	textData->pos		= pos;
	textData->color		= ImUiColorCreateWhite( 1.0f );;
	textData->layout	= layout;
}

void ImUiDrawTextColor( ImUiWidget* widget, ImUiPos pos, ImUiTextLayout* layout, ImUiColor color )
{
	ImUiDrawElement* element = ImUiDrawPushElement( widget, ImUiDrawElementType_Text, layout->font->texture.data );
	struct ImUiDrawElementDataText* textData = &element->data.text;
	textData->pos		= pos;
	textData->color		= color;
	textData->layout	= layout;
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

	if( !IMUI_MEMORY_ARRAY_CHECK_CAPACITY( draw->allocator, window->elements, window->elementCapacity, window->elementCount + 1u ) )
	{
		return NULL;
	}

	ImUiDrawElement* element = &window->elements[ window->elementCount++ ];
	element->type		= type;
	element->texture	= texture;
	element->clipRect	= widget->rect;

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

			ImUiDrawSurfacePreparePushVertices( draw, surface, 2u );

			uint32 vertexIndices[ 2u ];
			vertexIndices[ 0u ] = ImUiDrawSurfacePushVertex( draw, surface, lineData->p0.x, lineData->p0.y, 0.0f, 0.0f, lineData->color );
			vertexIndices[ 1u ] = ImUiDrawSurfacePushVertex( draw, surface, lineData->p1.x, lineData->p1.y, 0.0f, 0.0f, lineData->color );

			if( draw->triangleTopology == ImUiDrawTopology_IndexedTriangleList || draw->triangleTopology == ImUiDrawTopology_IndexedTriangleStrip )
			{
				ImUiDrawSurfacePushIndices( draw, surface, vertexIndices, IMUI_ARRAY_COUNT( vertexIndices ) );
			}

			elementCount = 2u;
		}
		break;

	case ImUiDrawElementType_Rectangle:
		{
			const struct ImUiDrawElementDataRectangle* rectData = &element->data.rect;

			const ImUiPos posTl = ImUiRectGetTopLeft( rectData->rect );
			const ImUiPos posBr = ImUiRectGetBottomRight( rectData->rect );

			ImUiDrawSurfacePreparePushRects( draw, surface, 1u );
			elementCount = ImUiDrawSurfacePushRect( draw, surface, posTl, posBr, rectData->uv, rectData->color );
		}
		break;

	case ImUiDrawElementType_Skin:
		{
			const struct ImUiDrawElementDataSkin* skinData = &element->data.skin;

			ImUiBorder uvBorder = skinData->border;
			uvBorder.top	/= skinData->texSize.height;
			uvBorder.left	/= skinData->texSize.width;
			uvBorder.bottom	/= skinData->texSize.height;
			uvBorder.right	/= skinData->texSize.width;

			const ImUiSize borderSize = ImUiBorderGetMinSize( skinData->border );
			const float xScale = skinData->rect.size.width >= borderSize.width ? 1.0f : skinData->rect.size.width / borderSize.width;
			const float yScale = skinData->rect.size.height >= borderSize.height ? 1.0f : skinData->rect.size.height / borderSize.height;

			const float xLeft			= skinData->rect.pos.x;
			const float xCenterLeft		= xLeft + (skinData->border.left * xScale);
			const float xRight			= xLeft + skinData->rect.size.width;
			const float xCenterRight	= xRight - (skinData->border.right * xScale);
			const float yTop			= skinData->rect.pos.y;
			const float yCenterTop		= yTop + (skinData->border.top * yScale);
			const float yBottom			= yTop + skinData->rect.size.height;
			const float yCenterBottom	= yBottom - (skinData->border.bottom * yScale);

			const float uLeft			= skinData->uv.u0;
			const float uCenterLeft		= uLeft + uvBorder.left;
			const float uRight			= skinData->uv.u1;
			const float uCenterRight	= uRight - uvBorder.right;
			const float vTop			= skinData->uv.v0;
			const float vCenterTop		= vTop + uvBorder.top;
			const float vBottom			= skinData->uv.v1;
			const float vCenterBottom	= vBottom - uvBorder.bottom;

			const float xPositions[] =
			{
				xLeft,
				xCenterLeft,
				xCenterRight,
				xRight
			};

			const float yPositions[] =
			{
				yTop,
				yCenterTop,
				yCenterBottom,
				yBottom
			};

			const float uPositions[] =
			{
				uLeft,
				uCenterLeft,
				uCenterRight,
				uRight
			};

			const float vPositions[] =
			{
				vTop,
				vCenterTop,
				vCenterBottom,
				vBottom
			};

			ImUiDrawSurfacePreparePushRects( draw, surface, 9u );

			for( uintsize y = 0; y < ImUiDrawSkinPointY_END; ++y )
			{
				const uintsize nextY = y + 1u;

				for( uintsize x = 0; x < ImUiDrawSkinPointX_END; ++x )
				{
					const uintsize nextX = x + 1u;

					const ImUiPos posTl = ImUiPosCreate( xPositions[ x ], yPositions[ y ] );
					const ImUiPos posBr = ImUiPosCreate( xPositions[ nextX ], yPositions[ nextY ] );

					const ImUiTexCoord uv =
					{
						uPositions[ x ], vPositions[ y ],
						uPositions[ nextX ], vPositions[ nextY ]
					};

					elementCount += ImUiDrawSurfacePushRect( draw, surface, posTl, posBr, uv, skinData->color );
				}
			}
		}
		break;

	case ImUiDrawElementType_Text:
		{
			const struct ImUiDrawElementDataText* textData = &element->data.text;

			ImUiDrawSurfacePreparePushRects( draw, surface, textData->layout->glyphCount );

			const float x = textData->pos.x;
			const float y = textData->pos.y;
			for( uintsize i = 0; i < textData->layout->glyphCount; ++i )
			{
				const ImUiTextGlyph* glyph = &textData->layout->glyphs[ i ];

				const ImUiPos posTl = ImUiPosCreate( x + glyph->pos.x, y + glyph->pos.y );
				const ImUiPos posBr = ImUiPosCreate( posTl.x + glyph->size.width, posTl.y + glyph->size.height );

				elementCount += ImUiDrawSurfacePushRect( draw, surface, posTl, posBr, glyph->uv, textData->color );
			}
		}
		break;
	}

	command->count += elementCount;
}

static ImUiDrawCommand* ImUiDrawSurfaceGetElementCommand( ImUiDraw* draw, ImUiDrawSurfaceData* surface, const ImUiDrawElement* element )
{
	const ImUiDrawTopology topology = element->type == ImUiDrawElementType_Line ? ImUiDrawTopology_LineList : draw->triangleTopology;
	if( surface->commandCount > 0u )
	{
		ImUiDrawCommand* command = &surface->commands[ surface->commandCount - 1u ];
		if( command->topology == topology &&
			command->texture == element->texture )
		{
			return command;
		}
	}

	if( !IMUI_MEMORY_ARRAY_CHECK_CAPACITY_ZERO( draw->allocator, surface->commands, surface->commandCapacity, surface->commandCount + 1u ) )
	{
		return NULL;
	}

	ImUiDrawCommand* command = &surface->commands[ surface->commandCount++ ];
	command->topology	= topology;
	command->texture	= element->texture;
	command->clipRect	= element->clipRect;
	command->count		= 0u;

	return command;
}

static bool ImUiDrawSurfacePreparePushVertices( ImUiDraw* draw, ImUiDrawSurfaceData* surface, uintsize vertexCount )
{
	const uintsize requiredSize = draw->vertexSize * (surface->vertexCount + vertexCount);
	return IMUI_MEMORY_ARRAY_CHECK_CAPACITY( draw->allocator, surface->vertexData, surface->vertexDataCapacity, requiredSize );
}

static bool ImUiDrawSurfacePreparePushRects( ImUiDraw* draw, ImUiDrawSurfaceData* surface, uintsize rectCount )
{
	uintsize elementCountPerRect = 0u;
	switch( draw->triangleTopology )
	{
	case ImUiDrawTopology_LineList:				elementCountPerRect = 2u; break;
	case ImUiDrawTopology_TriangleList:			elementCountPerRect = 6u; break;
	case ImUiDrawTopology_TriangleStrip:		elementCountPerRect = 4u; break;
	case ImUiDrawTopology_IndexedTriangleList:	elementCountPerRect = 6u; break;
	case ImUiDrawTopology_IndexedTriangleStrip:	elementCountPerRect = 4u; break;
	}

	return ImUiDrawSurfacePreparePushVertices( draw, surface, elementCountPerRect * rectCount );
}

static uint32 ImUiDrawSurfacePushVertex( ImUiDraw* draw, ImUiDrawSurfaceData* surface, float x, float y, float u, float v, ImUiColor color )
{
	const uint32 vertexIndex = (uint32)surface->vertexCount;
	surface->vertexCount++;

	uintsize vertexOffset = 0u;
	uint8* vertex = surface->vertexData + (draw->vertexSize * vertexIndex);
	IMUI_ASSERT( vertex + draw->vertexSize <= surface->vertexData + surface->vertexDataCapacity );

	for( uintsize i = 0u; i < draw->vertexFormat.elementCount; ++i )
	{
		const ImUiVertexElement* vertexElement = &draw->vertexFormat.elements[ i ];

		vertexOffset = (vertexOffset + vertexElement->align - 1u) & (0u - vertexElement->align);

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
					floatData[ 0u ] = x;
					floatData[ 1u ] = y;

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
					intData[ 0u ] = (sint32)x;
					intData[ 1u ] = (sint32)y;

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
					uintData[ 0u ] = (uint32)x;
					uintData[ 1u ] = (uint32)y;

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
				ImUiPos clipSpacePosition =
				{
					-1.0f + (x / surface->surface->size.width),
					 1.0f - (y / surface->surface->size.height),
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
					floatData[ 0u ] = u;
					floatData[ 1u ] = v;

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
					intData[ 0u ] = (sint32)(u * 2147483647.0f + 0.5f);
					intData[ 1u ] = (sint32)(v * 2147483647.0f + 0.5f);

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
					uintData[ 0u ] = (uint32)(u * 4294967295.0f + 0.5f);
					uintData[ 1u ] = (uint32)(v * 4294967295.0f + 0.5f);

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
					floatData[ 0u ] = color.red / 255.0f;
					floatData[ 1u ] = color.green / 255.0f;
					floatData[ 2u ] = color.blue / 255.0f;

					if( vertexElement->type == ImUiVertexElementType_Float4 )
					{
						floatData[ 3u ] = color.alpha / 255.0f;
					}
				}
				else if( vertexElement->type >= ImUiVertexElementType_Int3 && vertexElement->type <= ImUiVertexElementType_Int4 )
				{
					sint32* intData = (sint32*)elementData;
					intData[ 0u ] = color.red << 23u;
					intData[ 1u ] = color.green << 23u;
					intData[ 2u ] = color.blue << 23u;

					if( vertexElement->type == ImUiVertexElementType_Int4 )
					{
						intData[ 3u ] = color.alpha << 23u;
					}
				}
				else if( vertexElement->type == ImUiVertexElementType_UInt  )
				{
					*(uint32*)elementData = (color.red << 24u) | (color.green << 16u) | (color.blue << 8u) | color.alpha;
				}
				else if( vertexElement->type >= ImUiVertexElementType_UInt3 && vertexElement->type <= ImUiVertexElementType_UInt4 )
				{
					uint32* uintData = (uint32*)elementData;
					uintData[ 0u ] = color.red << 24u;
					uintData[ 1u ] = color.green << 24u;
					uintData[ 2u ] = color.blue << 24u;

					if( vertexElement->type == ImUiVertexElementType_UInt4 )
					{
						uintData[ 3u ] = color.alpha << 24u;
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
	if( !IMUI_MEMORY_ARRAY_CHECK_CAPACITY( draw->allocator, surface->indices, surface->indicesCapacity, requiredSize ) )
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

static uintsize ImUiVertexElementTypeGetSize( ImUiVertexElementType type )
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

static uintsize ImUiDrawSurfacePushRect( ImUiDraw* draw, ImUiDrawSurfaceData* surface, ImUiPos posTl, ImUiPos posBr, ImUiTexCoord uv, ImUiColor color )
{
	if( posBr.x - posTl.x <= 0.0f ||
		posBr.y - posTl.y <= 0.0f )
	{
		return 0u;
	}

	switch( draw->triangleTopology )
	{
	case ImUiDrawTopology_LineList:
		break;

	case ImUiDrawTopology_TriangleList:
		{
			ImUiDrawSurfacePushVertex( draw, surface, posTl.x, posTl.y, uv.u0, uv.v0, color );
			ImUiDrawSurfacePushVertex( draw, surface, posBr.x, posTl.y, uv.u1, uv.v0, color );
			ImUiDrawSurfacePushVertex( draw, surface, posTl.x, posBr.y, uv.u0, uv.v1, color );
			ImUiDrawSurfacePushVertex( draw, surface, posTl.x, posBr.y, uv.u0, uv.v1, color );
			ImUiDrawSurfacePushVertex( draw, surface, posBr.x, posTl.y, uv.u1, uv.v0, color );
			ImUiDrawSurfacePushVertex( draw, surface, posBr.x, posBr.y, uv.u1, uv.v1, color );
		}
		return 6u;

	case ImUiDrawTopology_TriangleStrip:
		{
			ImUiDrawSurfacePushVertex( draw, surface, posTl.x, posTl.y, uv.u0, uv.v0, color );
			ImUiDrawSurfacePushVertex( draw, surface, posBr.x, posTl.y, uv.u1, uv.v0, color );
			ImUiDrawSurfacePushVertex( draw, surface, posTl.x, posBr.y, uv.u0, uv.v1, color );
			ImUiDrawSurfacePushVertex( draw, surface, posBr.x, posBr.y, uv.u1, uv.v1, color );
		}
		return 4u;

	case ImUiDrawTopology_IndexedTriangleList:
		{
			const uint32 indexTl = ImUiDrawSurfacePushVertex( draw, surface, posTl.x, posTl.y, uv.u0, uv.v0, color );
			const uint32 indexTr = ImUiDrawSurfacePushVertex( draw, surface, posBr.x, posTl.y, uv.u1, uv.v0, color );
			const uint32 indexBl = ImUiDrawSurfacePushVertex( draw, surface, posTl.x, posBr.y, uv.u0, uv.v1, color );
			const uint32 indexBr = ImUiDrawSurfacePushVertex( draw, surface, posBr.x, posBr.y, uv.u1, uv.v1, color );

			const uint32 vertexIndices[ 6u ] =
			{
				indexTl, indexTr, indexBl,
				indexBl, indexTr, indexBr
			};
			ImUiDrawSurfacePushIndices( draw, surface, vertexIndices, IMUI_ARRAY_COUNT( vertexIndices ) );
		}
		return 6u;

	case ImUiDrawTopology_IndexedTriangleStrip:
		{
			const uint32 vertexIndices[ 4u ] =
			{
				ImUiDrawSurfacePushVertex( draw, surface, posTl.x, posTl.y, uv.u0, uv.v0, color ),
				ImUiDrawSurfacePushVertex( draw, surface, posBr.x, posTl.y, uv.u1, uv.v0, color ),
				ImUiDrawSurfacePushVertex( draw, surface, posTl.x, posBr.y, uv.u0, uv.v1, color ),
				ImUiDrawSurfacePushVertex( draw, surface, posBr.x, posBr.y, uv.u1, uv.v1, color )
			};
			ImUiDrawSurfacePushIndices( draw, surface, vertexIndices, IMUI_ARRAY_COUNT( vertexIndices ) );
		}
		return 4u;
	}

	return 0u;
}
