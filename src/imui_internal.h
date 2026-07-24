#pragma once

#include "imui/imui.h"

#include "imui_draw.h"
#include "imui_input.h"
#include "imui_helpers.h"
#include "imui_types.h"
#include "imui_text.h"

struct ImuiSurface
{
	bool					inUse;

	ImuiContext*			context;

	ImuiId					id;
	ImuiStringView			name;
	ImuiSize				size;
	const ImuiInputState*	input;
	float					dpiScale;

	uintsize				drawIndex;

	ImuiWindow*				windows;
	uintsize				windowCapacity;
	uintsize				windowCount;
};

struct ImuiWindow
{
	bool			inUse;

	ImuiContext*	context;
	ImuiSurface*	surface;

	ImuiId			id;
	ImuiStringView	name;
	ImuiRect		rect;
	uint32			zOrder;
	bool			hasFocus;
	bool			focusLocked;
	bool			focusWrap;
	ImuiPos			focusPoint;
	ImuiPos			focusWrapPoint;
	float			focusAngleThreshold;
	float			diagonalLength;

	uintsize		drawIndex;

	ImuiWidget*		rootWidget;
	ImuiWidget*		lastFrameRootWidget;
	ImuiWidget*		currentWidget;
	ImuiWidget*		lastFrameCurrentWidget;
	ImuiWidget*		focusWidget;
	ImuiWidget*		lastFrameFocusWidget;

	float			closesFocusWidgetFactor;
	ImuiWidget*		closesFocusWidget;
	float			wrapFocusWidgetFactor;
	ImuiWidget*		wrapFocusWidget;

	uint32			lastFocusIndex;
	ImuiWidget*		closesFocusIndexWidget;
	ImuiWidget*		wrapFocusIndexWidget;
};

typedef struct ImuiWidgetState ImuiWidgetState;
struct ImuiWidgetState
{
	ImuiWidgetState*		prevUsageState;
	ImuiWidgetState*		nextUsageState;
	ImuiWidgetState*		prevWidgetState;
	ImuiWidgetState*		nextWidgetState;

	ImuiId					id;
	uintsize				size;
	ImuiStateDestructFunc	destructFunc;

	uint8					data[ 1u ];
};

typedef struct ImuiLayoutScrollData
{
	ImuiPos								offset;
} ImuiLayoutScrollData;

typedef struct ImuiLayoutHorizontalVerticalData
{
	float								spacing;
} ImuiLayoutHorizontalVerticalData;

typedef struct ImuiLayoutGridData
{
	uint32								columnCount;
	float								colSpacing;
	float								rowSpacing;
} ImuiLayoutGridData;

typedef union ImuiLayoutData
{
	ImuiLayoutScrollData				scroll;
	ImuiLayoutHorizontalVerticalData	horizintalVertical;
	ImuiLayoutGridData					grid;
} ImuiLayoutData;

typedef struct ImuiLayoutGridElement ImuiLayoutGridElement;
struct ImuiLayoutGridElement
{
	float				childrenMaxStretch;
	float				childrenMinSize;

	float				pos;
	float				size;
};

typedef struct ImuiLayoutGridContext ImuiLayoutGridContext;
struct ImuiLayoutGridContext
{
	ImuiLayoutGridContext*	nextContext;
	ImuiLayoutGridContext*	prevContext;

	ImuiLayoutGridElement*	columns;
	uintsize				columnCount;

	ImuiLayoutGridElement*	rows;
	uintsize				rowCount;

	uint32					frameIndex;
};

typedef struct ImuiLayoutContext
{
	ImuiSize				minOuterSize;
	ImuiSize				childrenStretch;
	ImuiSize				childrenMaxStretch;
	ImuiSize				childrenStretchFinal;
	ImuiSize				childrenStretchMinSize;
	ImuiSize				childrenMinSize;
} ImuiLayoutContext;

typedef struct ImuiWidgetInputContext
{
	uint32					lastFrameIndex;
	bool					wasPressed;
	bool					wasMouseOver;
} ImuiWidgetInputContext;

struct ImuiWidget
{
	ImuiWindow*				window;
	ImuiWidget*				parent;

	ImuiWidget*				prevSibling;
	ImuiWidget*				nextSibling;

	ImuiWidget*				firstChild;
	ImuiWidget*				lastChild;
	uintsize				childCount;

	ImuiHash				hash;
	ImuiId					id;
	ImuiStringView			name;

	ImuiWidgetState*		firstState;

	ImuiBorder				margin;
	ImuiBorder				padding;

	ImuiSize				minSize;
	ImuiSize				maxSize;
	float					stretchH;
	float					stretchV;

	ImuiLayout				layout;
	ImuiLayoutData			layoutData;

	float					alignH;
	float					alignV;

	bool					canHaveFocus;
	uint32					focusIndex;

	// generated data
	ImuiRect				rect;
	ImuiRect				clipRect;
	ImuiWidget*				lastFrameWidget;
	ImuiLayoutContext		layoutContext;
	ImuiLayoutGridContext*	gridContext;
	ImuiWidgetInputContext	inputContext;
};

typedef struct ImuiWidgetChunk ImuiWidgetChunk;
struct ImuiWidgetChunk
{
	ImuiWidgetChunk*		nextChunk;
	ImuiWidget				data[ IMUI_DEFAULT_WIDGET_CHUNK_SIZE ];
	uintsize				usedCount;
};

struct ImuiFrame
{
	ImuiContext*			context;
	uint32					index;
	double					timeInSeconds;
};

struct ImuiContext
{
	ImuiAllocator			allocator;

	ImuiInput				input;
	ImuiDraw				draw;
	ImuiStringPool			strings;
	ImuiTextLayoutCache		layoutCache;

	ImuiFrame				frame;

	ImuiSurface*			surfaces;
	uintsize				surfaceCapacity;
	uintsize				surfaceCount;

	ImuiWidgetChunk*		firstChunk;
	ImuiWidgetChunk*		firstLastFrameChunk;
	ImuiWidgetChunk*		firstFreeChunk;

	ImuiWidgetState*		firstState;
	ImuiWidgetState*		firstUnusedState;

	ImuiLayoutGridContext*	firstGridContext;
	ImuiLayoutGridContext*	firstUnusedGridContext;
};

ImuiStringView				imuiStringViewCreate( const char* str );
ImuiStringView				imuiStringViewCreateLength( const char* str, size_t length );
ImuiStringView				imuiStringViewCreateEmpty();
bool						imuiStringViewIsEquals( ImuiStringView string1, ImuiStringView string2 );

ImuiHash					imuiHashString( ImuiStringView string );
ImuiHash					imuiHashStringSeed( ImuiStringView string, ImuiHash seed );
