#include "imui/imui.h"

#include "imui_draw.h"
#include "imui_internal.h"
#include "imui_memory.h"

#include <string.h>

static void			ImUiWindowLayout( ImUiWindow* window );

static ImUiWidget*	ImUiWidgetAlloc( ImUiContext* imui );
static void			ImUiWidgetUpdateLayoutContext( ImUiWidget* widget );
static void			ImUiWidgetLayout( ImUiWidget* widget, const ImUiRectangle* parentInnerRect );
static void			ImUiWidgetLayoutStackScroll( ImUiWidget* widget, const ImUiRectangle* parentInnerRect );
static void			ImUiWidgetLayoutHorizontal( ImUiWidget* widget, const ImUiRectangle* parentInnerRect );
static void			ImUiWidgetLayoutVertical( ImUiWidget* widget, const ImUiRectangle* parentInnerRect );
static void			ImUiWidgetLayoutGrid( ImUiWidget* widget, const ImUiRectangle* parentInnerRect );

static const ImUiWidget IMUI_DEFAULT_WIDGET =
{
	.maxSize = {
		.width = IMUI_FLOAT_MAX,
		.height = IMUI_FLOAT_MAX
	}
};

static const ImUiWidgetLayoutContext IMUI_DEFAULT_LAYOUT_CONTEXT =
{
	0
	/*.childrenMinSize = {
		.width = IMUI_FLOAT_MAX,
		.height = IMUI_FLOAT_MAX
	}*/
};

ImUiContext* ImUiCreate( const ImUiParameters* parameters )
{
	ImUiAllocator allocator = parameters->allocator;
	if( allocator.mallocFunc == NULL ||
		allocator.freeFunc == NULL )
	{
		allocator.mallocFunc	= ImUiMemoryDefaultAlloc;
		allocator.reallocFunc	= ImUiMemoryDefaultRealloc;
		allocator.freeFunc		= ImUiMemoryDefaultFree;
		allocator.userData		= NULL;
		allocator.internalData	= NULL;
	}
	else if( allocator.reallocFunc == NULL )
	{
		allocator.reallocFunc	= ImUiMemoryPseudoRealloc;
		allocator.internalData	= &allocator;
	}
	else
	{
		allocator.internalData	= NULL;
	}

	ImUiContext* imui = IMUI_MEMORY_NEW_ZERO( &allocator, ImUiContext );
	if( !imui )
	{
		return NULL;
	}
	imui->allocator					= allocator;
	imui->allocator.internalData	= &imui->allocator;

	ImUiInputConstruct( &imui->input, &imui->allocator );
	ImUiDrawConstruct( &imui->draw, &imui->allocator, &parameters->vertexFormat, parameters->vertexType );
	ImUiStringPoolConstruct( &imui->strings, &imui->allocator );

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

	ImUiMemoryFree( &imui->allocator, imui );
}

ImUiFrame* ImUiBegin( ImUiContext* imui )
{
	imui->frame.imui	= imui;
	imui->frame.input	= &imui->input;
	imui->frame.draw	= &imui->draw;

	return &imui->frame;
}

void ImUiEnd( ImUiFrame* frame )
{
	ImUiContext* imui = frame->imui;

	ImUiDrawEndFrame( &imui->draw );

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

	//ImUiStringPoolClear( &imui->strings );
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

ImUiSize ImUiSurfaceGetSize( const ImUiSurface* surface )
{
	return surface->size;
}

float ImUiSurfaceGetDpiScale( const ImUiSurface* surface )
{
	return surface->dpiScale;
}

ImUiWindow* ImUiWindowBegin( ImUiSurface* surface, ImUiStringView name, ImUiRectangle rectangle, uint32_t zOrder )
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
	window->surface		= surface;
	window->name		= ImUiStringPoolAdd( &imui->strings, name );
	window->hash		= ImUiHashString( name, 0 );
	window->rectangle	= rectangle;
	window->zOrder		= zOrder;
	window->drawIndex	= ImUiDrawRegisterWindow( &imui->draw, window->hash );

	ImUiWidget* rootWidget = ImUiWidgetAlloc( imui );
	rootWidget->window		= window;
	rootWidget->name		= window->name;
	rootWidget->hash		= ImUiHashString( window->name, 0u );
	ImUiWidgetSetFixedSize( rootWidget, rectangle.size );

	window->lastFrameRootWidget		= window->rootWidget;
	window->lastFrameCurrentWidget	= window->rootWidget;

	window->rootWidget		= rootWidget;
	window->currentWidget	= window->rootWidget;
	return window;
}

void ImUiWindowEnd( ImUiWindow* window )
{
	ImUiWidgetEnd( window->rootWidget );
	ImUiWindowLayout( window );
}

ImUiInput* ImUiInputBegin( ImUiContext* imui )
{
	ImUiInputNextTick( &imui->input );
	return &imui->input;
}

void ImUiInputEnd( ImUiContext* imui )
{
}

static void ImUiWindowLayout( ImUiWindow* window )
{
	window->rootWidget->rectangle = ImUiRectangleCreate( 0.0f, 0.0f, window->rootWidget->maxSize.width, window->rootWidget->maxSize.height );

	for( ImUiWidget* widget = window->rootWidget->firstChild; widget != NULL; widget = widget->nextSibling )
	{
		ImUiWidgetUpdateLayoutContext( widget );
	}

	for( ImUiWidget* widget = window->rootWidget->firstChild; widget != NULL; widget = widget->nextSibling )
	{
		ImUiWidgetLayout( widget, &window->rootWidget->rectangle );
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

static void ImUiWidgetUpdateLayoutContext( ImUiWidget* widget )
{
	ImUiWidgetLayoutContext* context		= &widget->layoutContext;
	ImUiWidgetLayoutContext* parentContext	= &widget->parent->layoutContext;

	parentContext->childrenStretch.width	+= widget->stretch.width;
	parentContext->childrenStretch.height	+= widget->stretch.height;

	for( ImUiWidget* childidget = widget->firstChild; childidget != NULL; childidget = childidget->nextSibling )
	{
		ImUiWidgetUpdateLayoutContext( childidget );
	}

	switch( widget->parent->layout )
	{
	case ImUiLayout_Stack:
	case ImUiLayout_Scroll:
		parentContext->childrenMinSize.width	= IMUI_MAX( parentContext->childrenMinSize.width, widget->minSize.width );
		parentContext->childrenMinSize.height	= IMUI_MAX( parentContext->childrenMinSize.height, widget->minSize.height );
		parentContext->childrenMaxSize.width	= IMUI_MAX( parentContext->childrenMaxSize.width, widget->maxSize.width );
		parentContext->childrenMaxSize.height	= IMUI_MAX( parentContext->childrenMaxSize.height, widget->maxSize.height );
		break;

	case ImUiLayout_Horizontal:
		break;

	case ImUiLayout_Vertical:
		break;

	case ImUiLayout_Grid:
		break;
	}

	//if( !widget->firstChild )
	//{
	//	context->minInnerRect	= ImUiRectangleShrinkThickness( parentContext->minInnerRect, widget->margin );
	//}
	//else
	//{

	//}

}


static void ImUiWidgetLayout( ImUiWidget* widget, const ImUiRectangle* parentInnerRect )
{
	switch( widget->layout )
	{
	case ImUiLayout_Stack:
	case ImUiLayout_Scroll:
		ImUiWidgetLayoutStackScroll( widget, parentInnerRect );
		break;

	case ImUiLayout_Horizontal:
		break;

	case ImUiLayout_Vertical:
		break;

	case ImUiLayout_Grid:
		break;
	}

	const ImUiRectangle innerRect = ImUiRectangleShrinkThickness( widget->rectangle, widget->padding );
	for( ImUiWidget* childWidget = widget->firstChild; childWidget != NULL; childWidget = childWidget->nextSibling )
	{
		ImUiWidgetLayout( childWidget, &innerRect );
	}
}

static void ImUiWidgetLayoutStackScroll( ImUiWidget* widget, const ImUiRectangle* parentInnerRect )
{
	const float factorWidth			= IMUI_MIN( widget->stretch.width, 1.0f );
	const float factorHeight		= IMUI_MIN( widget->stretch.height, 1.0f );
	const ImUiSize maxSize			= ImUiSizeShrinkThickness( parentInnerRect->size, widget->margin );
	const ImUiSize size				= ImUiSizeLerp2( widget->minSize, maxSize, factorWidth, factorHeight );

	ImUiPosition position;
	switch( widget->alignment.horizontal )
	{
	case ImUiHorizintalAlignment_Left:
		position.x = parentInnerRect->position.x + widget->margin.left;
		break;
	case ImUiHorizintalAlignment_Center:
		position.x = parentInnerRect->position.x + (parentInnerRect->size.width * 0.5f) - (size.width * 0.5f);
		break;

	case ImUiHorizintalAlignment_Right:
		position.x = (parentInnerRect->position.x + parentInnerRect->size.width) - (widget->margin.left + size.width);
		break;
	}
	switch( widget->alignment.vertical )
	{
	case ImUiVerticalAlignment_Top:
		position.y = parentInnerRect->position.y + widget->margin.top;
		break;
	case ImUiVerticalAlignment_Center:
		position.y = parentInnerRect->position.y + (parentInnerRect->size.height * 0.5f) - (size.height * 0.5f);
		break;

	case ImUiVerticalAlignment_Bottom:
		position.y = (parentInnerRect->position.y + parentInnerRect->size.height) - (widget->margin.top + size.height);
		break;
	}

	widget->rectangle.position	= position;  //ImUiPositionAdd( widget->parent->rectangle.position, widget->margin.left, widget->margin.top );
	widget->rectangle.size		= size;		 //ImUiSizeSub( widget->parent->rectangle.size, widget->margin.left + widget->margin.right, widget->margin.top + widget->margin.bottom );
}

static void ImUiWidgetLayoutHorizontal( ImUiWidget* widget, const ImUiRectangle* parentInnerRect )
{

}

static void ImUiWidgetLayoutVertical( ImUiWidget* widget, const ImUiRectangle* parentInnerRect )
{

}

static void ImUiWidgetLayoutGrid( ImUiWidget* widget, const ImUiRectangle* parentInnerRect )
{

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

	if( parent->firstChild == NULL )
	{
		parent->firstChild = widget;
		parent->lastChild = widget;
	}
	else
	{
		widget->previousSibling = parent->lastChild;
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
			widget->lastFrameHash	= lastFrameWidget->hash;
			widget->layoutContext	= lastFrameWidget->layoutContext;
			widget->rectangle		= lastFrameWidget->rectangle;
		}
	}

	return widget;
}

ImUiWidget* ImUiWidgetBeginNamed( ImUiWindow* window, ImUiStringView name )
{
	ImUiWidget* widget = ImUiWidgetBeginId( window, ImUiHashString( name, 0u ) );
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

	widget->hash = ImUiHashCreate( &widget->id, IMUI_OFFSETOF( ImUiWidget, rectangle ) - IMUI_OFFSETOF( ImUiWidget, id ), 0u );

	if( widget->parent )
	{
		widget->parent->hash = ImUiHashMix( widget->parent->hash, widget->hash );

		if( widget->window->lastFrameCurrentWidget &&
			widget->lastFrameHash != widget->hash )
		{
			widget->layoutContext = IMUI_DEFAULT_LAYOUT_CONTEXT;
		}
	}

	widget->window->currentWidget = widget->parent;
	if( widget->window->lastFrameCurrentWidget )
	{
		widget->window->lastFrameCurrentWidget = widget->window->lastFrameCurrentWidget->parent;
	}
}

ImUiLayout ImUiWidgetGetLayout( const ImUiWidget* widget )
{
	return widget->layout;
}

void ImUiWidgetSetStackLayout( ImUiWidget* widget )
{
	widget->layout = ImUiLayout_Stack;
}

void ImUiWidgetSetScrollLayout( ImUiWidget* widget, ImUiPosition offset )
{
	widget->layout = ImUiLayout_Scroll;
}

ImUiThickness ImUiWidgetGetMargin( const ImUiWidget* widget )
{
	return widget->margin;
}

void ImUiWidgetSetMargin( ImUiWidget* widget, ImUiThickness margin )
{
	widget->margin = margin;
}

ImUiThickness ImUiWidgetGetPadding( const ImUiWidget* widget )
{
	return widget->padding;
}

void ImUiWidgetSetPadding( ImUiWidget* widget, ImUiThickness padding )
{
	widget->padding = padding;
}

ImUiSize ImUiWidgetGetMinSize( const ImUiWidget* widget )
{
	return widget->minSize;
}

void ImUiWidgetSetMinSize( ImUiWidget* widget, ImUiSize size )
{
	widget->minSize = size;
}

ImUiSize ImUiWidgetGetMaxSize( const ImUiWidget* widget )
{
	return widget->maxSize;
}

void ImUiWidgetSetMaxSize( ImUiWidget* widget, ImUiSize size )
{
	widget->maxSize = size;
}

ImUiSize ImUiWidgetGetPrefSize( const ImUiWidget* widget )
{
	return widget->prefSize;
}

void ImUiWidgetSetPrefSize( ImUiWidget* widget, ImUiSize size )
{
	widget->prefSize = size;
}

void ImUiWidgetSetFixedSize( ImUiWidget* widget, ImUiSize size )
{
	widget->minSize		= size;
	widget->maxSize		= size;
	widget->prefSize	= size;
}

ImUiSize ImUiWidgetGetStretch( const ImUiWidget* widget )
{
	return widget->stretch;
}

void ImUiWidgetSetStretch( ImUiWidget* widget, ImUiSize stretch )
{
	IMUI_ASSERT( stretch.width >= 0.0f );
	IMUI_ASSERT( stretch.height >= 0.0f );
	widget->stretch = stretch;
}

ImUiAlignment ImUiWidgetGetAlignment( const ImUiWidget* widget )
{
	return widget->alignment;
}

void ImUiWidgetSetAlignment( ImUiWidget* widget, ImUiAlignment alignment )
{
	widget->alignment = alignment;
}

void ImUiWidgetSetHorizintalAlignment( ImUiWidget* widget, ImUiHorizontalAlignment alignment )
{
	widget->alignment.horizontal = alignment;
}

void ImUiWidgetSetVerticalAlignment( ImUiWidget* widget, ImUiVerticalAlignment alignment )
{
	widget->alignment.vertical = alignment;
}

ImUiRectangle ImUiWidgetGetRectangle( const ImUiWidget* widget )
{
	return widget->rectangle;
}
