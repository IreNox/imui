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
	ImUiFrame*		frame;
	ImUiSurface*	surface;

	ImUiHash		hash;
	ImUiStringView	name;

	ImUiRect		rect;
	uint32			zOrder;
	uintsize		drawIndex;

	ImUiWidget*		rootWidget;
	ImUiWidget*		lastFrameRootWidget;
	ImUiWidget*		currentWidget;
	ImUiWidget*		lastFrameCurrentWidget;
};

typedef struct ImUiWidgetState ImUiWidgetState;
struct ImUiWidgetState
{
	ImUiWidgetState*		prevState;
	ImUiWidgetState*		nextState;

	uintsize				stateSize;
	ImUiStateDestructFunc	stateDestructFunc;

	uint8					data[ 1u ];
};

struct ImUiLayoutScrollData
{
	ImUiPos									offset;
};

struct ImUiLayoutHorizontalVerticalData
{
	float									spacing;
};

struct ImUiLayoutGridData
{
	size_t									columnCount;
};

typedef union ImUiLayoutData ImUiLayoutData;
union ImUiLayoutData
{
	struct ImUiLayoutScrollData				scroll;
	struct ImUiLayoutHorizontalVerticalData	horizintalVertical;
	struct ImUiLayoutGridData				grid;
};

typedef struct ImUiLayoutContext ImUiLayoutContext;
struct ImUiLayoutContext
{
	//ImUiRect		minInnerRect;
	//ImUiRect		maxInnerRect;
	ImUiSize		minOuterSize;
	size_t			childCount;
	ImUiSize		childrenStretch;
	ImUiSize		childrenMaxStretch;
	ImUiSize		childrenMinSize;
	ImUiSize		childrenMargin;
};

struct ImUiWidget
{
	ImUiWindow*				window;
	ImUiWidget*				parent;

	ImUiWidget*				prevSibling;
	ImUiWidget*				nextSibling;

	ImUiWidget*				firstChild;
	ImUiWidget*				lastChild;

	ImUiHash				hash;
	ImUiId					id;
	ImUiStringView			name;

	ImUiWidget*				lastFrameWidget;

	ImUiWidgetState*		state;

	ImUiBorder				margin;
	ImUiBorder				padding;

	ImUiSize				minSize;
	ImUiSize				maxSize;

	ImUiSize				stretch;
	ImUiPos					offset;

	ImUiLayout				layout;
	ImUiLayoutData			layoutData;

	ImUiAlign				align;

	ImUiRect				rect;
	ImUiLayoutContext		layoutContext;
};

typedef struct ImUiWidgetChunk ImUiWidgetChunk;
struct ImUiWidgetChunk
{
	ImUiWidgetChunk*		nextChunk;
	ImUiWidget				data[ IMUI_DEFAULT_WIDGET_CHUNK_SIZE ];
	uintsize				usedCount;
};

struct ImUiFrame
{
	ImUiContext*			imui;
	float					timeInSeconds;
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

	ImUiWidgetState*		firstState;
	ImUiWidgetState*		firstUnusedState;
};
