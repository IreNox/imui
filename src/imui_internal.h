#pragma once

#include "imui/imui.h"

#include "imui_draw.h"
#include "imui_input.h"
#include "imui_helpers.h"
#include "imui_types.h"
#include "imui_text.h"

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

	ImUiRect	rectangle;
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
	//ImUiRect	minInnerRect;
	//ImUiRect	maxInnerRect;
	ImUiSize		minOuterSize;
	size_t			childCount;
	ImUiSize		childrenStretch;
	ImUiSize		childrenMaxStretch;
	ImUiSize		childrenMinSize;
	ImUiSize		childrenMargin;
};

struct ImUiLayoutScrollData
{
	ImUiPos	offset;
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

	ImUiBorder			margin;
	ImUiBorder			padding;

	ImUiSize				minSize;
	ImUiSize				maxSize;

	ImUiSize				stretch;
	ImUiPos			offset;

	ImUiLayout				layout;
	ImUiLayoutData			layoutData;

	ImUiAlign			align;

	ImUiRect			rect;
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
	ImUiTextLayoutCache		layoutCache;

	ImUiFrame				frame;

	ImUiSurface*			surfaces;
	uintsize				surfaceCapacity;
	uintsize				surfaceCount;

	ImUiWidgetChunk*		firstChunk;
	ImUiWidgetChunk*		firstLastFrameChunk;
	ImUiWidgetChunk*		firstFreeChunk;
};
