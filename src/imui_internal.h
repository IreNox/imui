#pragma once

#include "imui/imui.h"

#include "imui_draw.h"
#include "imui_input.h"
#include "imui_string_pool.h"
#include "imui_types.h"

struct ImUiSurface
{
	bool			inUse;

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
	bool			inUse;

	ImUiContext*	imui;
	ImUiSurface*	surface;

	ImUiHash		hash;
	ImUiStringView	name;

	ImUiRectangle	rectangle;
	uint32			zOrder;
	uintsize		drawIndex;

	ImUiWidget*		rootWidget;
	ImUiWidget*		lastFrameRootWidget;
	ImUiWidget*		currentWidget;
	ImUiWidget*		lastFrameCurrentWidget;
};

typedef struct ImUiWidgetLayoutContext ImUiWidgetLayoutContext;
struct ImUiWidgetLayoutContext
{
	//ImUiRectangle	minInnerRect;
	//ImUiRectangle	maxInnerRect;
	size_t			childCount;
	ImUiSize		childrenStretch;
	ImUiSize		childrenMaxStretch;
	ImUiSize		childrenMinSize;
	ImUiSize		childrenMaxSize;
};

struct ImUiLayoutScrollData
{
	ImUiPosition	offset;
};

struct ImUiLayoutHorizontalVerticalData
{
	float			spacing;
};

struct ImUiLayoutGridData
{
	size_t			columnCount;
};

typedef union ImUiLayoutData ImUiLayoutData;
union ImUiLayoutData
{
	struct ImUiLayoutScrollData				scroll;
	struct ImUiLayoutHorizontalVerticalData	horizintalVertical;
	struct ImUiLayoutGridData				grid;
};

struct ImUiWidget
{
	ImUiWindow*				window;
	ImUiWidget*				parent;

	ImUiWidget*				previousSibling;
	ImUiWidget*				nextSibling;

	ImUiWidget*				firstChild;
	ImUiWidget*				lastChild;

	ImUiHash				hash;
	ImUiHash				lastFrameHash;
	ImUiId					id;
	ImUiStringView			name;

	ImUiThickness			margin;
	ImUiThickness			padding;

	ImUiSize				minSize;
	ImUiSize				maxSize;
	ImUiSize				prefSize;

	ImUiSize				stretch;
	ImUiPosition			offset;

	ImUiLayout				layout;
	ImUiLayoutData			layoutData;

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
	ImUiAllocator			allocator;

	ImUiInput				input;
	ImUiDraw				draw;
	ImUiStringPool			strings;

	ImUiFrame				frame;

	ImUiSurface*			surfaces;
	uintsize				surfaceCapacity;
	uintsize				surfaceCount;

	ImUiWidgetChunk*		firstChunk;
	ImUiWidgetChunk*		firstLastFrameChunk;
	ImUiWidgetChunk*		firstFreeChunk;
};
