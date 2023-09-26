#include "imui/imui.h"

#include "imui_draw.h"
#include "imui_internal.h"
#include "imui_memory.h"

#include <string.h>
#include <math.h>

static void			ImUiWindowLayout( ImUiWindow* window );

static ImUiWidget*	ImUiWidgetAlloc( ImUiContext* imui );
static void			ImUiWidgetUpdateLayoutContext( ImUiWidget* widget, bool update );
static void			ImUiWidgetLayout( ImUiWidget* widget, const ImUiRect* parentInnerRect, float dpiScale );
static void			ImUiWidgetLayoutStackScroll( ImUiWidget* widget, const ImUiRect* parentInnerRect, float dpiScale );
static void			ImUiWidgetLayoutHorizontal( ImUiWidget* widget, const ImUiRect* parentInnerRect, float dpiScale );
static void			ImUiWidgetLayoutVertical( ImUiWidget* widget, const ImUiRect* parentInnerRect, float dpiScale );
static void			ImUiWidgetLayoutGrid( ImUiWidget* widget, const ImUiRect* parentInnerRect, float dpiScale );

static ImUiSize		ImUiWidgetCalculateSize( ImUiWidget* widget, ImUiSize minSize, ImUiSize maxSize, float factorWidth, float factorHeight );

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
		.width = IMUI_FLOAT_MAX,
		.height = IMUI_FLOAT_MAX
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

	ImUiWidgetState* state = imui->firstState;
	ImUiWidgetState* nextState = NULL;
	while( state )
	{
		nextState = state->nextState;
		ImUiMemoryFree( &imui->allocator, state );
		state = nextState;
	}

	state = imui->firstUnusedState;
	while( state )
	{
		nextState = state->nextState;
		ImUiMemoryFree( &imui->allocator, state );
		state = nextState;
	}

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
	{
		ImUiWidgetState* unusedState = imui->firstUnusedState;
		ImUiWidgetState* nextUnusedState = NULL;
		while( unusedState )
		{
			nextUnusedState = unusedState->nextState;
			ImUiMemoryFree( &imui->allocator, unusedState );
			unusedState = nextUnusedState;
		}
		imui->firstUnusedState	= imui->firstState;
		imui->firstState		= NULL;
	}

	ImUiTextLayoutCacheEndFrame( &imui->layoutCache );
}

ImUiSurface* ImUiSurfaceBegin( ImUiFrame* frame, ImUiStringView name, ImUiSize size, float dpiScale )
{
	ImUiContext* imui = frame->imui;

	ImUiSurface* surface = NULL;
	for( uintsize surfaceIndex = 0u; surfaceIndex < imui->surfaceCount; ++surfaceIndex )
	{
		if( !ImUiStringViewIsEquals( imui->surfaces[ surfaceIndex ].name, name ) )
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
	}

	surface->inUse		= true;
	surface->imui		= imui;
	surface->name		= ImUiStringPoolAdd( &imui->strings, name );
	surface->size		= size;
	surface->dpiScale	= dpiScale;

	return surface;
}

const ImUiDrawData* ImUiSurfaceEnd( ImUiSurface* surface )
{
	// sort windows by zOrder
	for( uintsize i = 1u; i < surface->windowCount; ++i )
	{
		while( surface->windows[ i - 1u ].zOrder > surface->windows[ i ].zOrder && i > 0u )
		{
			ImUiWindow tempWindow = surface->windows[ i ];
			surface->windows[ i ]		= surface->windows[ i - 1u ];
			surface->windows[ i - 1u ]	= tempWindow;

			i--;
		}
	}

	return ImUiDrawGenerateSurfaceData( &surface->imui->draw, surface );
}

ImUiContext* ImUiSurfaceGetContext( const ImUiSurface* surface )
{
	return surface->imui;
}

ImUiSize ImUiSurfaceGetSize( const ImUiSurface* surface )
{
	return surface->size;
}

float ImUiSurfaceGetDpiScale( const ImUiSurface* surface )
{
	return surface->dpiScale;
}

ImUiWindow* ImUiWindowBegin( ImUiSurface* surface, ImUiStringView name, ImUiRect rect, uint32_t zOrder )
{
	IMUI_ASSERT( surface );

	ImUiContext* imui = surface->imui;

	ImUiWindow* window = NULL;
	for( uintsize windowIndex = 0u; windowIndex < surface->windowCount; ++windowIndex )
	{
		if( !ImUiStringViewIsEquals( surface->windows[ windowIndex ].name, name ) )
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
	}

	window->inUse		= true;
	window->imui		= imui;
	window->frame		= &imui->frame;
	window->surface		= surface;
	window->name		= ImUiStringPoolAdd( &imui->strings, name );
	window->hash		= ImUiHashString( name, 0 );
	window->rect		= rect;
	window->zOrder		= zOrder;
	window->drawIndex	= ImUiDrawRegisterWindow( &imui->draw, window->hash );

	ImUiWidget* rootWidget = ImUiWidgetAlloc( imui );
	rootWidget->window		= window;
	rootWidget->name		= window->name;
	rootWidget->hash		= ImUiHashString( window->name, 0u );
	rootWidget->minSize		= rect.size;
	rootWidget->maxSize		= rect.size;
	rootWidget->rect		= rect;

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
	return window->frame->timeInSeconds;
}

static void ImUiWindowLayout( ImUiWindow* window )
{
	window->rootWidget->rect = ImUiRectCreate( 0.0f, 0.0f, window->rootWidget->maxSize.width, window->rootWidget->maxSize.height );

	const bool update = !window->rootWidget->lastFrameWidget || (window->rootWidget->hash != window->rootWidget->lastFrameWidget->hash);
	for( ImUiWidget* widget = window->rootWidget->firstChild; widget != NULL; widget = widget->nextSibling )
	{
		ImUiWidgetUpdateLayoutContext( widget, update );
	}

	for( ImUiWidget* widget = window->rootWidget->firstChild; widget != NULL; widget = widget->nextSibling )
	{
		ImUiWidgetLayout( widget, &window->rootWidget->rect, window->surface->dpiScale );
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

static void ImUiWidgetUpdateLayoutContext( ImUiWidget* widget, bool update )
{
	if( !update && widget->lastFrameWidget && widget->hash == widget->lastFrameWidget->hash )
	{
		for( ImUiWidget* childWidget = widget->firstChild; childWidget != NULL; childWidget = childWidget->nextSibling )
		{
			ImUiWidgetUpdateLayoutContext( childWidget, false );
		}
		return;
	}

	ImUiLayoutContext* context			= &widget->layoutContext;
	ImUiLayoutContext* parentContext	= &widget->parent->layoutContext;

	*context = IMUI_DEFAULT_LAYOUT_CONTEXT;

	context->minOuterSize = ImUiSizeExpandBorder( widget->minSize, widget->margin );

	parentContext->childCount++;
	parentContext->childrenStretch.width		+= widget->stretch.width;
	parentContext->childrenStretch.height		+= widget->stretch.height;
	parentContext->childrenMaxStretch.width		= IMUI_MAX( parentContext->childrenMaxStretch.width, widget->stretch.width );
	parentContext->childrenMaxStretch.height	= IMUI_MAX( parentContext->childrenMaxStretch.height, widget->stretch.height );

	for( ImUiWidget* childWidget = widget->firstChild; childWidget != NULL; childWidget = childWidget->nextSibling )
	{
		ImUiWidgetUpdateLayoutContext( childWidget, true );
	}

	switch( widget->parent->layout )
	{
	case ImUiLayout_Stack:
	case ImUiLayout_Scroll:
		parentContext->childrenMinSize.width	= IMUI_MAX( parentContext->childrenMinSize.width, widget->minSize.width );
		parentContext->childrenMinSize.height	= IMUI_MAX( parentContext->childrenMinSize.height, widget->minSize.height );
		parentContext->childrenMargin.width		= IMUI_MAX( parentContext->childrenMargin.width, widget->margin.left + widget->margin.right );
		parentContext->childrenMargin.height	= IMUI_MAX( parentContext->childrenMargin.height, widget->margin.top + widget->margin.bottom );
		//parentContext->childrenMaxSize.width	= IMUI_MAX( parentContext->childrenMaxSize.width, widget->maxSize.width );
		//parentContext->childrenMaxSize.height	= IMUI_MAX( parentContext->childrenMaxSize.height, widget->maxSize.height );
		break;

	case ImUiLayout_Horizontal:
		if( parentContext->childrenMinSize.width > 0.0f )
		{
			parentContext->childrenMinSize.width	+= widget->parent->layoutData.horizintalVertical.spacing;
		}
		parentContext->childrenMinSize.width		+= IMUI_MAX( context->minOuterSize.width, context->childrenMinSize.width ) + widget->padding.left + widget->padding.right;
		parentContext->childrenMinSize.height		= IMUI_MAX( parentContext->childrenMinSize.height, context->minOuterSize.height + context->childrenMinSize.height + widget->padding.top + widget->padding.bottom );
		parentContext->childrenMargin.width			+= widget->margin.left + widget->margin.right;
		parentContext->childrenMargin.height		= IMUI_MAX( parentContext->childrenMargin.height, widget->margin.top + widget->margin.bottom );
		break;

	case ImUiLayout_Vertical:
		if( parentContext->childrenMinSize.height > 0.0f )
		{
			parentContext->childrenMinSize.height	+= widget->parent->layoutData.horizintalVertical.spacing;
		}
		parentContext->childrenMinSize.width		= IMUI_MAX( parentContext->childrenMinSize.width, context->minOuterSize.width + context->childrenMinSize.width + widget->padding.left + widget->padding.right );
		parentContext->childrenMinSize.height		+= IMUI_MAX( context->minOuterSize.height, context->childrenMinSize.height ) + widget->padding.top + widget->padding.bottom;
		parentContext->childrenMargin.width			= IMUI_MAX( parentContext->childrenMargin.width, widget->margin.left + widget->margin.right );
		parentContext->childrenMargin.height		+= widget->margin.top + widget->margin.bottom;
		break;

	case ImUiLayout_Grid:
		break;
	}
}

static void ImUiWidgetLayout( ImUiWidget* widget, const ImUiRect* parentInnerRect, float dpiScale )
{
	switch( widget->parent->layout )
	{
	case ImUiLayout_Stack:
	case ImUiLayout_Scroll:
		ImUiWidgetLayoutStackScroll( widget, parentInnerRect, dpiScale );
		break;

	case ImUiLayout_Horizontal:
		ImUiWidgetLayoutHorizontal( widget, parentInnerRect, dpiScale );
		break;

	case ImUiLayout_Vertical:
		ImUiWidgetLayoutVertical( widget, parentInnerRect, dpiScale );
		break;

	case ImUiLayout_Grid:
		ImUiWidgetLayoutGrid( widget, parentInnerRect, dpiScale );
		break;
	}

	const ImUiRect innerRect = ImUiRectShrinkBorder( widget->rect, widget->padding );
	for( ImUiWidget* childWidget = widget->firstChild; childWidget != NULL; childWidget = childWidget->nextSibling )
	{
		ImUiWidgetLayout( childWidget, &innerRect, dpiScale );
	}
}

static void ImUiWidgetLayoutStackScroll( ImUiWidget* widget, const ImUiRect* parentInnerRect, float dpiScale )
{
	const float factorWidth			= IMUI_MIN( widget->stretch.width, widget->parent->layoutContext.childrenMaxStretch.width );
	const float factorHeight		= IMUI_MIN( widget->stretch.height, widget->parent->layoutContext.childrenMaxStretch.height );
	const ImUiSize minSize			= ImUiSizeMax( widget->minSize, ImUiSizeExpandBorder( widget->layoutContext.childrenMinSize, widget->padding ) );
	const ImUiSize maxSize			= ImUiSizeMax( ImUiSizeShrinkBorder( parentInnerRect->size, widget->margin ), ImUiSizeCreateZero() );
	ImUiSize size					= ImUiWidgetCalculateSize( widget, minSize, maxSize, factorWidth, factorHeight );

	ImUiPos pos;
	switch( widget->align.horizontal )
	{
	case ImUiHAlign_Left:
		pos.x = parentInnerRect->pos.x + widget->margin.left;
		break;
	case ImUiHAlign_Center:
		pos.x = parentInnerRect->pos.x + (parentInnerRect->size.width * 0.5f) - (size.width * 0.5f);
		break;

	case ImUiHAlign_Right:
		pos.x = (parentInnerRect->pos.x + parentInnerRect->size.width) - (widget->margin.left + size.width);
		break;
	}
	switch( widget->align.vertical )
	{
	case ImUiVAlign_Top:
		pos.y = parentInnerRect->pos.y + widget->margin.top;
		break;
	case ImUiVAlign_Center:
		pos.y = parentInnerRect->pos.y + (parentInnerRect->size.height * 0.5f) - (size.height * 0.5f);
		break;

	case ImUiVAlign_Bottom:
		pos.y = (parentInnerRect->pos.y + parentInnerRect->size.height) - (widget->margin.top + size.height);
		break;
	}

	// ???
	pos.x		= floorf( pos.x );
	pos.y		= floorf( pos.y );
	size.width	= ceilf( size.width );
	size.height	= ceilf( size.height );

	widget->rect.pos	= ImUiPosScale( pos, dpiScale );
	widget->rect.size	= ImUiSizeScale( size, dpiScale );
}

static void ImUiWidgetLayoutHorizontal( ImUiWidget* widget, const ImUiRect* parentInnerRect, float dpiScale )
{
	ImUiLayoutContext* parentContext = &widget->parent->layoutContext;

	const float factorWidth			= parentContext->childrenStretch.width ? widget->stretch.width / parentContext->childrenStretch.width : 0.0f;
	const float factorHeight		= parentContext->childrenMaxStretch.height ? widget->stretch.height / parentContext->childrenMaxStretch.height : 0.0f;
	const ImUiSize minSize			= ImUiSizeMax( widget->minSize, ImUiSizeExpandBorder( widget->layoutContext.childrenMinSize, widget->padding ) );
	const ImUiSize maxSize			= ImUiSizeMin( widget->maxSize, ImUiSizeSub( ImUiSizeMax( parentInnerRect->size, ImUiSizeCreate( 0.0f, parentContext->childrenMinSize.height ) ), parentContext->childrenMinSize.width - parentContext->childrenMargin.width, 0.0f ) );
	ImUiSize size					= ImUiWidgetCalculateSize( widget, minSize, maxSize, factorWidth, factorHeight );

	ImUiPos pos;
	if( widget->prevSibling )
	{
		pos.x = widget->prevSibling->rect.pos.x + widget->prevSibling->rect.size.width + widget->prevSibling->margin.right + widget->parent->layoutData.horizintalVertical.spacing + widget->margin.left;
	}
	else
	{
		pos.x = parentInnerRect->pos.x + widget->margin.left;
	}
	switch( widget->align.vertical )
	{
	case ImUiVAlign_Top:
		pos.y = parentInnerRect->pos.y + widget->margin.top;
		break;
	case ImUiVAlign_Center:
		pos.y = parentInnerRect->pos.y + (parentInnerRect->size.height * 0.5f) - (size.height * 0.5f);
		break;

	case ImUiVAlign_Bottom:
		pos.y = (parentInnerRect->pos.y + parentInnerRect->size.height) - (widget->margin.top + size.height);
		break;
	}

	// ???
	pos.x		= floorf( pos.x );
	pos.y		= floorf( pos.y );
	size.width	= ceilf( size.width );
	size.height	= ceilf( size.height );

	widget->rect.pos	= ImUiPosScale( pos, dpiScale );
	widget->rect.size	= ImUiSizeScale( size, dpiScale );
}

static void ImUiWidgetLayoutVertical( ImUiWidget* widget, const ImUiRect* parentInnerRect, float dpiScale )
{
	ImUiLayoutContext* parentContext = &widget->parent->layoutContext;

	const float factorWidth			= parentContext->childrenMaxStretch.width ? widget->stretch.width / parentContext->childrenMaxStretch.width : 0.0f;
	const float factorHeight		= parentContext->childrenStretch.height ? widget->stretch.height / parentContext->childrenStretch.height : 0.0f;
	const ImUiSize minSize			= ImUiSizeMax( widget->minSize, ImUiSizeExpandBorder( widget->layoutContext.childrenMinSize, widget->padding ) );
	const ImUiSize maxSize			= ImUiSizeMin( widget->maxSize, ImUiSizeSub( ImUiSizeMax( parentInnerRect->size, ImUiSizeCreate( parentContext->childrenMinSize.width, 0.0f ) ), 0.0f, parentContext->childrenMinSize.height - parentContext->childrenMargin.height ) );
	ImUiSize size					= ImUiWidgetCalculateSize( widget, minSize, maxSize, factorWidth, factorHeight );

	ImUiPos pos;
	switch( widget->align.horizontal )
	{
	case ImUiHAlign_Left:
		pos.x = parentInnerRect->pos.x + widget->margin.left;
		break;
	case ImUiHAlign_Center:
		pos.x = parentInnerRect->pos.x + (parentInnerRect->size.width * 0.5f) - (size.width * 0.5f);
		break;

	case ImUiHAlign_Right:
		pos.x = (parentInnerRect->pos.x + parentInnerRect->size.width) - (widget->margin.left + size.width);
		break;
	}
	if( widget->prevSibling )
	{
		pos.y = widget->prevSibling->rect.pos.y + widget->prevSibling->rect.size.height + widget->prevSibling->margin.bottom + widget->parent->layoutData.horizintalVertical.spacing + widget->margin.left;
	}
	else
	{
		pos.y = parentInnerRect->pos.y + widget->margin.top;
	}

	// ???
	pos.x		= floorf( pos.x );
	pos.y		= floorf( pos.y );
	size.width	= ceilf( size.width );
	size.height	= ceilf( size.height );

	widget->rect.pos	= ImUiPosScale( pos, dpiScale );
	widget->rect.size	= ImUiSizeScale( size, dpiScale );
}

static void ImUiWidgetLayoutGrid( ImUiWidget* widget, const ImUiRect* parentInnerRect, float dpiScale )
{

}

static ImUiSize ImUiWidgetCalculateSize( ImUiWidget* widget, ImUiSize minSize, ImUiSize maxSize, float factorWidth, float factorHeight )
{
	ImUiSize size = ImUiSizeLerp2( ImUiSizeCreateZero(), maxSize, factorWidth, factorHeight );
	size = ImUiSizeShrinkBorder( size, widget->margin );
	size = ImUiSizeMax( size, minSize );

	return size;
}

ImUiWidget* ImUiWidgetBegin( ImUiWindow* window )
{
	ImUiId id = 0u;
	if( window->currentWidget->lastChild )
	{
		id = window->currentWidget->lastChild->id + 1u;
	}

	return ImUiWidgetBeginId( window, id );
}

ImUiWidget* ImUiWidgetBeginId( ImUiWindow* window, ImUiId id )
{
	IMUI_ASSERT( window );

	ImUiWidget* widget = ImUiWidgetAlloc( window->imui );
	if( widget == NULL )
	{
		return NULL;
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

		window->lastFrameCurrentWidget = lastFrameWidget;
		if( lastFrameWidget )
		{
			widget->lastFrameWidget	= lastFrameWidget;
			widget->layoutContext	= lastFrameWidget->layoutContext;
			widget->rect			= lastFrameWidget->rect;
		}
	}

	return widget;
}

ImUiWidget* ImUiWidgetBeginNamed( ImUiWindow* window, ImUiStringView name )
{
	ImUiId id = ImUiHashString( name, 0u );
	if( window->currentWidget->lastChild )
	{
		id += window->currentWidget->lastChild->id + 1u;
	}

	ImUiWidget* widget = ImUiWidgetBeginId( window, id );
	if( widget == NULL )
	{
		return NULL;
	}

	widget->name = ImUiStringPoolAdd( &window->imui->strings, name );

	return widget;
}

void ImUiWidgetEnd( ImUiWidget* widget )
{
	IMUI_ASSERT( widget == widget->window->currentWidget );

	widget->hash = ImUiHashMix( widget->hash, ImUiHashCreate( &widget->id, IMUI_OFFSETOF( ImUiWidget, rect ) - IMUI_OFFSETOF( ImUiWidget, id ), 0u ) );

	if( widget->parent )
	{
		widget->parent->hash = ImUiHashMix( widget->parent->hash, widget->hash );
	}

	widget->window->currentWidget = widget->parent;
	if( widget->window->lastFrameCurrentWidget )
	{
		widget->window->lastFrameCurrentWidget = widget->window->lastFrameCurrentWidget->parent;
	}
}

void* ImUiWidgetAllocState( ImUiWidget* widget, size_t size )
{
	return ImUiWidgetAllocStateNewDestruct( widget, size, NULL, NULL );
}

void* ImUiWidgetAllocStateNew( ImUiWidget* widget, size_t size, bool* isNew )
{
	return ImUiWidgetAllocStateNewDestruct( widget, size, isNew, NULL );
}

void* ImUiWidgetAllocStateNewDestruct( ImUiWidget* widget, size_t size, bool* isNew, ImUiStateDestructFunc destructFunc )
{
	if( widget->state && widget->state->stateSize == size )
	{
		IMUI_ASSERT( widget->state->stateDestructFunc == destructFunc );

		if( isNew )
		{
			*isNew = false;
		}
		return widget->state->data;
	}
	else if( widget->lastFrameWidget &&
			 widget->lastFrameWidget->state &&
			 widget->lastFrameWidget->state->stateSize == size )
	{
		IMUI_ASSERT( widget->lastFrameWidget->state->stateDestructFunc == destructFunc );

		ImUiWidgetState* lastState = widget->lastFrameWidget->state;
		if( lastState->prevState )
		{
			lastState->prevState->nextState = lastState->nextState;
		}
		else
		{
			IMUI_ASSERT( widget->window->imui->firstUnusedState == lastState );
			widget->window->imui->firstUnusedState = lastState->nextState;
		}

		if( lastState->nextState )
		{
			lastState->nextState->prevState = lastState->prevState;
		}

		lastState->nextState = widget->window->imui->firstState;
		lastState->prevState = NULL;

		if( widget->window->imui->firstState )
		{
			widget->window->imui->firstState->prevState = lastState;
		}
		widget->window->imui->firstState = lastState;

		widget->state = lastState;

		if( isNew )
		{
			*isNew = false;
		}
		return widget->state->data;
	}

	ImUiWidgetState* newState = ImUiMemoryAllocZero( &widget->window->imui->allocator, IMUI_OFFSETOF( ImUiWidgetState, data ) + size );
	if( !newState )
	{
		return NULL;
	}

	newState->nextState			= widget->window->imui->firstState;
	newState->stateSize			= size;
	newState->stateDestructFunc	= destructFunc;

	if( widget->window->imui->firstState )
	{
		widget->window->imui->firstState->prevState = newState;
	}
	widget->window->imui->firstState = newState;

	widget->state = newState;

	if( isNew )
	{
		*isNew = true;
	}
	return widget->state->data;
}

ImUiLayout ImUiWidgetGetLayout( const ImUiWidget* widget )
{
	return widget->layout;
}

void ImUiWidgetSetLayoutStack( ImUiWidget* widget )
{
	widget->layout = ImUiLayout_Stack;
}

void ImUiWidgetSetLayoutScroll( ImUiWidget* widget, ImUiPos offset )
{
	widget->layout						= ImUiLayout_Scroll;
	widget->layoutData.scroll.offset	= offset;
}

void ImUiWidgetSetLayoutHorizontal( ImUiWidget* widget )
{
	widget->layout									= ImUiLayout_Horizontal;
	widget->layoutData.horizintalVertical.spacing	= 0.0f;
}

void ImUiWidgetSetLayoutHorizontalSpacing( ImUiWidget* widget, float spacing )
{
	widget->layout									= ImUiLayout_Horizontal;
	widget->layoutData.horizintalVertical.spacing	= spacing;
}

void ImUiWidgetSetLayoutVerical( ImUiWidget* widget )
{
	widget->layout									= ImUiLayout_Vertical;
	widget->layoutData.horizintalVertical.spacing	= 0.0f;
}

void ImUiWidgetSetLayoutVerticalSpacing( ImUiWidget* widget, float spacing )
{
	widget->layout									= ImUiLayout_Vertical;
	widget->layoutData.horizintalVertical.spacing	= spacing;
}

float ImUiWidgetGetTime( const ImUiWidget* widget )
{
	return widget->window->frame->timeInSeconds;
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

void ImUiWidgetSetMinSize( ImUiWidget* widget, ImUiSize size )
{
	IMUI_ASSERT( size.width >= 0.0f && size.height >= 0.0f );
	widget->minSize = size;
}

ImUiSize ImUiWidgetGetMaxSize( const ImUiWidget* widget )
{
	return widget->maxSize;
}

void ImUiWidgetSetMaxSize( ImUiWidget* widget, ImUiSize size )
{
	IMUI_ASSERT( size.width >= 0.0f && size.height >= 0.0f );
	widget->maxSize = size;
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

ImUiSize ImUiWidgetGetStretch( const ImUiWidget* widget )
{
	return widget->stretch;
}

void ImUiWidgetSetStretch( ImUiWidget* widget, ImUiSize stretch )
{
	IMUI_ASSERT( stretch.width >= 0.0f && stretch.height >= 0.0f );
	widget->stretch = stretch;
}

ImUiAlign ImUiWidgetGetAlign( const ImUiWidget* widget )
{
	return widget->align;
}

void ImUiWidgetSetAlign( ImUiWidget* widget, ImUiAlign align )
{
	widget->align = align;
}

void ImUiWidgetSetHAlign( ImUiWidget* widget, ImUiHAlign align )
{
	widget->align.horizontal = align;
}

void ImUiWidgetSetVAlign( ImUiWidget* widget, ImUiVAlign align )
{
	widget->align.vertical = align;
}

ImUiPos ImUiWidgetGetPos( const ImUiWidget* widget )
{
	return widget->rect.pos;
}

ImUiSize ImUiWidgetGetSize( const ImUiWidget* widget )
{
	return widget->rect.size;
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
