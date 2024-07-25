#include "imui/imui.h"

#include "imui_draw.h"
#include "imui_font.h"
#include "imui_internal.h"
#include "imui_memory.h"

#include <string.h>
#include <math.h>

static void			ImUiWindowLayout( ImUiWindow* window );

static ImUiWidget*	ImUiWidgetAlloc( ImUiContext* imui );
static void			ImUiWidgetUpdateLayoutContext( ImUiWidget* widget, uintsize widgetIndex, float dpiScale, bool update );
static void			ImUiWidgetUpdateLayoutContextCheckGridContextSize( ImUiWidget* widget );
static void			ImUiWidgetUpdateLayoutContextGrid( ImUiWidget* widget, float dpiScale );
static void			ImUiWidgetLayout( ImUiWidget* widget, const ImUiRect* parentInnerRect, float dpiScale, uintsize widgetIndex, bool update );
static void			ImUiWidgetLayoutPrepareGrid( ImUiWidget* widget, const ImUiRect* parentInnerRect, float dpiScale );
static void			ImUiWidgetLayoutStack( ImUiWidget* widget, const ImUiRect* parentInnerRect, float dpiScale );
static void			ImUiWidgetLayoutScroll( ImUiWidget* widget, const ImUiRect* parentInnerRect, float dpiScale );
static void			ImUiWidgetLayoutHorizontal( ImUiWidget* widget, const ImUiRect* parentInnerRect, float dpiScale );
static void			ImUiWidgetLayoutVertical( ImUiWidget* widget, const ImUiRect* parentInnerRect, float dpiScale );
static void			ImUiWidgetLayoutGrid( ImUiWidget* widget, const ImUiRect* parentInnerRect, float dpiScale, uintsize widgetIndex );
static ImUiSize		ImUiWidgetLayoutMinSize( ImUiWidget* widget, const ImUiRect* parentInnerRect, float dpiScale );
static float		ImUiWidgetLayoutPositionX( ImUiWidget* widget, const ImUiRect* parentInnerRect, float width, float dpiScale );
static float		ImUiWidgetLayoutPositionY( ImUiWidget* widget, const ImUiRect* parentInnerRect, float height, float dpiScale );
static ImUiSize		ImUiWidgetCalculateSize( ImUiWidget* widget, ImUiSize minSize, ImUiSize maxSize, float factorWidth, float factorHeight, float dpiScale );

static void			ImUiWidgetStateFreeList( ImUiAllocator* allocator, ImUiWidgetState* firstState );
static void			ImUiLayoutGridContextFreeList( ImUiAllocator* allocator, ImUiLayoutGridContext* firstContext );

static const ImUiLayoutContext IMUI_DEFAULT_LAYOUT_CONTEXT =
{
	.childrenMaxStretch =
	{
		.width = 1.0f,
		.height = 1.0f
	}
	/*.childrenMinSize = {
		.width = IMUI_FLOAT_MAX,
		.height = IMUI_FLOAT_MAX
	}*/
};

static const ImUiWidget IMUI_DEFAULT_WIDGET =
{
	.maxSize = {
		.width = IMUI_FLOAT_INF,
		.height = IMUI_FLOAT_INF
	},
	.layoutContext = {
		.childrenMaxStretch =
		{
			.width = 1.0f,
			.height = 1.0f
		}
	}
};

ImUiContext* ImUiCreate( const ImUiParameters* parameters )
{
	ImUiAllocator allocator;
	ImUiMemoryAllocatorPrepare( &allocator, &parameters->allocator );

	ImUiContext* imui = IMUI_MEMORY_NEW_ZERO( &allocator, ImUiContext );
	if( !imui )
	{
		return NULL;
	}

	ImUiMemoryAllocatorFinalize( &imui->allocator, &allocator );

	ImUiInputConstruct( &imui->input, &imui->allocator );

	if( !ImUiDrawConstruct( &imui->draw, &imui->allocator, &parameters->vertexFormat, parameters->vertexType ) ||
		!ImUiStringPoolConstruct( &imui->strings, &imui->allocator ) ||
		!ImUiTextLayoutCacheConstruct( &imui->layoutCache, &imui->allocator ) )
	{
		ImUiDestroy( imui );
		return NULL;
	}

	return imui;
}

void ImUiDestroy( ImUiContext* imui )
{
	for( uintsize i = 0u; i < imui->surfaceCapacity; ++i )
	{
		ImUiSurface* surface = &imui->surfaces[ i ];
		ImUiMemoryFree( &imui->allocator, surface->windows );
	}
	ImUiMemoryFree( &imui->allocator, imui->surfaces );

	ImUiWidgetStateFreeList( &imui->allocator, imui->firstState );
	ImUiWidgetStateFreeList( &imui->allocator, imui->firstUnusedState );

	ImUiLayoutGridContextFreeList( &imui->allocator, imui->firstGridContext );
	ImUiLayoutGridContextFreeList( &imui->allocator, imui->firstUnusedGridContext );

	for( ImUiWidgetChunk* pChunk = imui->firstChunk; pChunk != NULL; )
	{
		ImUiWidgetChunk* pNextChunk = pChunk->nextChunk;
		ImUiMemoryFree( &imui->allocator, pChunk );
		pChunk = pNextChunk;
	}

	for( ImUiWidgetChunk* pChunk = imui->firstLastFrameChunk; pChunk != NULL; )
	{
		ImUiWidgetChunk* pNextChunk = pChunk->nextChunk;
		ImUiMemoryFree( &imui->allocator, pChunk );
		pChunk = pNextChunk;
	}

	for( ImUiWidgetChunk* pChunk = imui->firstFreeChunk; pChunk != NULL; )
	{
		ImUiWidgetChunk* pNextChunk = pChunk->nextChunk;
		ImUiMemoryFree( &imui->allocator, pChunk );
		pChunk = pNextChunk;
	}

	ImUiInputDestruct( &imui->input );
	ImUiDrawDestruct( &imui->draw );
	ImUiStringPoolDestruct( &imui->strings );
	ImUiTextLayoutCacheDestruct( &imui->layoutCache );

	ImUiMemoryFree( &imui->allocator, imui );
}

ImUiFrame* ImUiBegin( ImUiContext* imui, float timeInSeconds )
{
	imui->frame.imui			= imui;
	imui->frame.index++;
	imui->frame.timeInSeconds	= timeInSeconds;

	return &imui->frame;
}

void ImUiEnd( ImUiFrame* frame )
{
	ImUiContext* imui = frame->imui;

	ImUiDrawEndFrame( &imui->draw );

	// deleted unused surfaces and windows
	for( uintsize surfaceIndex = 0u; surfaceIndex < imui->surfaceCount; ++surfaceIndex )
	{
		ImUiSurface* surface = &imui->surfaces[ surfaceIndex ];
		if( !surface->inUse )
		{
			ImUiMemoryFree( &imui->allocator, surface->windows );

			IMUI_MEMORY_ARRAY_REMOVE_UNSORTED_ZERO( imui->surfaces, imui->surfaceCount, surfaceIndex );

			surfaceIndex--;
			continue;
		}

		for( uintsize windowIndex = 0u; windowIndex < surface->windowCount; ++windowIndex )
		{
			ImUiWindow* window = &surface->windows[ windowIndex ];
			if( !window->inUse )
			{
				IMUI_MEMORY_ARRAY_REMOVE_UNSORTED_ZERO( surface->windows, surface->windowCount, windowIndex );

				windowIndex--;
				continue;
			}

			window->inUse = false;
		}

		surface->inUse = false;
	}

	// clear last frame widget chunks
	{
		ImUiWidgetChunk* lastFreeChunk = NULL;
		for( ImUiWidgetChunk* chunk = imui->firstLastFrameChunk; chunk != NULL; chunk = chunk->nextChunk )
		{
			chunk->usedCount = 0u;

			lastFreeChunk = chunk;
		}

		if( lastFreeChunk )
		{
			lastFreeChunk->nextChunk = imui->firstFreeChunk;
		}
		imui->firstFreeChunk = imui->firstLastFrameChunk;

		imui->firstLastFrameChunk	= imui->firstChunk;
		imui->firstChunk			= NULL;
	}

	// free unused states
	ImUiWidgetStateFreeList( &imui->allocator, imui->firstUnusedState );
	imui->firstUnusedState	= imui->firstState;
	imui->firstState		= NULL;


	// free unused grid context
	ImUiLayoutGridContextFreeList( &imui->allocator, imui->firstUnusedGridContext );
	imui->firstUnusedGridContext	= imui->firstGridContext;
	imui->firstGridContext			= NULL;

	ImUiTextLayoutCacheEndFrame( &imui->layoutCache );
}

ImUiContext* ImUiFrameGetContext( const ImUiFrame* frame )
{
	return frame->imui;
}

ImUiSurface* ImUiSurfaceBegin( ImUiFrame* frame, const char* name, ImUiSize size, float dpiScale )
{
	ImUiContext* imui = frame->imui;
	const ImUiStringView nameView = ImUiStringViewCreate( name );

	ImUiSurface* surface = NULL;
	for( uintsize surfaceIndex = 0u; surfaceIndex < imui->surfaceCount; ++surfaceIndex )
	{
		if( !ImUiStringViewIsEquals( imui->surfaces[ surfaceIndex ].name, nameView ) )
		{
			continue;
		}

		surface = &imui->surfaces[ surfaceIndex ];
		IMUI_ASSERT( !surface->inUse && "Surface name must be unique" );
		break;
	}

	if( !surface )
	{
		if( !IMUI_MEMORY_ARRAY_CHECK_CAPACITY_ZERO( &imui->allocator, imui->surfaces, imui->surfaceCapacity, imui->surfaceCount + 1u ) )
		{
			return NULL;
		}

		surface = &imui->surfaces[ imui->surfaceCount ];
		imui->surfaceCount++;

		memset( surface, 0, sizeof( *surface ) );
	}

	surface->inUse		= true;
	surface->imui		= imui;
	surface->name		= ImUiStringPoolAdd( &imui->strings, nameView );
	surface->size		= size;
	surface->dpiScale	= dpiScale;
	surface->drawIndex	= ImUiDrawRegisterSurface( &imui->draw, surface->name, size );

	return surface;
}

void ImUiSurfaceEnd( ImUiSurface* surface )
{
	ImUiDrawSurfaceEnd( &surface->imui->draw, surface->drawIndex );
}

void ImUiSurfaceGetMaxBufferSizes( ImUiSurface* surface, size_t* outVertexDataSize, size_t* outIndexDataSize )
{
	ImUiDrawGetSurfaceMaxBufferSizes( &surface->imui->draw, surface->drawIndex, outVertexDataSize, outIndexDataSize );
}

const ImUiDrawData* ImUiSurfaceGenerateDrawData( ImUiSurface* surface, void* outVertexData, size_t* inOutVertexDataSize, void* outIndexData, size_t* inOutIndexDataSize )
{
	return ImUiDrawGenerateSurfaceData( &surface->imui->draw, surface->drawIndex, outVertexData, inOutVertexDataSize, outIndexData, inOutIndexDataSize );
}

ImUiContext* ImUiSurfaceGetContext( const ImUiSurface* surface )
{
	return surface->imui;
}

float ImUiSurfaceGetTime( const ImUiSurface* surface )
{
	return surface->imui->frame.timeInSeconds;
}

ImUiSize ImUiSurfaceGetSize( const ImUiSurface* surface )
{
	return surface->size;
}

float ImUiSurfaceGetDpiScale( const ImUiSurface* surface )
{
	return surface->dpiScale;
}

ImUiWindow* ImUiWindowBegin( ImUiSurface* surface, const char* name, ImUiRect rect, uint32_t zOrder )
{
	IMUI_ASSERT( surface );

	ImUiContext* imui = surface->imui;
	const ImUiStringView nameView = ImUiStringViewCreate( name );

	ImUiWindow* window = NULL;
	for( uintsize windowIndex = 0u; windowIndex < surface->windowCount; ++windowIndex )
	{
		if( !ImUiStringViewIsEquals( surface->windows[ windowIndex ].name, nameView ) )
		{
			continue;
		}

		window = &surface->windows[ windowIndex ];
		IMUI_ASSERT( !window->inUse && "Window name must be unique" );
		break;
	}

	if( !window )
	{
		if( !IMUI_MEMORY_ARRAY_CHECK_CAPACITY_ZERO( &imui->allocator, surface->windows, surface->windowCapacity, surface->windowCount + 1u ) )
		{
			return NULL;
		}

		window = &surface->windows[ surface->windowCount ];
		surface->windowCount++;

		memset( window, 0, sizeof( *window ) );
	}

	window->inUse		= true;
	window->imui		= imui;
	window->surface		= surface;
	window->name		= ImUiStringPoolAdd( &imui->strings, nameView );
	window->rect		= rect;
	window->zOrder		= zOrder;
	window->drawIndex	= ImUiDrawRegisterWindow( &imui->draw, window->name, surface->drawIndex, zOrder );

	ImUiWidget* rootWidget = ImUiWidgetAlloc( imui );
	rootWidget->window		= window;
	rootWidget->name		= window->name;
	rootWidget->hash		= ImUiHashString( window->name, 0u );
	rootWidget->minSize		= rect.size;
	rootWidget->maxSize		= rect.size;
	rootWidget->rect		= rect;
	rootWidget->clipRect	= rect;

	window->lastFrameRootWidget		= window->rootWidget;
	window->lastFrameCurrentWidget	= window->rootWidget;

	if( window->lastFrameCurrentWidget )
	{
		rootWidget->lastFrameWidget	= window->lastFrameRootWidget;
		rootWidget->layoutContext	= window->lastFrameRootWidget->layoutContext;
	}

	window->rootWidget		= rootWidget;
	window->currentWidget	= window->rootWidget;
	return window;
}

void ImUiWindowEnd( ImUiWindow* window )
{
	ImUiWidgetEnd( window->rootWidget );
	ImUiWindowLayout( window );
}

ImUiContext* ImUiWindowGetContext( const ImUiWindow* window )
{
	return window->surface->imui;
}

ImUiSurface* ImUiWindowGetSurface( const ImUiWindow* window )
{
	return window->surface;
}

float ImUiWindowGetTime( const ImUiWindow* window )
{
	return window->imui->frame.timeInSeconds;
}

ImUiWidget* ImUiWindowGetFirstChild( const ImUiWindow* window )
{
	return window->rootWidget->firstChild;
}

ImUiWidget* ImUiWindowGetLastChild( const ImUiWindow* window )
{
	return window->rootWidget->lastChild;
}

static void ImUiWindowLayout( ImUiWindow* window )
{
	const bool update = true; // !window->rootWidget->lastFrameWidget || (window->rootWidget->hash != window->rootWidget->lastFrameWidget->hash);

	uintsize childIndex = 0u;
	for( ImUiWidget* widget = window->rootWidget->firstChild; widget != NULL; widget = widget->nextSibling )
	{
		ImUiWidgetUpdateLayoutContext( widget, childIndex, window->surface->dpiScale, update );
		childIndex++;
	}

	childIndex = 0u;
	for( ImUiWidget* widget = window->rootWidget->firstChild; widget != NULL; widget = widget->nextSibling )
	{
		ImUiWidgetLayout( widget, &window->rootWidget->rect, window->surface->dpiScale, childIndex, update );
		childIndex++;
	}
}

static ImUiWidget* ImUiWidgetAlloc( ImUiContext* imui )
{
	if( imui->firstChunk == NULL ||
		imui->firstChunk->usedCount == IMUI_DEFAULT_WIDGET_CHUNK_SIZE )
	{
		if( imui->firstFreeChunk )
		{
			ImUiWidgetChunk* newChunk = imui->firstFreeChunk;
			IMUI_ASSERT( newChunk->usedCount == 0u );

			imui->firstFreeChunk = newChunk->nextChunk;

			newChunk->nextChunk	= imui->firstChunk;
			imui->firstChunk = newChunk;
		}
		else
		{
			ImUiWidgetChunk* newChunk = IMUI_MEMORY_NEW( &imui->allocator, ImUiWidgetChunk );
			if( newChunk == NULL )
			{
				return NULL;
			}

			newChunk->nextChunk	= imui->firstChunk;
			newChunk->usedCount	= 0u;

			imui->firstChunk = newChunk;
		}
	}

	ImUiWidget* widget = &imui->firstChunk->data[ imui->firstChunk->usedCount ];
	imui->firstChunk->usedCount++;

	*widget = IMUI_DEFAULT_WIDGET;

	return widget;
}

static void ImUiWidgetUpdateLayoutContext( ImUiWidget* widget, uintsize widgetIndex, float dpiScale, bool update )
{
	if( !update && widget->lastFrameWidget && widget->hash == widget->lastFrameWidget->hash )
	{
		uintsize childIndex = 0u;
		for( ImUiWidget* childWidget = widget->firstChild; childWidget != NULL; childWidget = childWidget->nextSibling )
		{
			ImUiWidgetUpdateLayoutContext( childWidget, childIndex, dpiScale, false );
			childIndex++;
		}
		return;
	}
	ImUiLayoutContext* context			= &widget->layoutContext;
	ImUiLayoutContext* parentContext	= &widget->parent->layoutContext;

	*context = IMUI_DEFAULT_LAYOUT_CONTEXT;

	const ImUiSize paddingSize	= ImUiSizeScale( ImUiBorderGetMinSize( widget->padding ), dpiScale );
	context->minOuterSize		= ImUiSizeScale( ImUiSizeExpandBorder( ImUiSizeCeil( widget->minSize ), widget->margin ), dpiScale );

	if( widget->layout == ImUiLayout_Grid )
	{
		ImUiWidgetUpdateLayoutContextCheckGridContextSize( widget );
	}

	parentContext->childrenStretch.width		+= widget->stretchH;
	parentContext->childrenStretch.height		+= widget->stretchV;
	parentContext->childrenMaxStretch.width		= IMUI_MAX( parentContext->childrenMaxStretch.width, widget->stretchH );
	parentContext->childrenMaxStretch.height	= IMUI_MAX( parentContext->childrenMaxStretch.height, widget->stretchV );

	{
		uintsize childIndex = 0u;
		for( ImUiWidget* childWidget = widget->firstChild; childWidget != NULL; childWidget = childWidget->nextSibling )
		{
			ImUiWidgetUpdateLayoutContext( childWidget, childIndex, dpiScale, true );
			childIndex++;
		}
	}

	context->childrenStretch = ImUiSizeMax( context->childrenStretch, ImUiSizeCreateOne() );

	if( widget->layout == ImUiLayout_Grid )
	{
		ImUiWidgetUpdateLayoutContextGrid( widget, dpiScale );
	}

	switch( widget->parent->layout )
	{
	case ImUiLayout_Stack:
		parentContext->childrenMinSize.width	= IMUI_MAX( parentContext->childrenMinSize.width, IMUI_MAX( context->minOuterSize.width, context->childrenMinSize.width + paddingSize.width ) );
		parentContext->childrenMinSize.height	= IMUI_MAX( parentContext->childrenMinSize.height, IMUI_MAX( context->minOuterSize.height, context->childrenMinSize.height + paddingSize.height ) );
		break;

	case ImUiLayout_Scroll:
		break;

	case ImUiLayout_Horizontal:
		if( widgetIndex > 0u )
		{
			parentContext->childrenMinSize.width	+= widget->parent->layoutData.horizintalVertical.spacing * dpiScale;
		}
		parentContext->childrenMinSize.width		+= IMUI_MAX( context->minOuterSize.width, context->childrenMinSize.width + paddingSize.width );
		parentContext->childrenMinSize.height		= IMUI_MAX( parentContext->childrenMinSize.height, IMUI_MAX( context->minOuterSize.height, context->childrenMinSize.height + paddingSize.height ) );
		break;

	case ImUiLayout_Vertical:
		if( widgetIndex > 0u )
		{
			parentContext->childrenMinSize.height	+= widget->parent->layoutData.horizintalVertical.spacing * dpiScale;
		}
		parentContext->childrenMinSize.width		= IMUI_MAX( parentContext->childrenMinSize.width, IMUI_MAX( context->minOuterSize.width, context->childrenMinSize.width + paddingSize.width ) );
		parentContext->childrenMinSize.height		+= IMUI_MAX( context->minOuterSize.height, context->childrenMinSize.height + paddingSize.height );
		break;

	case ImUiLayout_Grid:
		{
			const uintsize colIndex				= widgetIndex % widget->parent->layoutData.grid.columnCount;
			const uintsize rowIndex				= widgetIndex / widget->parent->layoutData.grid.columnCount;
			ImUiLayoutGridElement* colElement	= &widget->parent->gridContext->columns[ colIndex ];
			ImUiLayoutGridElement* rowElement	= &widget->parent->gridContext->rows[ rowIndex ];

			colElement->childrenMinSize		= IMUI_MAX( colElement->childrenMinSize, IMUI_MAX( context->minOuterSize.width, context->childrenMinSize.width + paddingSize.width ) );
			rowElement->childrenMinSize		= IMUI_MAX( rowElement->childrenMinSize, IMUI_MAX( context->minOuterSize.height, context->childrenMinSize.height + paddingSize.height ) );

			colElement->childrenMaxStretch	= IMUI_MAX( colElement->childrenMaxStretch, widget->stretchH );
			rowElement->childrenMaxStretch	= IMUI_MAX( rowElement->childrenMaxStretch, widget->stretchV );
		}
		break;
	}
}

static void ImUiWidgetUpdateLayoutContextCheckGridContextSize( ImUiWidget* widget )
{
	ImUiContext* imui = widget->window->imui;

	const uintsize colCount = widget->layoutData.grid.columnCount;
	const uintsize rowCount = (widget->childCount + colCount - 1u) / colCount;
	if( !widget->gridContext ||
		widget->gridContext->columnCount != colCount ||
		widget->gridContext->rowCount != rowCount )
	{
		const uintsize rowCount = (widget->childCount + widget->layoutData.grid.columnCount - 1u) / widget->layoutData.grid.columnCount;
		const uintsize contextSize = sizeof( ImUiLayoutGridContext ) + (sizeof( ImUiLayoutGridElement ) * (widget->layoutData.grid.columnCount + rowCount));
		ImUiLayoutGridContext* gridContext = (ImUiLayoutGridContext*)ImUiMemoryAllocZero( &widget->window->imui->allocator, contextSize );

		gridContext->columns		= (ImUiLayoutGridElement*)&gridContext[ 1u ];
		gridContext->columnCount	= widget->layoutData.grid.columnCount;

		gridContext->rows			= gridContext->columns + gridContext->columnCount;
		gridContext->rowCount		= rowCount;

		gridContext->frameIndex		= imui->frame.index;

		gridContext->nextContext	= imui->firstGridContext;
		gridContext->prevContext	= NULL;

		if( gridContext->nextContext )
		{
			gridContext->nextContext->prevContext = gridContext;
		}
		imui->firstGridContext = gridContext;

		widget->gridContext = gridContext;
	}
	else if( widget->gridContext->frameIndex != imui->frame.index )
	{
		ImUiLayoutGridContext* gridContext = widget->gridContext;

		if( gridContext->prevContext )
		{
			gridContext->prevContext->nextContext = gridContext->nextContext;
		}

		if( gridContext->nextContext )
		{
			gridContext->nextContext->prevContext = gridContext->prevContext;
		}

		if( gridContext == imui->firstUnusedGridContext )
		{
			imui->firstUnusedGridContext = gridContext->nextContext;
		}

		memset( gridContext->columns, 0, sizeof( *gridContext->columns ) * gridContext->columnCount );
		memset( gridContext->rows, 0, sizeof( *gridContext->rows ) * gridContext->rowCount );

		gridContext->frameIndex = imui->frame.index;

		gridContext->nextContext = imui->firstGridContext;
		if( gridContext->nextContext )
		{
			gridContext->nextContext->prevContext = gridContext;
		}
		imui->firstGridContext = gridContext;
	}
}

static void ImUiWidgetUpdateLayoutContextGrid( ImUiWidget* widget, float dpiScale )
{
	ImUiLayoutContext* context = &widget->layoutContext;
	ImUiLayoutGridContext* gridContext = widget->gridContext;

	context->childrenMaxStretch.width	= 0.0f;
	context->childrenMaxStretch.height	= 0.0f;

	for( uintsize col = 0u; col < gridContext->columnCount; ++col )
	{
		ImUiLayoutGridElement* colElement = &gridContext->columns[ col ];

		context->childrenMaxStretch.width += colElement->childrenMaxStretch;
		context->childrenMinSize.width += colElement->childrenMinSize;
	}
	context->childrenMinSize.width += widget->layoutData.grid.colSpacing * (gridContext->columnCount - 1u);

	for( uintsize row = 0u; row < gridContext->rowCount; ++row )
	{
		ImUiLayoutGridElement* rowElement = &gridContext->rows[ row ];

		context->childrenMaxStretch.height += rowElement->childrenMaxStretch;
		context->childrenMinSize.height += rowElement->childrenMinSize;
	}
	context->childrenMinSize.height += widget->layoutData.grid.rowSpacing * (gridContext->rowCount - 1u);

	context->childrenMaxStretch.width	= IMUI_MAX( 1.0f, context->childrenMaxStretch.width );
	context->childrenMaxStretch.height	= IMUI_MAX( 1.0f, context->childrenMaxStretch.height );
}

static void ImUiWidgetLayout( ImUiWidget* widget, const ImUiRect* parentInnerRect, float dpiScale, uintsize widgetIndex, bool update )
{
	if( !update && widget->lastFrameWidget && widget->hash == widget->lastFrameWidget->hash )
	{
		const ImUiRect innerRect = ImUiRectShrinkBorder( widget->rect, ImUiBorderScale( widget->padding, dpiScale ) );
		for( ImUiWidget* childWidget = widget->firstChild; childWidget != NULL; childWidget = childWidget->nextSibling )
		{
			ImUiWidgetLayout( childWidget, &innerRect, dpiScale, widgetIndex, update );
		}
		return;
	}

	switch( widget->parent->layout )
	{
	case ImUiLayout_Stack:
		ImUiWidgetLayoutStack( widget, parentInnerRect, dpiScale );
		break;

	case ImUiLayout_Scroll:
		ImUiWidgetLayoutScroll( widget, parentInnerRect, dpiScale );
		break;

	case ImUiLayout_Horizontal:
		ImUiWidgetLayoutHorizontal( widget, parentInnerRect, dpiScale );
		break;

	case ImUiLayout_Vertical:
		ImUiWidgetLayoutVertical( widget, parentInnerRect, dpiScale );
		break;

	case ImUiLayout_Grid:
		ImUiWidgetLayoutGrid( widget, parentInnerRect, dpiScale, widgetIndex );
		break;
	}

	IMUI_ASSERT( widget->rect.size.width >= 0.0f );
	IMUI_ASSERT( widget->rect.size.height >= 0.0f );

	widget->clipRect = ImUiRectIntersection( widget->rect, widget->parent->clipRect );

	const ImUiRect innerRect = ImUiRectShrinkBorder( widget->rect, ImUiBorderScale( widget->padding, dpiScale ) );
	if( widget->layout == ImUiLayout_Grid )
	{
		ImUiWidgetLayoutPrepareGrid( widget, &innerRect, dpiScale );
	}

	uintsize childIndex = 0u;
	for( ImUiWidget* childWidget = widget->firstChild; childWidget != NULL; childWidget = childWidget->nextSibling )
	{
		ImUiWidgetLayout( childWidget, &innerRect, dpiScale, childIndex, true );
		childIndex++;
	}
}

static void ImUiWidgetLayoutPrepareGrid( ImUiWidget* widget, const ImUiRect* parentInnerRect, float dpiScale )
{
	ImUiLayoutContext* context = &widget->layoutContext;
	ImUiLayoutGridContext* gridContext = widget->gridContext;

	const float maxFreeWidth		= parentInnerRect->size.width - context->childrenMinSize.width;
	const float maxFreeHeight		= parentInnerRect->size.height - context->childrenMinSize.height;

	float pos = parentInnerRect->pos.x;
	for( uintsize col = 0u; col < gridContext->columnCount; ++col )
	{
		ImUiLayoutGridElement* colElement = &gridContext->columns[ col ];

		colElement->pos = pos;

		const float strechSize	= ((maxFreeWidth / context->childrenMaxStretch.width) * colElement->childrenMaxStretch) + colElement->childrenMinSize;
		const float size		= IMUI_MAX( strechSize, colElement->childrenMinSize );
		pos += size;
		colElement->size = size;
		pos += widget->layoutData.grid.colSpacing * dpiScale;
	}

	pos = parentInnerRect->pos.y;
	for( uintsize row = 0u; row < gridContext->rowCount; ++row )
	{
		ImUiLayoutGridElement* rowElement = &gridContext->rows[ row ];

		rowElement->pos = pos;

		const float strechSize	= ((maxFreeHeight / context->childrenMaxStretch.height) * rowElement->childrenMaxStretch) + rowElement->childrenMinSize;
		const float size		= IMUI_MAX( strechSize, rowElement->childrenMinSize );
		pos += size;
		rowElement->size = size;
		pos += widget->layoutData.grid.rowSpacing * dpiScale;
	}
}

static void ImUiWidgetLayoutStack( ImUiWidget* widget, const ImUiRect* parentInnerRect, float dpiScale )
{
	ImUiLayoutContext* context = &widget->layoutContext;

	const float factorWidth			= IMUI_MIN( widget->stretchH, widget->parent->layoutContext.childrenMaxStretch.width );
	const float factorHeight		= IMUI_MIN( widget->stretchV, widget->parent->layoutContext.childrenMaxStretch.height );
	const ImUiSize minSize			= ImUiWidgetLayoutMinSize( widget, parentInnerRect, dpiScale );
	const ImUiSize maxSize			= parentInnerRect->size;
	ImUiSize size					= ImUiWidgetCalculateSize( widget, minSize, maxSize, factorWidth, factorHeight, dpiScale );

	ImUiPos pos;
	pos.x = ImUiWidgetLayoutPositionX( widget, parentInnerRect, size.width, dpiScale );
	pos.y = ImUiWidgetLayoutPositionY( widget, parentInnerRect, size.height, dpiScale );

	// ???
	widget->rect.pos.x			= floorf( pos.x );
	widget->rect.pos.y			= floorf( pos.y );
	widget->rect.size.width		= ceilf( size.width );
	widget->rect.size.height	= ceilf( size.height );
}

static void ImUiWidgetLayoutScroll( ImUiWidget* widget, const ImUiRect* parentInnerRect, float dpiScale )
{
	ImUiLayoutContext* context = &widget->layoutContext;

	const float factorWidth			= IMUI_MIN( widget->stretchH, widget->parent->layoutContext.childrenMaxStretch.width );
	const float factorHeight		= IMUI_MIN( widget->stretchV, widget->parent->layoutContext.childrenMaxStretch.height );
	const ImUiSize minSize			= ImUiWidgetLayoutMinSize( widget, parentInnerRect, dpiScale );
	const ImUiSize maxSize			= ImUiSizeMax( parentInnerRect->size, minSize );
	ImUiSize size					= ImUiWidgetCalculateSize( widget, minSize, maxSize, factorWidth, factorHeight, dpiScale );

	ImUiPos pos;
	pos.x = ImUiWidgetLayoutPositionX( widget, parentInnerRect, size.width, dpiScale );
	pos.y = ImUiWidgetLayoutPositionY( widget, parentInnerRect, size.height, dpiScale );

	// ???
	widget->rect.pos.x			= floorf( pos.x - widget->parent->layoutData.scroll.offset.x );
	widget->rect.pos.y			= floorf( pos.y - widget->parent->layoutData.scroll.offset.y );
	widget->rect.size.width		= ceilf( size.width );
	widget->rect.size.height	= ceilf( size.height );
}

static void ImUiWidgetLayoutHorizontal( ImUiWidget* widget, const ImUiRect* parentInnerRect, float dpiScale )
{
	ImUiLayoutContext* context = &widget->layoutContext;
	ImUiLayoutContext* parentContext = &widget->parent->layoutContext;

	const float factorWidth			= parentContext->childrenStretch.width ? widget->stretchH / parentContext->childrenStretch.width : 0.0f;
	const float factorHeight		= parentContext->childrenMaxStretch.height ? widget->stretchV / parentContext->childrenMaxStretch.height : 0.0f;
	const ImUiSize minSize			= ImUiWidgetLayoutMinSize( widget, parentInnerRect, dpiScale );

	const float maxFreeWidth		= (parentInnerRect->size.width - parentContext->childrenMinSize.width) + minSize.width;
	const ImUiSize maxSize			= ImUiSizeMin( ImUiSizeScale( ImUiSizeExpandBorder( widget->maxSize, widget->margin ), dpiScale ), ImUiSizeCreate( maxFreeWidth, parentInnerRect->size.height ) );

	ImUiSize size					= ImUiWidgetCalculateSize( widget, minSize, maxSize, factorWidth, factorHeight, dpiScale );

	ImUiPos pos;
	if( widget->prevSibling )
	{
		pos.x = widget->prevSibling->rect.pos.x + widget->prevSibling->rect.size.width + ((widget->prevSibling->margin.right + widget->parent->layoutData.horizintalVertical.spacing + widget->margin.left) * dpiScale);
	}
	else
	{
		pos.x = parentInnerRect->pos.x + (widget->margin.left * dpiScale);
	}
	pos.y = ImUiWidgetLayoutPositionY( widget, parentInnerRect, size.height, dpiScale );

	// ???
	widget->rect.pos.x			= floorf( pos.x );
	widget->rect.pos.y			= floorf( pos.y );
	widget->rect.size.width		= ceilf( size.width );
	widget->rect.size.height	= ceilf( size.height );
}

static void ImUiWidgetLayoutVertical( ImUiWidget* widget, const ImUiRect* parentInnerRect, float dpiScale )
{
	ImUiLayoutContext* context = &widget->layoutContext;
	ImUiLayoutContext* parentContext = &widget->parent->layoutContext;

	const float factorWidth			= parentContext->childrenMaxStretch.width ? widget->stretchH / parentContext->childrenMaxStretch.width : 0.0f;
	const float factorHeight		= parentContext->childrenStretch.height ? widget->stretchV / parentContext->childrenStretch.height : 0.0f;
	const ImUiSize minSize			= ImUiWidgetLayoutMinSize( widget, parentInnerRect, dpiScale );

	const float maxFreeHeight		= (parentInnerRect->size.height - parentContext->childrenMinSize.height) + minSize.height;
	const ImUiSize maxSize			= ImUiSizeMin( ImUiSizeScale( ImUiSizeExpandBorder( widget->maxSize, widget->margin ), dpiScale ), ImUiSizeCreate( parentInnerRect->size.width, maxFreeHeight ) );

	ImUiSize size					= ImUiWidgetCalculateSize( widget, minSize, maxSize, factorWidth, factorHeight, dpiScale );

	ImUiPos pos;
	pos.x = ImUiWidgetLayoutPositionX( widget, parentInnerRect, size.width, dpiScale );
	if( widget->prevSibling )
	{
		pos.y = widget->prevSibling->rect.pos.y + widget->prevSibling->rect.size.height + ((widget->prevSibling->margin.bottom + widget->parent->layoutData.horizintalVertical.spacing + widget->margin.top) * dpiScale);
	}
	else
	{
		pos.y = parentInnerRect->pos.y + (widget->margin.top * dpiScale);
	}

	// ???
	widget->rect.pos.x			= floorf( pos.x );
	widget->rect.pos.y			= floorf( pos.y );
	widget->rect.size.width		= ceilf( size.width );
	widget->rect.size.height	= ceilf( size.height );
}

static void ImUiWidgetLayoutGrid( ImUiWidget* widget, const ImUiRect* parentInnerRect, float dpiScale, uintsize widgetIndex )
{
	ImUiLayoutContext* context = &widget->layoutContext;

	const uintsize colIndex				= widgetIndex % widget->parent->layoutData.grid.columnCount;
	const uintsize rowIndex				= widgetIndex / widget->parent->layoutData.grid.columnCount;
	ImUiLayoutGridElement* colElement	= &widget->parent->gridContext->columns[ colIndex ];
	ImUiLayoutGridElement* rowElement	= &widget->parent->gridContext->rows[ rowIndex ];

	const float factorWidth			= colElement->childrenMaxStretch ? widget->stretchH / colElement->childrenMaxStretch : 0.0f;
	const float factorHeight		= rowElement->childrenMaxStretch ? widget->stretchV / rowElement->childrenMaxStretch : 0.0f;
	const ImUiSize minSize			= ImUiWidgetLayoutMinSize( widget, parentInnerRect, dpiScale );
	const ImUiSize maxSize			= ImUiSizeMin( ImUiSizeScale( ImUiSizeExpandBorder( widget->maxSize, widget->margin ), dpiScale ), ImUiSizeCreate( colElement->size, rowElement->size ) );
	ImUiSize size					= ImUiWidgetCalculateSize( widget, minSize, maxSize, factorWidth, factorHeight, dpiScale );

	const ImUiRect cellInnerRect =
	{
		{ colElement->pos, rowElement->pos },
		{ colElement->size, rowElement->size }
	};

	ImUiPos pos;
	pos.x = ImUiWidgetLayoutPositionX( widget, &cellInnerRect, size.width, dpiScale );
	pos.y = ImUiWidgetLayoutPositionY( widget, &cellInnerRect, size.height, dpiScale );

	// ???
	widget->rect.pos.x			= floorf( pos.x );
	widget->rect.pos.y			= floorf( pos.y );
	widget->rect.size.width		= ceilf( size.width );
	widget->rect.size.height	= ceilf( size.height );
}

static ImUiSize ImUiWidgetLayoutMinSize( ImUiWidget* widget, const ImUiRect* parentInnerRect, float dpiScale )
{
	const ImUiBorder padding	= ImUiBorderScale( widget->padding, dpiScale );
	const ImUiSize minChildren	= ImUiSizeExpandBorder( widget->layoutContext.childrenMinSize, padding );
	const ImUiSize minSize		= ImUiSizeMax( widget->layoutContext.minOuterSize, minChildren );
	return minSize; // ImUiSizeMin( parentInnerRect->size, minSize );
}

static float ImUiWidgetLayoutPositionX( ImUiWidget* widget, const ImUiRect* parentInnerRect, float width, float dpiScale )
{
	const float remainingWidth = parentInnerRect->size.width - (width + ((widget->margin.left + widget->margin.right) * dpiScale));
	return parentInnerRect->pos.x + (widget->margin.left * dpiScale) + (remainingWidth * widget->alignH);
}

static float ImUiWidgetLayoutPositionY( ImUiWidget* widget, const ImUiRect* parentInnerRect, float height, float dpiScale )
{
	const float remainingHeight = parentInnerRect->size.height - (height + ((widget->margin.top + widget->margin.bottom) * dpiScale));
	return parentInnerRect->pos.y + (widget->margin.top * dpiScale) + (remainingHeight * widget->alignV);
}

static ImUiSize ImUiWidgetCalculateSize( ImUiWidget* widget, ImUiSize minSize, ImUiSize maxSize, float factorWidth, float factorHeight, float dpiScale )
{
	ImUiSize size = ImUiSizeLerp2( minSize, maxSize, factorWidth, factorHeight );
	size = ImUiSizeMax( size, minSize );
	size = ImUiSizeShrinkBorder( size, ImUiBorderScale( widget->margin, dpiScale ) );

	return size;
}

ImUiWidget* ImUiWidgetBegin( ImUiWindow* window )
{
	IMUI_ASSERT( window );
	return ImUiWidgetBeginId( window, 0u );
}

ImUiWidget* ImUiWidgetBeginId( ImUiWindow* window, ImUiId id )
{
	IMUI_ASSERT( window );

	ImUiWidget* widget = ImUiWidgetAlloc( window->imui );
	if( widget == NULL )
	{
		return NULL;
	}

	if( window->currentWidget->lastChild )
	{
		id += window->currentWidget->lastChild->id + 1u;
	}

	ImUiWidget* parent = window->currentWidget;
	widget->window	= window;
	widget->parent	= parent;
	widget->id		= id;
	widget->hash	= 0u;

	if( parent->firstChild == NULL )
	{
		parent->firstChild = widget;
		parent->lastChild = widget;
	}
	else
	{
		widget->prevSibling = parent->lastChild;
		parent->lastChild->nextSibling = widget;

		parent->lastChild = widget;
	}
	parent->childCount++;

	window->currentWidget = widget;

	if( window->lastFrameCurrentWidget )
	{
		ImUiWidget* lastFrameWidget = window->lastFrameCurrentWidget->firstChild;
		for( ; lastFrameWidget != NULL; lastFrameWidget = lastFrameWidget->nextSibling )
		{
			if( lastFrameWidget->id != widget->id )
			{
				continue;
			}

			break;
		}

		if( lastFrameWidget )
		{
			widget->rect			= lastFrameWidget->rect;
			widget->clipRect		= lastFrameWidget->clipRect;
			widget->lastFrameWidget	= lastFrameWidget;
			widget->layoutContext	= lastFrameWidget->layoutContext;
			widget->gridContext		= lastFrameWidget->gridContext;
			widget->inputContext	= lastFrameWidget->inputContext;

			window->lastFrameCurrentWidget = lastFrameWidget;
		}
	}

	return widget;
}

ImUiWidget* ImUiWidgetBeginNamed( ImUiWindow* window, const char* name )
{
	const ImUiStringView nameView = ImUiStringViewCreate( name );

	ImUiWidget* widget = ImUiWidgetBeginId( window, ImUiHashString( nameView, 0u ) );
	if( widget == NULL )
	{
		return NULL;
	}

	widget->name = ImUiStringPoolAdd( &window->imui->strings, nameView );

	return widget;
}

void ImUiWidgetEnd( ImUiWidget* widget )
{
	IMUI_ASSERT( widget == widget->window->currentWidget );

	widget->hash = ImUiHashMix( widget->hash, ImUiHashCreate( &widget->id, IMUI_OFFSETOF( ImUiWidget, rect ) - IMUI_OFFSETOF( ImUiWidget, id ), 0u ) );

	//IMUI_ASSERT( !widget->lastFrameWidget || widget->hash == widget->lastFrameWidget->hash );

	if( widget->parent )
	{
		widget->parent->hash = ImUiHashMix( widget->parent->hash, widget->hash );
	}

	widget->window->currentWidget = widget->parent;
	if( widget->lastFrameWidget &&
		widget->window->lastFrameCurrentWidget )
	{
		widget->window->lastFrameCurrentWidget = widget->window->lastFrameCurrentWidget->parent;
	}
}

ImUiContext* ImUiWidgetGetContext( const ImUiWidget* widget )
{
	return widget->window->imui;
}

ImUiSurface* ImUiWidgetGetSurface( const ImUiWidget* widget )
{
	return widget->window->surface;
}

ImUiWindow* ImUiWidgetGetWindow( const ImUiWidget* widget )
{
	return widget->window;
}

ImUiWidget* ImUiWidgetGetParent( const ImUiWidget* widget )
{
	IMUI_ASSERT( widget->parent != widget->window->rootWidget );
	return widget->parent;
}

ImUiWidget* ImUiWidgetGetFirstChild( const ImUiWidget* widget )
{
	return widget->firstChild;
}

ImUiWidget* ImUiWidgetGetLastChild( const ImUiWidget* widget )
{
	return widget->lastChild;
}

ImUiWidget* ImUiWidgetGetPrevSibling( const ImUiWidget* widget )
{
	return widget->prevSibling;
}

ImUiWidget* ImUiWidgetGetNextSibling( const ImUiWidget* widget )
{
	return widget->nextSibling;
}

float ImUiWidgetGetTime( const ImUiWidget* widget )
{
	return widget->window->imui->frame.timeInSeconds;
}

void* ImUiWidgetAllocState( ImUiWidget* widget, size_t size, ImUiId stateId )
{
	return ImUiWidgetAllocStateNewDestruct( widget, size, stateId, NULL, NULL );
}

void* ImUiWidgetAllocStateNew( ImUiWidget* widget, size_t size, ImUiId stateId, bool* isNew )
{
	return ImUiWidgetAllocStateNewDestruct( widget, size, stateId, isNew, NULL );
}

void* ImUiWidgetAllocStateNewDestruct( ImUiWidget* widget, size_t size, ImUiId stateId, bool* isNew, ImUiStateDestructFunc destructFunc )
{
	for( ImUiWidgetState* state = widget->firstState; state; state = state->nextWidgetState )
	{
		if( state->id != stateId )
		{
			continue;
		}

		IMUI_ASSERT( state->size == size );
		IMUI_ASSERT( state->destructFunc == destructFunc );

		if( isNew )
		{
			*isNew = false;
		}

		return state->data;
	}

	ImUiContext* imui = widget->window->imui;
	if( widget->lastFrameWidget )
	{
		for( ImUiWidgetState* lastFrameState = widget->lastFrameWidget->firstState; lastFrameState; lastFrameState = lastFrameState->nextWidgetState )
		{
			if( lastFrameState->id != stateId )
			{
				continue;
			}

			IMUI_ASSERT( lastFrameState->size == size );
			IMUI_ASSERT( lastFrameState->destructFunc == destructFunc );

			// usage list
			{
				if( lastFrameState->prevUsageState )
				{
					lastFrameState->prevUsageState->nextUsageState = lastFrameState->nextUsageState;
				}
				else
				{
					IMUI_ASSERT( imui->firstUnusedState == lastFrameState );
					imui->firstUnusedState = lastFrameState->nextUsageState;
				}

				if( lastFrameState->nextUsageState )
				{
					lastFrameState->nextUsageState->prevUsageState = lastFrameState->prevUsageState;
				}

				lastFrameState->nextUsageState = widget->window->imui->firstState;
				lastFrameState->prevUsageState = NULL;

				if( imui->firstState )
				{
					imui->firstState->prevUsageState = lastFrameState;
				}
				imui->firstState = lastFrameState;
			}

			// widget list
			{
				if( lastFrameState->prevWidgetState )
				{
					lastFrameState->prevWidgetState->nextWidgetState = lastFrameState->nextWidgetState;
				}
				else
				{
					IMUI_ASSERT( widget->lastFrameWidget->firstState == lastFrameState );
					widget->lastFrameWidget->firstState = lastFrameState->nextWidgetState;
				}

				if( lastFrameState->nextWidgetState )
				{
					lastFrameState->nextWidgetState->prevWidgetState = lastFrameState->prevWidgetState;
				}

				lastFrameState->nextUsageState = widget->firstState;
				lastFrameState->prevUsageState = NULL;

				if( widget->firstState )
				{
					widget->firstState->prevUsageState = lastFrameState;
				}
				widget->firstState = lastFrameState;
			}

			if( isNew )
			{
				*isNew = false;
			}

			return lastFrameState->data;
		}
	}

	ImUiWidgetState* newState = (ImUiWidgetState*)ImUiMemoryAllocZero( &widget->window->imui->allocator, IMUI_OFFSETOF( ImUiWidgetState, data ) + size );
	if( !newState )
	{
		return NULL;
	}

	newState->id				= stateId;
	newState->size				= size;
	newState->destructFunc		= destructFunc;

	// usage list
	{
		newState->nextUsageState = imui->firstState;

		if( imui->firstState )
		{
			imui->firstState->prevUsageState = newState;
		}

		imui->firstState = newState;
	}

	// widget list
	{
		newState->nextWidgetState = widget->firstState;

		if( widget->firstState )
		{
			widget->firstState->prevWidgetState = newState;
		}

		widget->firstState = newState;
	}

	if( isNew )
	{
		*isNew = true;
	}

	return newState->data;
}

ImUiLayout ImUiWidgetGetLayout( const ImUiWidget* widget )
{
	return widget->layout;
}

void ImUiWidgetSetLayoutStack( ImUiWidget* widget )
{
	widget->layout = ImUiLayout_Stack;
}

void ImUiWidgetSetLayoutScroll( ImUiWidget* widget, float offsetX, float offsetY )
{
	widget->layout						= ImUiLayout_Scroll;
	widget->layoutData.scroll.offset	= ImUiPosCreate( offsetX, offsetY );
}

void ImUiWidgetSetLayoutHorizontal( ImUiWidget* widget )
{
	widget->layout									= ImUiLayout_Horizontal;
	widget->layoutData.horizintalVertical.spacing	= 0.0f;
}

void ImUiWidgetSetLayoutHorizontalSpacing( ImUiWidget* widget, float spacing )
{
	IMUI_ASSERT( spacing >= 0.0f );

	widget->layout									= ImUiLayout_Horizontal;
	widget->layoutData.horizintalVertical.spacing	= spacing;
}

void ImUiWidgetSetLayoutVertical( ImUiWidget* widget )
{
	widget->layout									= ImUiLayout_Vertical;
	widget->layoutData.horizintalVertical.spacing	= 0.0f;
}

void ImUiWidgetSetLayoutVerticalSpacing( ImUiWidget* widget, float spacing )
{
	IMUI_ASSERT( spacing >= 0.0f );

	widget->layout									= ImUiLayout_Vertical;
	widget->layoutData.horizintalVertical.spacing	= spacing;
}

void ImUiWidgetSetLayoutGrid( ImUiWidget* widget, uint32_t columnCount, float colSpacing, float rowSpacing )
{
	IMUI_ASSERT( columnCount > 0u );

	widget->layout									= ImUiLayout_Grid;
	widget->layoutData.grid.columnCount				= columnCount;
	widget->layoutData.grid.colSpacing				= colSpacing;
	widget->layoutData.grid.rowSpacing				= rowSpacing;
}

ImUiBorder ImUiWidgetGetMargin( const ImUiWidget* widget )
{
	return widget->margin;
}

void ImUiWidgetSetMargin( ImUiWidget* widget, ImUiBorder margin )
{
	IMUI_ASSERT( margin.top >= 0.0f && margin.left >= 0.0f && margin.bottom >= 0.0f && margin.right >= 0.0f );
	widget->margin = margin;
}

ImUiBorder ImUiWidgetGetPadding( const ImUiWidget* widget )
{
	return widget->padding;
}

void ImUiWidgetSetPadding( ImUiWidget* widget, ImUiBorder padding )
{
	IMUI_ASSERT( padding.top >= 0.0f && padding.left >= 0.0f && padding.bottom >= 0.0f && padding.right >= 0.0f );
	widget->padding = padding;
}

ImUiSize ImUiWidgetGetMinSize( const ImUiWidget* widget )
{
	return widget->minSize;
}

void ImUiWidgetSetMinWidth( ImUiWidget* widget, float value )
{
	IMUI_ASSERT( value >= 0.0f );
	widget->minSize.width = value;
}

void ImUiWidgetSetMinHeight( ImUiWidget* widget, float value )
{
	IMUI_ASSERT( value >= 0.0f );
	widget->minSize.height = value;
}

void ImUiWidgetSetMinSize( ImUiWidget* widget, float width, float height )
{
	IMUI_ASSERT( width >= 0.0f && height >= 0.0f );
	widget->minSize = ImUiSizeCreate( width, height );
}

ImUiSize ImUiWidgetGetMaxSize( const ImUiWidget* widget )
{
	return widget->maxSize;
}

void ImUiWidgetSetMaxWidth( ImUiWidget* widget, float value )
{
	IMUI_ASSERT( value >= 0.0f );
	widget->maxSize.width = value;
}

void ImUiWidgetSetMaxHeight( ImUiWidget* widget, float value )
{
	IMUI_ASSERT( value >= 0.0f );
	widget->maxSize.height = value;
}

void ImUiWidgetSetMaxSize( ImUiWidget* widget, float width, float height )
{
	IMUI_ASSERT( width >= 0.0f && height >= 0.0f );
	widget->maxSize = ImUiSizeCreate( width, height );
}

void ImUiWidgetSetFixedWidth( ImUiWidget* widget, float value )
{
	IMUI_ASSERT( value >= 0.0f );
	widget->minSize.width = value;
	widget->maxSize.width = value;
}

void ImUiWidgetSetFixedHeight( ImUiWidget* widget, float value )
{
	IMUI_ASSERT( value >= 0.0f );
	widget->minSize.height = value;
	widget->maxSize.height = value;
}

void ImUiWidgetSetFixedSize( ImUiWidget* widget, ImUiSize size )
{
	IMUI_ASSERT( size.width >= 0.0f && size.height >= 0.0f );
	widget->minSize = size;
	widget->maxSize = size;
}

void ImUiWidgetSetFixedSizeFloat( ImUiWidget* widget, float width, float height )
{
	IMUI_ASSERT( width >= 0.0f && height >= 0.0f );
	widget->minSize = ImUiSizeCreate( width, height );
	widget->maxSize = ImUiSizeCreate( width, height );
}

void ImUiWidgetSetStretch( ImUiWidget* widget, float horizontal, float vertical )
{
	IMUI_ASSERT( horizontal >= 0.0f && vertical >= 0.0f );
	widget->stretchH = horizontal;
	widget->stretchV = vertical;
}

void ImUiWidgetSetStretchOne( ImUiWidget* widget )
{
	widget->stretchH = 1.0f;
	widget->stretchV = 1.0f;
}

float ImUiWidgetGetHStretch( const ImUiWidget* widget )
{
	return widget->stretchH;
}

void ImUiWidgetSetHStretch( ImUiWidget* widget, float stretch )
{
	IMUI_ASSERT( stretch >= 0.0f );
	widget->stretchH = stretch;
}

float ImUiWidgetGetVStretch( const ImUiWidget* widget )
{
	return widget->stretchV;
}

void ImUiWidgetSetVStretch( ImUiWidget* widget, float stretch )
{
	IMUI_ASSERT( stretch >= 0.0f );
	widget->stretchV = stretch;
}

void ImUiWidgetSetAlign( ImUiWidget* widget, float horizontal, float vertical )
{
	widget->alignH = horizontal;
	widget->alignV = vertical;
}

float ImUiWidgetGetHAlign( const ImUiWidget* widget )
{
	return widget->alignH;
}

void ImUiWidgetSetHAlign( ImUiWidget* widget, float align )
{
	widget->alignH = align;
}

float ImUiWidgetGetVAlign( const ImUiWidget* widget )
{
	return widget->alignV;
}

void ImUiWidgetSetVAlign( ImUiWidget* widget, float align )
{
	widget->alignV = align;
}

ImUiPos ImUiWidgetGetPos( const ImUiWidget* widget )
{
	return widget->rect.pos;
}

float ImUiWidgetGetPosX( const ImUiWidget* widget )
{
	return widget->rect.pos.x;
}

float ImUiWidgetGetPosY( const ImUiWidget* widget )
{
	return widget->rect.pos.y;
}

ImUiSize ImUiWidgetGetSize( const ImUiWidget* widget )
{
	return widget->rect.size;
}

float ImUiWidgetGetSizeWidth( const ImUiWidget* widget )
{
	return widget->rect.size.width;
}

float ImUiWidgetGetSizeHeight( const ImUiWidget* widget )
{
	return widget->rect.size.height;
}

ImUiRect ImUiWidgetGetRect( const ImUiWidget* widget )
{
	return widget->rect;
}

ImUiSize ImUiWidgetGetInnerSize( const ImUiWidget* widget )
{
	return ImUiSizeShrinkBorder( widget->rect.size, widget->padding );
}

ImUiRect ImUiWidgetGetInnerRect( const ImUiWidget* widget )
{
	return ImUiRectShrinkBorder( widget->rect, widget->padding );
}

void ImUiWidgetGetInputState( ImUiWidget* widget, ImUiWidgetInputState* target )
{
	ImUiWindow* window = widget->window;
	ImUiSurface* surface = window->surface;
	ImUiContext* imui = window->imui;
	ImUiInput* input = &imui->input;

	bool hasOverlappingWindow = false;
	if( surface->windowCount > 1u )
	{
		for( uintsize i = 0; i < surface->windowCount && !hasOverlappingWindow; ++i )
		{
			const ImUiWindow* testWindow = &surface->windows[ i ];
			if( testWindow == window ||
				testWindow->zOrder < window->zOrder )
			{
				continue;
			}

			hasOverlappingWindow |= ImUiRectIncludesPos( window->rect, input->currentState.mousePos );
		}
	}

	target->relativeMousePos	= ImUiPosSubPos( input->currentState.mousePos, widget->rect.pos );

	target->isMouseOver			= !hasOverlappingWindow && ImUiRectIncludesPos( widget->clipRect, input->currentState.mousePos );
	target->isMouseDown			= target->isMouseOver && input->currentState.mouseButtons[ ImUiInputMouseButton_Left ];
	target->hasMousePressed		= target->isMouseOver && ImUiInputHasMouseButtonPressed( imui, ImUiInputMouseButton_Left );
	target->hasMouseReleased	= target->isMouseOver && ImUiInputHasMouseButtonReleased( imui, ImUiInputMouseButton_Left );

	if( (input->currentState.mouseButtons[ ImUiInputMouseButton_Left ] || input->lastState.mouseButtons[ ImUiInputMouseButton_Left ]) &&
		widget->inputContext.lastFrameIndex >= imui->frame.index - 1u )
	{
		widget->inputContext.wasPressed	|= target->isMouseOver && ImUiInputHasMouseButtonPressed( imui, ImUiInputMouseButton_Left );
		widget->inputContext.wasMouseOver	|= target->isMouseOver;
	}
	else
	{
		widget->inputContext.wasPressed		= false;
		widget->inputContext.wasMouseOver	= false;
	}

	target->wasPressed		= widget->inputContext.wasPressed;
	target->wasMouseOver	= widget->inputContext.wasMouseOver;

	widget->inputContext.lastFrameIndex = imui->frame.index;
}

void ImUiWidgetDrawColor( ImUiWidget* widget, ImUiColor color )
{
	ImUiDrawElement* element = ImUiDrawPushElement( widget, ImUiDrawElementType_Rect, NULL );
	ImUiDrawElementDataRect* rectData = &element->data.rect;
	rectData->color = color;
	memset( &rectData->uv, 0, sizeof( rectData->uv ) );
}

void ImUiWidgetDrawImage( ImUiWidget* widget, const ImUiImage* image )
{
	IMUI_ASSERT( image );

	ImUiDrawElement* element = ImUiDrawPushElement( widget, ImUiDrawElementType_Rect, image->textureData );
	ImUiDrawElementDataRect* rectData = &element->data.rect;
	rectData->color		= ImUiColorCreateWhite();
	rectData->uv		= image->uv;
}

void ImUiWidgetDrawImageColor( ImUiWidget* widget, const ImUiImage* image, ImUiColor color )
{
	IMUI_ASSERT( image );

	ImUiDrawElement* element = ImUiDrawPushElement( widget, ImUiDrawElementType_Rect, image->textureData );
	ImUiDrawElementDataRect* rectData = &element->data.rect;
	rectData->color		= color;
	rectData->uv		= image->uv;
}

void ImUiWidgetDrawSkin( ImUiWidget* widget, const ImUiSkin* skin, ImUiColor color )
{
	IMUI_ASSERT( skin );

	ImUiDrawElement* element = ImUiDrawPushElement( widget, ImUiDrawElementType_Skin, skin->textureData );
	struct ImUiDrawElementDataSkin* skinData = &element->data.skin;
	skinData->border	= skin->border;
	skinData->uv		= skin->uv;
	skinData->texSize	= ImUiSizeCreateSkin( skin );
	skinData->color		= color;
}

void ImUiWidgetDrawText( ImUiWidget* widget, ImUiTextLayout* layout, ImUiColor color )
{
	ImUiDrawElement* element = ImUiDrawPushElementText( widget, ImUiDrawElementType_Text, layout );
	struct ImUiDrawElementDataText* textData = &element->data.text;
	textData->color		= color;
	textData->layout	= layout;
}

void ImUiWidgetDrawPartialColor( ImUiWidget* widget, ImUiRect relativRect, ImUiColor color )
{
	ImUiDrawElement* element = ImUiDrawPushElement( widget, ImUiDrawElementType_RectPartial, NULL );
	ImUiDrawElementDataRect* rectData = &element->data.rect;
	rectData->relativRect	= relativRect;
	rectData->color			= color;
	memset( &rectData->uv, 0, sizeof( rectData->uv ) );
}

void ImUiWidgetDrawPartialImage( ImUiWidget* widget, ImUiRect relativRect, const ImUiImage* image )
{
	IMUI_ASSERT( image );

	ImUiDrawElement* element = ImUiDrawPushElement( widget, ImUiDrawElementType_RectPartial, image->textureData );
	ImUiDrawElementDataRect* rectData = &element->data.rect;
	rectData->relativRect	= relativRect;
	rectData->color			= ImUiColorCreateWhite();
	rectData->uv			= image->uv;
}

void ImUiWidgetDrawPartialImageColor( ImUiWidget* widget, ImUiRect relativRect, const ImUiImage* image, ImUiColor color )
{
	IMUI_ASSERT( image );

	ImUiDrawElement* element = ImUiDrawPushElement( widget, ImUiDrawElementType_RectPartial, image->textureData );
	ImUiDrawElementDataRect* rectData = &element->data.rect;
	rectData->relativRect	= relativRect;
	rectData->color			= color;
	rectData->uv			= image->uv;
}

void ImUiWidgetDrawPartialSkin( ImUiWidget* widget, ImUiRect relativRect, const ImUiSkin* skin, ImUiColor color )
{
	IMUI_ASSERT( skin );

	ImUiDrawElement* element = ImUiDrawPushElement( widget, ImUiDrawElementType_SkinPartial, skin->textureData );
	struct ImUiDrawElementDataSkin* skinData = &element->data.skin;
	skinData->relativRect	= relativRect;
	skinData->border		= skin->border;
	skinData->uv			= skin->uv;
	skinData->texSize		= ImUiSizeCreateSkin( skin );
	skinData->color			= color;
}

void ImUiWidgetDrawPositionText( ImUiWidget* widget, ImUiPos offset, ImUiTextLayout* layout, ImUiColor color )
{
	ImUiDrawElement* element = ImUiDrawPushElementText( widget, ImUiDrawElementType_TextOffset, layout );
	struct ImUiDrawElementDataText* textData = &element->data.text;
	textData->relativPos	= offset;
	textData->color			= color;
	textData->layout		= layout;
}

void ImUiWidgetDrawLine( ImUiWidget* widget, ImUiPos p0, ImUiPos p1, ImUiColor color )
{
	ImUiDrawElement* element = ImUiDrawPushElement( widget, ImUiDrawElementType_Line, NULL );
	ImUiDrawElementDataPrimitive* primitiveData = &element->data.primitive;
	primitiveData->p0		= p0;
	primitiveData->p1		= p1;
	primitiveData->color	= color;
}

void ImUiWidgetDrawTriangle( ImUiWidget* widget, ImUiPos p0, ImUiPos p1, ImUiPos p2, ImUiColor color )
{
	ImUiDrawElement* element = ImUiDrawPushElement( widget, ImUiDrawElementType_Triangle, NULL );
	ImUiDrawElementDataPrimitive* primitiveData = &element->data.primitive;
	primitiveData->p0		= p0;
	primitiveData->p1		= p1;
	primitiveData->p2		= p2;
	primitiveData->color	= color;
}

static void ImUiWidgetStateFreeList( ImUiAllocator* allocator, ImUiWidgetState* firstState )
{
	ImUiWidgetState* state = firstState;
	ImUiWidgetState* nextState = NULL;
	while( state )
	{
		nextState = state->nextUsageState;
		ImUiMemoryFree( allocator, state );
		state = nextState;
	}
}

static void ImUiLayoutGridContextFreeList( ImUiAllocator* allocator, ImUiLayoutGridContext* firstContext )
{
	ImUiLayoutGridContext* gridContext = firstContext;
	ImUiLayoutGridContext* nextGridContext = NULL;
	while( gridContext )
	{
		nextGridContext = gridContext->nextContext;
		ImUiMemoryFree( allocator, gridContext );
		gridContext = nextGridContext;
	}
}
