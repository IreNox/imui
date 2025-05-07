#include "imui_draw.h"

#include "imui_font.h"
#include "imui_internal.h"
#include "imui_memory.h"

#include <string.h>

typedef struct ImUiDrawSurfaceBuffers
{
	uint32*					indices;
	uintsize				indexCount;
	uintsize				indexCapacity;
	byte*					vertexData;
	uintsize				vertexCount;
	uintsize				vertexDataCapacity;
} ImUiDrawSurfaceBuffers;

typedef struct ImUiDrawSurfaceData
{
	bool					used;

	ImUiStringView			name;
	ImUiSize				size;

	uintsize*				windows;
	uintsize				windowCapacity;
	uintsize				windowCount;

	ImUiDrawCommand*		commands;
	uintsize				commandCapacity;
	uintsize				commandCount;

	uintsize				approximatedIndexCount;
	uintsize				approximatedVertexCount;

	ImUiDrawData			data;

	ImUiDrawSurfaceBuffers*	buffers;
} ImUiDrawSurfaceData;

typedef struct ImUiDrawWindowData
{
	bool					used;

	ImUiStringView			name;
	uintsize				surfaceIndex;
	uint32					zOrder;

	ImUiDrawElement*		elements;
	uintsize				elementCapacity;
	uintsize				elementCount;
} ImUiDrawWindowData;

typedef enum ImUiDrawSkinPointX
{
	ImUiDrawSkinPointX_Left,
	ImUiDrawSkinPointX_CenterLeft,
	ImUiDrawSkinPointX_CenterRight,
	ImUiDrawSkinPointX_Right,

	ImUiDrawSkinPointX_END = ImUiDrawSkinPointX_Right
} ImUiDrawSkinPointX;

typedef enum ImUiDrawSkinPointY
{
	ImUiDrawSkinPointY_Top,
	ImUiDrawSkinPointY_CenterTop,
	ImUiDrawSkinPointY_CenterBottom,
	ImUiDrawSkinPointY_Bottom,

	ImUiDrawSkinPointY_END = ImUiDrawSkinPointY_Bottom
} ImUiDrawSkinPointY;

static void					ImUiDrawFreeWindow( ImUiDraw* draw, ImUiDrawWindowData* window );
static void					ImUiDrawFreeSurface( ImUiDraw* draw, ImUiDrawSurfaceData* surface );
static ImUiDrawWindowData*	ImUiDrawGetWindow( ImUiDraw* draw, ImUiWidget* widget );
static void					ImUiDrawSurfaceGenerateElementData( ImUiDraw* draw, ImUiDrawSurfaceData* surface, const ImUiDrawElement* element );
static ImUiRect				ImUiDrawSurfaceGenerateWidgetRect( ImUiWidget* widget );
static uint32				ImUiDrawSurfacePushVertex( ImUiDraw* draw, ImUiDrawSurfaceData* surface, float x, float y, float u, float v, ImUiColor color );
static void					ImUiDrawSurfacePushIndices( ImUiDrawSurfaceBuffers* buffers, const uint32* indices, uintsize count );
static uintsize				ImUiDrawSurfacePushRect( ImUiDraw* draw, ImUiDrawSurfaceData* surface, ImUiPos posTl, ImUiPos posBr, ImUiTexCoord uv, ImUiColor color );
static uintsize				ImUiVertexElementTypeGetSize( ImUiVertexElementType type );

static const bool s_supportedVertexElementSemanicFormats[][ ImUiVertexElementType_MAX ] =
{
	{ true,		true,	true,	true,	true,	true,	true,	true,	true,	true,	true,	true },		// ImUiVertexElementSemantic_None
	{ false,	true,	true,	true,	false,	true,	true,	true,	false,	true,	true,	true },		// ImUiVertexElementSemantic_PositionScreenSpace
	{ false,	true,	true,	true,	false,	true,	true,	true,	false,	false,	false,	false },	// ImUiVertexElementSemantic_PositionClipSpace
	{ false,	true,	true,	true,	false,	true,	true,	true,	false,	true,	true,	true },		// ImUiVertexElementSemantic_TextureCoordinate
	{ false,	false,	true,	true,	false,	false,	true,	true,	true,	false,	true,	true },		// ImUiVertexElementSemantic_ColorRGBA
	{ false,	false,	true,	true,	false,	false,	false,	true,	true,	false,	false,	true }		// ImUiVertexElementSemantic_ColorARGB
};

bool ImUiDrawConstruct( ImUiDraw* draw, ImUiAllocator* allocator, const ImUiVertexFormat* vertexFormat, ImUiVertexType vertexType )
{
	draw->allocator		= allocator;
	draw->vertexType	= vertexType;

	switch( vertexType )
	{
	case ImUiVertexType_VertexList:			draw->triangleTopology = ImUiDrawTopology_TriangleList; break;
	case ImUiVertexType_IndexedVertexList:	draw->triangleTopology = ImUiDrawTopology_IndexedTriangleList; break;
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
		elements[ 0u ].type		= ImUiVertexElementType_Float2;
		elements[ 0u ].semantic	= ImUiVertexElementSemantic_PositionScreenSpace;
		elements[ 1u ].align	= 1u;
		elements[ 1u ].type		= ImUiVertexElementType_Float2;
		elements[ 1u ].semantic	= ImUiVertexElementSemantic_TextureCoordinate;
		elements[ 2u ].align	= 1u;
		elements[ 2u ].type		= ImUiVertexElementType_Float4;
		elements[ 2u ].semantic	= ImUiVertexElementSemantic_ColorRGBA;
	}

	draw->vertexFormat.elements		= elements;
	draw->vertexFormat.elementCount	= vertexElementCount;

	for( uintsize i = 0u; i < vertexElementCount; ++i )
	{
		const ImUiVertexElement* vertexElement = &elements[ i ];
		IMUI_ASSERT( vertexElement->align > 0u );
		IMUI_ASSERT( s_supportedVertexElementSemanicFormats[ vertexElement->semantic ][ vertexElement->type ] );

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

uintsize ImUiDrawRegisterSurface( ImUiDraw* draw, ImUiStringView name, ImUiSize size )
{
	for( uintsize i = 0; i < draw->surfaceCount; ++i )
	{
		ImUiDrawSurfaceData* surface = &draw->surfaces[ i ];
		if( !ImUiStringViewIsEquals( surface->name, name ) )
		{
			continue;
		}

		IMUI_ASSERT( !surface->used && "Surface name must be unique" );

		surface->used	= true;
		surface->size	= size;

		return i;
	}

	if( !IMUI_MEMORY_ARRAY_CHECK_CAPACITY_ZERO( draw->allocator, draw->surfaces, draw->surfaceCapacity, draw->surfaceCount + 1u ) )
	{
		return IMUI_SIZE_MAX;
	}

	const uintsize surfaceIndex = draw->surfaceCount++;
	ImUiDrawSurfaceData* surface = &draw->surfaces[ surfaceIndex ];
	surface->used	= true;
	surface->name	= name;
	surface->size	= size;

	return surfaceIndex;
}

uintsize ImUiDrawRegisterWindow( ImUiDraw* draw, ImUiStringView name, uintsize surfaceIndex, uint32 zOrder )
{
	uintsize windowIndex = IMUI_SIZE_MAX;
	for( uintsize i = 0; i < draw->windowCount; ++i )
	{
		ImUiDrawWindowData* window = &draw->windows[ i ];
		if( window->surfaceIndex != surfaceIndex ||
			!ImUiStringViewIsEquals( window->name, name ) )
		{
			continue;
		}

		IMUI_ASSERT( !window->used && "Window name must be unique" );

		windowIndex = i;
		break;
	}

	if( windowIndex == IMUI_SIZE_MAX )
	{
		if( !IMUI_MEMORY_ARRAY_CHECK_CAPACITY_ZERO( draw->allocator, draw->windows, draw->windowCapacity, draw->windowCount + 1u ) )
		{
			return IMUI_SIZE_MAX;
		}

		windowIndex = draw->windowCount++;
	}

	ImUiDrawSurfaceData* surface = &draw->surfaces[ surfaceIndex ];

	if( !IMUI_MEMORY_ARRAY_CHECK_CAPACITY_ZERO( draw->allocator, surface->windows, surface->windowCapacity, surface->windowCount + 1u ) )
	{
		return IMUI_SIZE_MAX;
	}

	ImUiDrawWindowData* window = &draw->windows[ windowIndex ];
	window->used			= true;
	window->name			= name;
	window->surfaceIndex	= surfaceIndex;
	window->zOrder			= zOrder;
	window->elementCount	= 0u;

	surface->windows[ surface->windowCount++ ] = windowIndex;

	return windowIndex;
}

void ImUiDrawSurfaceEnd( ImUiDraw* draw, uintsize surfaceIndex )
{
	ImUiDrawSurfaceData* surface = &draw->surfaces[ surfaceIndex ];

	// sort windows by zOrder
	for( uintsize i = 1u; i < surface->windowCount; ++i )
	{
		while( i > 0u && draw->windows[ surface->windows[ i - 1u ] ].zOrder > draw->windows[ surface->windows[ i ] ].zOrder )
		{
			const uintsize tempWindowIndex = surface->windows[ i ];
			surface->windows[ i ]		= surface->windows[ i - 1u ];
			surface->windows[ i - 1u ]	= tempWindowIndex;

			i--;
		}
	}
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

		window->used = false;
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

		IMUI_MEMORY_ARRAY_SHRINK( draw->allocator, surface->windows, surface->windowCapacity, surface->windowCount );
		IMUI_MEMORY_ARRAY_SHRINK( draw->allocator, surface->commands, surface->commandCapacity, surface->commandCount );

		surface->used						= false;
		surface->windowCount				= 0u;
		surface->commandCount				= 0u;
		surface->approximatedIndexCount		= 0u;
		surface->approximatedVertexCount	= 0u;
	}

	IMUI_MEMORY_ARRAY_SHRINK( draw->allocator, draw->surfaces, draw->surfaceCapacity, draw->surfaceCount );
	IMUI_MEMORY_ARRAY_SHRINK( draw->allocator, draw->windows, draw->windowCapacity, draw->windowCount );
}

ImUiDrawElement* ImUiDrawPushElement( ImUiWidget* widget, ImUiDrawElementType type, uint64_t textureHandle )
{
	ImUiDraw* draw = &widget->window->surface->context->draw;
	ImUiDrawWindowData* window = ImUiDrawGetWindow( draw, widget );

	if( !IMUI_MEMORY_ARRAY_CHECK_CAPACITY( draw->allocator, window->elements, window->elementCapacity, window->elementCount + 1u ) )
	{
		return NULL;
	}

	if( textureHandle == IMUI_TEXTURE_HANDLE_INVALID )
	{
		if( type == ImUiDrawElementType_Skin )
		{
			type = ImUiDrawElementType_Rect;
		}
		else if( type == ImUiDrawElementType_SkinPartial )
		{
			type = ImUiDrawElementType_RectPartial;
		}
	}

	ImUiDrawElement* element = &window->elements[ window->elementCount++ ];
	element->type			= type;
	element->textureHandle	= textureHandle;
	element->widget			= widget;

	uintsize indexCount = 0u;
	uintsize vertexCount = 0u;
	switch( type )
	{
	case ImUiDrawElementType_Line:			indexCount = 2u; vertexCount = 2u; break;
	case ImUiDrawElementType_Triangle:		indexCount = 3u; vertexCount = 3u; break;
	case ImUiDrawElementType_Rect:
	case ImUiDrawElementType_RectPartial:	indexCount = 6u; vertexCount = 4u; break;
	case ImUiDrawElementType_Skin:
	case ImUiDrawElementType_SkinPartial:	indexCount = 54u; vertexCount = 36u; break;
	case ImUiDrawElementType_Text:
	case ImUiDrawElementType_TextOffset:	break;
	}

	ImUiDrawSurfaceData* surface = &draw->surfaces[ widget->window->surface->drawIndex ];
	surface->approximatedIndexCount		+= indexCount;
	surface->approximatedVertexCount	+= vertexCount;

	return element;
}

ImUiDrawElement* ImUiDrawPushElementText( ImUiWidget* widget, ImUiDrawElementType type, ImUiTextLayout* layout )
{
	ImUiDrawElement* element = ImUiDrawPushElement( widget, type, layout->font->image.textureHandle );

	ImUiDraw* draw = &widget->window->surface->context->draw;
	ImUiDrawSurfaceData* surface = &draw->surfaces[ widget->window->surface->drawIndex ];
	surface->approximatedIndexCount		+= 6u * layout->glyphCount;
	surface->approximatedVertexCount	+= 4u * layout->glyphCount;

	return element;
}

void ImUiDrawGetSurfaceMaxBufferSizes( ImUiDraw* draw, uintsize surfaceIndex, size_t* outVertexDataSize, size_t* outIndexDataSize )
{
	ImUiDrawSurfaceData* drawSurface = &draw->surfaces[ surfaceIndex ];

	if( draw->triangleTopology == ImUiDrawTopology_IndexedTriangleList )
	{
		*outIndexDataSize	= drawSurface->approximatedIndexCount * sizeof( uint32 );
		*outVertexDataSize	= drawSurface->approximatedVertexCount * draw->vertexSize;
	}
	else
	{
		if( outIndexDataSize )
		{
			*outIndexDataSize = 0u;
		}

		*outVertexDataSize	= drawSurface->approximatedIndexCount * draw->vertexSize;
	}
}

const ImUiDrawData* ImUiDrawGenerateSurfaceData( ImUiDraw* draw, uintsize surfaceIndex, void* outVertexData, size_t* inOutVertexDataSize, void* outIndexData, size_t* inOutIndexDataSize )
{
	ImUiDrawSurfaceData* surface = &draw->surfaces[ surfaceIndex ];

	IMUI_ASSERT( !inOutIndexDataSize || *inOutIndexDataSize >= surface->approximatedIndexCount * sizeof( uint32 ) );
	IMUI_ASSERT( *inOutVertexDataSize >= surface->approximatedVertexCount );

	ImUiDrawSurfaceBuffers buffers;
	buffers.indices				= (uint32*)outIndexData;
	buffers.indexCount			= 0u;
	buffers.indexCapacity		= inOutIndexDataSize ? *inOutIndexDataSize / sizeof( uint32 ) : 0u;
	buffers.vertexData			= (byte*)outVertexData;
	buffers.vertexCount			= 0u;
	buffers.vertexDataCapacity	= *inOutVertexDataSize;

	surface->buffers = &buffers;

	for( uintsize i = 0u; i < surface->windowCount; ++i )
	{
		const uintsize windowIndex = surface->windows[ i ];
		const ImUiDrawWindowData* window = &draw->windows[ windowIndex ];

		for( uintsize elementIndex = 0u; elementIndex < window->elementCount; ++elementIndex )
		{
			const ImUiDrawElement* element = &window->elements[ elementIndex ];
			ImUiDrawSurfaceGenerateElementData( draw, surface, element );
		}

#if 0
		// widget debug draw
		IMUI_MEMORY_ARRAY_CHECK_CAPACITY_ZERO( draw->allocator, drawSurface->commands, drawSurface->commandCapacity, drawSurface->commandCount + 1u );

		ImUiDrawCommand* command = &drawSurface->commands[ drawSurface->commandCount++ ];
		command->topology	= ImUiDrawTopology_TriangleList;
		command->texture	= NULL;
		command->clipRect	= ImUiRectCreatePosSize( ImUiPosCreateZero(), surface->size );
		command->count		= 0u;

		const ImUiTexCoord uv = { 0.0f, 0.0f, 0.0f, 0.0f };
		ImUiWidget* widget = window->rootWidget;
		while( widget )
		{
			ImUiDrawSurfacePreparePushRects( draw, drawSurface, 1u );

			const uint8* idBytes = (const uint8*)&widget->id;
			const ImUiColor color = ImUiColorCreate( idBytes[ 0u ], idBytes[ 1u ], idBytes[ 2u ], 0x40u );

			const ImUiRect rect = widget->rect;
			//const ImUiRect rect = widget->clipRect;
			//const ImUiRect rect = ImUiRectCreatePosSize( widget->rect.pos, ImUiSizeMax( widget->minSize, widget->layoutContext.childrenMinSize ) );
			command->count += ImUiDrawSurfacePushRect( draw, drawSurface, rect.pos, ImUiRectGetBottomRight( rect ), uv, color );

			if( widget->firstChild )
			{
				widget = widget->firstChild;
			}
			else if( widget->nextSibling )
			{
				widget = widget->nextSibling;
			}
			else
			{
				ImUiWidget* nextWidget = widget->parent;
				while( nextWidget )
				{
					ImUiWidget* nextNextWidget = nextWidget->nextSibling;
					if( nextNextWidget )
					{
						nextWidget = nextNextWidget;
						break;
					}

					nextWidget = nextWidget->parent;
				}

				widget = nextWidget;
			}
		}
#endif
	}

	surface->buffers = NULL;

	ImUiDrawData* data = &surface->data;
	data->commands		= surface->commands;
	data->commandCount	= surface->commandCount;

	if( inOutIndexDataSize )
	{
		*inOutIndexDataSize = buffers.indexCount * sizeof( uint32 );
	}
	*inOutVertexDataSize = buffers.vertexCount * draw->vertexSize;

	return data;
}

static void ImUiDrawFreeWindow( ImUiDraw* draw, ImUiDrawWindowData* window )
{
	IMUI_MEMORY_ARRAY_FREE( draw->allocator, window->elements, window->elementCapacity );
}

static void ImUiDrawFreeSurface( ImUiDraw* draw, ImUiDrawSurfaceData* surface )
{
	IMUI_MEMORY_ARRAY_FREE( draw->allocator, surface->windows, surface->windowCapacity );
	IMUI_MEMORY_ARRAY_FREE( draw->allocator, surface->commands, surface->commandCapacity );
}

static ImUiDrawWindowData* ImUiDrawGetWindow( ImUiDraw* draw, ImUiWidget* widget )
{
	return &draw->windows[ widget->window->drawIndex ];
}

static void ImUiDrawSurfaceGenerateElementData( ImUiDraw* draw, ImUiDrawSurfaceData* surface, const ImUiDrawElement* element )
{
	if( !IMUI_MEMORY_ARRAY_CHECK_CAPACITY_ZERO( draw->allocator, surface->commands, surface->commandCapacity, surface->commandCount + 1u ) )
	{
		return;
	}

	ImUiDrawCommand* command = &surface->commands[ surface->commandCount++ ];
	command->topology		= element->type == ImUiDrawElementType_Line ? ImUiDrawTopology_LineList : draw->triangleTopology;
	command->textureHandle	= element->textureHandle;
	command->clipRect		= element->widget->clipRect;
	command->count			= 0u;

	ImUiRect rect = ImUiDrawSurfaceGenerateWidgetRect( element->widget );
	switch( element->type )
	{
	case ImUiDrawElementType_Line:
		{
			const struct ImUiDrawElementDataPrimitive* primitiveData = &element->data.primitive;

			uint32 vertexIndices[ 2u ];
			vertexIndices[ 0u ] = ImUiDrawSurfacePushVertex( draw, surface, rect.pos.x + primitiveData->p0.x, rect.pos.y + primitiveData->p0.y, 0.0f, 0.0f, primitiveData->color );
			vertexIndices[ 1u ] = ImUiDrawSurfacePushVertex( draw, surface, rect.pos.x + primitiveData->p1.x, rect.pos.y + primitiveData->p1.y, 0.0f, 0.0f, primitiveData->color );

			if( draw->triangleTopology == ImUiDrawTopology_IndexedTriangleList )
			{
				ImUiDrawSurfacePushIndices( surface->buffers, vertexIndices, IMUI_ARRAY_COUNT( vertexIndices ) );
			}

			command->count = IMUI_ARRAY_COUNT( vertexIndices );
		}
		break;

	case ImUiDrawElementType_Triangle:
		{
			const struct ImUiDrawElementDataPrimitive* primitiveData = &element->data.primitive;

			uint32 vertexIndices[ 3u ];
			vertexIndices[ 0u ] = ImUiDrawSurfacePushVertex( draw, surface, rect.pos.x + primitiveData->p0.x, rect.pos.y + primitiveData->p0.y, 0.0f, 0.0f, primitiveData->color );
			vertexIndices[ 1u ] = ImUiDrawSurfacePushVertex( draw, surface, rect.pos.x + primitiveData->p1.x, rect.pos.y + primitiveData->p1.y, 0.0f, 0.0f, primitiveData->color );
			vertexIndices[ 2u ] = ImUiDrawSurfacePushVertex( draw, surface, rect.pos.x + primitiveData->p2.x, rect.pos.y + primitiveData->p2.y, 0.0f, 0.0f, primitiveData->color );

			if( draw->triangleTopology == ImUiDrawTopology_IndexedTriangleList )
			{
				ImUiDrawSurfacePushIndices( surface->buffers, vertexIndices, IMUI_ARRAY_COUNT( vertexIndices ) );
			}

			command->count = IMUI_ARRAY_COUNT( vertexIndices );
		}
		break;

	case ImUiDrawElementType_RectPartial:
		{
			const struct ImUiDrawElementDataRect* rectData = &element->data.rect;

			rect.pos.x	+= rectData->relativRect.pos.x;
			rect.pos.y	+= rectData->relativRect.pos.y;
			rect.size	= rectData->relativRect.size;
		}
		// no break;

	case ImUiDrawElementType_Rect:
		{
			const struct ImUiDrawElementDataRect* rectData = &element->data.rect;

			const ImUiPos posTl = ImUiRectGetTopLeft( rect );
			const ImUiPos posBr = ImUiRectGetBottomRight( rect );

			command->count = ImUiDrawSurfacePushRect( draw, surface, posTl, posBr, rectData->uv, rectData->color );
		}
		break;

	case ImUiDrawElementType_SkinPartial:
		{
			const struct ImUiDrawElementDataSkin* skinData = &element->data.skin;

			rect.pos.x	+= skinData->relativRect.pos.x;
			rect.pos.y	+= skinData->relativRect.pos.y;
			rect.size	= skinData->relativRect.size;
		}
		// no break;

	case ImUiDrawElementType_Skin:
		{
			const struct ImUiDrawElementDataSkin* skinData = &element->data.skin;

			const float uScale = skinData->texSize.width ? (skinData->uv.u1 - skinData->uv.u0) / skinData->texSize.width : 0.0f;
			const float vScale = skinData->texSize.height ? (skinData->uv.v1 - skinData->uv.v0) / skinData->texSize.height : 0.0f;

			ImUiBorder uvBorder = skinData->border;
			uvBorder.top	*= vScale;
			uvBorder.left	*= uScale;
			uvBorder.bottom	*= vScale;
			uvBorder.right	*= uScale;

			const ImUiSize borderSize = ImUiBorderGetMinSize( skinData->border );
			const float xScale = rect.size.width >= borderSize.width ? 1.0f : rect.size.width / borderSize.width;
			const float yScale = rect.size.height >= borderSize.height ? 1.0f : rect.size.height / borderSize.height;

			const float xLeft			= rect.pos.x;
			const float xCenterLeft		= xLeft + (skinData->border.left * xScale);
			const float xRight			= xLeft + rect.size.width;
			const float xCenterRight	= xRight - (skinData->border.right * xScale);
			const float yTop			= rect.pos.y;
			const float yCenterTop		= yTop + (skinData->border.top * yScale);
			const float yBottom			= yTop + rect.size.height;
			const float yCenterBottom	= yBottom - (skinData->border.bottom * yScale);

			const float uLeft			= skinData->uv.u0;
			const float uCenterLeft		= uLeft + uvBorder.left;
			const float uRight			= skinData->uv.u1;
			const float uCenterRight	= uRight - uvBorder.right;
			const float vTop			= skinData->uv.v0;
			const float vCenterTop		= vTop + uvBorder.top;
			const float vBottom			= skinData->uv.v1;
			const float vCenterBottom	= vBottom - uvBorder.bottom;

			float xPositions[ 4u ];
			xPositions[ 0u ] = xLeft;
			xPositions[ 1u ] = xCenterLeft;
			xPositions[ 2u ] = xCenterRight;
			xPositions[ 3u ] = xRight;

			float yPositions[ 4u ];
			yPositions[ 0u ] = yTop;
			yPositions[ 1u ] = yCenterTop;
			yPositions[ 2u ] = yCenterBottom;
			yPositions[ 3u ] = yBottom;

			float uPositions[ 4u ];
			uPositions[ 0u ] = uLeft;
			uPositions[ 1u ] = uCenterLeft;
			uPositions[ 2u ] = uCenterRight;
			uPositions[ 3u ] = uRight;

			float vPositions[ 4u ];
			vPositions[ 0u ] = vTop;
			vPositions[ 1u ] = vCenterTop;
			vPositions[ 2u ] = vCenterBottom;
			vPositions[ 3u ] = vBottom;

			for( uintsize y = 0; y < ImUiDrawSkinPointY_END; ++y )
			{
				const uintsize nextY = y + 1u;

				for( uintsize x = 0; x < ImUiDrawSkinPointX_END; ++x )
				{
					const uintsize nextX = x + 1u;

					const ImUiPos posTl = ImUiPosCreate( xPositions[ x ], yPositions[ y ] );
					const ImUiPos posBr = ImUiPosCreate( xPositions[ nextX ], yPositions[ nextY ] );

					ImUiTexCoord uv;
					uv.u0 = uPositions[ x ];
					uv.v0 = vPositions[ y ];
					uv.u1 = uPositions[ nextX ];
					uv.v1 = vPositions[ nextY ];

					command->count += ImUiDrawSurfacePushRect( draw, surface, posTl, posBr, uv, skinData->color );
				}
			}
		}
		break;

	case ImUiDrawElementType_TextOffset:
		{
			const struct ImUiDrawElementDataText* textData = &element->data.text;

			rect.pos.x	+= textData->relativPos.x;
			rect.pos.y	+= textData->relativPos.y;
		}
		// no break;


	case ImUiDrawElementType_Text:
		{
			const struct ImUiDrawElementDataText* textData = &element->data.text;
			const float scale = textData->size / textData->layout->font->fontSize;

			const float x = rect.pos.x;
			const float y = rect.pos.y;
			for( uintsize i = 0; i < textData->layout->glyphCount; ++i )
			{
				const ImUiTextGlyph* glyph = &textData->layout->glyphs[ i ];

				const ImUiPos glyphPos		= ImUiPosScale( glyph->pos, scale );
				const ImUiSize glyphSize	= ImUiSizeScale( glyph->size, scale );
				const ImUiPos posTl			= ImUiPosCreate( x + glyphPos.x, y + glyphPos.y );
				const ImUiPos posBr			= ImUiPosCreate( posTl.x + glyphSize.width, posTl.y + glyphSize.height );

				command->count += ImUiDrawSurfacePushRect( draw, surface, posTl, posBr, glyph->uv, textData->color );
			}
		}
		break;
	}
}

static ImUiRect ImUiDrawSurfaceGenerateWidgetRect( ImUiWidget* widget )
{
	//ImUiRect rect;
	//rect.pos.x			= floorf( widget->rect.pos.x );
	//rect.pos.y			= floorf( widget->rect.pos.y );
	//rect.size.width		= floorf( widget->rect.size.width );
	//rect.size.height	= floorf( widget->rect.size.height );
	//return rect;
	return widget->rect;
}

static uint32 ImUiDrawSurfacePushVertex( ImUiDraw* draw, ImUiDrawSurfaceData* surface, float x, float y, float u, float v, ImUiColor color )
{
	const uint32 vertexIndex = (uint32)surface->buffers->vertexCount;
	surface->buffers->vertexCount++;

	uintsize vertexOffset = 0u;
	uint8* vertex = surface->buffers->vertexData + (draw->vertexSize * vertexIndex);
	IMUI_ASSERT( vertex + draw->vertexSize <= surface->buffers->vertexData + surface->buffers->vertexDataCapacity );

	for( uintsize i = 0u; i < draw->vertexFormat.elementCount; ++i )
	{
		const ImUiVertexElement* vertexElement = &draw->vertexFormat.elements[ i ];
		const uintsize vertexElementSize = ImUiVertexElementTypeGetSize( vertexElement->type );

		vertexOffset = (vertexOffset + vertexElement->align - 1u) & (0u - vertexElement->align);

		uint8* elementData = vertex + vertexOffset;
		switch( vertexElement->semantic )
		{
		case ImUiVertexElementSemantic_Zero:
			memset( elementData, 0, vertexElementSize );
			break;

		case ImUiVertexElementSemantic_PositionScreenSpace:
			{
				switch( vertexElement->type )
				{
				case ImUiVertexElementType_Float:
					break;

				case ImUiVertexElementType_Float2:
					{
						float* floatData = (float*)elementData;
						floatData[ 0u ] = x;
						floatData[ 1u ] = y;
					}
					break;

				case ImUiVertexElementType_Float3:
					{
						float* floatData = (float*)elementData;
						floatData[ 0u ] = x;
						floatData[ 1u ] = y;
						floatData[ 2u ] = 0.0f;
					}
					break;

				case ImUiVertexElementType_Float4:
					{
						float* floatData = (float*)elementData;
						floatData[ 0u ] = x;
						floatData[ 1u ] = y;
						floatData[ 2u ] = 0.0f;
						floatData[ 3u ] = 1.0f;
					}
					break;

				case ImUiVertexElementType_Int:
					break;

				case ImUiVertexElementType_Int2:
					{
						sint32* intData = (sint32*)elementData;
						intData[ 0u ] = (sint32)x;
						intData[ 1u ] = (sint32)y;
					}
					break;

				case ImUiVertexElementType_Int3:
					{
						sint32* intData = (sint32*)elementData;
						intData[ 0u ] = (sint32)x;
						intData[ 1u ] = (sint32)y;
						intData[ 2u ] = 0;
					}
					break;

				case ImUiVertexElementType_Int4:
					{
						sint32* intData = (sint32*)elementData;
						intData[ 0u ] = (sint32)x;
						intData[ 1u ] = (sint32)y;
						intData[ 2u ] = 0;
						intData[ 3u ] = 0x7fffffff;
					}
					break;

				case ImUiVertexElementType_UInt:
					break;

				case ImUiVertexElementType_UInt2:
					{
						uint32* uintData = (uint32*)elementData;
						uintData[ 0u ] = (uint32)x;
						uintData[ 1u ] = (uint32)y;
					}
					break;

				case ImUiVertexElementType_UInt3:
					{
						uint32* uintData = (uint32*)elementData;
						uintData[ 0u ] = (uint32)x;
						uintData[ 1u ] = (uint32)y;
						uintData[ 2u ] = 0u;
					}
					break;

				case ImUiVertexElementType_UInt4:
					{
						uint32* uintData = (uint32*)elementData;
						uintData[ 0u ] = (uint32)x;
						uintData[ 1u ] = (uint32)y;
						uintData[ 2u ] = 0u;
						uintData[ 3u ] = 0xffffffffu;
					}
					break;

				case ImUiVertexElementType_MAX:
					break;
				}
			}
			break;

		case ImUiVertexElementSemantic_PositionClipSpace:
			{
				const ImUiPos clipSpacePosition = ImUiPosCreate(
					-1.0f + (x / surface->size.width),
					1.0f - (y / surface->size.height)
				);

				switch( vertexElement->type )
				{
				case ImUiVertexElementType_Float:
					break;

				case ImUiVertexElementType_Float2:
					{
						float* floatData = (float*)elementData;
						floatData[ 0u ] = clipSpacePosition.x;
						floatData[ 1u ] = clipSpacePosition.y;
					}
					break;

				case ImUiVertexElementType_Float3:
					{
						float* floatData = (float*)elementData;
						floatData[ 0u ] = clipSpacePosition.x;
						floatData[ 1u ] = clipSpacePosition.y;
						floatData[ 2u ] = 0.0f;
					}
					break;

				case ImUiVertexElementType_Float4:
					{
						float* floatData = (float*)elementData;
						floatData[ 0u ] = clipSpacePosition.x;
						floatData[ 1u ] = clipSpacePosition.y;
						floatData[ 2u ] = 0.0f;
						floatData[ 3u ] = 1.0f;
					}
					break;

				case ImUiVertexElementType_Int:
					break;

				case ImUiVertexElementType_Int2:
					{
						sint32* intData = (sint32*)elementData;
						intData[ 0u ] = (sint32)(clipSpacePosition.x * 2147483647.0f + 0.5f);
						intData[ 1u ] = (sint32)(clipSpacePosition.y * 2147483647.0f + 0.5f);
					}
					break;

				case ImUiVertexElementType_Int3:
					{
						sint32* intData = (sint32*)elementData;
						intData[ 0u ] = (sint32)(clipSpacePosition.x * 2147483647.0f + 0.5f);
						intData[ 1u ] = (sint32)(clipSpacePosition.y * 2147483647.0f + 0.5f);
						intData[ 2u ] = 0;
					}
					break;

				case ImUiVertexElementType_Int4:
					{
						sint32* intData = (sint32*)elementData;
						intData[ 0u ] = (sint32)(clipSpacePosition.x * 2147483647.0f + 0.5f);
						intData[ 1u ] = (sint32)(clipSpacePosition.y * 2147483647.0f + 0.5f);
						intData[ 2u ] = 0;
						intData[ 3u ] = 0x7fffffff;
					}
					break;

				case ImUiVertexElementType_UInt:
					break;

				case ImUiVertexElementType_UInt2:
					break;

				case ImUiVertexElementType_UInt3:
					break;

				case ImUiVertexElementType_UInt4:
					break;

				case ImUiVertexElementType_MAX:
					break;
				}
			}
			break;

		case ImUiVertexElementSemantic_TextureCoordinate:
			{
				switch( vertexElement->type )
				{
				case ImUiVertexElementType_Float:
					break;

				case ImUiVertexElementType_Float2:
					{
						float* floatData = (float*)elementData;
						floatData[ 0u ] = u;
						floatData[ 1u ] = v;
					}
					break;

				case ImUiVertexElementType_Float3:
					{
						float* floatData = (float*)elementData;
						floatData[ 0u ] = u;
						floatData[ 1u ] = v;
						floatData[ 2u ] = 0.0f;
					}
					break;

				case ImUiVertexElementType_Float4:
					{
						float* floatData = (float*)elementData;
						floatData[ 0u ] = u;
						floatData[ 1u ] = v;
						floatData[ 2u ] = 0.0f;
						floatData[ 3u ] = 0.0f;
					}
					break;

				case ImUiVertexElementType_Int:
					break;

				case ImUiVertexElementType_Int2:
					{
						sint32* intData = (sint32*)elementData;
						intData[ 0u ] = (sint32)(u * 2147483647.0f + 0.5f);
						intData[ 1u ] = (sint32)(v * 2147483647.0f + 0.5f);
					}
					break;

				case ImUiVertexElementType_Int3:
					{
						sint32* intData = (sint32*)elementData;
						intData[ 0u ] = (sint32)(u * 2147483647.0f + 0.5f);
						intData[ 1u ] = (sint32)(v * 2147483647.0f + 0.5f);
						intData[ 2u ] = 0;
					}
					break;

				case ImUiVertexElementType_Int4:
					{
						sint32* intData = (sint32*)elementData;
						intData[ 0u ] = (sint32)(u * 2147483647.0f + 0.5f);
						intData[ 1u ] = (sint32)(v * 2147483647.0f + 0.5f);
						intData[ 2u ] = 0;
						intData[ 3u ] = 0;
					}
					break;

				case ImUiVertexElementType_UInt:
					break;

				case ImUiVertexElementType_UInt2:
					{
						uint32* uintData = (uint32*)elementData;
						uintData[ 0u ] = (uint32)(u * 4294967295.0f + 0.5f);
						uintData[ 1u ] = (uint32)(v * 4294967295.0f + 0.5f);
					}
					break;

				case ImUiVertexElementType_UInt3:
					{
						uint32* uintData = (uint32*)elementData;
						uintData[ 0u ] = (uint32)(u * 4294967295.0f + 0.5f);
						uintData[ 1u ] = (uint32)(v * 4294967295.0f + 0.5f);
						uintData[ 2u ] = 0u;
					}
					break;

				case ImUiVertexElementType_UInt4:
					{
						uint32* uintData = (uint32*)elementData;
						uintData[ 0u ] = (uint32)(u * 4294967295.0f + 0.5f);
						uintData[ 1u ] = (uint32)(v * 4294967295.0f + 0.5f);
						uintData[ 2u ] = 0u;
						uintData[ 3u ] = 0u;
					}
					break;

				case ImUiVertexElementType_MAX:
					break;
				}
			}
			break;

		case ImUiVertexElementSemantic_ColorRGBA:
			{
				switch( vertexElement->type )
				{
				case ImUiVertexElementType_Float:
					break;

				case ImUiVertexElementType_Float2:
					break;

				case ImUiVertexElementType_Float3:
					{
						float* floatData = (float*)elementData;
						floatData[ 0u ] = color.red / 255.0f;
						floatData[ 1u ] = color.green / 255.0f;
						floatData[ 2u ] = color.blue / 255.0f;
					}
					break;

				case ImUiVertexElementType_Float4:
					{
						float* floatData = (float*)elementData;
						floatData[ 0u ] = color.red / 255.0f;
						floatData[ 1u ] = color.green / 255.0f;
						floatData[ 2u ] = color.blue / 255.0f;
						floatData[ 3u ] = color.alpha / 255.0f;
					}
					break;

				case ImUiVertexElementType_Int:
					break;

				case ImUiVertexElementType_Int2:
					break;

				case ImUiVertexElementType_Int3:
					{
						sint32* intData = (sint32*)elementData;
						intData[ 0u ] = (sint32)color.red << 23;
						intData[ 1u ] = (sint32)color.green << 23;
						intData[ 2u ] = (sint32)color.blue << 23;
					}
					break;

				case ImUiVertexElementType_Int4:
					{
						sint32* intData = (sint32*)elementData;
						intData[ 0u ] = (sint32)color.red << 23;
						intData[ 1u ] = (sint32)color.green << 23;
						intData[ 2u ] = (sint32)color.blue << 23;
						intData[ 3u ] = (sint32)color.alpha << 23;
					}
					break;

				case ImUiVertexElementType_UInt:
					*(uint32*)elementData = (color.red << 24u) | (color.green << 16u) | (color.blue << 8u) | color.alpha;
					break;

				case ImUiVertexElementType_UInt2:
					break;

				case ImUiVertexElementType_UInt3:
					{
						uint32* uintData = (uint32*)elementData;
						uintData[ 0u ] = color.red << 24u;
						uintData[ 1u ] = color.green << 24u;
						uintData[ 2u ] = color.blue << 24u;
					}
					break;

				case ImUiVertexElementType_UInt4:
					{
						uint32* uintData = (uint32*)elementData;
						uintData[ 0u ] = color.red << 24u;
						uintData[ 1u ] = color.green << 24u;
						uintData[ 2u ] = color.blue << 24u;
						uintData[ 3u ] = color.alpha << 24u;
					}
					break;

				case ImUiVertexElementType_MAX:
					break;
				}
			}
			break;

		case ImUiVertexElementSemantic_ColorABGR:
			{
				switch( vertexElement->type )
				{
				case ImUiVertexElementType_Float:
					break;

				case ImUiVertexElementType_Float2:
					break;

				case ImUiVertexElementType_Float3:
					break;

				case ImUiVertexElementType_Float4:
					{
						float* floatData = (float*)elementData;
						floatData[ 0u ] = color.alpha / 255.0f;
						floatData[ 1u ] = color.blue / 255.0f;
						floatData[ 2u ] = color.green / 255.0f;
						floatData[ 3u ] = color.red / 255.0f;
					}
					break;

				case ImUiVertexElementType_Int:
					break;

				case ImUiVertexElementType_Int2:
					break;

				case ImUiVertexElementType_Int3:
					break;

				case ImUiVertexElementType_Int4:
					{
						sint32* intData = (sint32*)elementData;
						intData[ 0u ] = (sint32)color.alpha << 23;
						intData[ 1u ] = (sint32)color.blue << 23;
						intData[ 2u ] = (sint32)color.green << 23;
						intData[ 3u ] = (sint32)color.red << 23;
					}
					break;

				case ImUiVertexElementType_UInt:
					*(uint32*)elementData = (color.alpha << 24u) | (color.blue << 16u) | (color.green << 8u) | color.red;
					break;

				case ImUiVertexElementType_UInt2:
					break;

				case ImUiVertexElementType_UInt3:
					break;

				case ImUiVertexElementType_UInt4:
					{
						uint32* uintData = (uint32*)elementData;
						uintData[ 0u ] = color.alpha << 24u;
						uintData[ 1u ] = color.blue << 24u;
						uintData[ 2u ] = color.green << 24u;
						uintData[ 3u ] = color.red << 24u;
					}
					break;

				case ImUiVertexElementType_MAX:
					break;
				}
			}
		break;
		}

		vertexOffset += vertexElementSize;
	}

	return vertexIndex;
}

static void ImUiDrawSurfacePushIndices( ImUiDrawSurfaceBuffers* buffers, const uint32* indices, uintsize count )
{
	IMUI_ASSERT( buffers->indexCount + count <= buffers->indexCapacity );

	uint32* targetIndices = buffers->indices + buffers->indexCount;
	for( uintsize i = 0u; i < count; ++i )
	{
		targetIndices[ i ] = indices[ i ];
	}

	buffers->indexCount += count;
}

static uintsize ImUiVertexElementTypeGetSize( ImUiVertexElementType type )
{
	switch( type )
	{
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
	case ImUiVertexElementType_MAX:		break;
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

	case ImUiDrawTopology_IndexedTriangleList:
		{
			const uint32 indexTl = ImUiDrawSurfacePushVertex( draw, surface, posTl.x, posTl.y, uv.u0, uv.v0, color );
			const uint32 indexTr = ImUiDrawSurfacePushVertex( draw, surface, posBr.x, posTl.y, uv.u1, uv.v0, color );
			const uint32 indexBl = ImUiDrawSurfacePushVertex( draw, surface, posTl.x, posBr.y, uv.u0, uv.v1, color );
			const uint32 indexBr = ImUiDrawSurfacePushVertex( draw, surface, posBr.x, posBr.y, uv.u1, uv.v1, color );

			uint32 vertexIndices[ 6u ];
			vertexIndices[ 0u ] = indexTl;
			vertexIndices[ 1u ] = indexTr;
			vertexIndices[ 2u ] = indexBl;
			vertexIndices[ 3u ] = indexBl;
			vertexIndices[ 4u ] = indexTr;
			vertexIndices[ 5u ] = indexBr;

			ImUiDrawSurfacePushIndices( surface->buffers, vertexIndices, IMUI_ARRAY_COUNT( vertexIndices ) );
		}
		return 6u;

	case ImUiDrawTopology_MAX:
		break;
	}

	return 0u;
}
