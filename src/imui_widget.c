#include "imui_widget.h"

#include "imui_internal.h"
#include "imui_memory.h"
#include "imui_string_pool.h"

#include <string.h>

ImUiWidget* ImUiWidgetAlloc( ImUiContext* imui )
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

	memcpy( widget, &imui->defaultWidget, sizeof( *widget ) );

	return widget;
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

	widget->window->currentWidget = widget->parent;
	widget->window->lastFrameCurrentWidget = widget->parent;
}

ImUiLayout ImUiWidgetGetLayout( const ImUiWidget* widget )
{
	return widget->layout;
}

void ImUiWidgetSetLayout( ImUiWidget* widget, ImUiLayout layout )
{
	widget->layout = layout;
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
