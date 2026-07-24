#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define IMUI_ID_DEFAULT			0
#define IMUI_ID_STR( STR )		(ImuiId)(size_t)(STR)
#define IMUI_ID_TYPE( TYPE )	(ImuiId)(size_t)(#TYPE)

typedef struct ImuiContext ImuiContext;
typedef struct ImuiDraw ImuiDraw;
typedef struct ImuiFrame ImuiFrame;
typedef struct ImuiInput ImuiInput;
typedef struct ImuiInputShortcutConfig ImuiInputShortcutConfig;
typedef struct ImuiInputState ImuiInputState;
typedef struct ImuiSurface ImuiSurface;
typedef struct ImuiTextLayout ImuiTextLayout;
typedef struct ImuiWidget ImuiWidget;
typedef struct ImuiWindow ImuiWindow;

typedef uint32_t ImuiId;
typedef uint32_t ImuiHash;

typedef void*(*ImuiAllocatorMallocFunc)(size_t size, void* userData);
typedef void*(*ImuiAllocatorReallocFunc)(void* memory, size_t oldSize, size_t newSize, void* userData);
typedef void( *ImuiAllocatorFreeFunc )(void* memory, void* userData);

typedef struct ImuiAllocator
{
	ImuiAllocatorMallocFunc		mallocFunc;		// set to NULL to use default malloc/free
	ImuiAllocatorReallocFunc	reallocFunc;	// can be NULL
	ImuiAllocatorFreeFunc		freeFunc;
	void*						userData;
	void*						internalData;	// internal use only
} ImuiAllocator;

typedef void(*ImuiStateDestructFunc)(void* state);

// needed?
//typedef bool(*ImuiWindowEvalFocusFunc)(imuiWindow* window, imuiWidget* widget, void* userData);

typedef enum ImuiVertexElementType
{
	ImuiVertexElementType_Float,
	ImuiVertexElementType_Float2,
	ImuiVertexElementType_Float3,
	ImuiVertexElementType_Float4,

	ImuiVertexElementType_Int,
	ImuiVertexElementType_Int2,
	ImuiVertexElementType_Int3,
	ImuiVertexElementType_Int4,

	ImuiVertexElementType_UInt,
	ImuiVertexElementType_UInt2,
	ImuiVertexElementType_UInt3,
	ImuiVertexElementType_UInt4,

	ImuiVertexElementType_MAX
} ImuiVertexElementType;

typedef enum ImuiVertexElementSemantic
{
	ImuiVertexElementSemantic_Zero,
	ImuiVertexElementSemantic_PositionScreenSpace,
	ImuiVertexElementSemantic_PositionClipSpace,
	ImuiVertexElementSemantic_TextureCoordinate,
	ImuiVertexElementSemantic_ColorRGBA,
	ImuiVertexElementSemantic_ColorABGR
} ImuiVertexElementSemantic;

typedef struct ImuiVertexElement
{
	uint32_t					align;
	ImuiVertexElementType		type;
	ImuiVertexElementSemantic	semantic;
} ImuiVertexElement;

typedef struct ImuiVertexFormat
{
	const ImuiVertexElement*	elements;
	size_t						elementCount;
} ImuiVertexFormat;

typedef enum ImuiVertexType
{
	ImuiVertexType_VertexList,
	ImuiVertexType_IndexedVertexList
} ImuiVertexType;

typedef struct ImuiParameters					// Fill with zero for default parameters
{
	ImuiAllocator					allocator;			// Override memory Allocator. Default use malloc/free
	const ImuiInputShortcutConfig*	shortcuts;			// Define keyboard shortcuts
	size_t							shortcutCount;
	ImuiVertexFormat				vertexFormat;		// Override vertex format. Default: float2 pos screen-space, float2 uv, float4 color
	ImuiVertexType					vertexType;			// Override vertex type, Default: ImuiVertexType_VertexList
} ImuiParameters;

ImuiContext*				imuiCreate( const ImuiParameters* parameters );
void						imuiDestroy( ImuiContext* imui );

ImuiFrame*					imuiBegin( ImuiContext* imui, double timeInSeconds );
void						imuiEnd( ImuiFrame* frame );

ImuiContext*				imuiFrameGetContext( const ImuiFrame* frame );

//////////////////////////////////////////////////////////////////////////
// Types

#define IMUI_TEXTURE_HANDLE_INVALID 0

typedef struct ImuiBorder
{
	float					top;
	float					left;
	float					bottom;
	float					right;
} ImuiBorder;

typedef struct ImuiColor
{
	uint8_t					red;
	uint8_t					green;
	uint8_t					blue;
	uint8_t					alpha;
} ImuiColor;

typedef struct ImuiPos
{
	float					x;
	float					y;
} ImuiPos;

typedef struct ImuiSize
{
	float					width;
	float					height;
} ImuiSize;

typedef struct ImuiRect
{
	ImuiPos					pos;
	ImuiSize				size;
} ImuiRect;

typedef struct ImuiTexCoord
{
	float					u0;
	float					v0;
	float					u1;
	float					v1;
} ImuiTexCoord;

typedef struct ImuiImage
{
	uint64_t				textureHandle;
	uint32_t				width;
	uint32_t				height;
	ImuiTexCoord			uv;
} ImuiImage;

typedef struct ImuiSkin
{
	uint64_t				textureHandle;
	uint32_t				width;
	uint32_t				height;
	ImuiTexCoord			uv;
	ImuiBorder				border;
} ImuiSkin;

typedef enum ImuiLayout
{
	ImuiLayout_Stack,
	ImuiLayout_Scroll,
	ImuiLayout_Horizontal,
	ImuiLayout_Vertical,
	ImuiLayout_Grid
} ImuiLayout;

typedef enum ImuiDrawTopology
{
	ImuiDrawTopology_LineList,
	ImuiDrawTopology_TriangleList,
	ImuiDrawTopology_IndexedTriangleList,

	ImuiDrawTopology_MAX
} ImuiDrawTopology;

typedef struct ImuiDrawCommand
{
	ImuiDrawTopology		topology;
	uint64_t				textureHandle;
	ImuiRect				clipRect;
	size_t					count;				// index count if index buffer is used otherwise vertex count
} ImuiDrawCommand;

typedef struct ImuiDrawData
{
	const ImuiDrawCommand*	commands;
	size_t					commandCount;
} ImuiDrawData;

//////////////////////////////////////////////////////////////////////////
// Surface - Presents a OS window or a screen

ImuiSurface*				imuiSurfaceBegin( ImuiFrame* frame, const char* name, ImuiSize size, const ImuiInputState* input, float dpiScale );
ImuiSurface*				imuiSurfaceBeginId( ImuiFrame* frame, const char* name, ImuiId id, ImuiSize size, const ImuiInputState* input, float dpiScale );
void						imuiSurfaceEnd( ImuiSurface* surface );

ImuiContext*				imuiSurfaceGetContext( const ImuiSurface* surface );

double						imuiSurfaceGetTime( const ImuiSurface* surface );
ImuiSize					imuiSurfaceGetSize( const ImuiSurface* surface );
ImuiRect					imuiSurfaceGetRect( const ImuiSurface* surface );
const ImuiInputState*		imuiSurfaceGetInput( const ImuiSurface* surface );
float						imuiSurfaceGetDpiScale( const ImuiSurface* surface );

// call after surface end but before end frame
void						imuiSurfaceGetMaxBufferSizes( ImuiSurface* surface, size_t* outVertexDataSize, size_t* outIndexDataSize );
const ImuiDrawData*			imuiSurfaceGenerateDrawData( ImuiSurface* surface, void* outVertexData, size_t* inOutVertexDataSize, void* outIndexData, size_t* inOutIndexDataSize );


//////////////////////////////////////////////////////////////////////////
// Window - A part of a Surface with z ordering

ImuiWindow*					imuiWindowBegin( ImuiSurface* surface, const char* name, ImuiRect rect, uint32_t zOrder );
ImuiWindow*					imuiWindowBeginId( ImuiSurface* surface, const char* name, ImuiId id, ImuiRect rect, uint32_t zOrder );
void						imuiWindowEnd( ImuiWindow* window );

ImuiContext*				imuiWindowGetContext( const ImuiWindow* window );
ImuiSurface*				imuiWindowGetSurface( const ImuiWindow* window );
const ImuiInputState*		imuiWindowGetInput( const ImuiWindow* window );

double						imuiWindowGetTime( const ImuiWindow* window );
float						imuiWindowGetDpiScale( const ImuiWindow* window );
ImuiRect					imuiWindowGetRect( const ImuiWindow* window );

bool						imuiWindowHasFocus( const ImuiWindow* window );
void						imuiWindowSetFocus( ImuiWindow* window, float angleThreshold, bool wrap );

bool						imuiWindowIsWidgetFocusLocked( const ImuiWindow* window );
void						imuiWindowSetWidgetFocusLock( ImuiWindow* window, bool locked );
void						imuiWindowClearWidgetFocus( ImuiWindow* window );
const ImuiWidget*			imuiWindowGetFocusWidget( const ImuiWindow* window );
const ImuiWidget*			imuiWindowPeekFocusWidget( const ImuiWindow* window );

void*						imuiWindowAllocState( ImuiWindow* window, size_t size, ImuiId stateId );
void*						imuiWindowAllocStateNew( ImuiWindow* window, size_t size, ImuiId stateId, bool* isNew );
void*						imuiWindowAllocStateNewDestruct( ImuiWindow* window, size_t size, ImuiId stateId, bool* isNew, ImuiStateDestructFunc destructFunc );

ImuiWidget*					imuiWindowGetFirstChild( const ImuiWindow* window );
ImuiWidget*					imuiWindowGetLastChild( const ImuiWindow* window );

//////////////////////////////////////////////////////////////////////////
// Widget - A layout element in the tree

typedef struct ImuiWidgetInputState
{
	ImuiPos					relativeMousePos;

	bool					hasFocus;
	bool					wasPressed;
	bool					wasMouseOver;
	bool					isMouseOver;
	bool					isMouseDown;
	bool					hasMousePressed;
	bool					hasMouseReleased;
} ImuiWidgetInputState;

ImuiWidget*					imuiWidgetBegin( ImuiWindow* window );
ImuiWidget*					imuiWidgetBeginId( ImuiWindow* window, ImuiId id );
ImuiWidget*					imuiWidgetBeginNamed( ImuiWindow* window, const char* name );
void						imuiWidgetEnd( ImuiWidget* widget );

ImuiContext*				imuiWidgetGetContext( const ImuiWidget* widget );
ImuiSurface*				imuiWidgetGetSurface( const ImuiWidget* widget );
const ImuiInputState*		imuiWidgetGetInput( const ImuiWidget* widget );
ImuiWindow*					imuiWidgetGetWindow( const ImuiWidget* widget );

ImuiWidget*					imuiWidgetGetParent( const ImuiWidget* widget );
ImuiWidget*					imuiWidgetGetFirstChild( const ImuiWidget* widget );
ImuiWidget*					imuiWidgetGetLastChild( const ImuiWidget* widget );
ImuiWidget*					imuiWidgetGetPrevSibling( const ImuiWidget* widget );
ImuiWidget*					imuiWidgetGetNextSibling( const ImuiWidget* widget );

double						imuiWidgetGetTime( const ImuiWidget* widget );
float						imuiWidgetGetDpiScale( const ImuiWidget* widget );

void*						imuiWidgetAllocState( ImuiWidget* widget, size_t size, ImuiId stateId );
void*						imuiWidgetAllocStateNew( ImuiWidget* widget, size_t size, ImuiId stateId, bool* isNew );
void*						imuiWidgetAllocStateNewDestruct( ImuiWidget* widget, size_t size, ImuiId stateId, bool* isNew, ImuiStateDestructFunc destructFunc );

ImuiLayout					imuiWidgetGetLayout( const ImuiWidget* widget );
void						imuiWidgetSetLayoutStack( ImuiWidget* widget );		// default
void						imuiWidgetSetLayoutScroll( ImuiWidget* widget, float offsetX, float offsetY );
void						imuiWidgetSetLayoutHorizontal( ImuiWidget* widget );
void						imuiWidgetSetLayoutHorizontalSpacing( ImuiWidget* widget, float spacing );
void						imuiWidgetSetLayoutVertical( ImuiWidget* widget );
void						imuiWidgetSetLayoutVerticalSpacing( ImuiWidget* widget, float spacing );
void						imuiWidgetSetLayoutGrid( ImuiWidget* widget, uint32_t columnCount, float colSpacing, float rowSpacing );

ImuiBorder					imuiWidgetGetMargin( const ImuiWidget* widget );
void						imuiWidgetSetMargin( ImuiWidget* widget, ImuiBorder margin );
ImuiBorder					imuiWidgetGetPadding( const ImuiWidget* widget );
void						imuiWidgetSetPadding( ImuiWidget* widget, ImuiBorder padding );

ImuiSize					imuiWidgetGetMinSize( const ImuiWidget* widget );
void						imuiWidgetSetMinWidth( ImuiWidget* widget, float value );
void						imuiWidgetSetMinHeight( ImuiWidget* widget, float value );
void						imuiWidgetSetMinSize( ImuiWidget* widget, ImuiSize size );
void						imuiWidgetSetMinSizeFloat( ImuiWidget* widget, float width, float height );
ImuiSize					imuiWidgetGetMaxSize( const ImuiWidget* widget );
void						imuiWidgetSetMaxWidth( ImuiWidget* widget, float value );
void						imuiWidgetSetMaxHeight( ImuiWidget* widget, float value );
void						imuiWidgetSetMaxSize( ImuiWidget* widget, ImuiSize size );
void						imuiWidgetSetMaxSizeFloat( ImuiWidget* widget, float width, float height );
void						imuiWidgetSetFixedWidth( ImuiWidget* widget, float value );
void						imuiWidgetSetFixedHeight( ImuiWidget* widget, float value );
void						imuiWidgetSetFixedSize( ImuiWidget* widget, ImuiSize size );
void						imuiWidgetSetFixedSizeFloat( ImuiWidget* widget, float width, float height );

void						imuiWidgetSetStretch( ImuiWidget* widget, float horizontal, float vertical );
void						imuiWidgetSetStretchOne( ImuiWidget* widget );
float						imuiWidgetGetHStretch( const ImuiWidget* widget );
void						imuiWidgetSetHStretch( ImuiWidget* widget, float stretch );
float						imuiWidgetGetVStretch( const ImuiWidget* widget );
void						imuiWidgetSetVStretch( ImuiWidget* widget, float stretch );

void						imuiWidgetSetAlign( ImuiWidget* widget, float horizontal, float vertical );
float						imuiWidgetGetHAlign( const ImuiWidget* widget );
void						imuiWidgetSetHAlign( ImuiWidget* widget, float align );
float						imuiWidgetGetVAlign( const ImuiWidget* widget );
void						imuiWidgetSetVAlign( ImuiWidget* widget, float align );

bool						imuiWidgetHasFocus( const ImuiWidget* widget );
void						imuiWidgetSetFocus( ImuiWidget* widget );
bool						imuiWidgetGetCanHaveFocus( const ImuiWidget* widget );
void						imuiWidgetSetCanHaveFocus( ImuiWidget* widget );
void						imuiWidgetSetCanHaveFocusIndex( ImuiWidget* widget, uint32_t focusIndex );

ImuiPos						imuiWidgetGetPos( const ImuiWidget* widget );
float						imuiWidgetGetPosX( const ImuiWidget* widget );
float						imuiWidgetGetPosY( const ImuiWidget* widget );
ImuiSize					imuiWidgetGetSize( const ImuiWidget* widget );
float						imuiWidgetGetSizeWidth( const ImuiWidget* widget );
float						imuiWidgetGetSizeHeight( const ImuiWidget* widget );
ImuiRect					imuiWidgetGetRect( const ImuiWidget* widget );
ImuiSize					imuiWidgetGetInnerSize( const ImuiWidget* widget );
ImuiRect					imuiWidgetGetInnerRect( const ImuiWidget* widget );

void						imuiWidgetGetInputState( ImuiWidget* widget, ImuiWidgetInputState* target );

// all positions are relative to the widget
void						imuiWidgetDrawColor( ImuiWidget* widget, ImuiColor color );
void						imuiWidgetDrawImage( ImuiWidget* widget, const ImuiImage* image);
void						imuiWidgetDrawImageColor( ImuiWidget* widget, const ImuiImage* image, ImuiColor color );
void						imuiWidgetDrawSkin( ImuiWidget* widget, const ImuiSkin* skin, ImuiColor color );
void						imuiWidgetDrawText( ImuiWidget* widget, ImuiTextLayout* layout, ImuiColor color );
void						imuiWidgetDrawTextSize( ImuiWidget* widget, ImuiTextLayout* layout, ImuiColor color, float size );
void						imuiWidgetDrawPartialColor( ImuiWidget* widget, ImuiRect relativRect, ImuiColor color );
void						imuiWidgetDrawPartialImage( ImuiWidget* widget, ImuiRect relativRect, const ImuiImage* image );
void						imuiWidgetDrawPartialImageColor( ImuiWidget* widget, ImuiRect relativRect, const ImuiImage* image, ImuiColor color );
void						imuiWidgetDrawPartialSkin( ImuiWidget* widget, ImuiRect relativRect, const ImuiSkin* skin, ImuiColor color );
void						imuiWidgetDrawPositionText( ImuiWidget* widget, ImuiPos offset, ImuiTextLayout* layout, ImuiColor color );
void						imuiWidgetDrawPositionTextSize( ImuiWidget* widget, ImuiPos offset, ImuiTextLayout* layout, ImuiColor color, float size );
void						imuiWidgetDrawLine( ImuiWidget* widget, ImuiPos p0, ImuiPos p1, ImuiColor color );
void						imuiWidgetDrawTriangle( ImuiWidget* widget, ImuiPos p0, ImuiPos p1, ImuiPos p2, ImuiColor color );

//////////////////////////////////////////////////////////////////////////
// Input
// see imui_input.c

typedef enum ImuiInputMouseButton
{
	ImuiInputMouseButton_Left,
	ImuiInputMouseButton_Right,
	ImuiInputMouseButton_Middle,

	ImuiInputMouseButton_X1,
	ImuiInputMouseButton_X2,

	ImuiInputMouseButton_MAX
} ImuiInputMouseButton;

typedef enum ImuiInputKey
{
	ImuiInputKey_None,

	ImuiInputKey_A,
	ImuiInputKey_B,
	ImuiInputKey_C,
	ImuiInputKey_D,
	ImuiInputKey_E,
	ImuiInputKey_F,
	ImuiInputKey_G,
	ImuiInputKey_H,
	ImuiInputKey_I,
	ImuiInputKey_J,
	ImuiInputKey_K,
	ImuiInputKey_L,
	ImuiInputKey_M,
	ImuiInputKey_N,
	ImuiInputKey_O,
	ImuiInputKey_P,
	ImuiInputKey_Q,
	ImuiInputKey_R,
	ImuiInputKey_S,
	ImuiInputKey_T,
	ImuiInputKey_U,
	ImuiInputKey_V,
	ImuiInputKey_W,
	ImuiInputKey_X,
	ImuiInputKey_Y,
	ImuiInputKey_Z,

	ImuiInputKey_1,
	ImuiInputKey_2,
	ImuiInputKey_3,
	ImuiInputKey_4,
	ImuiInputKey_5,
	ImuiInputKey_6,
	ImuiInputKey_7,
	ImuiInputKey_8,
	ImuiInputKey_9,
	ImuiInputKey_0,

	ImuiInputKey_Enter,
	ImuiInputKey_Escape,
	ImuiInputKey_Backspace,
	ImuiInputKey_Tab,
	ImuiInputKey_Space,

	ImuiInputKey_LeftShift,
	ImuiInputKey_RightShift,
	ImuiInputKey_LeftControl,
	ImuiInputKey_RightControl,
	ImuiInputKey_LeftAlt,
	ImuiInputKey_RightAlt,

	ImuiInputKey_Minus,
	ImuiInputKey_Equals,
	ImuiInputKey_LeftBracket,
	ImuiInputKey_RightBracket,
	ImuiInputKey_Backslash,
	ImuiInputKey_Semicolon,
	ImuiInputKey_Apostrophe,
	ImuiInputKey_Grave,
	ImuiInputKey_Comma,
	ImuiInputKey_Period,
	ImuiInputKey_Slash,

	ImuiInputKey_F1,
	ImuiInputKey_F2,
	ImuiInputKey_F3,
	ImuiInputKey_F4,
	ImuiInputKey_F5,
	ImuiInputKey_F6,
	ImuiInputKey_F7,
	ImuiInputKey_F8,
	ImuiInputKey_F9,
	ImuiInputKey_F10,
	ImuiInputKey_F11,
	ImuiInputKey_F12,

	ImuiInputKey_Print,
	ImuiInputKey_Pause,

	ImuiInputKey_Insert,
	ImuiInputKey_Delete,
	ImuiInputKey_Home,
	ImuiInputKey_End,
	ImuiInputKey_PageUp,
	ImuiInputKey_PageDown,

	ImuiInputKey_Up,
	ImuiInputKey_Left,
	ImuiInputKey_Down,
	ImuiInputKey_Right,

	ImuiInputKey_Numpad_Divide,
	ImuiInputKey_Numpad_Multiply,
	ImuiInputKey_Numpad_Minus,
	ImuiInputKey_Numpad_Plus,
	ImuiInputKey_Numpad_Enter,
	ImuiInputKey_Numpad_1,
	ImuiInputKey_Numpad_2,
	ImuiInputKey_Numpad_3,
	ImuiInputKey_Numpad_4,
	ImuiInputKey_Numpad_5,
	ImuiInputKey_Numpad_6,
	ImuiInputKey_Numpad_7,
	ImuiInputKey_Numpad_8,
	ImuiInputKey_Numpad_9,
	ImuiInputKey_Numpad_0,
	ImuiInputKey_Numpad_Period,

	ImuiInputKey_Gamepad_Dpad_Up,
	ImuiInputKey_Gamepad_Dpad_Down,
	ImuiInputKey_Gamepad_Dpad_Left,
	ImuiInputKey_Gamepad_Dpad_Right,
	ImuiInputKey_Gamepad_Start,
	ImuiInputKey_Gamepad_Back,
	ImuiInputKey_Gamepad_LeftThumb,
	ImuiInputKey_Gamepad_RightThumb,
	ImuiInputKey_Gamepad_LeftShoulder,
	ImuiInputKey_Gamepad_RightShoulder,
	ImuiInputKey_Gamepad_A,
	ImuiInputKey_Gamepad_B,
	ImuiInputKey_Gamepad_X,
	ImuiInputKey_Gamepad_Y,

	ImuiInputKey_MAX
} ImuiInputKey;

typedef enum ImuiInputModifier
{
	ImuiInputModifier_None			= 0u,
	ImuiInputModifier_LeftShift		= 1u << 0u,
	ImuiInputModifier_RightShift	= 1u << 1u,
	ImuiInputModifier_LeftCtrl		= 1u << 2u,
	ImuiInputModifier_RightCtrl		= 1u << 3u,
	ImuiInputModifier_LeftAlt		= 1u << 4u,
	ImuiInputModifier_RightAlt		= 1u << 5u,
} ImuiInputModifier;

typedef enum ImuiInputMouseCursor
{
	ImuiInputMouseCursor_Arrow,
	ImuiInputMouseCursor_Wait,
	ImuiInputMouseCursor_WaitArrow,
	ImuiInputMouseCursor_IBeam,
	ImuiInputMouseCursor_Crosshair,
	ImuiInputMouseCursor_Hand,
	ImuiInputMouseCursor_ResizeNorthwestSoutheast,
	ImuiInputMouseCursor_ResizeNortheastSouthwest,
	ImuiInputMouseCursor_ResizeWestEast,
	ImuiInputMouseCursor_ResizeNorthSouth,
	ImuiInputMouseCursor_Move,

	ImuiInputMouseCursor_MAX
} ImuiInputMouseCursor;

typedef enum ImuiInputShortcut
{
	ImuiInputShortcut_None,
	ImuiInputShortcut_Confirm,
	ImuiInputShortcut_Back,
	ImuiInputShortcut_ToggleInsertReplace,
	ImuiInputShortcut_Home,
	ImuiInputShortcut_End,
	ImuiInputShortcut_Undo,
	ImuiInputShortcut_Redo,
	ImuiInputShortcut_Cut,
	ImuiInputShortcut_Copy,			// use imuiInputGetCopyText to set text to copy
	ImuiInputShortcut_Paste,		// call imuiInputGetPasteText before UI tick to set text to paste
	ImuiInputShortcut_SelectAll,
	ImuiInputShortcut_Backward,
	ImuiInputShortcut_Forward,
	ImuiInputShortcut_FocusNext,
	ImuiInputShortcut_FocusPrevious
} ImuiInputShortcut;

struct ImuiInputShortcutConfig
{
	ImuiInputShortcut			type;
	uint32_t					modifiers;	// imuiInputModifier
	ImuiInputKey				key;
};

// Get/Set

ImuiInputMouseCursor			imuiInputGetMouseCursor( const ImuiContext* imui );
void							imuiInputSetMouseCursor( ImuiContext* imui, ImuiInputMouseCursor cursor );

const char*						imuiInputGetCopyText( const ImuiContext* imui );
void							imuiInputSetCopyText( ImuiContext* imui, const char* text, size_t textLength );
const char*						imuiInputGetPasteText( const ImuiContext* imui );
void							imuiInputSetPasteText( ImuiContext* imui, const char* text );
char*							imuiInputBeginWritePasteText( ImuiContext* imui, size_t maxLength );
void							imuiInputEndWritePasteText( ImuiContext* imui, size_t finalLength );

// Push

ImuiInput*						imuiInputBegin( ImuiContext* imui, const ImuiInputState* previousState );
// A imuiInputState is valid for exactly two ticks and get freed automatically afterwards
const ImuiInputState*			imuiInputEnd( ImuiContext* imui );

const ImuiInputState*			imuiInputGetPushState( ImuiInput* input );

void							imuiInputPushKeyDown( ImuiInput* input, ImuiInputKey key );
void							imuiInputPushKeyUp( ImuiInput* input, ImuiInputKey key );
void							imuiInputPushKeyRepeat( ImuiInput* input, ImuiInputKey key );
void							imuiInputPushText( ImuiInput* input, const char* text );
void							imuiInputPushTextChar( ImuiInput* input, uint32_t c );

void							imuiInputPushMouseDown( ImuiInput* input, ImuiInputMouseButton button );
void							imuiInputPushMouseUp( ImuiInput* input, ImuiInputMouseButton button );
void							imuiInputPushMouseDoubleClick( ImuiInput* input, ImuiInputMouseButton button );
void							imuiInputPushMouseMove( ImuiInput* input, float x, float y );
void							imuiInputPushMouseMoveDelta( ImuiInput* input, float deltaX, float deltaY );
void							imuiInputPushMouseScroll( ImuiInput* input, float horizontalOffset, float verticalOffset );
void							imuiInputPushMouseScrollDelta( ImuiInput* input, float horizontalDelta, float verticalDelta );

void							imuiInputPushDirection( ImuiInput* input, float x, float y );
void							imuiInputPushFocusExecute( ImuiInput* input );

// Read

uint32_t						imuiInputGetKeyModifiers( const ImuiInputState* input );	// returns imuiInputModifier
bool							imuiInputIsKeyDown( const ImuiInputState* input, ImuiInputKey key );
bool							imuiInputIsKeyUp( const ImuiInputState* input, ImuiInputKey key );
bool							imuiInputHasKeyPressed( const ImuiInputState* input, ImuiInputKey key );
bool							imuiInputHasKeyReleased( const ImuiInputState* input, ImuiInputKey key );
ImuiInputShortcut				imuiInputGetShortcut( const ImuiInputState* input );

const char*						imuiInputGetText( const ImuiInputState* input );

ImuiPos							imuiInputGetMousePos( const ImuiInputState* input );
bool							imuiInputIsMouseInRect( const ImuiInputState* input, ImuiRect rect );
bool							imuiInputIsMouseButtonDown( const ImuiInputState* input, ImuiInputMouseButton button );
bool							imuiInputIsMouseButtonUp( const ImuiInputState* input, ImuiInputMouseButton button );
bool							imuiInputHasMouseButtonPressed( const ImuiInputState* input, ImuiInputMouseButton button );
bool							imuiInputHasMouseButtonReleased( const ImuiInputState* input, ImuiInputMouseButton button );
bool							imuiInputHasMouseButtonDoubleClicked( const ImuiInputState* input, ImuiInputMouseButton button );
ImuiPos							imuiInputGetMouseScrollDelta( const ImuiInputState* input );

ImuiPos							imuiInputGetDirection( const ImuiInputState* input );
bool							imuiInputGetFocusExecute( const ImuiInputState* input );

//////////////////////////////////////////////////////////////////////////
// Font
// see imui_font.c

typedef struct ImuiFont ImuiFont;
typedef struct ImuiFontTrueTypeData ImuiFontTrueTypeData;
typedef struct ImuiFontTrueTypeImage ImuiFontTrueTypeImage;

typedef struct ImuiFontCodepoint
{
	uint32_t					codepoint;
	float						width;
	float						height;
	float						advance;
	float						xOffset;
	float						ascentOffset;
	ImuiTexCoord				uv;
} ImuiFontCodepoint;

typedef struct ImuiFontParameters
{
	ImuiImage					image;

	const ImuiFontCodepoint*	codepoints;
	size_t						codepointCount;

	float						fontSize;		// for scalable fonts this is the default size when no one is given
	float						lineGap;
	bool						isScalable;
} ImuiFontParameters;

ImuiFont*						imuiFontCreate( ImuiContext* imui, const ImuiFontParameters* parameters );
ImuiFont*						imuiFontCreateTrueType( ImuiContext* imui, ImuiFontTrueTypeImage* ttfImage, ImuiImage image );
void							imuiFontDestroy( ImuiContext* imui, ImuiFont* font );

ImuiFontTrueTypeData*			imuiFontTrueTypeDataCreate( ImuiContext* imui, const void* data, size_t dataSize  ); // data must stay valid
ImuiFontTrueTypeData*			imuiFontTrueTypeDataCreateCopy( ImuiContext* imui, const void* data, size_t dataSize ); // data copied into an internal buffer
void							imuiFontTrueTypeDataDestroy( ImuiFontTrueTypeData* ttf );
bool							imuiFontTrueTypeDataAddCodepoints( ImuiFontTrueTypeData* ttf, const uint32_t* codepoints, size_t codepointCount );
bool							imuiFontTrueTypeDataAddCodepointRange( ImuiFontTrueTypeData* ttf, uint32_t firstCodepoint, uint32_t lastCodepoint );
void							imuiFontTrueTypeDataCalculateMinTextureSize( ImuiFontTrueTypeData* ttf, float fontSizeInPixel, uint32_t* targetWidth, uint32_t* targetHeight );
void							imuiFontTrueTypeDataCalculateMinSDFTextureSize( ImuiFontTrueTypeData* ttf, float fontSizeInPixel, uint32_t* targetWidth, uint32_t* targetHeight, float sdfSpread );
ImuiFontTrueTypeImage*			imuiFontTrueTypeDataGenerateTextureData( ImuiFontTrueTypeData* ttf, float fontSizeInPixel, void* targetData, size_t targetDataSize, uint32_t width, uint32_t height );
ImuiFontTrueTypeImage*			imuiFontTrueTypeDataGenerateSDFTextureData( ImuiFontTrueTypeData* ttf, float fontSizeInPixel, void* targetData, size_t targetDataSize, uint32_t width, uint32_t height, float sdfSpread );

void							imuiFontTrueTypeImageGetCodepoints( ImuiFontTrueTypeImage* ttfImage, const ImuiFontCodepoint** codepoints, size_t* codepointCount );
void							imuiFontTrueTypeImageDestroy( ImuiFontTrueTypeImage* ttfImage );

//////////////////////////////////////////////////////////////////////////
// Text
// see imui_text.c

ImuiTextLayout*					imuiTextLayoutCreate( ImuiContext* imui, ImuiFont* font, const char* text );
ImuiTextLayout*					imuiTextLayoutCreateLength( ImuiContext* imui, ImuiFont* font, const char* text, size_t length );
ImuiTextLayout*					imuiTextLayoutCreateWidget( ImuiWidget* widget, ImuiFont* font, const char* text );
ImuiTextLayout*					imuiTextLayoutCreateWidgetLength( ImuiWidget* widget, ImuiFont* font, const char* text, size_t length );

size_t							imuiTextLayoutCalculateGlyphCount( const char* text, size_t length );
ImuiSize						imuiTextLayoutCalculateSize( ImuiContext* imui, ImuiFont* font, const char* text, size_t length );

size_t							imuiTextLayoutGetGlyphCount( const ImuiTextLayout* layout );
size_t							imuiTextLayoutFindGlyphIndex( const ImuiTextLayout* layout, ImuiPos pos, float scale );
size_t							imuiTextLayoutGetGlyphCharIndex( const ImuiTextLayout* layout, size_t glyphIndex );
ImuiSize						imuiTextLayoutGetSize( const ImuiTextLayout* layout );
ImuiPos							imuiTextLayoutGetGlyphPos( const ImuiTextLayout* layout, size_t glyphIndex, float scale );

//////////////////////////////////////////////////////////////////////////
// Data Type Functions
// see imui_data_types.c

ImuiHash						imuiHashCreate( const void* data, size_t dataSize );
ImuiHash						imuiHashCreateSeed( const void* data, size_t dataSize, ImuiHash seed );
ImuiHash						imuiHashMix( ImuiHash hash1, ImuiHash hash2 );

ImuiPos							imuiPosCreate( float x, float y );
ImuiPos							imuiPosCreateZero();
ImuiPos							imuiPosAdd( ImuiPos pos, float x, float y );
ImuiPos							imuiPosAddPos( ImuiPos pos, ImuiPos add );
ImuiPos							imuiPosSub( ImuiPos pos, float x, float y );
ImuiPos							imuiPosSubPos( ImuiPos pos, ImuiPos sub );
ImuiPos							imuiPosScale( ImuiPos pos, float factor );
ImuiPos							imuiPosMin( ImuiPos a, ImuiPos b );
ImuiPos							imuiPosMax( ImuiPos a, ImuiPos b );

ImuiSize						imuiSizeCreate( float width, float height );
ImuiSize						imuiSizeCreateAll( float value );
ImuiSize						imuiSizeCreateOne();
ImuiSize						imuiSizeCreateZero();
ImuiSize						imuiSizeCreateSkin( const ImuiSkin* skin );
ImuiSize						imuiSizeCreateImage( const ImuiImage* image );
ImuiSize						imuiSizeAdd( ImuiSize size, float width, float height );
ImuiSize						imuiSizeAddSize( ImuiSize size, ImuiSize add );
ImuiSize						imuiSizeSub( ImuiSize size, float width, float height );
ImuiSize						imuiSizeSubSize( ImuiSize size, ImuiSize sub );
ImuiSize						imuiSizeScale( ImuiSize size, float factor );
ImuiSize						imuiSizeShrinkBorder( ImuiSize size, ImuiBorder border );
ImuiSize						imuiSizeExpandBorder( ImuiSize size, ImuiBorder border );
ImuiSize						imuiSizeLerp( ImuiSize a, ImuiSize b, float t );
ImuiSize						imuiSizeLerp2( ImuiSize a, ImuiSize b, float widthT, float heightT );
ImuiSize						imuiSizeMin( ImuiSize a, ImuiSize b );
ImuiSize						imuiSizeMax( ImuiSize a, ImuiSize b );
ImuiSize						imuiSizeFloor( ImuiSize size );
ImuiSize						imuiSizeCeil( ImuiSize size );
ImuiPos							imuiSizeToPos( ImuiSize size );

ImuiBorder						imuiBorderCreate( float top, float left, float bottom, float right );
ImuiBorder						imuiBorderCreateAll( float all );
ImuiBorder						imuiBorderCreateZero();
ImuiBorder						imuiBorderCreateHorizontalVertical( float horizontal, float vertical );
ImuiBorder						imuiBorderScale( ImuiBorder border, float factor );
ImuiSize						imuiBorderGetMinSize( ImuiBorder border );

ImuiRect						imuiRectCreate( float x, float y, float width, float height );
ImuiRect						imuiRectCreatePos( ImuiPos pos, float width, float height );
ImuiRect						imuiRectCreateSize( float x, float y, ImuiSize size );
ImuiRect						imuiRectCreatePosSize( ImuiPos pos, ImuiSize size );
ImuiRect						imuiRectCreateMinMax( float minX, float minY, float maxX, float maxY );
ImuiRect						imuiRectCreateMinMaxPos( ImuiPos tl, ImuiPos br );
ImuiRect						imuiRectCreateCenter( float x, float y, float width, float height );
ImuiRect						imuiRectCreateCenterPos( ImuiPos pos, float width, float height );
ImuiRect						imuiRectCreateCenterSize( float x, float y, ImuiSize size );
ImuiRect						imuiRectCreateCenterPosSize( ImuiPos pos, ImuiSize size );
ImuiRect						imuiRectCreateZero();
ImuiRect						imuiRectShrinkBorder( ImuiRect rect, ImuiBorder border );
ImuiRect						imuiRectIntersection( ImuiRect rect1, ImuiRect rect2 );
bool							imuiRectIncludesPos( ImuiRect rect, ImuiPos pos );
bool							imuiRectIntersectsRect( ImuiRect rect1, ImuiRect rect2 );
ImuiPos							imuiRectGetTopLeft( ImuiRect rect );
ImuiPos							imuiRectGetTopRight( ImuiRect rect );
ImuiPos							imuiRectGetBottomLeft( ImuiRect rect );
ImuiPos							imuiRectGetBottomRight( ImuiRect rect );
ImuiPos							imuiRectGetCenter( ImuiRect rect );
float							imuiRectGetRight( ImuiRect rect );
float							imuiRectGetBottom( ImuiRect rect );

ImuiColor						imuiColorCreate( uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha );
ImuiColor						imuiColorCreateFloat( float red, float green, float blue, float alpha );
ImuiColor						imuiColorCreateBlack();
ImuiColor						imuiColorCreateBlackA( uint8_t alpha );
ImuiColor						imuiColorCreateWhite();
ImuiColor						imuiColorCreateWhiteA( uint8_t alpha );
ImuiColor						imuiColorCreateGray( uint8_t gray );
ImuiColor						imuiColorCreateGrayA( uint8_t gray, uint8_t alpha );
ImuiColor						imuiColorCreateTransparentBlack();

#ifdef __cplusplus
}
#endif
