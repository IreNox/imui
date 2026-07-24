#include "imui/imui.h"

#include "imui_draw.h"
#include "imui_font.h"
#include "imui_internal.h"
#include "imui_memory.h"

#include <string.h>
#include <math.h>

static void			imuiWindowLayout( ImuiWindow* window );

static ImuiWidget*	imuiWidgetAlloc( ImuiContext* imui );
static void			imuiWidgetUpdateLayoutContext( ImuiWidget* widget, uintsize widgetIndex, float dpiScale, bool update );
static void			imuiWidgetUpdateLayoutContextCheckGridContextSize( ImuiWidget* widget );
static void			imuiWidgetUpdateLayoutContextGrid( ImuiWidget* widget );
static void			imuiWidgetLayout( ImuiWidget* widget, const ImuiRect* parentInnerRect, float dpiScale, uintsize widgetIndex, bool update );
static void			imuiWidgetLayoutPrepareGrid( ImuiWidget* widget, const ImuiRect* parentInnerRect, float dpiScale );
static void			imuiWidgetLayoutStack( ImuiWidget* widget, const ImuiRect* parentInnerRect, float dpiScale );
static void			imuiWidgetLayoutScroll( ImuiWidget* widget, const ImuiRect* parentInnerRect, float dpiScale );
static void			imuiWidgetLayoutHorizontalCollectStrecher( ImuiWidget* widget, const ImuiRect* innerRect, float dpiScale );
static void			imuiWidgetLayoutHorizontal( ImuiWidget* widget, const ImuiRect* parentInnerRect, float dpiScale );
static void			imuiWidgetLayoutVerticalCollectStrecher( ImuiWidget* widget, const ImuiRect* innerRect, float dpiScale );
static void			imuiWidgetLayoutVertical( ImuiWidget* widget, const ImuiRect* parentInnerRect, float dpiScale );
static void			imuiWidgetLayoutGrid( ImuiWidget* widget, const ImuiRect* parentInnerRect, float dpiScale, uintsize widgetIndex );
static ImuiSize		imuiWidgetLayoutMinSize( ImuiWidget* widget, const ImuiRect* parentInnerRect, float dpiScale );
static ImuiSize		imuiWidgetCalculateSize( ImuiWidget* widget, ImuiSize minSize, ImuiSize maxSize, float factorWidth, float factorHeight, float dpiScale );
static float		imuiWidgetLayoutPositionX( ImuiWidget* widget, const ImuiRect* parentInnerRect, float width, float dpiScale );
static float		imuiWidgetLayoutPositionY( ImuiWidget* widget, const ImuiRect* parentInnerRect, float height, float dpiScale );
static void			imuiWidgetLayoutRect( ImuiWidget* widget, ImuiPos pos, ImuiSize size );

static void			imuiWidgetStateFreeList( ImuiAllocator* allocator, ImuiWidgetState* firstState );
static void			imuiLayoutGridContextFreeList( ImuiAllocator* allocator, ImuiLayoutGridContext* firstContext );

static const ImuiLayoutContext IMUI_DEFAULT_LAYOUT_CONTEXT =
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

static const ImuiWidget IMUI_DEFAULT_WIDGET =
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

ImuiContext* imuiCreate( const ImuiParameters* parameters )
{
	ImuiAllocator allocator;
	imuiMemoryAllocatorPrepare( &allocator, &parameters->allocator );

	ImuiContext* imui = IMUI_MEMORY_NEW_ZERO( &allocator, ImuiContext );
	if( !imui )
	{
		return NULL;
	}

	imuiMemoryAllocatorFinalize( &imui->allocator, &allocator );

	if( !imuiInputConstruct( &imui->input, &imui->allocator, parameters->shortcuts, parameters->shortcutCount ) ||
		!imuiDrawConstruct( &imui->draw, &imui->allocator, &parameters->vertexFormat, parameters->vertexType ) ||
		!imuiStringPoolConstruct( &imui->strings, &imui->allocator ) ||
		!imuiTextLayoutCacheConstruct( &imui->layoutCache, &imui->allocator ) )
	{
		imuiDestroy( imui );
		return NULL;
	}

	return imui;
}

void imuiDestroy( ImuiContext* imui )
{
	for( uintsize i = 0u; i < imui->surfaceCapacity; ++i )
	{
		ImuiSurface* surface = &imui->surfaces[ i ];
		imuiMemoryFree( &imui->allocator, surface->windows );
	}
	imuiMemoryFree( &imui->allocator, imui->surfaces );

	imuiWidgetStateFreeList( &imui->allocator, imui->firstState );
	imuiWidgetStateFreeList( &imui->allocator, imui->firstUnusedState );

	imuiLayoutGridContextFreeList( &imui->allocator, imui->firstGridContext );
	imuiLayoutGridContextFreeList( &imui->allocator, imui->firstUnusedGridContext );

	for( ImuiWidgetChunk* pChunk = imui->firstChunk; pChunk != NULL; )
	{
		ImuiWidgetChunk* pNextChunk = pChunk->nextChunk;
		imuiMemoryFree( &imui->allocator, pChunk );
		pChunk = pNextChunk;
	}

	for( ImuiWidgetChunk* pChunk = imui->firstLastFrameChunk; pChunk != NULL; )
	{
		ImuiWidgetChunk* pNextChunk = pChunk->nextChunk;
		imuiMemoryFree( &imui->allocator, pChunk );
		pChunk = pNextChunk;
	}

	for( ImuiWidgetChunk* pChunk = imui->firstFreeChunk; pChunk != NULL; )
	{
		ImuiWidgetChunk* pNextChunk = pChunk->nextChunk;
		imuiMemoryFree( &imui->allocator, pChunk );
		pChunk = pNextChunk;
	}

	imuiInputDestruct( &imui->input );
	imuiDrawDestruct( &imui->draw );
	imuiStringPoolDestruct( &imui->strings );
	imuiTextLayoutCacheDestruct( &imui->layoutCache );

	imuiMemoryFree( &imui->allocator, imui );
}

ImuiFrame* imuiBegin( ImuiContext* imui, double timeInSeconds )
{
	imui->frame.context			= imui;
	imui->frame.index++;
	imui->frame.timeInSeconds	= timeInSeconds;

	return &imui->frame;
}

void imuiEnd( ImuiFrame* frame )
{
	ImuiContext* imui = frame->context;

	imuiDrawEndFrame( &imui->draw );
	imuiInputEndFrame( &imui->input );

	// deleted unused surfaces and windows
	for( uintsize surfaceIndex = 0u; surfaceIndex < imui->surfaceCount; ++surfaceIndex )
	{
		ImuiSurface* surface = &imui->surfaces[ surfaceIndex ];
		if( !surface->inUse )
		{
			imuiMemoryFree( &imui->allocator, surface->windows );

			IMUI_MEMORY_ARRAY_REMOVE_UNSORTED_ZERO( imui->surfaces, imui->surfaceCount, surfaceIndex );

			surfaceIndex--;
			continue;
		}

		for( uintsize windowIndex = 0u; windowIndex < surface->windowCount; ++windowIndex )
		{
			ImuiWindow* window = &surface->windows[ windowIndex ];
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
		while( imui->firstFreeChunk )
		{
			ImuiWidgetChunk* nextFreeChunk = imui->firstFreeChunk->nextChunk;

			imuiMemoryFree( &imui->allocator, imui->firstFreeChunk );

			imui->firstFreeChunk = nextFreeChunk;
		}

		for( ImuiWidgetChunk* chunk = imui->firstLastFrameChunk; chunk != NULL; chunk = chunk->nextChunk )
		{
			chunk->usedCount = 0u;
		}

		imui->firstFreeChunk		= imui->firstLastFrameChunk;
		imui->firstLastFrameChunk	= imui->firstChunk;
		imui->firstChunk			= NULL;
	}

	// free unused states
	imuiWidgetStateFreeList( &imui->allocator, imui->firstUnusedState );
	imui->firstUnusedState	= imui->firstState;
	imui->firstState		= NULL;

	// free unused grid context
	imuiLayoutGridContextFreeList( &imui->allocator, imui->firstUnusedGridContext );
	imui->firstUnusedGridContext	= imui->firstGridContext;
	imui->firstGridContext			= NULL;

	imuiTextLayoutCacheEndFrame( &imui->layoutCache );
}

ImuiInput* imuiInputBegin( ImuiContext* imui, const ImuiInputState* previousState )
{
	if( !imuiInputBeginState( &imui->input, previousState ) )
	{
		return NULL;
	}

	return &imui->input;
}

const ImuiInputState* imuiInputEnd( ImuiContext* imui )
{
	return imuiInputEndState( &imui->input );
}

const ImuiInputState* imuiInputGetPushState( ImuiInput* input )
{
	return input->pushState;
}

ImuiContext* imuiFrameGetContext( const ImuiFrame* frame )
{
	return frame->context;
}

ImuiSurface* imuiSurfaceBegin( ImuiFrame* frame, const char* name, ImuiSize size, const ImuiInputState* input, float dpiScale )
{
	return imuiSurfaceBeginId( frame, name, (ImuiId)imuiHashCreate( name, strlen( name ) ), size, input, dpiScale );
}

ImuiSurface* imuiSurfaceBeginId( ImuiFrame* frame, const char* name, ImuiId id, ImuiSize size, const ImuiInputState* input, float dpiScale )
{
	ImuiContext* imui = frame->context;
	const ImuiStringView nameView = imuiStringViewCreate( name );

	ImuiSurface* surface = NULL;
	for( uintsize surfaceIndex = 0u; surfaceIndex < imui->surfaceCount; ++surfaceIndex )
	{
		if( imui->surfaces[ surfaceIndex ].id != id )
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

		surface->id			= id;
		surface->context	= imui;
		surface->name		= imuiStringPoolAdd( &imui->strings, nameView );
	}

	surface->inUse		= true;
	surface->size		= size;
	surface->input		= input;
	surface->dpiScale	= dpiScale;
	surface->drawIndex	= imuiDrawRegisterSurface( &imui->draw, surface->name, size );

	return surface;
}

void imuiSurfaceEnd( ImuiSurface* surface )
{
	imuiDrawSurfaceEnd( &surface->context->draw, surface->drawIndex );
}

void imuiSurfaceGetMaxBufferSizes( ImuiSurface* surface, size_t* outVertexDataSize, size_t* outIndexDataSize )
{
	imuiDrawGetSurfaceMaxBufferSizes( &surface->context->draw, surface->drawIndex, outVertexDataSize, outIndexDataSize );
}

const ImuiDrawData* imuiSurfaceGenerateDrawData( ImuiSurface* surface, void* outVertexData, size_t* inOutVertexDataSize, void* outIndexData, size_t* inOutIndexDataSize )
{
	return imuiDrawGenerateSurfaceData( &surface->context->draw, surface->drawIndex, outVertexData, inOutVertexDataSize, outIndexData, inOutIndexDataSize );
}

ImuiContext* imuiSurfaceGetContext( const ImuiSurface* surface )
{
	return surface->context;
}

double imuiSurfaceGetTime( const ImuiSurface* surface )
{
	return surface->context->frame.timeInSeconds;
}

ImuiSize imuiSurfaceGetSize( const ImuiSurface* surface )
{
	return surface->size;
}

ImuiRect imuiSurfaceGetRect( const ImuiSurface* surface )
{
	return imuiRectCreateSize( 0.0f, 0.0f, surface->size );
}

const ImuiInputState* imuiSurfaceGetInput( const ImuiSurface* surface )
{
	return surface->input;
}

float imuiSurfaceGetDpiScale( const ImuiSurface* surface )
{
	return surface->dpiScale;
}

ImuiWindow* imuiWindowBegin( ImuiSurface* surface, const char* name, ImuiRect rect, uint32_t zOrder )
{
	return imuiWindowBeginId( surface, name, (ImuiId)imuiHashCreate( name, strlen( name ) ), rect, zOrder );
}

ImuiWindow* imuiWindowBeginId( ImuiSurface* surface, const char* name, ImuiId id, ImuiRect rect, uint32_t zOrder )
{
	IMUI_ASSERT( surface );

	ImuiContext* imui = surface->context;
	const ImuiStringView nameView = imuiStringViewCreate( name );

	ImuiWindow* window = NULL;
	for( uintsize windowIndex = 0u; windowIndex < surface->windowCount; ++windowIndex )
	{
		if( surface->windows[ windowIndex ].id != id )
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

	window->inUse						= true;
	window->context						= imui;
	window->surface						= surface;
	window->id							= id;
	window->name						= imuiStringPoolAdd( &imui->strings, nameView );
	window->rect						= rect;
	window->zOrder						= zOrder;
	window->drawIndex					= imuiDrawRegisterWindow( &imui->draw, window->name, surface->drawIndex, zOrder );

	ImuiWidget* rootWidget = imuiWidgetAlloc( imui );
	rootWidget->window		= window;
	rootWidget->name		= window->name;
	rootWidget->hash		= imuiHashString( window->name );
	rootWidget->minSize		= rect.size;
	rootWidget->maxSize		= rect.size;
	rootWidget->rect		= rect;
	rootWidget->clipRect	= rect;

	window->lastFrameRootWidget		= window->rootWidget;
	window->lastFrameCurrentWidget	= window->rootWidget;
	window->lastFrameFocusWidget	= window->focusWidget;

	if( window->lastFrameCurrentWidget )
	{
		rootWidget->lastFrameWidget		= window->lastFrameRootWidget;
		rootWidget->layoutContext		= window->lastFrameRootWidget->layoutContext;
	}

	window->rootWidget		= rootWidget;
	window->currentWidget	= window->rootWidget;
	window->focusWidget		= NULL;
	window->lastFocusIndex	= 0;
	return window;
}

void imuiWindowEnd( ImuiWindow* window )
{
	imuiWidgetEnd( window->rootWidget );
	imuiWindowLayout( window );
}

ImuiContext* imuiWindowGetContext( const ImuiWindow* window )
{
	return window->surface->context;
}

ImuiSurface* imuiWindowGetSurface( const ImuiWindow* window )
{
	return window->surface;
}

const ImuiInputState* imuiWindowGetInput( const ImuiWindow* window )
{
	return window->surface->input;
}

double imuiWindowGetTime( const ImuiWindow* window )
{
	return window->context->frame.timeInSeconds;
}

float imuiWindowGetDpiScale( const ImuiWindow* window )
{
	return window->surface->dpiScale;
}

ImuiRect imuiWindowGetRect( const ImuiWindow* window )
{
	return window->rect;
}

bool imuiWindowHasFocus( const ImuiWindow* window )
{
	return window->hasFocus;
}

void imuiWindowSetFocus( ImuiWindow* window, float angleThreshold, bool wrap )
{
	window->hasFocus			= true;
	window->focusWrap			= wrap;
	window->focusAngleThreshold	= 1.0f - angleThreshold;
}

bool imuiWindowIsWidgetFocusLocked( const ImuiWindow* window )
{
	return window->focusLocked;
}

void imuiWindowSetWidgetFocusLock( ImuiWindow* window, bool locked )
{
	window->focusLocked = locked;
}

void imuiWindowClearWidgetFocus( ImuiWindow* window )
{
	window->focusWidget = NULL;
}

const ImuiWidget* imuiWindowGetFocusWidget( const ImuiWindow* window )
{
	if( window->focusWidget )
	{
		return window->focusWidget;
	}

	return window->lastFrameFocusWidget;
}

const ImuiWidget* imuiWindowPeekFocusWidget( const ImuiWindow* window )
{
	return window->closesFocusWidget;
}

void* imuiWindowAllocState( ImuiWindow* window, size_t size, ImuiId stateId )
{
	return imuiWidgetAllocState( window->rootWidget, size, stateId );
}

void* imuiWindowAllocStateNew( ImuiWindow* window, size_t size, ImuiId stateId, bool* isNew )
{
	return imuiWidgetAllocStateNew( window->rootWidget, size, stateId, isNew );
}

void* imuiWindowAllocStateNewDestruct( ImuiWindow* window, size_t size, ImuiId stateId, bool* isNew, ImuiStateDestructFunc destructFunc )
{
	return imuiWidgetAllocStateNewDestruct( window->rootWidget, size, stateId, isNew, destructFunc );
}

ImuiWidget* imuiWindowGetFirstChild( const ImuiWindow* window )
{
	return window->rootWidget->firstChild;
}

ImuiWidget* imuiWindowGetLastChild( const ImuiWindow* window )
{
	return window->rootWidget->lastChild;
}

static void imuiWindowLayout( ImuiWindow* window )
{
	const bool update = true; // !window->rootWidget->lastFrameWidget || (window->rootWidget->hash != window->rootWidget->lastFrameWidget->hash);

	window->hasFocus &= !window->focusLocked;

	if( window->hasFocus )
	{
		window->diagonalLength				= sqrtf( (window->rect.size.width * window->rect.size.width) + (window->rect.size.height * window->rect.size.height) );

		window->closesFocusWidgetFactor		= 0.0f;
		window->closesFocusWidget			= NULL;
		window->wrapFocusWidgetFactor		= 0.0f;
		window->wrapFocusWidget				= NULL;

		window->closesFocusIndexWidget		= NULL;
		window->wrapFocusIndexWidget		= NULL;

		if( window->focusWidget )
		{
			window->focusPoint = imuiRectGetCenter( window->focusWidget->rect );
		}
		else
		{
			window->focusPoint = window->rect.pos;
		}

		const ImuiPos direction = imuiInputGetDirection( window->surface->input );
		if( window->focusWidget && window->focusWrap && (direction.x != 0.0f || direction.y != 0.0f) )
		{
			const ImuiPos dirStart = window->focusPoint;

			ImuiPos dirEnd = imuiInputGetDirection( window->surface->input );
			dirEnd.x *= -1.0f * window->diagonalLength;
			dirEnd.y *= -1.0f * window->diagonalLength;

			const ImuiPos windowTopLeft		= imuiRectGetTopLeft( window->rect );
			const ImuiPos windowTopRight	= imuiRectGetTopRight( window->rect );
			const ImuiPos windowBottomLeft	= imuiRectGetBottomLeft( window->rect );
			const ImuiPos windowBottomRight	= imuiRectGetBottomRight( window->rect );

			const ImuiPos lineStart[]		= { windowTopLeft, windowTopLeft, windowBottomRight, windowBottomRight };
			const ImuiPos lineEnd[]			= { windowTopRight, windowBottomLeft, windowTopRight, windowBottomLeft };

			for( uintsize i = 0; i < IMUI_ARRAY_COUNT( lineStart ); ++i )
			{
				const ImuiPos rectStart = lineStart[ i ];
				const ImuiPos rectEnd = lineEnd[ i ];

				const float uA = ((rectEnd.x - rectStart.x) * (dirStart.y - rectStart.y) - (rectEnd.y - rectStart.y) * (dirStart.x - rectStart.x)) / ((rectEnd.y - rectStart.y) * (dirEnd.x - dirStart.x) - (rectEnd.x-rectStart.x) * (dirEnd.y - dirStart.y));
				const float uB = ((dirEnd.x - dirStart.x) * (dirStart.y - rectStart.y) - (dirEnd.y - dirStart.y) * (dirStart.x - rectStart.x)) / ((rectEnd.y - rectStart.y) * (dirEnd.x - dirStart.x) - (rectEnd.x - rectStart.x) * (dirEnd.y - dirStart.y));

				if( uA >= 0 && uA <= 1 && uB >= 0 && uB <= 1 )
				{

					const float intersectionX = dirStart.x + (uA * (dirEnd.x-dirStart.x));
					const float intersectionY = dirStart.y + (uA * (dirEnd.y-dirStart.y));

					window->focusWrapPoint = imuiPosCreate( intersectionX, intersectionY );
					break;
				}
			}
		}
	}

	uintsize childIndex = 0u;
	for( ImuiWidget* widget = window->rootWidget->firstChild; widget != NULL; widget = widget->nextSibling )
	{
		imuiWidgetUpdateLayoutContext( widget, childIndex, window->surface->dpiScale, update );
		childIndex++;
	}

	childIndex = 0u;
	for( ImuiWidget* widget = window->rootWidget->firstChild; widget != NULL; widget = widget->nextSibling )
	{
		imuiWidgetLayout( widget, &window->rootWidget->rect, window->surface->dpiScale, childIndex, update );
		childIndex++;
	}

	const ImuiInputShortcut shortcut = imuiInputGetShortcut( window->surface->input );
	if( window->surface->input->current.focusExecute )
	{
		if( window->closesFocusWidget )
		{
			window->focusWidget = window->closesFocusWidget;
		}
		else if( window->focusWrap && window->wrapFocusWidget )
		{
			window->focusWidget = window->wrapFocusWidget;
		}
	}
	else if( shortcut == ImuiInputShortcut_FocusNext ||
			 shortcut == ImuiInputShortcut_FocusPrevious )
	{
		IMUI_ASSERT( !window->focusWidget || window->focusWidget != window->closesFocusIndexWidget );
		window->focusWidget = window->closesFocusIndexWidget ? window->closesFocusIndexWidget : window->wrapFocusIndexWidget;
	}
}

static ImuiWidget* imuiWidgetAlloc( ImuiContext* imui )
{
	if( imui->firstChunk == NULL ||
		imui->firstChunk->usedCount == IMUI_DEFAULT_WIDGET_CHUNK_SIZE )
	{
		if( imui->firstFreeChunk )
		{
			ImuiWidgetChunk* newChunk = imui->firstFreeChunk;
			IMUI_ASSERT( newChunk->usedCount == 0u );

			imui->firstFreeChunk = newChunk->nextChunk;

			newChunk->nextChunk	= imui->firstChunk;
			imui->firstChunk = newChunk;
		}
		else
		{
			ImuiWidgetChunk* newChunk = IMUI_MEMORY_NEW( &imui->allocator, ImuiWidgetChunk );
			if( newChunk == NULL )
			{
				return NULL;
			}

			newChunk->nextChunk	= imui->firstChunk;
			newChunk->usedCount	= 0u;

			imui->firstChunk = newChunk;
		}
	}

	ImuiWidget* widget = &imui->firstChunk->data[ imui->firstChunk->usedCount ];
	imui->firstChunk->usedCount++;

	*widget = IMUI_DEFAULT_WIDGET;

	return widget;
}

static void imuiWidgetUpdateLayoutContext( ImuiWidget* widget, uintsize widgetIndex, float dpiScale, bool update )
{
	if( !update && widget->lastFrameWidget && widget->hash == widget->lastFrameWidget->hash )
	{
		uintsize childIndex = 0u;
		for( ImuiWidget* childWidget = widget->firstChild; childWidget != NULL; childWidget = childWidget->nextSibling )
		{
			imuiWidgetUpdateLayoutContext( childWidget, childIndex, dpiScale, false );
			childIndex++;
		}
		return;
	}
	ImuiLayoutContext* context			= &widget->layoutContext;
	ImuiLayoutContext* parentContext	= &widget->parent->layoutContext;

	*context = IMUI_DEFAULT_LAYOUT_CONTEXT;

	if( widget->layout == ImuiLayout_Grid )
	{
		imuiWidgetUpdateLayoutContextCheckGridContextSize( widget );
	}

	parentContext->childrenStretch.width		+= widget->stretchH;
	parentContext->childrenStretch.height		+= widget->stretchV;
	parentContext->childrenMaxStretch.width		= IMUI_MAX( parentContext->childrenMaxStretch.width, widget->stretchH );
	parentContext->childrenMaxStretch.height	= IMUI_MAX( parentContext->childrenMaxStretch.height, widget->stretchV );

	{
		uintsize childIndex = 0u;
		for( ImuiWidget* childWidget = widget->firstChild; childWidget != NULL; childWidget = childWidget->nextSibling )
		{
			imuiWidgetUpdateLayoutContext( childWidget, childIndex, dpiScale, true );
			childIndex++;
		}
	}

	if( widget->layout == ImuiLayout_Grid )
	{
		imuiWidgetUpdateLayoutContextGrid( widget );
	}

	const ImuiSize marginPaddingSize	= imuiSizeScale( imuiSizeAddSize( imuiBorderGetMinSize( widget->margin ), imuiBorderGetMinSize( widget->padding ) ), dpiScale );
	context->minOuterSize				= imuiSizeAddSize( imuiSizeMax( imuiSizeScale( imuiSizeShrinkBorder( imuiSizeCeil( widget->minSize ), widget->padding ), dpiScale ), context->childrenMinSize ), marginPaddingSize );
	context->childrenStretch			= imuiSizeMax( context->childrenStretch, imuiSizeCreateOne() );

	switch( widget->parent->layout )
	{
	case ImuiLayout_Stack:
		parentContext->childrenMinSize.width	= IMUI_MAX( parentContext->childrenMinSize.width, context->minOuterSize.width );
		parentContext->childrenMinSize.height	= IMUI_MAX( parentContext->childrenMinSize.height, context->minOuterSize.height );
		break;

	case ImuiLayout_Scroll:
		break;

	case ImuiLayout_Horizontal:
		if( widgetIndex > 0u )
		{
			parentContext->childrenMinSize.width	+= widget->parent->layoutData.horizintalVertical.spacing * dpiScale;
		}
		parentContext->childrenMinSize.width		+= context->minOuterSize.width;
		parentContext->childrenMinSize.height		= IMUI_MAX( parentContext->childrenMinSize.height, context->minOuterSize.height );
		break;

	case ImuiLayout_Vertical:
		if( widgetIndex > 0u )
		{
			parentContext->childrenMinSize.height	+= widget->parent->layoutData.horizintalVertical.spacing * dpiScale;
		}
		parentContext->childrenMinSize.width		= IMUI_MAX( parentContext->childrenMinSize.width, context->minOuterSize.width );
		parentContext->childrenMinSize.height		+= context->minOuterSize.height;
		break;

	case ImuiLayout_Grid:
		{
			const uintsize colIndex				= widgetIndex % widget->parent->layoutData.grid.columnCount;
			const uintsize rowIndex				= widgetIndex / widget->parent->layoutData.grid.columnCount;
			ImuiLayoutGridElement* colElement	= &widget->parent->gridContext->columns[ colIndex ];
			ImuiLayoutGridElement* rowElement	= &widget->parent->gridContext->rows[ rowIndex ];

			colElement->childrenMinSize		= IMUI_MAX( colElement->childrenMinSize, context->minOuterSize.width );
			rowElement->childrenMinSize		= IMUI_MAX( rowElement->childrenMinSize, context->minOuterSize.height );

			colElement->childrenMaxStretch	= IMUI_MAX( colElement->childrenMaxStretch, widget->stretchH );
			rowElement->childrenMaxStretch	= IMUI_MAX( rowElement->childrenMaxStretch, widget->stretchV );
		}
		break;
	}
}

static void imuiWidgetUpdateLayoutContextCheckGridContextSize( ImuiWidget* widget )
{
	ImuiContext* imui = widget->window->context;

	const uintsize colCount = widget->layoutData.grid.columnCount;
	const uintsize rowCount = (widget->childCount + colCount - 1u) / colCount;
	if( !widget->gridContext ||
		widget->gridContext->columnCount != colCount ||
		widget->gridContext->rowCount != rowCount )
	{
		const uintsize contextSize = sizeof( ImuiLayoutGridContext ) + (sizeof( ImuiLayoutGridElement ) * (widget->layoutData.grid.columnCount + rowCount));
		ImuiLayoutGridContext* gridContext = (ImuiLayoutGridContext*)imuiMemoryAllocZero( &widget->window->context->allocator, contextSize );

		gridContext->columns		= (ImuiLayoutGridElement*)&gridContext[ 1u ];
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
		ImuiLayoutGridContext* gridContext = widget->gridContext;

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

static void imuiWidgetUpdateLayoutContextGrid( ImuiWidget* widget )
{
	ImuiLayoutContext* context = &widget->layoutContext;
	ImuiLayoutGridContext* gridContext = widget->gridContext;

	context->childrenMaxStretch.width	= 0.0f;
	context->childrenMaxStretch.height	= 0.0f;

	for( uintsize col = 0u; col < gridContext->columnCount; ++col )
	{
		ImuiLayoutGridElement* colElement = &gridContext->columns[ col ];

		context->childrenMaxStretch.width += colElement->childrenMaxStretch;
		context->childrenMinSize.width += colElement->childrenMinSize;
	}
	context->childrenMinSize.width += widget->layoutData.grid.colSpacing * (gridContext->columnCount - 1u);

	for( uintsize row = 0u; row < gridContext->rowCount; ++row )
	{
		ImuiLayoutGridElement* rowElement = &gridContext->rows[ row ];

		context->childrenMaxStretch.height += rowElement->childrenMaxStretch;
		context->childrenMinSize.height += rowElement->childrenMinSize;
	}
	context->childrenMinSize.height += widget->layoutData.grid.rowSpacing * (gridContext->rowCount - 1u);

	context->childrenMaxStretch.width	= IMUI_MAX( 1.0f, context->childrenMaxStretch.width );
	context->childrenMaxStretch.height	= IMUI_MAX( 1.0f, context->childrenMaxStretch.height );
}

static void imuiWidgetLayout( ImuiWidget* widget, const ImuiRect* parentInnerRect, float dpiScale, uintsize widgetIndex, bool update )
{
	if( !update && widget->lastFrameWidget && widget->hash == widget->lastFrameWidget->hash )
	{
		const ImuiRect innerRect = imuiRectShrinkBorder( widget->rect, imuiBorderScale( widget->padding, dpiScale ) );
		for( ImuiWidget* childWidget = widget->firstChild; childWidget != NULL; childWidget = childWidget->nextSibling )
		{
			imuiWidgetLayout( childWidget, &innerRect, dpiScale, widgetIndex, update );
		}
		return;
	}

	switch( widget->parent->layout )
	{
	case ImuiLayout_Stack:
		imuiWidgetLayoutStack( widget, parentInnerRect, dpiScale );
		break;

	case ImuiLayout_Scroll:
		imuiWidgetLayoutScroll( widget, parentInnerRect, dpiScale );
		break;

	case ImuiLayout_Horizontal:
		imuiWidgetLayoutHorizontal( widget, parentInnerRect, dpiScale );
		break;

	case ImuiLayout_Vertical:
		imuiWidgetLayoutVertical( widget, parentInnerRect, dpiScale );
		break;

	case ImuiLayout_Grid:
		imuiWidgetLayoutGrid( widget, parentInnerRect, dpiScale, widgetIndex );
		break;
	}

	widget->clipRect = imuiRectIntersection( widget->rect, widget->parent->clipRect );

	const ImuiRect innerRect = imuiRectShrinkBorder( widget->rect, imuiBorderScale( widget->padding, dpiScale ) );
	if( widget->layout == ImuiLayout_Horizontal )
	{
		imuiWidgetLayoutHorizontalCollectStrecher( widget, &innerRect, dpiScale );
	}
	else if( widget->layout == ImuiLayout_Vertical )
	{
		imuiWidgetLayoutVerticalCollectStrecher( widget, &innerRect, dpiScale );
	}
	else if( widget->layout == ImuiLayout_Grid )
	{
		imuiWidgetLayoutPrepareGrid( widget, &innerRect, dpiScale );
	}

	uintsize childIndex = 0u;
	for( ImuiWidget* childWidget = widget->firstChild; childWidget != NULL; childWidget = childWidget->nextSibling )
	{
		imuiWidgetLayout( childWidget, &innerRect, dpiScale, childIndex, true );
		childIndex++;
	}

	ImuiWindow* window = widget->window;
	if( widget->canHaveFocus && window->hasFocus && widget != window->focusWidget )
	{
		const ImuiPos focusDirection	= imuiInputGetDirection( window->surface->input );
		const ImuiPos center			= imuiRectGetCenter( widget->rect );

		const ImuiPos distance			= imuiPosSubPos( center, window->focusPoint );
		const float distanceLength		= sqrtf( (distance.x * distance.x) + (distance.y * distance.y) );
		const ImuiPos direction			= imuiPosScale( distance, 1.0f / distanceLength );

		const float angleFactor			= (direction.x * focusDirection.x) + (direction.y * focusDirection.y);
		const float distanceFactor		= 1.0f - (distanceLength / window->diagonalLength);
		const float focusFactor			= (angleFactor * angleFactor) + (distanceFactor * distanceFactor);

		if( angleFactor > window->focusAngleThreshold && focusFactor > window->closesFocusWidgetFactor )
		{
			window->closesFocusWidgetFactor		= focusFactor;
			window->closesFocusWidget			= widget;
		}
		else
		{
			const ImuiPos wrapDistance		= imuiPosSubPos( center, window->focusWrapPoint );
			const float wrapDistanceLength	= sqrtf( (wrapDistance.x * wrapDistance.x) + (wrapDistance.y * wrapDistance.y) );
			const ImuiPos wrapDirection		= imuiPosScale( wrapDistance, 1.0f / wrapDistanceLength );

			const float wrapAngleFactor		= (wrapDirection.x * focusDirection.x) + (wrapDirection.y * focusDirection.y);
			const float wrapDistanceFactor	= 1.0f - (wrapDistanceLength / window->diagonalLength);
			const float wrapFocusFactor		= (wrapAngleFactor * wrapAngleFactor) + (wrapDistanceFactor * wrapDistanceFactor);

			if( wrapAngleFactor > window->focusAngleThreshold && wrapFocusFactor > window->wrapFocusWidgetFactor )
			{
				window->wrapFocusWidgetFactor	= wrapFocusFactor;
				window->wrapFocusWidget			= widget;
			}
		}

		const ImuiInputShortcut shortcut = imuiInputGetShortcut( window->surface->input );
		if( shortcut == ImuiInputShortcut_FocusNext )
		{
			const uint32 currentFocusIndex = window->focusWidget ? window->focusWidget->focusIndex : 0;
			const uint32 closesFocusIndex = window->closesFocusIndexWidget ? window->closesFocusIndexWidget->focusIndex : 0xffffffffu;

			if( widget->focusIndex > currentFocusIndex && widget->focusIndex < closesFocusIndex )
			{
				window->closesFocusIndexWidget = widget;
			}

			if( !window->wrapFocusIndexWidget || widget->focusIndex < window->wrapFocusIndexWidget->focusIndex )
			{
				window->wrapFocusIndexWidget = widget;
			}
		}
		else if( shortcut == ImuiInputShortcut_FocusPrevious )
		{
			const uint32 currentFocusIndex = window->focusWidget ? window->focusWidget->focusIndex : 0;
			const uint32 closesFocusIndex = window->closesFocusIndexWidget ? window->closesFocusIndexWidget->focusIndex : 0;

			if( widget->focusIndex < currentFocusIndex && widget->focusIndex > closesFocusIndex )
			{
				window->closesFocusIndexWidget = widget;
			}

			if( !window->wrapFocusIndexWidget || widget->focusIndex > window->wrapFocusIndexWidget->focusIndex )
			{
				window->wrapFocusIndexWidget = widget;
			}
		}
	}
}

static void imuiWidgetLayoutPrepareGrid( ImuiWidget* widget, const ImuiRect* parentInnerRect, float dpiScale )
{
	const ImuiLayoutContext* context = &widget->layoutContext;
	ImuiLayoutGridContext* gridContext = widget->gridContext;

	const float maxFreeWidth		= parentInnerRect->size.width - context->childrenMinSize.width;
	const float maxFreeHeight		= parentInnerRect->size.height - context->childrenMinSize.height;

	float pos = parentInnerRect->pos.x;
	for( uintsize col = 0u; col < gridContext->columnCount; ++col )
	{
		ImuiLayoutGridElement* colElement = &gridContext->columns[ col ];

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
		ImuiLayoutGridElement* rowElement = &gridContext->rows[ row ];

		rowElement->pos = pos;

		const float strechSize	= ((maxFreeHeight / context->childrenMaxStretch.height) * rowElement->childrenMaxStretch) + rowElement->childrenMinSize;
		const float size		= IMUI_MAX( strechSize, rowElement->childrenMinSize );
		pos += size;
		rowElement->size = size;
		pos += widget->layoutData.grid.rowSpacing * dpiScale;
	}
}

static void imuiWidgetLayoutStack( ImuiWidget* widget, const ImuiRect* parentInnerRect, float dpiScale )
{
	const float factorWidth			= IMUI_MIN( widget->stretchH, widget->parent->layoutContext.childrenMaxStretch.width );
	const float factorHeight		= IMUI_MIN( widget->stretchV, widget->parent->layoutContext.childrenMaxStretch.height );
	const ImuiSize minSize			= imuiWidgetLayoutMinSize( widget, parentInnerRect, dpiScale );
	const ImuiSize maxSize			= parentInnerRect->size;
	ImuiSize size					= imuiWidgetCalculateSize( widget, minSize, maxSize, factorWidth, factorHeight, dpiScale );

	ImuiPos pos;
	pos.x = imuiWidgetLayoutPositionX( widget, parentInnerRect, size.width, dpiScale );
	pos.y = imuiWidgetLayoutPositionY( widget, parentInnerRect, size.height, dpiScale );

	imuiWidgetLayoutRect( widget, pos, size );
}

static void imuiWidgetLayoutScroll( ImuiWidget* widget, const ImuiRect* parentInnerRect, float dpiScale )
{
	const float factorWidth			= IMUI_MIN( widget->stretchH, widget->parent->layoutContext.childrenMaxStretch.width );
	const float factorHeight		= IMUI_MIN( widget->stretchV, widget->parent->layoutContext.childrenMaxStretch.height );
	const ImuiSize minSize			= imuiWidgetLayoutMinSize( widget, parentInnerRect, dpiScale );
	const ImuiSize maxSize			= imuiSizeMax( parentInnerRect->size, minSize );
	ImuiSize size					= imuiWidgetCalculateSize( widget, minSize, maxSize, factorWidth, factorHeight, dpiScale );

	ImuiPos pos;
	pos.x = imuiWidgetLayoutPositionX( widget, parentInnerRect, size.width, dpiScale );
	pos.y = imuiWidgetLayoutPositionY( widget, parentInnerRect, size.height, dpiScale );
	pos.x -= widget->parent->layoutData.scroll.offset.x;
	pos.y -= widget->parent->layoutData.scroll.offset.y;

	imuiWidgetLayoutRect( widget, pos, size );
}

static void imuiWidgetLayoutHorizontalCollectStrecher( ImuiWidget* widget, const ImuiRect* innerRect, float dpiScale )
{
	ImuiLayoutContext* context = &widget->layoutContext;

	const float extraChildrenWidth	= ((widget->childCount - 1) * widget->layoutData.horizintalVertical.spacing);
	const float maxChildrenWidth	= innerRect->size.width - (extraChildrenWidth * dpiScale);

	for( ImuiWidget* childWidget = widget->firstChild; childWidget != NULL; childWidget = childWidget->nextSibling )
	{
		const float factorWidth = context->childrenStretch.width ? childWidget->stretchH / context->childrenStretch.width : 0.0f;

		if( childWidget != widget->firstChild )
		{
			context->childrenStretchMinSize.width += widget->layoutData.horizintalVertical.spacing * dpiScale;
		}

		if( childWidget->layoutContext.minOuterSize.width > maxChildrenWidth * factorWidth )
		{
			context->childrenStretchMinSize.width += childWidget->layoutContext.minOuterSize.width;
		}
		else
		{
			context->childrenStretchFinal.width += childWidget->stretchH;
		}
	}
}

static void imuiWidgetLayoutHorizontal( ImuiWidget* widget, const ImuiRect* parentInnerRect, float dpiScale )
{
	const ImuiLayoutContext* parentContext = &widget->parent->layoutContext;

	const float factorWidth			= parentContext->childrenStretch.width ? widget->stretchH / parentContext->childrenStretch.width : 0.0f;
	const float factorHeight		= parentContext->childrenMaxStretch.height ? widget->stretchV / parentContext->childrenMaxStretch.height : 0.0f;
	ImuiSize minSize				= imuiWidgetLayoutMinSize( widget, parentInnerRect, dpiScale );

	const float extraChildrenWidth	= ((widget->parent->childCount - 1) * widget->parent->layoutData.horizintalVertical.spacing);
	const float maxChildrenWidth	= parentInnerRect->size.width - (extraChildrenWidth * dpiScale);
	const float factorStretchWidth	= parentContext->childrenStretchFinal.width ? widget->stretchH / parentContext->childrenStretchFinal.width : 0.0f;
	const float freeWidth			= parentInnerRect->size.width - parentContext->childrenStretchMinSize.width;
	const ImuiSize maxSize			= imuiSizeMin( imuiSizeScale( imuiSizeExpandBorder( widget->maxSize, widget->margin ), dpiScale ), imuiSizeCreate( minSize.width, parentInnerRect->size.height ) );

	if( widget->layoutContext.minOuterSize.width < maxChildrenWidth * factorWidth )
	{
		minSize.width				= IMUI_MAX( minSize.width, freeWidth * factorStretchWidth );
	}

	const ImuiSize size				= imuiWidgetCalculateSize( widget, minSize, maxSize, factorWidth, factorHeight, dpiScale );

	ImuiPos pos;
	if( widget->prevSibling )
	{
		pos.x = widget->prevSibling->rect.pos.x + widget->prevSibling->rect.size.width + ((widget->prevSibling->margin.right + widget->parent->layoutData.horizintalVertical.spacing + widget->margin.left) * dpiScale);
	}
	else
	{
		pos.x = parentInnerRect->pos.x + (widget->margin.left * dpiScale);
	}
	pos.y = imuiWidgetLayoutPositionY( widget, parentInnerRect, size.height, dpiScale );

	imuiWidgetLayoutRect( widget, pos, size );
}

static void imuiWidgetLayoutVerticalCollectStrecher( ImuiWidget* widget, const ImuiRect* innerRect, float dpiScale )
{
	ImuiLayoutContext* context = &widget->layoutContext;

	const float extraChildrenHeight	= ((widget->childCount - 1) * widget->layoutData.horizintalVertical.spacing);
	const float maxChildrenHeight	= innerRect->size.height - (extraChildrenHeight * dpiScale);

	for( ImuiWidget* childWidget = widget->firstChild; childWidget != NULL; childWidget = childWidget->nextSibling )
	{
		const float factorHeight = context->childrenStretch.height ? childWidget->stretchV / context->childrenStretch.height : 0.0f;

		if( childWidget != widget->firstChild )
		{
			context->childrenStretchMinSize.height += widget->layoutData.horizintalVertical.spacing * dpiScale;
		}

		if( childWidget->layoutContext.minOuterSize.height > maxChildrenHeight * factorHeight )
		{
			context->childrenStretchMinSize.height += childWidget->layoutContext.minOuterSize.height;
		}
		else
		{
			context->childrenStretchFinal.height += childWidget->stretchV;
		}
	}
}

static void imuiWidgetLayoutVertical( ImuiWidget* widget, const ImuiRect* parentInnerRect, float dpiScale )
{
	const ImuiLayoutContext* parentContext = &widget->parent->layoutContext;

	const float factorWidth			= parentContext->childrenMaxStretch.width ? widget->stretchH / parentContext->childrenMaxStretch.width : 0.0f;
	const float factorHeight		= parentContext->childrenStretch.height ? widget->stretchV / parentContext->childrenStretch.height : 0.0f;
	ImuiSize minSize				= imuiWidgetLayoutMinSize( widget, parentInnerRect, dpiScale );

	const float extraChildrenHeight	= ((widget->parent->childCount - 1) * widget->parent->layoutData.horizintalVertical.spacing);
	const float maxChildrenHeight	= parentInnerRect->size.height - (extraChildrenHeight * dpiScale);
	const float factorStretchHeight	= parentContext->childrenStretchFinal.height ? widget->stretchV / parentContext->childrenStretchFinal.height : 0.0f;
	const float freeHeight			= parentInnerRect->size.height - parentContext->childrenStretchMinSize.height;
	const ImuiSize maxSize			= imuiSizeMin( imuiSizeScale( imuiSizeExpandBorder( widget->maxSize, widget->margin ), dpiScale ), imuiSizeCreate( parentInnerRect->size.width, minSize.height ) );

	if( widget->layoutContext.minOuterSize.height < maxChildrenHeight * factorHeight )
	{
		minSize.height				= IMUI_MAX( minSize.height, freeHeight * factorStretchHeight );
	}

	const ImuiSize size				= imuiWidgetCalculateSize( widget, minSize, maxSize, factorWidth, factorHeight, dpiScale );

	ImuiPos pos;
	pos.x = imuiWidgetLayoutPositionX( widget, parentInnerRect, size.width, dpiScale );
	if( widget->prevSibling )
	{
		pos.y = widget->prevSibling->rect.pos.y + widget->prevSibling->rect.size.height + ((widget->prevSibling->margin.bottom + widget->parent->layoutData.horizintalVertical.spacing + widget->margin.top) * dpiScale);
	}
	else
	{
		pos.y = parentInnerRect->pos.y + (widget->margin.top * dpiScale);
	}

	imuiWidgetLayoutRect( widget, pos, size );
}

static void imuiWidgetLayoutGrid( ImuiWidget* widget, const ImuiRect* parentInnerRect, float dpiScale, uintsize widgetIndex )
{
	const uintsize colIndex				= widgetIndex % widget->parent->layoutData.grid.columnCount;
	const uintsize rowIndex				= widgetIndex / widget->parent->layoutData.grid.columnCount;
	ImuiLayoutGridElement* colElement	= &widget->parent->gridContext->columns[ colIndex ];
	ImuiLayoutGridElement* rowElement	= &widget->parent->gridContext->rows[ rowIndex ];

	const float factorWidth			= colElement->childrenMaxStretch ? widget->stretchH / colElement->childrenMaxStretch : 0.0f;
	const float factorHeight		= rowElement->childrenMaxStretch ? widget->stretchV / rowElement->childrenMaxStretch : 0.0f;
	const ImuiSize minSize			= imuiWidgetLayoutMinSize( widget, parentInnerRect, dpiScale );
	const ImuiSize maxSize			= imuiSizeMin( imuiSizeScale( imuiSizeExpandBorder( widget->maxSize, widget->margin ), dpiScale ), imuiSizeCreate( colElement->size, rowElement->size ) );
	ImuiSize size					= imuiWidgetCalculateSize( widget, minSize, maxSize, factorWidth, factorHeight, dpiScale );
	const ImuiRect cellInnerRect	= imuiRectCreate( colElement->pos, rowElement->pos, colElement->size, rowElement->size );

	ImuiPos pos;
	pos.x = imuiWidgetLayoutPositionX( widget, &cellInnerRect, size.width, dpiScale );
	pos.y = imuiWidgetLayoutPositionY( widget, &cellInnerRect, size.height, dpiScale );

	imuiWidgetLayoutRect( widget, pos, size );
}

static ImuiSize imuiWidgetLayoutMinSize( ImuiWidget* widget, const ImuiRect* parentInnerRect, float dpiScale )
{
	(void)parentInnerRect;
	(void)dpiScale;

	//const imuiBorder margin		= imuiBorderScale( widget->margin, dpiScale );
	//const imuiBorder padding	= imuiBorderScale( widget->padding, dpiScale );
	//const imuiSize minChildren	= imuiSizeExpandBorder( imuiSizeExpandBorder( widget->layoutContext.childrenMinSize, padding ), margin );
	const ImuiSize minSize		= widget->layoutContext.minOuterSize; // imuiSizeMax( , minChildren );
	return minSize; // imuiSizeMin( parentInnerRect->size, minSize );
}

static ImuiSize imuiWidgetCalculateSize( ImuiWidget* widget, ImuiSize minSize, ImuiSize maxSize, float factorWidth, float factorHeight, float dpiScale )
{
	ImuiSize size = imuiSizeLerp2( minSize, maxSize, factorWidth, factorHeight );
	size = imuiSizeMax( size, minSize );
	size = imuiSizeShrinkBorder( size, imuiBorderScale( widget->margin, dpiScale ) );

	return size;
}

static float imuiWidgetLayoutPositionX( ImuiWidget* widget, const ImuiRect* parentInnerRect, float width, float dpiScale )
{
	const float remainingWidth = parentInnerRect->size.width - (width + ((widget->margin.left + widget->margin.right) * dpiScale));
	return parentInnerRect->pos.x + (widget->margin.left * dpiScale) + (remainingWidth * widget->alignH);
}

static float imuiWidgetLayoutPositionY( ImuiWidget* widget, const ImuiRect* parentInnerRect, float height, float dpiScale )
{
	const float remainingHeight = parentInnerRect->size.height - (height + ((widget->margin.top + widget->margin.bottom) * dpiScale));
	return parentInnerRect->pos.y + (widget->margin.top * dpiScale) + (remainingHeight * widget->alignV);
}

static void imuiWidgetLayoutRect( ImuiWidget* widget, ImuiPos pos, ImuiSize size )
{
	//widget->rect.pos = pos;
	//widget->rect.size = size;
	// ???
	widget->rect.pos.x			= floorf( pos.x );
	widget->rect.pos.y			= floorf( pos.y );
	widget->rect.size.width		= floorf( size.width );
	widget->rect.size.height	= floorf( size.height );

	IMUI_ASSERT( widget->rect.size.width >= 0.0f );
	IMUI_ASSERT( widget->rect.size.height >= 0.0f );
}

ImuiWidget* imuiWidgetBegin( ImuiWindow* window )
{
	IMUI_ASSERT( window );
	return imuiWidgetBeginId( window, IMUI_ID_DEFAULT );
}

ImuiWidget* imuiWidgetBeginId( ImuiWindow* window, ImuiId id )
{
	IMUI_ASSERT( window );

	ImuiWidget* widget = imuiWidgetAlloc( window->context );
	if( widget == NULL )
	{
		return NULL;
	}

	ImuiWidget* parent = window->currentWidget;
	if( id == IMUI_ID_DEFAULT )
	{
		id = parent->id + 1;
		if( parent->lastChild )
		{
			id += parent->lastChild->id + 1;
		}
	}

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
		ImuiWidget* lastFrameWidget = window->lastFrameCurrentWidget->firstChild;
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

			if( lastFrameWidget == window->lastFrameFocusWidget )
			{
				window->focusWidget = widget;
			}
		}
	}

	return widget;
}

ImuiWidget* imuiWidgetBeginNamed( ImuiWindow* window, const char* name )
{
	const ImuiStringView nameView = imuiStringViewCreate( name );

	ImuiWidget* widget = imuiWidgetBeginId( window, imuiHashString( nameView ) );
	if( widget == NULL )
	{
		return NULL;
	}

	widget->name = imuiStringPoolAdd( &window->context->strings, nameView );

	return widget;
}

void imuiWidgetEnd( ImuiWidget* widget )
{
	IMUI_ASSERT( widget == widget->window->currentWidget );

	widget->hash = imuiHashMix( widget->hash, imuiHashCreate( &widget->id, IMUI_OFFSETOF( ImuiWidget, rect ) - IMUI_OFFSETOF( ImuiWidget, id ) ) );

	//IMUI_ASSERT( !widget->lastFrameWidget || widget->hash == widget->lastFrameWidget->hash );

	if( widget->parent )
	{
		widget->parent->hash = imuiHashMix( widget->parent->hash, widget->hash );
	}

	widget->window->currentWidget = widget->parent;
	if( widget->lastFrameWidget &&
		widget->window->lastFrameCurrentWidget )
	{
		widget->window->lastFrameCurrentWidget = widget->window->lastFrameCurrentWidget->parent;
	}
}

ImuiContext* imuiWidgetGetContext( const ImuiWidget* widget )
{
	return widget->window->context;
}

ImuiSurface* imuiWidgetGetSurface( const ImuiWidget* widget )
{
	return widget->window->surface;
}

const ImuiInputState* imuiWidgetGetInput( const ImuiWidget* widget )
{
	return widget->window->surface->input;
}

ImuiWindow* imuiWidgetGetWindow( const ImuiWidget* widget )
{
	return widget->window;
}

ImuiWidget* imuiWidgetGetParent( const ImuiWidget* widget )
{
	IMUI_ASSERT( widget->parent != widget->window->rootWidget );
	return widget->parent;
}

ImuiWidget* imuiWidgetGetFirstChild( const ImuiWidget* widget )
{
	return widget->firstChild;
}

ImuiWidget* imuiWidgetGetLastChild( const ImuiWidget* widget )
{
	return widget->lastChild;
}

ImuiWidget* imuiWidgetGetPrevSibling( const ImuiWidget* widget )
{
	return widget->prevSibling;
}

ImuiWidget* imuiWidgetGetNextSibling( const ImuiWidget* widget )
{
	return widget->nextSibling;
}

double imuiWidgetGetTime( const ImuiWidget* widget )
{
	return widget->window->context->frame.timeInSeconds;
}

float imuiWidgetGetDpiScale( const ImuiWidget* widget )
{
	return widget->window->surface->dpiScale;
}

void* imuiWidgetAllocState( ImuiWidget* widget, size_t size, ImuiId stateId )
{
	return imuiWidgetAllocStateNewDestruct( widget, size, stateId, NULL, NULL );
}

void* imuiWidgetAllocStateNew( ImuiWidget* widget, size_t size, ImuiId stateId, bool* isNew )
{
	return imuiWidgetAllocStateNewDestruct( widget, size, stateId, isNew, NULL );
}

void* imuiWidgetAllocStateNewDestruct( ImuiWidget* widget, size_t size, ImuiId stateId, bool* isNew, ImuiStateDestructFunc destructFunc )
{
	for( ImuiWidgetState* state = widget->firstState; state; state = state->nextWidgetState )
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

	ImuiContext* imui = widget->window->context;
	if( widget->lastFrameWidget )
	{
		for( ImuiWidgetState* lastFrameState = widget->lastFrameWidget->firstState; lastFrameState; lastFrameState = lastFrameState->nextWidgetState )
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

				lastFrameState->nextUsageState = imui->firstState;
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

				lastFrameState->nextWidgetState = widget->firstState;
				lastFrameState->prevWidgetState = NULL;

				if( widget->firstState )
				{
					widget->firstState->prevWidgetState = lastFrameState;
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

	ImuiWidgetState* newState = (ImuiWidgetState*)imuiMemoryAllocZero( &widget->window->context->allocator, IMUI_OFFSETOF( ImuiWidgetState, data ) + size );
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

ImuiLayout imuiWidgetGetLayout( const ImuiWidget* widget )
{
	return widget->layout;
}

void imuiWidgetSetLayoutStack( ImuiWidget* widget )
{
	widget->layout = ImuiLayout_Stack;
}

void imuiWidgetSetLayoutScroll( ImuiWidget* widget, float offsetX, float offsetY )
{
	widget->layout						= ImuiLayout_Scroll;
	widget->layoutData.scroll.offset	= imuiPosCreate( offsetX, offsetY );
}

void imuiWidgetSetLayoutHorizontal( ImuiWidget* widget )
{
	widget->layout									= ImuiLayout_Horizontal;
	widget->layoutData.horizintalVertical.spacing	= 0.0f;
}

void imuiWidgetSetLayoutHorizontalSpacing( ImuiWidget* widget, float spacing )
{
	IMUI_ASSERT( spacing >= 0.0f );

	widget->layout									= ImuiLayout_Horizontal;
	widget->layoutData.horizintalVertical.spacing	= spacing;
}

void imuiWidgetSetLayoutVertical( ImuiWidget* widget )
{
	widget->layout									= ImuiLayout_Vertical;
	widget->layoutData.horizintalVertical.spacing	= 0.0f;
}

void imuiWidgetSetLayoutVerticalSpacing( ImuiWidget* widget, float spacing )
{
	IMUI_ASSERT( spacing >= 0.0f );

	widget->layout									= ImuiLayout_Vertical;
	widget->layoutData.horizintalVertical.spacing	= spacing;
}

void imuiWidgetSetLayoutGrid( ImuiWidget* widget, uint32_t columnCount, float colSpacing, float rowSpacing )
{
	IMUI_ASSERT( columnCount > 0u );

	widget->layout									= ImuiLayout_Grid;
	widget->layoutData.grid.columnCount				= columnCount;
	widget->layoutData.grid.colSpacing				= colSpacing;
	widget->layoutData.grid.rowSpacing				= rowSpacing;
}

ImuiBorder imuiWidgetGetMargin( const ImuiWidget* widget )
{
	return widget->margin;
}

void imuiWidgetSetMargin( ImuiWidget* widget, ImuiBorder margin )
{
	IMUI_ASSERT( margin.top >= 0.0f && margin.left >= 0.0f && margin.bottom >= 0.0f && margin.right >= 0.0f );
	widget->margin = margin;
}

ImuiBorder imuiWidgetGetPadding( const ImuiWidget* widget )
{
	return widget->padding;
}

void imuiWidgetSetPadding( ImuiWidget* widget, ImuiBorder padding )
{
	IMUI_ASSERT( padding.top >= 0.0f && padding.left >= 0.0f && padding.bottom >= 0.0f && padding.right >= 0.0f );
	widget->padding = padding;
}

ImuiSize imuiWidgetGetMinSize( const ImuiWidget* widget )
{
	return widget->minSize;
}

void imuiWidgetSetMinWidth( ImuiWidget* widget, float value )
{
	IMUI_ASSERT( value >= 0.0f );
	widget->minSize.width = value;
}

void imuiWidgetSetMinHeight( ImuiWidget* widget, float value )
{
	IMUI_ASSERT( value >= 0.0f );
	widget->minSize.height = value;
}

void imuiWidgetSetMinSize( ImuiWidget* widget, ImuiSize size )
{
	IMUI_ASSERT( size.width >= 0.0f && size.height >= 0.0f );
	widget->minSize = size;
}

void imuiWidgetSetMinSizeFloat( ImuiWidget* widget, float width, float height )
{
	IMUI_ASSERT( width >= 0.0f && height >= 0.0f );
	widget->minSize = imuiSizeCreate( width, height );
}

ImuiSize imuiWidgetGetMaxSize( const ImuiWidget* widget )
{
	return widget->maxSize;
}

void imuiWidgetSetMaxWidth( ImuiWidget* widget, float value )
{
	IMUI_ASSERT( value >= 0.0f );
	widget->maxSize.width = value;
}

void imuiWidgetSetMaxHeight( ImuiWidget* widget, float value )
{
	IMUI_ASSERT( value >= 0.0f );
	widget->maxSize.height = value;
}

void imuiWidgetSetMaxSize( ImuiWidget* widget, ImuiSize size )
{
	IMUI_ASSERT( size.width >= 0.0f && size.height >= 0.0f );
	widget->maxSize = size;
}

void imuiWidgetSetMaxSizeFloat( ImuiWidget* widget, float width, float height )
{
	IMUI_ASSERT( width >= 0.0f && height >= 0.0f );
	widget->maxSize = imuiSizeCreate( width, height );
}

void imuiWidgetSetFixedWidth( ImuiWidget* widget, float value )
{
	IMUI_ASSERT( value >= 0.0f );
	widget->minSize.width = value;
	widget->maxSize.width = value;
}

void imuiWidgetSetFixedHeight( ImuiWidget* widget, float value )
{
	IMUI_ASSERT( value >= 0.0f );
	widget->minSize.height = value;
	widget->maxSize.height = value;
}

void imuiWidgetSetFixedSize( ImuiWidget* widget, ImuiSize size )
{
	IMUI_ASSERT( size.width >= 0.0f && size.height >= 0.0f );
	widget->minSize = size;
	widget->maxSize = size;
}

void imuiWidgetSetFixedSizeFloat( ImuiWidget* widget, float width, float height )
{
	IMUI_ASSERT( width >= 0.0f && height >= 0.0f );
	widget->minSize = imuiSizeCreate( width, height );
	widget->maxSize = imuiSizeCreate( width, height );
}

void imuiWidgetSetStretch( ImuiWidget* widget, float horizontal, float vertical )
{
	IMUI_ASSERT( horizontal >= 0.0f && vertical >= 0.0f );
	widget->stretchH = horizontal;
	widget->stretchV = vertical;
}

void imuiWidgetSetStretchOne( ImuiWidget* widget )
{
	widget->stretchH = 1.0f;
	widget->stretchV = 1.0f;
}

float imuiWidgetGetHStretch( const ImuiWidget* widget )
{
	return widget->stretchH;
}

void imuiWidgetSetHStretch( ImuiWidget* widget, float stretch )
{
	IMUI_ASSERT( stretch >= 0.0f );
	widget->stretchH = stretch;
}

float imuiWidgetGetVStretch( const ImuiWidget* widget )
{
	return widget->stretchV;
}

void imuiWidgetSetVStretch( ImuiWidget* widget, float stretch )
{
	IMUI_ASSERT( stretch >= 0.0f );
	widget->stretchV = stretch;
}

void imuiWidgetSetAlign( ImuiWidget* widget, float horizontal, float vertical )
{
	widget->alignH = horizontal;
	widget->alignV = vertical;
}

float imuiWidgetGetHAlign( const ImuiWidget* widget )
{
	return widget->alignH;
}

void imuiWidgetSetHAlign( ImuiWidget* widget, float align )
{
	widget->alignH = align;
}

float imuiWidgetGetVAlign( const ImuiWidget* widget )
{
	return widget->alignV;
}

void imuiWidgetSetVAlign( ImuiWidget* widget, float align )
{
	widget->alignV = align;
}

bool imuiWidgetHasFocus( const ImuiWidget* widget )
{
	return widget->window->hasFocus && widget->window->focusWidget == widget;
}

void imuiWidgetSetFocus( ImuiWidget* widget )
{
	widget->window->focusWidget = widget;
}

bool imuiWidgetGetCanHaveFocus( const ImuiWidget* widget )
{
	return widget->canHaveFocus;
}

void imuiWidgetSetCanHaveFocus( ImuiWidget* widget )
{
	widget->canHaveFocus	= true;
	widget->focusIndex		= ++widget->window->lastFocusIndex;
}

void imuiWidgetSetCanHaveFocusIndex( ImuiWidget* widget, uint32_t focusIndex )
{
	widget->canHaveFocus			= true;
	widget->focusIndex				= focusIndex;

	widget->window->lastFocusIndex	= focusIndex;
}

ImuiPos imuiWidgetGetPos( const ImuiWidget* widget )
{
	return widget->rect.pos;
}

float imuiWidgetGetPosX( const ImuiWidget* widget )
{
	return widget->rect.pos.x;
}

float imuiWidgetGetPosY( const ImuiWidget* widget )
{
	return widget->rect.pos.y;
}

ImuiSize imuiWidgetGetSize( const ImuiWidget* widget )
{
	return widget->rect.size;
}

float imuiWidgetGetSizeWidth( const ImuiWidget* widget )
{
	return widget->rect.size.width;
}

float imuiWidgetGetSizeHeight( const ImuiWidget* widget )
{
	return widget->rect.size.height;
}

ImuiRect imuiWidgetGetRect( const ImuiWidget* widget )
{
	return widget->rect;
}

ImuiSize imuiWidgetGetInnerSize( const ImuiWidget* widget )
{
	return imuiSizeShrinkBorder( widget->rect.size, widget->padding );
}

ImuiRect imuiWidgetGetInnerRect( const ImuiWidget* widget )
{
	return imuiRectShrinkBorder( widget->rect, widget->padding );
}

void imuiWidgetGetInputState( ImuiWidget* widget, ImuiWidgetInputState* target )
{
	ImuiWindow* window = widget->window;
	ImuiSurface* surface = window->surface;
	ImuiContext* imui = window->context;
	const ImuiInputState* input = surface->input;

	bool hasOverlappingWindow = false;
	if( surface->windowCount > 1u )
	{
		for( uintsize i = 0; i < surface->windowCount && !hasOverlappingWindow; ++i )
		{
			const ImuiWindow* testWindow = &surface->windows[ i ];
			if( testWindow == window ||
				testWindow->zOrder < window->zOrder )
			{
				continue;
			}

			hasOverlappingWindow |= imuiRectIncludesPos( testWindow->rect, input->current.mousePos );
		}
	}

	target->relativeMousePos	= imuiPosSubPos( input->current.mousePos, widget->rect.pos );

	target->hasFocus			= window->focusWidget == widget;
	target->isMouseOver			= !hasOverlappingWindow && imuiRectIncludesPos( widget->clipRect, input->current.mousePos );
	target->isMouseDown			= target->isMouseOver && input->current.mouseButtons[ ImuiInputMouseButton_Left ];
	target->hasMousePressed		= target->isMouseOver && imuiInputHasMouseButtonPressed( input, ImuiInputMouseButton_Left );
	target->hasMouseReleased	= target->isMouseOver && imuiInputHasMouseButtonReleased( input, ImuiInputMouseButton_Left );

	if( (input->current.mouseButtons[ ImuiInputMouseButton_Left ] || input->last.mouseButtons[ ImuiInputMouseButton_Left ]) &&
		widget->inputContext.lastFrameIndex >= imui->frame.index - 1u )
	{
		widget->inputContext.wasPressed	|= target->isMouseOver && imuiInputHasMouseButtonPressed( input, ImuiInputMouseButton_Left );
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

void imuiWidgetDrawColor( ImuiWidget* widget, ImuiColor color )
{
	ImuiDrawElement* element = imuiDrawPushElement( widget, ImuiDrawElementType_Rect, IMUI_TEXTURE_HANDLE_INVALID );
	ImuiDrawElementDataRect* rectData = &element->data.rect;
	rectData->color = color;
	memset( &rectData->uv, 0, sizeof( rectData->uv ) );
}

void imuiWidgetDrawImage( ImuiWidget* widget, const ImuiImage* image )
{
	IMUI_ASSERT( image );

	ImuiDrawElement* element = imuiDrawPushElement( widget, ImuiDrawElementType_Rect, image->textureHandle );
	ImuiDrawElementDataRect* rectData = &element->data.rect;
	rectData->color		= imuiColorCreateWhite();
	rectData->uv		= image->uv;
}

void imuiWidgetDrawImageColor( ImuiWidget* widget, const ImuiImage* image, ImuiColor color )
{
	IMUI_ASSERT( image );

	ImuiDrawElement* element = imuiDrawPushElement( widget, ImuiDrawElementType_Rect, image->textureHandle );
	ImuiDrawElementDataRect* rectData = &element->data.rect;
	rectData->color		= color;
	rectData->uv		= image->uv;
}

void imuiWidgetDrawSkin( ImuiWidget* widget, const ImuiSkin* skin, ImuiColor color )
{
	IMUI_ASSERT( skin );

	ImuiDrawElement* element = imuiDrawPushElement( widget, ImuiDrawElementType_Skin, skin->textureHandle );
	struct ImuiDrawElementDataSkin* skinData = &element->data.skin;
	skinData->border	= skin->border;
	skinData->uv		= skin->uv;
	skinData->texSize	= imuiSizeCreateSkin( skin );
	skinData->color		= color;
}

void imuiWidgetDrawText( ImuiWidget* widget, ImuiTextLayout* layout, ImuiColor color )
{
	imuiWidgetDrawTextSize( widget, layout, color, layout->font->fontSize );
}

void imuiWidgetDrawTextSize( ImuiWidget* widget, ImuiTextLayout* layout, ImuiColor color, float size )
{
	ImuiDrawElement* element = imuiDrawPushElementText( widget, ImuiDrawElementType_Text, layout );
	struct ImuiDrawElementDataText* textData = &element->data.text;
	textData->color		= color;
	textData->layout	= layout;
	textData->size		= size * widget->window->surface->dpiScale;
}

void imuiWidgetDrawPartialColor( ImuiWidget* widget, ImuiRect relativRect, ImuiColor color )
{
	ImuiDrawElement* element = imuiDrawPushElement( widget, ImuiDrawElementType_RectPartial, IMUI_TEXTURE_HANDLE_INVALID );
	ImuiDrawElementDataRect* rectData = &element->data.rect;
	rectData->relativRect	= relativRect;
	rectData->color			= color;
	memset( &rectData->uv, 0, sizeof( rectData->uv ) );
}

void imuiWidgetDrawPartialImage( ImuiWidget* widget, ImuiRect relativRect, const ImuiImage* image )
{
	IMUI_ASSERT( image );

	ImuiDrawElement* element = imuiDrawPushElement( widget, ImuiDrawElementType_RectPartial, image->textureHandle );
	ImuiDrawElementDataRect* rectData = &element->data.rect;
	rectData->relativRect	= relativRect;
	rectData->color			= imuiColorCreateWhite();
	rectData->uv			= image->uv;
}

void imuiWidgetDrawPartialImageColor( ImuiWidget* widget, ImuiRect relativRect, const ImuiImage* image, ImuiColor color )
{
	IMUI_ASSERT( image );

	ImuiDrawElement* element = imuiDrawPushElement( widget, ImuiDrawElementType_RectPartial, image->textureHandle );
	ImuiDrawElementDataRect* rectData = &element->data.rect;
	rectData->relativRect	= relativRect;
	rectData->color			= color;
	rectData->uv			= image->uv;
}

void imuiWidgetDrawPartialSkin( ImuiWidget* widget, ImuiRect relativRect, const ImuiSkin* skin, ImuiColor color )
{
	IMUI_ASSERT( skin );

	ImuiDrawElement* element = imuiDrawPushElement( widget, ImuiDrawElementType_SkinPartial, skin->textureHandle );
	struct ImuiDrawElementDataSkin* skinData = &element->data.skin;
	skinData->relativRect	= relativRect;
	skinData->border		= skin->border;
	skinData->uv			= skin->uv;
	skinData->texSize		= imuiSizeCreateSkin( skin );
	skinData->color			= color;
}

void imuiWidgetDrawPositionText( ImuiWidget* widget, ImuiPos offset, ImuiTextLayout* layout, ImuiColor color )
{
	imuiWidgetDrawPositionTextSize( widget, offset, layout, color, layout->font->fontSize );
}

void imuiWidgetDrawPositionTextSize( ImuiWidget* widget, ImuiPos offset, ImuiTextLayout* layout, ImuiColor color, float size )
{
	ImuiDrawElement* element = imuiDrawPushElementText( widget, ImuiDrawElementType_TextOffset, layout );
	struct ImuiDrawElementDataText* textData = &element->data.text;
	textData->relativPos	= offset;
	textData->color			= color;
	textData->layout		= layout;
	textData->size			= size * widget->window->surface->dpiScale;
}

void imuiWidgetDrawLine( ImuiWidget* widget, ImuiPos p0, ImuiPos p1, ImuiColor color )
{
	ImuiDrawElement* element = imuiDrawPushElement( widget, ImuiDrawElementType_Line, IMUI_TEXTURE_HANDLE_INVALID );
	ImuiDrawElementDataPrimitive* primitiveData = &element->data.primitive;
	primitiveData->p0		= p0;
	primitiveData->p1		= p1;
	primitiveData->color	= color;
}

void imuiWidgetDrawTriangle( ImuiWidget* widget, ImuiPos p0, ImuiPos p1, ImuiPos p2, ImuiColor color )
{
	ImuiDrawElement* element = imuiDrawPushElement( widget, ImuiDrawElementType_Triangle, IMUI_TEXTURE_HANDLE_INVALID );
	ImuiDrawElementDataPrimitive* primitiveData = &element->data.primitive;
	primitiveData->p0		= p0;
	primitiveData->p1		= p1;
	primitiveData->p2		= p2;
	primitiveData->color	= color;
}

static void imuiWidgetStateFreeList( ImuiAllocator* allocator, ImuiWidgetState* firstState )
{
	ImuiWidgetState* state = firstState;
	ImuiWidgetState* nextState = NULL;
	while( state )
	{
		nextState = state->nextUsageState;

		if( state->destructFunc )
		{
			state->destructFunc( state );
		}
		imuiMemoryFree( allocator, state );
		state = nextState;
	}
}

static void imuiLayoutGridContextFreeList( ImuiAllocator* allocator, ImuiLayoutGridContext* firstContext )
{
	ImuiLayoutGridContext* gridContext = firstContext;
	ImuiLayoutGridContext* nextGridContext = NULL;
	while( gridContext )
	{
		nextGridContext = gridContext->nextContext;
		imuiMemoryFree( allocator, gridContext );
		gridContext = nextGridContext;
	}
}
