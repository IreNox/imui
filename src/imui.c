#include "imui/imui.h"

#include "imui_draw.h"
#include "imui_internal.h"
#include "imui_memory.h"
#include "imui_widget.h"

static void			ImUiWindowLayout( ImUiWindow* window );
static void			ImUiWindowLayoutWidget( ImUiWindow* window, ImUiWidget* widget, const ImUiRectangle* minInnerRect, const ImUiRectangle* maxInnerRect );
static void			ImUiWidgetUpdateLayoutContext( ImUiWidget* widget );

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

	const ImUiWidget defaultWidget =
	{
		.maxSize = {
			.width = IMUI_FLOAT_MAX,
			.height = IMUI_FLOAT_MAX
		}
	};
	imui->defaultWidget = defaultWidget;

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

	ImUiWidgetChunk* pChunk = imui->firstChunk;
	while( pChunk )
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

	for( uintsize i = 0u; i < imui->surfaceCount; ++i )
	{
		ImUiSurface* surface = &imui->surfaces[ i ];
		surface->windowCount = 0u;
	}
	imui->surfaceCount = 0u;

	for( ImUiWidgetChunk* chunk = imui->firstChunk; chunk != NULL; chunk = chunk->nextChunk )
	{
		chunk->usedCount = 0u;
	}

	ImUiStringPoolClear( &imui->strings );
}

ImUiSurface* ImUiSurfaceBegin( ImUiFrame* frame, ImUiStringView name, ImUiSize size, float dpiScale )
{
	ImUiContext* imui = frame->imui;

	if( !IMUI_MEMORY_ARRAY_CHECK_CAPACITY_ZERO( &imui->allocator, imui->surfaces, imui->surfaceCapacity, imui->surfaceCount + 1u ) )
	{
		return NULL;
	}

	ImUiSurface* surface = &imui->surfaces[ imui->surfaceCount ];
	imui->surfaceCount++;

	surface->imui			= imui;
	surface->name			= ImUiStringPoolAdd( &imui->strings, name );
	surface->size			= size;
	surface->dpiScale		= dpiScale;

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

	if( !IMUI_MEMORY_ARRAY_CHECK_CAPACITY_ZERO( &imui->allocator, surface->windows, surface->windowCapacity, surface->windowCount + 1u ) )
	{
		return NULL;
	}

	ImUiWindow* window = &surface->windows[ surface->windowCount ];
	surface->windowCount++;

	window->imui			= imui;
	window->surface			= surface;
	window->name			= ImUiStringPoolAdd( &imui->strings, name );
	window->hash			= ImUiHashString( name, 0 );
	window->rectangle		= rectangle;
	window->zOrder			= zOrder;
	window->drawIndex		= ImUiDrawRegisterWindow( &imui->draw, window->hash );

	ImUiWidget* rootWidget = ImUiWidgetAlloc( imui );
	rootWidget->window		= window;
	rootWidget->name		= window->name;
	rootWidget->hash		= ImUiHashString( window->name, 0u );
	ImUiWidgetSetFixedSize( rootWidget, rectangle.size );

	window->rootWidget		= rootWidget;

	return window;
}

void ImUiWindowEnd( ImUiWindow* window )
{
	ImUiWindowLayout( window );
}

ImUiWidget* ImUiWindowGetRootWidget( ImUiWindow* window )
{
	return window->rootWidget;
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
	ImUiWidgetUpdateLayoutContext( window->rootWidget );

	for( ImUiWidget* widget = window->rootWidget->firstChild; widget != NULL; widget = widget->nextSibling )
	{
		ImUiWindowLayoutWidget( window, widget, &widget->rectangle, &widget->rectangle );
	}
}

static void ImUiWindowLayoutWidget( ImUiWindow* window, ImUiWidget* widget, const ImUiRectangle* minInnerRect, const ImUiRectangle* maxInnerRect )
{
	ImUiWidgetUpdateLayoutContext( widget ); // , &childrenMinSize, &childrenMaxSize, &childrenPrefSize );

	//ImUiRectangle minRect = ImUiRectangleShrinkThickness( *minInnerRect, widget->margin );
	//ImUiRectangle maxRect = ImUiRectangleShrinkThickness( *maxInnerRect, widget->margin );

	widget->rectangle.position	= ImUiPositionAdd( widget->parent->rectangle.position, widget->margin.left, widget->margin.top );
	widget->rectangle.size		= ImUiSizeSub( widget->parent->rectangle.size, widget->margin.left + widget->margin.right, widget->margin.top + widget->margin.bottom );
}

static void ImUiWidgetUpdateLayoutContext( ImUiWidget* widget )
{
	ImUiWidgetLayoutContext* context		= &widget->layoutContext;
	ImUiWidgetLayoutContext* parentContext	= &widget->parent->layoutContext;

	if( !widget->firstChild )
	{
		context->minInnerRect	= ImUiRectangleShrinkThickness( parentContext->minInnerRect, widget->margin );
	}
	else
	{

	}
}
