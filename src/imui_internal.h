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

	uintsize		drawIndex;

	ImUiWindow*		windows;
	uintsize		windowCapacity;
	uintsize		windowCount;
};

struct ImUiWindow
{
	bool			inUse;

	ImUiContext*	imui;
	ImUiSurface*	surface;

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
typedef struct ImUiWidgetState
{
	ImUiWidgetState*		prevUsageState;
	ImUiWidgetState*		nextUsageState;
	ImUiWidgetState*		prevWidgetState;
	ImUiWidgetState*		nextWidgetState;

	ImUiId					id;
	uintsize				size;
	ImUiStateDestructFunc	destructFunc;

	uint8					data[ 1u ];
} ImUiWidgetState;

typedef struct ImUiLayoutScrollData
{
	ImUiPos								offset;
} ImUiLayoutScrollData;

typedef struct ImUiLayoutHorizontalVerticalData
{
	float								spacing;
} ImUiLayoutHorizontalVerticalData;

typedef struct ImUiLayoutGridData
{
	uint32								columnCount;
	float								colSpacing;
	float								rowSpacing;
} ImUiLayoutGridData;

typedef union ImUiLayoutData
{
	ImUiLayoutScrollData				scroll;
	ImUiLayoutHorizontalVerticalData	horizintalVertical;
	ImUiLayoutGridData					grid;
} ImUiLayoutData;

typedef struct ImUiLayoutGridElement ImUiLayoutGridElement;
struct ImUiLayoutGridElement
{
	float				childrenMaxStretch;
	float				childrenMinSize;

	float				pos;
	float				size;
};

typedef struct ImUiLayoutGridContext ImUiLayoutGridContext;
struct ImUiLayoutGridContext
{
	ImUiLayoutGridContext*	nextContext;
	ImUiLayoutGridContext*	prevContext;

	ImUiLayoutGridElement*	columns;
	uintsize				columnCount;

	ImUiLayoutGridElement*	rows;
	uintsize				rowCount;

	uint32					frameIndex;
};

typedef struct ImUiLayoutContext ImUiLayoutContext;
struct ImUiLayoutContext
{
	ImUiSize				minOuterSize;
	ImUiSize				childrenStretch;
	ImUiSize				childrenMaxStretch;
	ImUiSize				childrenMinSize;
};

typedef struct ImUiWidgetInputContext ImUiWidgetInputContext;
struct ImUiWidgetInputContext
{
	uint32					lastFrameIndex;
	bool					wasPressed;
	bool					wasMouseOver;
};

struct ImUiWidget
{
	ImUiWindow*				window;
	ImUiWidget*				parent;

	ImUiWidget*				prevSibling;
	ImUiWidget*				nextSibling;

	ImUiWidget*				firstChild;
	ImUiWidget*				lastChild;
	uintsize				childCount;

	ImUiHash				hash;
	ImUiId					id;
	ImUiStringView			name;

	ImUiWidgetState*		firstState;

	ImUiBorder				margin;
	ImUiBorder				padding;

	ImUiSize				minSize;
	ImUiSize				maxSize;
	float					stretchH;
	float					stretchV;

	ImUiLayout				layout;
	ImUiLayoutData			layoutData;

	float					alignH;
	float					alignV;

	// generated data
	ImUiRect				rect;
	ImUiRect				clipRect;
	ImUiWidget*				lastFrameWidget;
	ImUiLayoutContext		layoutContext;
	ImUiLayoutGridContext*	gridContext;
	ImUiWidgetInputContext	inputContext;
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
	uint32					index;
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

	ImUiLayoutGridContext*	firstGridContext;
	ImUiLayoutGridContext*	firstUnusedGridContext;
};

ImUiStringView				ImUiStringViewCreate( const char* str );
ImUiStringView				ImUiStringViewCreateLength( const char* str, size_t length );
ImUiStringView				ImUiStringViewCreateEmpty();
bool						ImUiStringViewIsEquals( ImUiStringView string1, ImUiStringView string2 );

ImUiHash					ImUiHashString( ImUiStringView string, ImUiHash seed );
