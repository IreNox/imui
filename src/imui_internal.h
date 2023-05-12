#pragma once

#include "imui/imui.h"

#include "imui_draw.h"
#include "imui_input.h"
#include "imui_string_pool.h"
#include "imui_types.h"

struct ImUiSurface
{
	ImUiContext*	imui;

	ImUiStringView	name;

	ImUiSize		size;
	float			dpiScale;

	ImUiWindow*		windows;
	uintsize		windowCapacity;
	uintsize		windowCount;
};

struct ImUiWindow
{
	ImUiContext*	imui;
	ImUiSurface*	surface;

	ImUiHash		hash;
	ImUiStringView	name;

	ImUiRectangle	rectangle;
	uint32			zOrder;
	uintsize		drawIndex;

	ImUiWidget*		rootWidget;
};

typedef struct ImUiWidgetLayoutContext ImUiWidgetLayoutContext;
struct ImUiWidgetLayoutContext
{
	ImUiRectangle	minInnerRect;
	ImUiRectangle	maxInnerRect;
	ImUiSize		childrenStretch;
	ImUiSize		childrenMinSize;
	ImUiSize		childrenMaxSize;
	ImUiSize		childrenPrefSize;
};

struct ImUiWidget
{
	ImUiWindow*				window;
	ImUiWidget*				parent;

	ImUiWidget*				previousSibling;
	ImUiWidget*				nextSibling;

	ImUiWidget*				firstChild;
	ImUiWidget*				lastChild;

	ImUiId					id;
	ImUiHash				hash;
	ImUiStringView			name;

	ImUiThickness			margin;
	ImUiThickness			padding;

	ImUiSize				minSize;
	ImUiSize				maxSize;
	ImUiSize				prefSize;

	ImUiSize				stretch;
	ImUiPosition			offset;

	ImUiLayout				layout;
	ImUiSize				layoutSpacing;

	ImUiAlignment			alignment;

	ImUiRectangle			rectangle;
	ImUiWidgetLayoutContext	layoutContext;
};

typedef struct ImUiWidgetChunk ImUiWidgetChunk;
struct ImUiWidgetChunk
{
	ImUiWidgetChunk*	nextChunk;
	ImUiWidget			data[ IMUI_DEFAULT_WIDGET_CHUNK_SIZE ];
	uintsize			usedCount;
};

struct ImUiContext
{
	ImUiAllocator		allocator;

	ImUiInput			input;
	ImUiDraw			draw;
	ImUiStringPool		strings;

	ImUiFrame			frame;

	ImUiSurface*		surfaces;
	uintsize			surfaceCapacity;
	uintsize			surfaceCount;

	ImUiWidget			defaultWidget;
	ImUiWidgetChunk*	firstChunk;
	ImUiWidgetChunk*	lastChunk;
};
