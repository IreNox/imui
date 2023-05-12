#include "imui_widget.h"

#include "imui_internal.h"
#include "imui_memory.h"
#include "imui_string_pool.h"

#include <string.h>

ImUiWidget* ImUiWidgetAlloc( ImUiContext* imui )
{
	if( imui->firstChunk == NULL )
	{
		imui->firstChunk = IMUI_MEMORY_NEW( &imui->allocator, ImUiWidgetChunk );
		if( imui->firstChunk == NULL )
		{
			return NULL;
		}

		IMUI_ASSERT( imui->lastChunk == NULL );
		imui->lastChunk = imui->firstChunk;

		imui->firstChunk->nextChunk	= NULL;
		imui->firstChunk->usedCount	= 0u;
	}
	else if( imui->firstChunk->usedCount == IMUI_DEFAULT_WIDGET_CHUNK_SIZE )
	{
		ImUiWidgetChunk* nextChunk = imui->firstChunk->nextChunk;
		if( nextChunk != NULL &&
			nextChunk->usedCount < IMUI_DEFAULT_WIDGET_CHUNK_SIZE )
		{
			IMUI_ASSERT( nextChunk != imui->lastChunk );

			imui->firstChunk->nextChunk = NULL;

			imui->lastChunk->nextChunk = imui->firstChunk;
			imui->lastChunk = imui->firstChunk;

			imui->firstChunk = nextChunk;
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

ImUiWidget* ImUiWidgetCreate( ImUiWidget* parent )
{
	IMUI_ASSERT( parent != NULL );

	ImUiWidget* widget = ImUiWidgetAlloc( parent->window->imui );
	if( widget == NULL )
	{
		return NULL;
	}

	widget->window	= parent->window;
	widget->parent	= parent;

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

	return widget;
}

ImUiWidget* ImUiWidgetCreateId( ImUiWidget* parent, ImUiId id )
{
	ImUiWidget* widget = ImUiWidgetCreate( parent );
	if( widget == NULL )
	{
		return NULL;
	}

	widget->id = id;

	return widget;
}

ImUiWidget* ImUiWidgetCreateNamed( ImUiWidget* parent, ImUiStringView name )
{
	ImUiWidget* widget = ImUiWidgetCreate( parent );
	if( widget == NULL )
	{
		return NULL;
	}

	widget->id		= ImUiHashString( name, 0u );
	widget->name	= ImUiStringPoolAdd( &parent->window->imui->strings, name );

	return widget;
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
