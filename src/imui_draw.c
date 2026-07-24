#include "imui_draw.h"

#include "imui_font.h"
#include "imui_internal.h"
#include "imui_memory.h"

#include <string.h>

typedef struct ImuiDrawSurfaceBuffers
{
	uint32*					indices;
	uintsize				indexCount;
	uintsize				indexCapacity;
	byte*					vertexData;
	uintsize				vertexCount;
	uintsize				vertexDataCapacity;
} ImuiDrawSurfaceBuffers;

struct ImuiDrawSurfaceData
{
	bool					used;

	ImuiStringView			name;
	ImuiSize				size;

	uintsize*				windows;
	uintsize				windowCapacity;
	uintsize				windowCount;

	ImuiDrawCommand*		commands;
	uintsize				commandCapacity;
	uintsize				commandCount;

	uintsize				approximatedIndexCount;
	uintsize				approximatedVertexCount;

	ImuiDrawData			data;

	ImuiDrawSurfaceBuffers*	buffers;
};

struct ImuiDrawWindowData
{
	bool					used;

	ImuiStringView			name;
	uintsize				surfaceIndex;
	uint32					zOrder;

	ImuiDrawElement*		elements;
	uintsize				elementCapacity;
	uintsize				elementCount;
};

typedef enum ImuiDrawSkinPointX
{
	ImuiDrawSkinPointX_Left,
	ImuiDrawSkinPointX_CenterLeft,
	ImuiDrawSkinPointX_CenterRight,
	ImuiDrawSkinPointX_Right,

	ImuiDrawSkinPointX_END = ImuiDrawSkinPointX_Right
} ImuiDrawSkinPointX;

typedef enum ImuiDrawSkinPointY
{
	ImuiDrawSkinPointY_Top,
	ImuiDrawSkinPointY_CenterTop,
	ImuiDrawSkinPointY_CenterBottom,
	ImuiDrawSkinPointY_Bottom,

	ImuiDrawSkinPointY_END = ImuiDrawSkinPointY_Bottom
} ImuiDrawSkinPointY;

static void					imuiDrawFreeWindow( ImuiDraw* draw, ImuiDrawWindowData* window );
static void					imuiDrawFreeSurface( ImuiDraw* draw, ImuiDrawSurfaceData* surface );
static ImuiDrawWindowData*	imuiDrawGetWindow( ImuiDraw* draw, ImuiWidget* widget );
static void					imuiDrawSurfaceGenerateElementData( ImuiDraw* draw, ImuiDrawSurfaceData* surface, const ImuiDrawElement* element );
static ImuiRect				imuiDrawSurfaceGenerateWidgetRect( ImuiWidget* widget );
static uint32				imuiDrawSurfacePushVertex( ImuiDraw* draw, ImuiDrawSurfaceData* surface, float x, float y, float u, float v, ImuiColor color );
static void					imuiDrawSurfacePushIndices( ImuiDrawSurfaceBuffers* buffers, const uint32* indices, uintsize count );
static uintsize				imuiDrawSurfacePushRect( ImuiDraw* draw, ImuiDrawSurfaceData* surface, ImuiPos posTl, ImuiPos posBr, ImuiTexCoord uv, ImuiColor color );
static uintsize				imuiVertexElementTypeGetSize( ImuiVertexElementType type );

#ifdef _DEBUG
static const bool s_supportedVertexElementSemanicFormats[][ ImuiVertexElementType_MAX ] =
{
	{ true,		true,	true,	true,	true,	true,	true,	true,	true,	true,	true,	true },		// ImuiVertexElementSemantic_None
	{ false,	true,	true,	true,	false,	true,	true,	true,	false,	true,	true,	true },		// ImuiVertexElementSemantic_PositionScreenSpace
	{ false,	true,	true,	true,	false,	true,	true,	true,	false,	false,	false,	false },	// ImuiVertexElementSemantic_PositionClipSpace
	{ false,	true,	true,	true,	false,	true,	true,	true,	false,	true,	true,	true },		// ImuiVertexElementSemantic_TextureCoordinate
	{ false,	false,	true,	true,	false,	false,	true,	true,	true,	false,	true,	true },		// ImuiVertexElementSemantic_ColorRGBA
	{ false,	false,	true,	true,	false,	false,	false,	true,	true,	false,	false,	true }		// ImuiVertexElementSemantic_ColorARGB
};
#endif

bool imuiDrawConstruct( ImuiDraw* draw, ImuiAllocator* allocator, const ImuiVertexFormat* vertexFormat, ImuiVertexType vertexType )
{
	draw->allocator		= allocator;
	draw->vertexType	= vertexType;

	switch( vertexType )
	{
	case ImuiVertexType_VertexList:			draw->triangleTopology = ImuiDrawTopology_TriangleList; break;
	case ImuiVertexType_IndexedVertexList:	draw->triangleTopology = ImuiDrawTopology_IndexedTriangleList; break;
	}

	const bool hasVertexFormat = vertexFormat->elementCount != 0u;
	const uintsize vertexElementCount = hasVertexFormat ? vertexFormat->elementCount : 3u;
	ImuiVertexElement* elements = IMUI_MEMORY_ARRAY_NEW( allocator, ImuiVertexElement, vertexElementCount );
	if( !elements  )
	{
		imuiDrawDestruct( draw );
		return false;
	}

	if( hasVertexFormat )
	{
		memcpy( elements, vertexFormat->elements, sizeof( ImuiVertexElement ) * vertexFormat->elementCount );
	}
	else
	{
		elements[ 0u ].align	= 1u;
		elements[ 0u ].type		= ImuiVertexElementType_Float2;
		elements[ 0u ].semantic	= ImuiVertexElementSemantic_PositionScreenSpace;
		elements[ 1u ].align	= 1u;
		elements[ 1u ].type		= ImuiVertexElementType_Float2;
		elements[ 1u ].semantic	= ImuiVertexElementSemantic_TextureCoordinate;
		elements[ 2u ].align	= 1u;
		elements[ 2u ].type		= ImuiVertexElementType_Float4;
		elements[ 2u ].semantic	= ImuiVertexElementSemantic_ColorRGBA;
	}

	draw->vertexFormat.elements		= elements;
	draw->vertexFormat.elementCount	= vertexElementCount;

	for( uintsize i = 0u; i < vertexElementCount; ++i )
	{
		const ImuiVertexElement* vertexElement = &elements[ i ];
		IMUI_ASSERT( vertexElement->align > 0u );
		IMUI_ASSERT( s_supportedVertexElementSemanicFormats[ vertexElement->semantic ][ vertexElement->type ] );

		draw->vertexSize = (draw->vertexSize + vertexElement->align - 1u) & (0u - vertexElement->align);
		draw->vertexSize += imuiVertexElementTypeGetSize( vertexElement->type );
	}

	return true;
}

void imuiDrawDestruct( ImuiDraw* draw )
{
	for( uintsize i = 0; i < draw->windowCapacity; ++i )
	{
		ImuiDrawWindowData* window = &draw->windows[ i ];
		imuiDrawFreeWindow( draw, window );
	}
	imuiMemoryFree( draw->allocator, draw->windows );

	for( uintsize i = 0; i < draw->surfaceCapacity; ++i )
	{
		ImuiDrawSurfaceData* surface = &draw->surfaces[ i ];
		imuiDrawFreeSurface( draw, surface );
	}
	imuiMemoryFree( draw->allocator, draw->surfaces );

	imuiMemoryFree( draw->allocator, draw->vertexFormat.elements );
}

uintsize imuiDrawRegisterSurface( ImuiDraw* draw, ImuiStringView name, ImuiSize size )
{
	for( uintsize i = 0; i < draw->surfaceCount; ++i )
	{
		ImuiDrawSurfaceData* surface = &draw->surfaces[ i ];
		if( !imuiStringViewIsEquals( surface->name, name ) )
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
	ImuiDrawSurfaceData* surface = &draw->surfaces[ surfaceIndex ];
	surface->used	= true;
	surface->name	= name;
	surface->size	= size;

	return surfaceIndex;
}

uintsize imuiDrawRegisterWindow( ImuiDraw* draw, ImuiStringView name, uintsize surfaceIndex, uint32 zOrder )
{
	uintsize windowIndex = IMUI_SIZE_MAX;
	for( uintsize i = 0; i < draw->windowCount; ++i )
	{
		ImuiDrawWindowData* window = &draw->windows[ i ];
		if( window->surfaceIndex != surfaceIndex ||
			!imuiStringViewIsEquals( window->name, name ) )
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

	ImuiDrawSurfaceData* surface = &draw->surfaces[ surfaceIndex ];

	if( !IMUI_MEMORY_ARRAY_CHECK_CAPACITY_ZERO( draw->allocator, surface->windows, surface->windowCapacity, surface->windowCount + 1u ) )
	{
		return IMUI_SIZE_MAX;
	}

	ImuiDrawWindowData* window = &draw->windows[ windowIndex ];
	window->used			= true;
	window->name			= name;
	window->surfaceIndex	= surfaceIndex;
	window->zOrder			= zOrder;
	window->elementCount	= 0u;

	surface->windows[ surface->windowCount++ ] = windowIndex;

	return windowIndex;
}

void imuiDrawSurfaceEnd( ImuiDraw* draw, uintsize surfaceIndex )
{
	ImuiDrawSurfaceData* surface = &draw->surfaces[ surfaceIndex ];

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

void imuiDrawEndFrame( ImuiDraw* draw )
{
	for( uintsize i = 0; i < draw->windowCount; ++i )
	{
		ImuiDrawWindowData* window = &draw->windows[ i ];

		if( !window->used )
		{
			imuiDrawFreeWindow( draw, window );

			IMUI_MEMORY_ARRAY_REMOVE_UNSORTED_ZERO( draw->windows, draw->windowCount, i );
			i--;
			continue;
		}

		window->used = false;
	}

	for( uintsize i = 0; i < draw->surfaceCount; ++i )
	{
		ImuiDrawSurfaceData* surface = &draw->surfaces[ i ];

		if( !surface->used )
		{
			imuiDrawFreeSurface( draw, surface );

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

ImuiDrawElement* imuiDrawPushElement( ImuiWidget* widget, ImuiDrawElementType type, uint64_t textureHandle )
{
	ImuiDraw* draw = &widget->window->surface->context->draw;
	ImuiDrawWindowData* window = imuiDrawGetWindow( draw, widget );

	if( !IMUI_MEMORY_ARRAY_CHECK_CAPACITY( draw->allocator, window->elements, window->elementCapacity, window->elementCount + 1u ) )
	{
		return NULL;
	}

	if( textureHandle == IMUI_TEXTURE_HANDLE_INVALID )
	{
		if( type == ImuiDrawElementType_Skin )
		{
			type = ImuiDrawElementType_Rect;
		}
		else if( type == ImuiDrawElementType_SkinPartial )
		{
			type = ImuiDrawElementType_RectPartial;
		}
	}

	ImuiDrawElement* element = &window->elements[ window->elementCount++ ];
	element->type			= type;
	element->textureHandle	= textureHandle;
	element->widget			= widget;

	uintsize indexCount = 0u;
	uintsize vertexCount = 0u;
	switch( type )
	{
	case ImuiDrawElementType_Line:			indexCount = 2u; vertexCount = 2u; break;
	case ImuiDrawElementType_Triangle:		indexCount = 3u; vertexCount = 3u; break;
	case ImuiDrawElementType_Rect:
	case ImuiDrawElementType_RectPartial:	indexCount = 6u; vertexCount = 4u; break;
	case ImuiDrawElementType_Skin:
	case ImuiDrawElementType_SkinPartial:	indexCount = 54u; vertexCount = 36u; break;
	case ImuiDrawElementType_Text:
	case ImuiDrawElementType_TextOffset:	break;
	}

	ImuiDrawSurfaceData* surface = &draw->surfaces[ widget->window->surface->drawIndex ];
	surface->approximatedIndexCount		+= indexCount;
	surface->approximatedVertexCount	+= vertexCount;

	return element;
}

ImuiDrawElement* imuiDrawPushElementText( ImuiWidget* widget, ImuiDrawElementType type, ImuiTextLayout* layout )
{
	ImuiDrawElement* element = imuiDrawPushElement( widget, type, layout->font->image.textureHandle );

	ImuiDraw* draw = &widget->window->surface->context->draw;
	ImuiDrawSurfaceData* surface = &draw->surfaces[ widget->window->surface->drawIndex ];
	surface->approximatedIndexCount		+= 6u * layout->glyphCount;
	surface->approximatedVertexCount	+= 4u * layout->glyphCount;

	return element;
}

void imuiDrawGetSurfaceMaxBufferSizes( ImuiDraw* draw, uintsize surfaceIndex, size_t* outVertexDataSize, size_t* outIndexDataSize )
{
	ImuiDrawSurfaceData* drawSurface = &draw->surfaces[ surfaceIndex ];

	if( draw->triangleTopology == ImuiDrawTopology_IndexedTriangleList )
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

const ImuiDrawData* imuiDrawGenerateSurfaceData( ImuiDraw* draw, uintsize surfaceIndex, void* outVertexData, size_t* inOutVertexDataSize, void* outIndexData, size_t* inOutIndexDataSize )
{
	ImuiDrawSurfaceData* surface = &draw->surfaces[ surfaceIndex ];

	IMUI_ASSERT( !inOutIndexDataSize || *inOutIndexDataSize >= surface->approximatedIndexCount * sizeof( uint32 ) );
	IMUI_ASSERT( *inOutVertexDataSize >= surface->approximatedVertexCount );

	ImuiDrawSurfaceBuffers buffers;
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
		const ImuiDrawWindowData* window = &draw->windows[ windowIndex ];

		for( uintsize elementIndex = 0u; elementIndex < window->elementCount; ++elementIndex )
		{
			const ImuiDrawElement* element = &window->elements[ elementIndex ];
			imuiDrawSurfaceGenerateElementData( draw, surface, element );
		}

#if 0
		// widget debug draw
		IMUI_MEMORY_ARRAY_CHECK_CAPACITY_ZERO( draw->allocator, drawSurface->commands, drawSurface->commandCapacity, drawSurface->commandCount + 1u );

		imuiDrawCommand* command = &drawSurface->commands[ drawSurface->commandCount++ ];
		command->topology	= ImuiDrawTopology_TriangleList;
		command->texture	= NULL;
		command->clipRect	= imuiRectCreatePosSize( imuiPosCreateZero(), surface->size );
		command->count		= 0u;

		const imuiTexCoord uv = { 0.0f, 0.0f, 0.0f, 0.0f };
		imuiWidget* widget = window->rootWidget;
		while( widget )
		{
			imuiDrawSurfacePreparePushRects( draw, drawSurface, 1u );

			const uint8* idBytes = (const uint8*)&widget->id;
			const imuiColor color = imuiColorCreate( idBytes[ 0u ], idBytes[ 1u ], idBytes[ 2u ], 0x40u );

			const imuiRect rect = widget->rect;
			//const imuiRect rect = widget->clipRect;
			//const imuiRect rect = imuiRectCreatePosSize( widget->rect.pos, imuiSizeMax( widget->minSize, widget->layoutContext.childrenMinSize ) );
			command->count += imuiDrawSurfacePushRect( draw, drawSurface, rect.pos, imuiRectGetBottomRight( rect ), uv, color );

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
				imuiWidget* nextWidget = widget->parent;
				while( nextWidget )
				{
					imuiWidget* nextNextWidget = nextWidget->nextSibling;
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

	ImuiDrawData* data = &surface->data;
	data->commands		= surface->commands;
	data->commandCount	= surface->commandCount;

	if( inOutIndexDataSize )
	{
		*inOutIndexDataSize = buffers.indexCount * sizeof( uint32 );
	}
	*inOutVertexDataSize = buffers.vertexCount * draw->vertexSize;

	return data;
}

static void imuiDrawFreeWindow( ImuiDraw* draw, ImuiDrawWindowData* window )
{
	IMUI_MEMORY_ARRAY_FREE( draw->allocator, window->elements, window->elementCapacity );
}

static void imuiDrawFreeSurface( ImuiDraw* draw, ImuiDrawSurfaceData* surface )
{
	IMUI_MEMORY_ARRAY_FREE( draw->allocator, surface->windows, surface->windowCapacity );
	IMUI_MEMORY_ARRAY_FREE( draw->allocator, surface->commands, surface->commandCapacity );
}

static ImuiDrawWindowData* imuiDrawGetWindow( ImuiDraw* draw, ImuiWidget* widget )
{
	return &draw->windows[ widget->window->drawIndex ];
}

static void imuiDrawSurfaceGenerateElementData( ImuiDraw* draw, ImuiDrawSurfaceData* surface, const ImuiDrawElement* element )
{
	if( !IMUI_MEMORY_ARRAY_CHECK_CAPACITY_ZERO( draw->allocator, surface->commands, surface->commandCapacity, surface->commandCount + 1u ) )
	{
		return;
	}

	ImuiDrawCommand* command = &surface->commands[ surface->commandCount++ ];
	command->topology		= element->type == ImuiDrawElementType_Line ? ImuiDrawTopology_LineList : draw->triangleTopology;
	command->textureHandle	= element->textureHandle;
	command->clipRect		= element->widget->clipRect;
	command->count			= 0u;

	ImuiRect rect = imuiDrawSurfaceGenerateWidgetRect( element->widget );
	switch( element->type )
	{
	case ImuiDrawElementType_Line:
		{
			const struct ImuiDrawElementDataPrimitive* primitiveData = &element->data.primitive;

			uint32 vertexIndices[ 2u ];
			vertexIndices[ 0u ] = imuiDrawSurfacePushVertex( draw, surface, rect.pos.x + primitiveData->p0.x, rect.pos.y + primitiveData->p0.y, 0.0f, 0.0f, primitiveData->color );
			vertexIndices[ 1u ] = imuiDrawSurfacePushVertex( draw, surface, rect.pos.x + primitiveData->p1.x, rect.pos.y + primitiveData->p1.y, 0.0f, 0.0f, primitiveData->color );

			if( draw->triangleTopology == ImuiDrawTopology_IndexedTriangleList )
			{
				imuiDrawSurfacePushIndices( surface->buffers, vertexIndices, IMUI_ARRAY_COUNT( vertexIndices ) );
			}

			command->count = IMUI_ARRAY_COUNT( vertexIndices );
		}
		break;

	case ImuiDrawElementType_Triangle:
		{
			const struct ImuiDrawElementDataPrimitive* primitiveData = &element->data.primitive;

			uint32 vertexIndices[ 3u ];
			vertexIndices[ 0u ] = imuiDrawSurfacePushVertex( draw, surface, rect.pos.x + primitiveData->p0.x, rect.pos.y + primitiveData->p0.y, 0.0f, 0.0f, primitiveData->color );
			vertexIndices[ 1u ] = imuiDrawSurfacePushVertex( draw, surface, rect.pos.x + primitiveData->p1.x, rect.pos.y + primitiveData->p1.y, 0.0f, 0.0f, primitiveData->color );
			vertexIndices[ 2u ] = imuiDrawSurfacePushVertex( draw, surface, rect.pos.x + primitiveData->p2.x, rect.pos.y + primitiveData->p2.y, 0.0f, 0.0f, primitiveData->color );

			if( draw->triangleTopology == ImuiDrawTopology_IndexedTriangleList )
			{
				imuiDrawSurfacePushIndices( surface->buffers, vertexIndices, IMUI_ARRAY_COUNT( vertexIndices ) );
			}

			command->count = IMUI_ARRAY_COUNT( vertexIndices );
		}
		break;

	case ImuiDrawElementType_RectPartial:
		{
			const struct ImuiDrawElementDataRect* rectData = &element->data.rect;

			rect.pos.x	+= rectData->relativRect.pos.x;
			rect.pos.y	+= rectData->relativRect.pos.y;
			rect.size	= rectData->relativRect.size;
		}
		// no break;

	case ImuiDrawElementType_Rect:
		{
			const struct ImuiDrawElementDataRect* rectData = &element->data.rect;

			const ImuiPos posTl = imuiRectGetTopLeft( rect );
			const ImuiPos posBr = imuiRectGetBottomRight( rect );

			command->count = imuiDrawSurfacePushRect( draw, surface, posTl, posBr, rectData->uv, rectData->color );
		}
		break;

	case ImuiDrawElementType_SkinPartial:
		{
			const struct ImuiDrawElementDataSkin* skinData = &element->data.skin;

			rect.pos.x	+= skinData->relativRect.pos.x;
			rect.pos.y	+= skinData->relativRect.pos.y;
			rect.size	= skinData->relativRect.size;
		}
		// no break;

	case ImuiDrawElementType_Skin:
		{
			const struct ImuiDrawElementDataSkin* skinData = &element->data.skin;

			const float uScale = skinData->texSize.width ? (skinData->uv.u1 - skinData->uv.u0) / skinData->texSize.width : 0.0f;
			const float vScale = skinData->texSize.height ? (skinData->uv.v1 - skinData->uv.v0) / skinData->texSize.height : 0.0f;

			ImuiBorder uvBorder = skinData->border;
			uvBorder.top	*= vScale;
			uvBorder.left	*= uScale;
			uvBorder.bottom	*= vScale;
			uvBorder.right	*= uScale;

			const ImuiSize borderSize = imuiBorderGetMinSize( skinData->border );
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

			for( uintsize y = 0; y < ImuiDrawSkinPointY_END; ++y )
			{
				const uintsize nextY = y + 1u;

				for( uintsize x = 0; x < ImuiDrawSkinPointX_END; ++x )
				{
					const uintsize nextX = x + 1u;

					const ImuiPos posTl = imuiPosCreate( xPositions[ x ], yPositions[ y ] );
					const ImuiPos posBr = imuiPosCreate( xPositions[ nextX ], yPositions[ nextY ] );

					ImuiTexCoord uv;
					uv.u0 = uPositions[ x ];
					uv.v0 = vPositions[ y ];
					uv.u1 = uPositions[ nextX ];
					uv.v1 = vPositions[ nextY ];

					command->count += imuiDrawSurfacePushRect( draw, surface, posTl, posBr, uv, skinData->color );
				}
			}
		}
		break;

	case ImuiDrawElementType_TextOffset:
		{
			const struct ImuiDrawElementDataText* textData = &element->data.text;

			rect.pos.x	+= textData->relativPos.x;
			rect.pos.y	+= textData->relativPos.y;
		}
		// no break;


	case ImuiDrawElementType_Text:
		{
			const struct ImuiDrawElementDataText* textData = &element->data.text;
			const float scale = textData->size / textData->layout->font->fontSize;

			const float x = rect.pos.x;
			const float y = rect.pos.y;
			for( uintsize i = 0; i < textData->layout->glyphCount; ++i )
			{
				const ImuiTextGlyph* glyph = &textData->layout->glyphs[ i ];

				const ImuiPos glyphPos		= imuiPosScale( glyph->pos, scale );
				const ImuiSize glyphSize	= imuiSizeScale( glyph->size, scale );
				const ImuiPos posTl			= imuiPosCreate( x + glyphPos.x, y + glyphPos.y );
				const ImuiPos posBr			= imuiPosCreate( posTl.x + glyphSize.width, posTl.y + glyphSize.height );

				command->count += imuiDrawSurfacePushRect( draw, surface, posTl, posBr, glyph->uv, textData->color );
			}
		}
		break;
	}
}

static ImuiRect imuiDrawSurfaceGenerateWidgetRect( ImuiWidget* widget )
{
	//imuiRect rect;
	//rect.pos.x			= floorf( widget->rect.pos.x );
	//rect.pos.y			= floorf( widget->rect.pos.y );
	//rect.size.width		= floorf( widget->rect.size.width );
	//rect.size.height	= floorf( widget->rect.size.height );
	//return rect;
	return widget->rect;
}

static uint32 imuiDrawSurfacePushVertex( ImuiDraw* draw, ImuiDrawSurfaceData* surface, float x, float y, float u, float v, ImuiColor color )
{
	const uint32 vertexIndex = (uint32)surface->buffers->vertexCount;
	surface->buffers->vertexCount++;

	uintsize vertexOffset = 0u;
	uint8* vertex = surface->buffers->vertexData + (draw->vertexSize * vertexIndex);
	IMUI_ASSERT( vertex + draw->vertexSize <= surface->buffers->vertexData + surface->buffers->vertexDataCapacity );

	for( uintsize i = 0u; i < draw->vertexFormat.elementCount; ++i )
	{
		const ImuiVertexElement* vertexElement = &draw->vertexFormat.elements[ i ];
		const uintsize vertexElementSize = imuiVertexElementTypeGetSize( vertexElement->type );

		vertexOffset = (vertexOffset + vertexElement->align - 1u) & (0u - vertexElement->align);

		uint8* elementData = vertex + vertexOffset;
		switch( vertexElement->semantic )
		{
		case ImuiVertexElementSemantic_Zero:
			memset( elementData, 0, vertexElementSize );
			break;

		case ImuiVertexElementSemantic_PositionScreenSpace:
			{
				switch( vertexElement->type )
				{
				case ImuiVertexElementType_Float:
					break;

				case ImuiVertexElementType_Float2:
					{
						float* floatData = (float*)elementData;
						floatData[ 0u ] = x;
						floatData[ 1u ] = y;
					}
					break;

				case ImuiVertexElementType_Float3:
					{
						float* floatData = (float*)elementData;
						floatData[ 0u ] = x;
						floatData[ 1u ] = y;
						floatData[ 2u ] = 0.0f;
					}
					break;

				case ImuiVertexElementType_Float4:
					{
						float* floatData = (float*)elementData;
						floatData[ 0u ] = x;
						floatData[ 1u ] = y;
						floatData[ 2u ] = 0.0f;
						floatData[ 3u ] = 1.0f;
					}
					break;

				case ImuiVertexElementType_Int:
					break;

				case ImuiVertexElementType_Int2:
					{
						sint32* intData = (sint32*)elementData;
						intData[ 0u ] = (sint32)x;
						intData[ 1u ] = (sint32)y;
					}
					break;

				case ImuiVertexElementType_Int3:
					{
						sint32* intData = (sint32*)elementData;
						intData[ 0u ] = (sint32)x;
						intData[ 1u ] = (sint32)y;
						intData[ 2u ] = 0;
					}
					break;

				case ImuiVertexElementType_Int4:
					{
						sint32* intData = (sint32*)elementData;
						intData[ 0u ] = (sint32)x;
						intData[ 1u ] = (sint32)y;
						intData[ 2u ] = 0;
						intData[ 3u ] = 0x7fffffff;
					}
					break;

				case ImuiVertexElementType_UInt:
					break;

				case ImuiVertexElementType_UInt2:
					{
						uint32* uintData = (uint32*)elementData;
						uintData[ 0u ] = (uint32)x;
						uintData[ 1u ] = (uint32)y;
					}
					break;

				case ImuiVertexElementType_UInt3:
					{
						uint32* uintData = (uint32*)elementData;
						uintData[ 0u ] = (uint32)x;
						uintData[ 1u ] = (uint32)y;
						uintData[ 2u ] = 0u;
					}
					break;

				case ImuiVertexElementType_UInt4:
					{
						uint32* uintData = (uint32*)elementData;
						uintData[ 0u ] = (uint32)x;
						uintData[ 1u ] = (uint32)y;
						uintData[ 2u ] = 0u;
						uintData[ 3u ] = 0xffffffffu;
					}
					break;

				case ImuiVertexElementType_MAX:
					break;
				}
			}
			break;

		case ImuiVertexElementSemantic_PositionClipSpace:
			{
				const ImuiPos clipSpacePosition = imuiPosCreate(
					-1.0f + (x / surface->size.width),
					1.0f - (y / surface->size.height)
				);

				switch( vertexElement->type )
				{
				case ImuiVertexElementType_Float:
					break;

				case ImuiVertexElementType_Float2:
					{
						float* floatData = (float*)elementData;
						floatData[ 0u ] = clipSpacePosition.x;
						floatData[ 1u ] = clipSpacePosition.y;
					}
					break;

				case ImuiVertexElementType_Float3:
					{
						float* floatData = (float*)elementData;
						floatData[ 0u ] = clipSpacePosition.x;
						floatData[ 1u ] = clipSpacePosition.y;
						floatData[ 2u ] = 0.0f;
					}
					break;

				case ImuiVertexElementType_Float4:
					{
						float* floatData = (float*)elementData;
						floatData[ 0u ] = clipSpacePosition.x;
						floatData[ 1u ] = clipSpacePosition.y;
						floatData[ 2u ] = 0.0f;
						floatData[ 3u ] = 1.0f;
					}
					break;

				case ImuiVertexElementType_Int:
					break;

				case ImuiVertexElementType_Int2:
					{
						sint32* intData = (sint32*)elementData;
						intData[ 0u ] = (sint32)(clipSpacePosition.x * 2147483647.0f + 0.5f);
						intData[ 1u ] = (sint32)(clipSpacePosition.y * 2147483647.0f + 0.5f);
					}
					break;

				case ImuiVertexElementType_Int3:
					{
						sint32* intData = (sint32*)elementData;
						intData[ 0u ] = (sint32)(clipSpacePosition.x * 2147483647.0f + 0.5f);
						intData[ 1u ] = (sint32)(clipSpacePosition.y * 2147483647.0f + 0.5f);
						intData[ 2u ] = 0;
					}
					break;

				case ImuiVertexElementType_Int4:
					{
						sint32* intData = (sint32*)elementData;
						intData[ 0u ] = (sint32)(clipSpacePosition.x * 2147483647.0f + 0.5f);
						intData[ 1u ] = (sint32)(clipSpacePosition.y * 2147483647.0f + 0.5f);
						intData[ 2u ] = 0;
						intData[ 3u ] = 0x7fffffff;
					}
					break;

				case ImuiVertexElementType_UInt:
					break;

				case ImuiVertexElementType_UInt2:
					break;

				case ImuiVertexElementType_UInt3:
					break;

				case ImuiVertexElementType_UInt4:
					break;

				case ImuiVertexElementType_MAX:
					break;
				}
			}
			break;

		case ImuiVertexElementSemantic_TextureCoordinate:
			{
				switch( vertexElement->type )
				{
				case ImuiVertexElementType_Float:
					break;

				case ImuiVertexElementType_Float2:
					{
						float* floatData = (float*)elementData;
						floatData[ 0u ] = u;
						floatData[ 1u ] = v;
					}
					break;

				case ImuiVertexElementType_Float3:
					{
						float* floatData = (float*)elementData;
						floatData[ 0u ] = u;
						floatData[ 1u ] = v;
						floatData[ 2u ] = 0.0f;
					}
					break;

				case ImuiVertexElementType_Float4:
					{
						float* floatData = (float*)elementData;
						floatData[ 0u ] = u;
						floatData[ 1u ] = v;
						floatData[ 2u ] = 0.0f;
						floatData[ 3u ] = 0.0f;
					}
					break;

				case ImuiVertexElementType_Int:
					break;

				case ImuiVertexElementType_Int2:
					{
						sint32* intData = (sint32*)elementData;
						intData[ 0u ] = (sint32)(u * 2147483647.0f + 0.5f);
						intData[ 1u ] = (sint32)(v * 2147483647.0f + 0.5f);
					}
					break;

				case ImuiVertexElementType_Int3:
					{
						sint32* intData = (sint32*)elementData;
						intData[ 0u ] = (sint32)(u * 2147483647.0f + 0.5f);
						intData[ 1u ] = (sint32)(v * 2147483647.0f + 0.5f);
						intData[ 2u ] = 0;
					}
					break;

				case ImuiVertexElementType_Int4:
					{
						sint32* intData = (sint32*)elementData;
						intData[ 0u ] = (sint32)(u * 2147483647.0f + 0.5f);
						intData[ 1u ] = (sint32)(v * 2147483647.0f + 0.5f);
						intData[ 2u ] = 0;
						intData[ 3u ] = 0;
					}
					break;

				case ImuiVertexElementType_UInt:
					break;

				case ImuiVertexElementType_UInt2:
					{
						uint32* uintData = (uint32*)elementData;
						uintData[ 0u ] = (uint32)(u * 4294967295.0f + 0.5f);
						uintData[ 1u ] = (uint32)(v * 4294967295.0f + 0.5f);
					}
					break;

				case ImuiVertexElementType_UInt3:
					{
						uint32* uintData = (uint32*)elementData;
						uintData[ 0u ] = (uint32)(u * 4294967295.0f + 0.5f);
						uintData[ 1u ] = (uint32)(v * 4294967295.0f + 0.5f);
						uintData[ 2u ] = 0u;
					}
					break;

				case ImuiVertexElementType_UInt4:
					{
						uint32* uintData = (uint32*)elementData;
						uintData[ 0u ] = (uint32)(u * 4294967295.0f + 0.5f);
						uintData[ 1u ] = (uint32)(v * 4294967295.0f + 0.5f);
						uintData[ 2u ] = 0u;
						uintData[ 3u ] = 0u;
					}
					break;

				case ImuiVertexElementType_MAX:
					break;
				}
			}
			break;

		case ImuiVertexElementSemantic_ColorRGBA:
			{
				switch( vertexElement->type )
				{
				case ImuiVertexElementType_Float:
					break;

				case ImuiVertexElementType_Float2:
					break;

				case ImuiVertexElementType_Float3:
					{
						float* floatData = (float*)elementData;
						floatData[ 0u ] = color.red / 255.0f;
						floatData[ 1u ] = color.green / 255.0f;
						floatData[ 2u ] = color.blue / 255.0f;
					}
					break;

				case ImuiVertexElementType_Float4:
					{
						float* floatData = (float*)elementData;
						floatData[ 0u ] = color.red / 255.0f;
						floatData[ 1u ] = color.green / 255.0f;
						floatData[ 2u ] = color.blue / 255.0f;
						floatData[ 3u ] = color.alpha / 255.0f;
					}
					break;

				case ImuiVertexElementType_Int:
					break;

				case ImuiVertexElementType_Int2:
					break;

				case ImuiVertexElementType_Int3:
					{
						sint32* intData = (sint32*)elementData;
						intData[ 0u ] = (sint32)color.red << 23;
						intData[ 1u ] = (sint32)color.green << 23;
						intData[ 2u ] = (sint32)color.blue << 23;
					}
					break;

				case ImuiVertexElementType_Int4:
					{
						sint32* intData = (sint32*)elementData;
						intData[ 0u ] = (sint32)color.red << 23;
						intData[ 1u ] = (sint32)color.green << 23;
						intData[ 2u ] = (sint32)color.blue << 23;
						intData[ 3u ] = (sint32)color.alpha << 23;
					}
					break;

				case ImuiVertexElementType_UInt:
					*(uint32*)elementData = (color.red << 24u) | (color.green << 16u) | (color.blue << 8u) | color.alpha;
					break;

				case ImuiVertexElementType_UInt2:
					break;

				case ImuiVertexElementType_UInt3:
					{
						uint32* uintData = (uint32*)elementData;
						uintData[ 0u ] = color.red << 24u;
						uintData[ 1u ] = color.green << 24u;
						uintData[ 2u ] = color.blue << 24u;
					}
					break;

				case ImuiVertexElementType_UInt4:
					{
						uint32* uintData = (uint32*)elementData;
						uintData[ 0u ] = color.red << 24u;
						uintData[ 1u ] = color.green << 24u;
						uintData[ 2u ] = color.blue << 24u;
						uintData[ 3u ] = color.alpha << 24u;
					}
					break;

				case ImuiVertexElementType_MAX:
					break;
				}
			}
			break;

		case ImuiVertexElementSemantic_ColorABGR:
			{
				switch( vertexElement->type )
				{
				case ImuiVertexElementType_Float:
					break;

				case ImuiVertexElementType_Float2:
					break;

				case ImuiVertexElementType_Float3:
					break;

				case ImuiVertexElementType_Float4:
					{
						float* floatData = (float*)elementData;
						floatData[ 0u ] = color.alpha / 255.0f;
						floatData[ 1u ] = color.blue / 255.0f;
						floatData[ 2u ] = color.green / 255.0f;
						floatData[ 3u ] = color.red / 255.0f;
					}
					break;

				case ImuiVertexElementType_Int:
					break;

				case ImuiVertexElementType_Int2:
					break;

				case ImuiVertexElementType_Int3:
					break;

				case ImuiVertexElementType_Int4:
					{
						sint32* intData = (sint32*)elementData;
						intData[ 0u ] = (sint32)color.alpha << 23;
						intData[ 1u ] = (sint32)color.blue << 23;
						intData[ 2u ] = (sint32)color.green << 23;
						intData[ 3u ] = (sint32)color.red << 23;
					}
					break;

				case ImuiVertexElementType_UInt:
					*(uint32*)elementData = (color.alpha << 24u) | (color.blue << 16u) | (color.green << 8u) | color.red;
					break;

				case ImuiVertexElementType_UInt2:
					break;

				case ImuiVertexElementType_UInt3:
					break;

				case ImuiVertexElementType_UInt4:
					{
						uint32* uintData = (uint32*)elementData;
						uintData[ 0u ] = color.alpha << 24u;
						uintData[ 1u ] = color.blue << 24u;
						uintData[ 2u ] = color.green << 24u;
						uintData[ 3u ] = color.red << 24u;
					}
					break;

				case ImuiVertexElementType_MAX:
					break;
				}
			}
		break;
		}

		vertexOffset += vertexElementSize;
	}

	return vertexIndex;
}

static void imuiDrawSurfacePushIndices( ImuiDrawSurfaceBuffers* buffers, const uint32* indices, uintsize count )
{
	IMUI_ASSERT( buffers->indexCount + count <= buffers->indexCapacity );

	uint32* targetIndices = buffers->indices + buffers->indexCount;
	for( uintsize i = 0u; i < count; ++i )
	{
		targetIndices[ i ] = indices[ i ];
	}

	buffers->indexCount += count;
}

static uintsize imuiVertexElementTypeGetSize( ImuiVertexElementType type )
{
	switch( type )
	{
	case ImuiVertexElementType_Float:	return 4u;
	case ImuiVertexElementType_Float2:	return 8u;
	case ImuiVertexElementType_Float3:	return 12u;
	case ImuiVertexElementType_Float4:	return 16u;
	case ImuiVertexElementType_Int:		return 4u;
	case ImuiVertexElementType_Int2:	return 8u;
	case ImuiVertexElementType_Int3:	return 12u;
	case ImuiVertexElementType_Int4:	return 16u;
	case ImuiVertexElementType_UInt:	return 4u;
	case ImuiVertexElementType_UInt2:	return 8u;
	case ImuiVertexElementType_UInt3:	return 12u;
	case ImuiVertexElementType_UInt4:	return 16u;
	case ImuiVertexElementType_MAX:		break;
	}

	return 0u;
}

static uintsize imuiDrawSurfacePushRect( ImuiDraw* draw, ImuiDrawSurfaceData* surface, ImuiPos posTl, ImuiPos posBr, ImuiTexCoord uv, ImuiColor color )
{
	if( posBr.x - posTl.x <= 0.0f ||
		posBr.y - posTl.y <= 0.0f )
	{
		return 0u;
	}

	switch( draw->triangleTopology )
	{
	case ImuiDrawTopology_LineList:
		break;

	case ImuiDrawTopology_TriangleList:
		{
			imuiDrawSurfacePushVertex( draw, surface, posTl.x, posTl.y, uv.u0, uv.v0, color );
			imuiDrawSurfacePushVertex( draw, surface, posBr.x, posTl.y, uv.u1, uv.v0, color );
			imuiDrawSurfacePushVertex( draw, surface, posTl.x, posBr.y, uv.u0, uv.v1, color );
			imuiDrawSurfacePushVertex( draw, surface, posTl.x, posBr.y, uv.u0, uv.v1, color );
			imuiDrawSurfacePushVertex( draw, surface, posBr.x, posTl.y, uv.u1, uv.v0, color );
			imuiDrawSurfacePushVertex( draw, surface, posBr.x, posBr.y, uv.u1, uv.v1, color );
		}
		return 6u;

	case ImuiDrawTopology_IndexedTriangleList:
		{
			const uint32 indexTl = imuiDrawSurfacePushVertex( draw, surface, posTl.x, posTl.y, uv.u0, uv.v0, color );
			const uint32 indexTr = imuiDrawSurfacePushVertex( draw, surface, posBr.x, posTl.y, uv.u1, uv.v0, color );
			const uint32 indexBl = imuiDrawSurfacePushVertex( draw, surface, posTl.x, posBr.y, uv.u0, uv.v1, color );
			const uint32 indexBr = imuiDrawSurfacePushVertex( draw, surface, posBr.x, posBr.y, uv.u1, uv.v1, color );

			uint32 vertexIndices[ 6u ];
			vertexIndices[ 0u ] = indexTl;
			vertexIndices[ 1u ] = indexTr;
			vertexIndices[ 2u ] = indexBl;
			vertexIndices[ 3u ] = indexBl;
			vertexIndices[ 4u ] = indexTr;
			vertexIndices[ 5u ] = indexBr;

			imuiDrawSurfacePushIndices( surface->buffers, vertexIndices, IMUI_ARRAY_COUNT( vertexIndices ) );
		}
		return 6u;

	case ImuiDrawTopology_MAX:
		break;
	}

	return 0u;
}
