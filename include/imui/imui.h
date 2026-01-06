#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define IMUI_ID_DEFAULT			0
#define IMUI_ID_STR( STR )		(ImUiId)(size_t)(STR)
#define IMUI_ID_TYPE( TYPE )	(ImUiId)(size_t)(#TYPE)

typedef struct ImUiContext ImUiContext;
typedef struct ImUiDraw ImUiDraw;
typedef struct ImUiFrame ImUiFrame;
typedef struct ImUiInput ImUiInput;
typedef struct ImUiInputShortcutConfig ImUiInputShortcutConfig;
typedef struct ImUiSurface ImUiSurface;
typedef struct ImUiTextLayout ImUiTextLayout;
typedef struct ImUiWidget ImUiWidget;
typedef struct ImUiWindow ImUiWindow;

typedef uint32_t ImUiId;
typedef uint32_t ImUiHash;

typedef void*(*ImUiAllocatorMallocFunc)(size_t size, void* userData);
typedef void*(*ImUiAllocatorReallocFunc)(void* memory, size_t oldSize, size_t newSize, void* userData);
typedef void( *ImUiAllocatorFreeFunc )(void* memory, void* userData);

typedef struct ImUiAllocator
{
	ImUiAllocatorMallocFunc		mallocFunc;		// set to NULL to use default malloc/free
	ImUiAllocatorReallocFunc	reallocFunc;	// can be NULL
	ImUiAllocatorFreeFunc		freeFunc;
	void*						userData;
	void*						internalData;	// internal use only
} ImUiAllocator;

typedef void(*ImUiStateDestructFunc)(void* state);

// needed?
//typedef bool(*ImUiWindowEvalFocusFunc)(ImUiWindow* window, ImUiWidget* widget, void* userData);

typedef enum ImUiVertexElementType
{
	ImUiVertexElementType_Float,
	ImUiVertexElementType_Float2,
	ImUiVertexElementType_Float3,
	ImUiVertexElementType_Float4,

	ImUiVertexElementType_Int,
	ImUiVertexElementType_Int2,
	ImUiVertexElementType_Int3,
	ImUiVertexElementType_Int4,

	ImUiVertexElementType_UInt,
	ImUiVertexElementType_UInt2,
	ImUiVertexElementType_UInt3,
	ImUiVertexElementType_UInt4,

	ImUiVertexElementType_MAX
} ImUiVertexElementType;

typedef enum ImUiVertexElementSemantic
{
	ImUiVertexElementSemantic_Zero,
	ImUiVertexElementSemantic_PositionScreenSpace,
	ImUiVertexElementSemantic_PositionClipSpace,
	ImUiVertexElementSemantic_TextureCoordinate,
	ImUiVertexElementSemantic_ColorRGBA,
	ImUiVertexElementSemantic_ColorABGR
} ImUiVertexElementSemantic;

typedef struct ImUiVertexElement
{
	uint32_t					align;
	ImUiVertexElementType		type;
	ImUiVertexElementSemantic	semantic;
} ImUiVertexElement;

typedef struct ImUiVertexFormat
{
	const ImUiVertexElement*	elements;
	size_t						elementCount;
} ImUiVertexFormat;

typedef enum ImUiVertexType
{
	ImUiVertexType_VertexList,
	ImUiVertexType_IndexedVertexList
} ImUiVertexType;

typedef struct ImUiParameters					// Fill with zero for default parameters
{
	ImUiAllocator					allocator;			// Override memory Allocator. Default use malloc/free
	const ImUiInputShortcutConfig*	shortcuts;			// Define keyboard shortcuts
	size_t							shortcutCount;
	ImUiVertexFormat				vertexFormat;		// Override vertex format. Default: float2 pos screen-space, float2 uv, float4 color
	ImUiVertexType					vertexType;			// Override vertex type, Default: ImUiVertexType_VertexList
} ImUiParameters;

ImUiContext*				ImUiCreate( const ImUiParameters* parameters );
void						ImUiDestroy( ImUiContext* imui );

ImUiFrame*					ImUiBegin( ImUiContext* imui, double timeInSeconds );
void						ImUiEnd( ImUiFrame* frame );

ImUiContext*				ImUiFrameGetContext( const ImUiFrame* frame );

//////////////////////////////////////////////////////////////////////////
// Types

#define IMUI_TEXTURE_HANDLE_INVALID 0

typedef struct ImUiBorder
{
	float					top;
	float					left;
	float					bottom;
	float					right;
} ImUiBorder;

typedef struct ImUiColor
{
	uint8_t					red;
	uint8_t					green;
	uint8_t					blue;
	uint8_t					alpha;
} ImUiColor;

typedef struct ImUiPos
{
	float					x;
	float					y;
} ImUiPos;

typedef struct ImUiSize
{
	float					width;
	float					height;
} ImUiSize;

typedef struct ImUiRect
{
	ImUiPos					pos;
	ImUiSize				size;
} ImUiRect;

typedef struct ImUiTexCoord
{
	float					u0;
	float					v0;
	float					u1;
	float					v1;
} ImUiTexCoord;

typedef struct ImUiImage
{
	uint64_t				textureHandle;
	uint32_t				width;
	uint32_t				height;
	ImUiTexCoord			uv;
} ImUiImage;

typedef struct ImUiSkin
{
	uint64_t				textureHandle;
	uint32_t				width;
	uint32_t				height;
	ImUiTexCoord			uv;
	ImUiBorder				border;
} ImUiSkin;

typedef enum ImUiLayout
{
	ImUiLayout_Stack,
	ImUiLayout_Scroll,
	ImUiLayout_Horizontal,
	ImUiLayout_Vertical,
	ImUiLayout_Grid
} ImUiLayout;

typedef enum ImUiDrawTopology
{
	ImUiDrawTopology_LineList,
	ImUiDrawTopology_TriangleList,
	ImUiDrawTopology_IndexedTriangleList,

	ImUiDrawTopology_MAX
} ImUiDrawTopology;

typedef struct ImUiDrawCommand
{
	ImUiDrawTopology		topology;
	uint64_t				textureHandle;
	ImUiRect				clipRect;
	size_t					count;				// index count if index buffer is used otherwise vertex count
} ImUiDrawCommand;

typedef struct ImUiDrawData
{
	const ImUiDrawCommand*	commands;
	size_t					commandCount;
} ImUiDrawData;

//////////////////////////////////////////////////////////////////////////
// Surface - Presents a OS window or a screen

ImUiSurface*				ImUiSurfaceBegin( ImUiFrame* frame, const char* name, ImUiSize size, float dpiScale );
void						ImUiSurfaceEnd( ImUiSurface* surface );

ImUiContext*				ImUiSurfaceGetContext( const ImUiSurface* surface );

double						ImUiSurfaceGetTime( const ImUiSurface* surface );
ImUiSize					ImUiSurfaceGetSize( const ImUiSurface* surface );
ImUiRect					ImUiSurfaceGetRect( const ImUiSurface* surface );
float						ImUiSurfaceGetDpiScale( const ImUiSurface* surface );

// call after surface end but before end frame
void						ImUiSurfaceGetMaxBufferSizes( ImUiSurface* surface, size_t* outVertexDataSize, size_t* outIndexDataSize );
const ImUiDrawData*			ImUiSurfaceGenerateDrawData( ImUiSurface* surface, void* outVertexData, size_t* inOutVertexDataSize, void* outIndexData, size_t* inOutIndexDataSize );


//////////////////////////////////////////////////////////////////////////
// Window - A part of a Surface with z ordering

ImUiWindow*					ImUiWindowBegin( ImUiSurface* surface, const char* name, ImUiRect rect, uint32_t zOrder );
void						ImUiWindowEnd( ImUiWindow* window );

ImUiContext*				ImUiWindowGetContext( const ImUiWindow* window );
ImUiSurface*				ImUiWindowGetSurface( const ImUiWindow* window );

double						ImUiWindowGetTime( const ImUiWindow* window );
float						ImUiWindowGetDpiScale( const ImUiWindow* window );
ImUiRect					ImUiWindowGetRect( const ImUiWindow* window );

bool						ImUiWindowHasFocus( const ImUiWindow* window );
void						ImUiWindowSetFocus( ImUiWindow* window, float angleThreshold, bool wrap );

bool						ImUiWindowIsWidgetFocusLocked( const ImUiWindow* window );
void						ImUiWindowSetWidgetFocusLock( ImUiWindow* window, bool locked );
void						ImUiWindowClearWidgetFocus( ImUiWindow* window );
const ImUiWidget*			ImUiWindowGetFocusWidget( const ImUiWindow* window );
const ImUiWidget*			ImUiWindowPeekFocusWidget( const ImUiWindow* window );

void*						ImUiWindowAllocState( ImUiWindow* window, size_t size, ImUiId stateId );
void*						ImUiWindowAllocStateNew( ImUiWindow* window, size_t size, ImUiId stateId, bool* isNew );
void*						ImUiWindowAllocStateNewDestruct( ImUiWindow* window, size_t size, ImUiId stateId, bool* isNew, ImUiStateDestructFunc destructFunc );

ImUiWidget*					ImUiWindowGetFirstChild( const ImUiWindow* window );
ImUiWidget*					ImUiWindowGetLastChild( const ImUiWindow* window );

//////////////////////////////////////////////////////////////////////////
// Widget - A layout element in the tree

typedef struct ImUiWidgetInputState
{
	ImUiPos					relativeMousePos;

	bool					hasFocus;
	bool					wasPressed;
	bool					wasMouseOver;
	bool					isMouseOver;
	bool					isMouseDown;
	bool					hasMousePressed;
	bool					hasMouseReleased;
} ImUiWidgetInputState;

ImUiWidget*					ImUiWidgetBegin( ImUiWindow* window );
ImUiWidget*					ImUiWidgetBeginId( ImUiWindow* window, ImUiId id );
ImUiWidget*					ImUiWidgetBeginNamed( ImUiWindow* window, const char* name );
void						ImUiWidgetEnd( ImUiWidget* widget );

ImUiContext*				ImUiWidgetGetContext( const ImUiWidget* widget );
ImUiSurface*				ImUiWidgetGetSurface( const ImUiWidget* widget );
ImUiWindow*					ImUiWidgetGetWindow( const ImUiWidget* widget );

ImUiWidget*					ImUiWidgetGetParent( const ImUiWidget* widget );
ImUiWidget*					ImUiWidgetGetFirstChild( const ImUiWidget* widget );
ImUiWidget*					ImUiWidgetGetLastChild( const ImUiWidget* widget );
ImUiWidget*					ImUiWidgetGetPrevSibling( const ImUiWidget* widget );
ImUiWidget*					ImUiWidgetGetNextSibling( const ImUiWidget* widget );

double						ImUiWidgetGetTime( const ImUiWidget* widget );
float						ImUiWidgetGetDpiScale( const ImUiWidget* widget );

void*						ImUiWidgetAllocState( ImUiWidget* widget, size_t size, ImUiId stateId );
void*						ImUiWidgetAllocStateNew( ImUiWidget* widget, size_t size, ImUiId stateId, bool* isNew );
void*						ImUiWidgetAllocStateNewDestruct( ImUiWidget* widget, size_t size, ImUiId stateId, bool* isNew, ImUiStateDestructFunc destructFunc );

ImUiLayout					ImUiWidgetGetLayout( const ImUiWidget* widget );
void						ImUiWidgetSetLayoutStack( ImUiWidget* widget );		// default
void						ImUiWidgetSetLayoutScroll( ImUiWidget* widget, float offsetX, float offsetY );
void						ImUiWidgetSetLayoutHorizontal( ImUiWidget* widget );
void						ImUiWidgetSetLayoutHorizontalSpacing( ImUiWidget* widget, float spacing );
void						ImUiWidgetSetLayoutVertical( ImUiWidget* widget );
void						ImUiWidgetSetLayoutVerticalSpacing( ImUiWidget* widget, float spacing );
void						ImUiWidgetSetLayoutGrid( ImUiWidget* widget, uint32_t columnCount, float colSpacing, float rowSpacing );

ImUiBorder					ImUiWidgetGetMargin( const ImUiWidget* widget );
void						ImUiWidgetSetMargin( ImUiWidget* widget, ImUiBorder margin );
ImUiBorder					ImUiWidgetGetPadding( const ImUiWidget* widget );
void						ImUiWidgetSetPadding( ImUiWidget* widget, ImUiBorder padding );

ImUiSize					ImUiWidgetGetMinSize( const ImUiWidget* widget );
void						ImUiWidgetSetMinWidth( ImUiWidget* widget, float value );
void						ImUiWidgetSetMinHeight( ImUiWidget* widget, float value );
void						ImUiWidgetSetMinSize( ImUiWidget* widget, ImUiSize size );
void						ImUiWidgetSetMinSizeFloat( ImUiWidget* widget, float width, float height );
ImUiSize					ImUiWidgetGetMaxSize( const ImUiWidget* widget );
void						ImUiWidgetSetMaxWidth( ImUiWidget* widget, float value );
void						ImUiWidgetSetMaxHeight( ImUiWidget* widget, float value );
void						ImUiWidgetSetMaxSize( ImUiWidget* widget, ImUiSize size );
void						ImUiWidgetSetMaxSizeFloat( ImUiWidget* widget, float width, float height );
void						ImUiWidgetSetFixedWidth( ImUiWidget* widget, float value );
void						ImUiWidgetSetFixedHeight( ImUiWidget* widget, float value );
void						ImUiWidgetSetFixedSize( ImUiWidget* widget, ImUiSize size );
void						ImUiWidgetSetFixedSizeFloat( ImUiWidget* widget, float width, float height );

void						ImUiWidgetSetStretch( ImUiWidget* widget, float horizontal, float vertical );
void						ImUiWidgetSetStretchOne( ImUiWidget* widget );
float						ImUiWidgetGetHStretch( const ImUiWidget* widget );
void						ImUiWidgetSetHStretch( ImUiWidget* widget, float stretch );
float						ImUiWidgetGetVStretch( const ImUiWidget* widget );
void						ImUiWidgetSetVStretch( ImUiWidget* widget, float stretch );

void						ImUiWidgetSetAlign( ImUiWidget* widget, float horizontal, float vertical );
float						ImUiWidgetGetHAlign( const ImUiWidget* widget );
void						ImUiWidgetSetHAlign( ImUiWidget* widget, float align );
float						ImUiWidgetGetVAlign( const ImUiWidget* widget );
void						ImUiWidgetSetVAlign( ImUiWidget* widget, float align );

bool						ImUiWidgetHasFocus( const ImUiWidget* widget );
void						ImUiWidgetSetFocus( ImUiWidget* widget );
bool						ImUiWidgetGetCanHaveFocus( const ImUiWidget* widget );
void						ImUiWidgetSetCanHaveFocus( ImUiWidget* widget );
void						ImUiWidgetSetCanHaveFocusIndex( ImUiWidget* widget, uint32_t focusIndex );

ImUiPos						ImUiWidgetGetPos( const ImUiWidget* widget );
float						ImUiWidgetGetPosX( const ImUiWidget* widget );
float						ImUiWidgetGetPosY( const ImUiWidget* widget );
ImUiSize					ImUiWidgetGetSize( const ImUiWidget* widget );
float						ImUiWidgetGetSizeWidth( const ImUiWidget* widget );
float						ImUiWidgetGetSizeHeight( const ImUiWidget* widget );
ImUiRect					ImUiWidgetGetRect( const ImUiWidget* widget );
ImUiSize					ImUiWidgetGetInnerSize( const ImUiWidget* widget );
ImUiRect					ImUiWidgetGetInnerRect( const ImUiWidget* widget );

void						ImUiWidgetGetInputState( ImUiWidget* widget, ImUiWidgetInputState* target );

// all positions are relative to the widget
void						ImUiWidgetDrawColor( ImUiWidget* widget, ImUiColor color );
void						ImUiWidgetDrawImage( ImUiWidget* widget, const ImUiImage* image);
void						ImUiWidgetDrawImageColor( ImUiWidget* widget, const ImUiImage* image, ImUiColor color );
void						ImUiWidgetDrawSkin( ImUiWidget* widget, const ImUiSkin* skin, ImUiColor color );
void						ImUiWidgetDrawText( ImUiWidget* widget, ImUiTextLayout* layout, ImUiColor color );
void						ImUiWidgetDrawTextSize( ImUiWidget* widget, ImUiTextLayout* layout, ImUiColor color, float size );
void						ImUiWidgetDrawPartialColor( ImUiWidget* widget, ImUiRect relativRect, ImUiColor color );
void						ImUiWidgetDrawPartialImage( ImUiWidget* widget, ImUiRect relativRect, const ImUiImage* image );
void						ImUiWidgetDrawPartialImageColor( ImUiWidget* widget, ImUiRect relativRect, const ImUiImage* image, ImUiColor color );
void						ImUiWidgetDrawPartialSkin( ImUiWidget* widget, ImUiRect relativRect, const ImUiSkin* skin, ImUiColor color );
void						ImUiWidgetDrawPositionText( ImUiWidget* widget, ImUiPos offset, ImUiTextLayout* layout, ImUiColor color );
void						ImUiWidgetDrawPositionTextSize( ImUiWidget* widget, ImUiPos offset, ImUiTextLayout* layout, ImUiColor color, float size );
void						ImUiWidgetDrawLine( ImUiWidget* widget, ImUiPos p0, ImUiPos p1, ImUiColor color );
void						ImUiWidgetDrawTriangle( ImUiWidget* widget, ImUiPos p0, ImUiPos p1, ImUiPos p2, ImUiColor color );

//////////////////////////////////////////////////////////////////////////
// Input
// see imui_input.c

typedef enum ImUiInputMouseButton
{
	ImUiInputMouseButton_Left,
	ImUiInputMouseButton_Right,
	ImUiInputMouseButton_Middle,

	ImUiInputMouseButton_X1,
	ImUiInputMouseButton_X2,

	ImUiInputMouseButton_MAX
} ImUiInputMouseButton;

typedef enum ImUiInputKey
{
	ImUiInputKey_None,

	ImUiInputKey_A,
	ImUiInputKey_B,
	ImUiInputKey_C,
	ImUiInputKey_D,
	ImUiInputKey_E,
	ImUiInputKey_F,
	ImUiInputKey_G,
	ImUiInputKey_H,
	ImUiInputKey_I,
	ImUiInputKey_J,
	ImUiInputKey_K,
	ImUiInputKey_L,
	ImUiInputKey_M,
	ImUiInputKey_N,
	ImUiInputKey_O,
	ImUiInputKey_P,
	ImUiInputKey_Q,
	ImUiInputKey_R,
	ImUiInputKey_S,
	ImUiInputKey_T,
	ImUiInputKey_U,
	ImUiInputKey_V,
	ImUiInputKey_W,
	ImUiInputKey_X,
	ImUiInputKey_Y,
	ImUiInputKey_Z,

	ImUiInputKey_1,
	ImUiInputKey_2,
	ImUiInputKey_3,
	ImUiInputKey_4,
	ImUiInputKey_5,
	ImUiInputKey_6,
	ImUiInputKey_7,
	ImUiInputKey_8,
	ImUiInputKey_9,
	ImUiInputKey_0,

	ImUiInputKey_Enter,
	ImUiInputKey_Escape,
	ImUiInputKey_Backspace,
	ImUiInputKey_Tab,
	ImUiInputKey_Space,

	ImUiInputKey_LeftShift,
	ImUiInputKey_RightShift,
	ImUiInputKey_LeftControl,
	ImUiInputKey_RightControl,
	ImUiInputKey_LeftAlt,
	ImUiInputKey_RightAlt,

	ImUiInputKey_Minus,
	ImUiInputKey_Equals,
	ImUiInputKey_LeftBracket,
	ImUiInputKey_RightBracket,
	ImUiInputKey_Backslash,
	ImUiInputKey_Semicolon,
	ImUiInputKey_Apostrophe,
	ImUiInputKey_Grave,
	ImUiInputKey_Comma,
	ImUiInputKey_Period,
	ImUiInputKey_Slash,

	ImUiInputKey_F1,
	ImUiInputKey_F2,
	ImUiInputKey_F3,
	ImUiInputKey_F4,
	ImUiInputKey_F5,
	ImUiInputKey_F6,
	ImUiInputKey_F7,
	ImUiInputKey_F8,
	ImUiInputKey_F9,
	ImUiInputKey_F10,
	ImUiInputKey_F11,
	ImUiInputKey_F12,

	ImUiInputKey_Print,
	ImUiInputKey_Pause,

	ImUiInputKey_Insert,
	ImUiInputKey_Delete,
	ImUiInputKey_Home,
	ImUiInputKey_End,
	ImUiInputKey_PageUp,
	ImUiInputKey_PageDown,

	ImUiInputKey_Up,
	ImUiInputKey_Left,
	ImUiInputKey_Down,
	ImUiInputKey_Right,

	ImUiInputKey_Numpad_Divide,
	ImUiInputKey_Numpad_Multiply,
	ImUiInputKey_Numpad_Minus,
	ImUiInputKey_Numpad_Plus,
	ImUiInputKey_Numpad_Enter,
	ImUiInputKey_Numpad_1,
	ImUiInputKey_Numpad_2,
	ImUiInputKey_Numpad_3,
	ImUiInputKey_Numpad_4,
	ImUiInputKey_Numpad_5,
	ImUiInputKey_Numpad_6,
	ImUiInputKey_Numpad_7,
	ImUiInputKey_Numpad_8,
	ImUiInputKey_Numpad_9,
	ImUiInputKey_Numpad_0,
	ImUiInputKey_Numpad_Period,

	ImUiInputKey_Gamepad_Dpad_Up,
	ImUiInputKey_Gamepad_Dpad_Down,
	ImUiInputKey_Gamepad_Dpad_Left,
	ImUiInputKey_Gamepad_Dpad_Right,
	ImUiInputKey_Gamepad_Start,
	ImUiInputKey_Gamepad_Back,
	ImUiInputKey_Gamepad_LeftThumb,
	ImUiInputKey_Gamepad_RightThumb,
	ImUiInputKey_Gamepad_LeftShoulder,
	ImUiInputKey_Gamepad_RightShoulder,
	ImUiInputKey_Gamepad_A,
	ImUiInputKey_Gamepad_B,
	ImUiInputKey_Gamepad_X,
	ImUiInputKey_Gamepad_Y,

	ImUiInputKey_MAX
} ImUiInputKey;

typedef enum ImUiInputModifier
{
	ImUiInputModifier_None			= 0u,
	ImUiInputModifier_LeftShift		= 1u << 0u,
	ImUiInputModifier_RightShift	= 1u << 1u,
	ImUiInputModifier_LeftCtrl		= 1u << 2u,
	ImUiInputModifier_RightCtrl		= 1u << 3u,
	ImUiInputModifier_LeftAlt		= 1u << 4u,
	ImUiInputModifier_RightAlt		= 1u << 5u,
} ImUiInputModifier;

typedef enum ImUiInputMouseCursor
{
	ImUiInputMouseCursor_Arrow,
	ImUiInputMouseCursor_Wait,
	ImUiInputMouseCursor_WaitArrow,
	ImUiInputMouseCursor_IBeam,
	ImUiInputMouseCursor_Crosshair,
	ImUiInputMouseCursor_Hand,
	ImUiInputMouseCursor_ResizeNorthwestSoutheast,
	ImUiInputMouseCursor_ResizeNortheastSouthwest,
	ImUiInputMouseCursor_ResizeWestEast,
	ImUiInputMouseCursor_ResizeNorthSouth,
	ImUiInputMouseCursor_Move,

	ImUiInputMouseCursor_MAX
} ImUiInputMouseCursor;

typedef enum ImUiInputShortcut
{
	ImUiInputShortcut_None,
	ImUiInputShortcut_Confirm,
	ImUiInputShortcut_Back,
	ImUiInputShortcut_ToggleInsertReplace,
	ImUiInputShortcut_Home,
	ImUiInputShortcut_End,
	ImUiInputShortcut_Undo,
	ImUiInputShortcut_Redo,
	ImUiInputShortcut_Cut,
	ImUiInputShortcut_Copy,			// use ImUiInputGetCopyText to set text to copy
	ImUiInputShortcut_Paste,		// call ImUiInputGetPasteText before UI tick to set text to paste
	ImUiInputShortcut_SelectAll,
	ImUiInputShortcut_Backward,
	ImUiInputShortcut_Forward,
	ImUiInputShortcut_FocusNext,
	ImUiInputShortcut_FocusPrevious
} ImUiInputShortcut;

struct ImUiInputShortcutConfig
{
	ImUiInputShortcut			type;
	uint32_t					modifiers;	// ImUiInputModifier
	ImUiInputKey				key;
};

// Get/Set

void							ImUiInputSetMouseCursor( ImUiContext* imui, ImUiInputMouseCursor cursor );

const char*						ImUiInputGetCopyText( const ImUiContext* imui );
void							ImUiInputSetCopyText( ImUiContext* imui, const char* text, size_t textLength );
const char*						ImUiInputGetPasteText( const ImUiContext* imui );
void							ImUiInputSetPasteText( ImUiContext* imui, const char* text );
char*							ImUiInputBeginWritePasteText( ImUiContext* imui, size_t maxLength );
void							ImUiInputEndWritePasteText( ImUiContext* imui, size_t finalLength );

// Push

ImUiInput*						ImUiInputBegin( ImUiContext* imui );
void							ImUiInputEnd( ImUiContext* imui );

void							ImUiInputPushKeyDown( ImUiInput* input, ImUiInputKey key );
void							ImUiInputPushKeyUp( ImUiInput* input, ImUiInputKey key );
void							ImUiInputPushKeyRepeat( ImUiInput* input, ImUiInputKey key );
void							ImUiInputPushText( ImUiInput* input, const char* text );
void							ImUiInputPushTextChar( ImUiInput* input, uint32_t c );

void							ImUiInputPushMouseDown( ImUiInput* input, ImUiInputMouseButton button );
void							ImUiInputPushMouseUp( ImUiInput* input, ImUiInputMouseButton button );
void							ImUiInputPushMouseDoubleClick( ImUiInput* input, ImUiInputMouseButton button );
void							ImUiInputPushMouseMove( ImUiInput* input, float x, float y );
void							ImUiInputPushMouseMoveDelta( ImUiInput* input, float deltaX, float deltaY );
void							ImUiInputPushMouseScroll( ImUiInput* input, float horizontalOffset, float verticalOffset );
void							ImUiInputPushMouseScrollDelta( ImUiInput* input, float horizontalDelta, float verticalDelta );

void							ImUiInputPushDirection( ImUiInput* input, float x, float y );
void							ImUiInputPushFocusExecute( ImUiInput* input );

// Read

uint32_t						ImUiInputGetKeyModifiers( const ImUiContext* imui );	// returns ImUiInputModifier
bool							ImUiInputIsKeyDown( const ImUiContext* imui, ImUiInputKey key );
bool							ImUiInputIsKeyUp( const ImUiContext* imui, ImUiInputKey key );
bool							ImUiInputHasKeyPressed( const ImUiContext* imui, ImUiInputKey key );
bool							ImUiInputHasKeyReleased( const ImUiContext* imui, ImUiInputKey key );
ImUiInputShortcut				ImUiInputGetShortcut( const ImUiContext* imui );

const char*						ImUiInputGetText( const ImUiContext* imui );

ImUiPos							ImUiInputGetMousePos( const ImUiContext* imui );
ImUiInputMouseCursor			ImUiInputGetMouseCursor( ImUiContext* imui );
bool							ImUiInputIsMouseInRect( const ImUiContext* imui, ImUiRect rect );
bool							ImUiInputIsMouseButtonDown( const ImUiContext* imui, ImUiInputMouseButton button );
bool							ImUiInputIsMouseButtonUp( const ImUiContext* imui, ImUiInputMouseButton button );
bool							ImUiInputHasMouseButtonPressed( const ImUiContext* imui, ImUiInputMouseButton button );
bool							ImUiInputHasMouseButtonReleased( const ImUiContext* imui, ImUiInputMouseButton button );
bool							ImUiInputHasMouseButtonDoubleClicked( const ImUiContext* imui, ImUiInputMouseButton button );
ImUiPos							ImUiInputGetMouseScrollDelta( const ImUiContext* imui );

ImUiPos							ImUiInputGetDirection( const ImUiContext* imui );
bool							ImUiInputGetFocusExecute( const ImUiContext* imui );

//////////////////////////////////////////////////////////////////////////
// Font
// see imui_font.c

typedef struct ImUiFont ImUiFont;
typedef struct ImUiFontTrueTypeData ImUiFontTrueTypeData;
typedef struct ImUiFontTrueTypeImage ImUiFontTrueTypeImage;

typedef struct ImUiFontCodepoint
{
	uint32_t					codepoint;
	float						width;
	float						height;
	float						advance;
	float						ascentOffset;
	ImUiTexCoord				uv;
} ImUiFontCodepoint;

typedef struct ImUiFontParameters
{
	ImUiImage					image;

	const ImUiFontCodepoint*	codepoints;
	size_t						codepointCount;

	float						fontSize;		// for scalable fonts this is the default size when no one is given
	float						lineGap;
	bool						isScalable;
} ImUiFontParameters;

ImUiFont*						ImUiFontCreate( ImUiContext* imui, const ImUiFontParameters* parameters );
ImUiFont*						ImUiFontCreateTrueType( ImUiContext* imui, ImUiFontTrueTypeImage* ttfImage, ImUiImage image );
void							ImUiFontDestroy( ImUiContext* imui, ImUiFont* font );

ImUiFontTrueTypeData*			ImUiFontTrueTypeDataCreate( ImUiContext* imui, const void* data, size_t dataSize  ); // data must stay valid
ImUiFontTrueTypeData*			ImUiFontTrueTypeDataCreateCopy( ImUiContext* imui, const void* data, size_t dataSize ); // data copied into an internal buffer
void							ImUiFontTrueTypeDataDestroy( ImUiFontTrueTypeData* ttf );
bool							ImUiFontTrueTypeDataAddCodepoints( ImUiFontTrueTypeData* ttf, const uint32_t* codepoints, size_t codepointCount );
bool							ImUiFontTrueTypeDataAddCodepointRange( ImUiFontTrueTypeData* ttf, uint32_t firstCodepoint, uint32_t lastCodepoint );
void							ImUiFontTrueTypeDataCalculateMinTextureSize( ImUiFontTrueTypeData* ttf, float fontSizeInPixel, uint32_t* targetWidth, uint32_t* targetHeight );
ImUiFontTrueTypeImage*			ImUiFontTrueTypeDataGenerateTextureData( ImUiFontTrueTypeData* ttf, float fontSizeInPixel, void* targetData, size_t targetDataSize, uint32_t width, uint32_t height );

void							ImUiFontTrueTypeImageGetCodepoints( ImUiFontTrueTypeImage* ttfImage, const ImUiFontCodepoint** codepoints, size_t* codepointCount );
void							ImUiFontTrueTypeImageDestroy( ImUiFontTrueTypeImage* ttfImage );

//////////////////////////////////////////////////////////////////////////
// Text
// see imui_text.c

ImUiTextLayout*					ImUiTextLayoutCreate( ImUiContext* imui, ImUiFont* font, const char* text );
ImUiTextLayout*					ImUiTextLayoutCreateLength( ImUiContext* imui, ImUiFont* font, const char* text, size_t length );
ImUiTextLayout*					ImUiTextLayoutCreateWidget( ImUiWidget* widget, ImUiFont* font, const char* text );
ImUiTextLayout*					ImUiTextLayoutCreateWidgetLength( ImUiWidget* widget, ImUiFont* font, const char* text, size_t length );

size_t							ImUiTextLayoutCalculateGlyphCount( const char* text, size_t length );

size_t							ImUiTextLayoutGetGlyphCount( const ImUiTextLayout* layout );
size_t							ImUiTextLayoutFindGlyphIndex( const ImUiTextLayout* layout, ImUiPos pos, float scale );
size_t							ImUiTextLayoutGetGlyphCharIndex( const ImUiTextLayout* layout, size_t glyphIndex );
ImUiSize						ImUiTextLayoutGetSize( const ImUiTextLayout* layout );
ImUiPos							ImUiTextLayoutGetGlyphPos( const ImUiTextLayout* layout, size_t glyphIndex, float scale );

//////////////////////////////////////////////////////////////////////////
// Data Type Functions
// see imui_data_types.c

ImUiHash						ImUiHashCreate( const void* data, size_t dataSize );
ImUiHash						ImUiHashCreateSeed( const void* data, size_t dataSize, ImUiHash seed );
ImUiHash						ImUiHashMix( ImUiHash hash1, ImUiHash hash2 );

ImUiPos							ImUiPosCreate( float x, float y );
ImUiPos							ImUiPosCreateZero();
ImUiPos							ImUiPosAdd( ImUiPos pos, float x, float y );
ImUiPos							ImUiPosAddPos( ImUiPos pos, ImUiPos add );
ImUiPos							ImUiPosSub( ImUiPos pos, float x, float y );
ImUiPos							ImUiPosSubPos( ImUiPos pos, ImUiPos sub );
ImUiPos							ImUiPosScale( ImUiPos pos, float factor );
ImUiPos							ImUiPosMin( ImUiPos a, ImUiPos b );
ImUiPos							ImUiPosMax( ImUiPos a, ImUiPos b );

ImUiSize						ImUiSizeCreate( float width, float height );
ImUiSize						ImUiSizeCreateAll( float value );
ImUiSize						ImUiSizeCreateOne();
ImUiSize						ImUiSizeCreateZero();
ImUiSize						ImUiSizeCreateSkin( const ImUiSkin* skin );
ImUiSize						ImUiSizeCreateImage( const ImUiImage* image );
ImUiSize						ImUiSizeAdd( ImUiSize size, float width, float height );
ImUiSize						ImUiSizeAddSize( ImUiSize size, ImUiSize add );
ImUiSize						ImUiSizeSub( ImUiSize size, float width, float height );
ImUiSize						ImUiSizeSubSize( ImUiSize size, ImUiSize sub );
ImUiSize						ImUiSizeScale( ImUiSize size, float factor );
ImUiSize						ImUiSizeShrinkBorder( ImUiSize size, ImUiBorder border );
ImUiSize						ImUiSizeExpandBorder( ImUiSize size, ImUiBorder border );
ImUiSize						ImUiSizeLerp( ImUiSize a, ImUiSize b, float t );
ImUiSize						ImUiSizeLerp2( ImUiSize a, ImUiSize b, float widthT, float heightT );
ImUiSize						ImUiSizeMin( ImUiSize a, ImUiSize b );
ImUiSize						ImUiSizeMax( ImUiSize a, ImUiSize b );
ImUiSize						ImUiSizeFloor( ImUiSize size );
ImUiSize						ImUiSizeCeil( ImUiSize size );
ImUiPos							ImUiSizeToPos( ImUiSize size );

ImUiBorder						ImUiBorderCreate( float top, float left, float bottom, float right );
ImUiBorder						ImUiBorderCreateAll( float all );
ImUiBorder						ImUiBorderCreateZero();
ImUiBorder						ImUiBorderCreateHorizontalVertical( float horizontal, float vertical );
ImUiBorder						ImUiBorderScale( ImUiBorder border, float factor );
ImUiSize						ImUiBorderGetMinSize( ImUiBorder border );

ImUiRect						ImUiRectCreate( float x, float y, float width, float height );
ImUiRect						ImUiRectCreatePos( ImUiPos pos, float width, float height );
ImUiRect						ImUiRectCreateSize( float x, float y, ImUiSize size );
ImUiRect						ImUiRectCreatePosSize( ImUiPos pos, ImUiSize size );
ImUiRect						ImUiRectCreateMinMax( float minX, float minY, float maxX, float maxY );
ImUiRect						ImUiRectCreateMinMaxPos( ImUiPos tl, ImUiPos br );
ImUiRect						ImUiRectCreateCenter( float x, float y, float width, float height );
ImUiRect						ImUiRectCreateCenterPos( ImUiPos pos, float width, float height );
ImUiRect						ImUiRectCreateCenterSize( float x, float y, ImUiSize size );
ImUiRect						ImUiRectCreateCenterPosSize( ImUiPos pos, ImUiSize size );
ImUiRect						ImUiRectCreateZero();
ImUiRect						ImUiRectShrinkBorder( ImUiRect rect, ImUiBorder border );
ImUiRect						ImUiRectIntersection( ImUiRect rect1, ImUiRect rect2 );
bool							ImUiRectIncludesPos( ImUiRect rect, ImUiPos pos );
bool							ImUiRectIntersectsRect( ImUiRect rect1, ImUiRect rect2 );
ImUiPos							ImUiRectGetTopLeft( ImUiRect rect );
ImUiPos							ImUiRectGetTopRight( ImUiRect rect );
ImUiPos							ImUiRectGetBottomLeft( ImUiRect rect );
ImUiPos							ImUiRectGetBottomRight( ImUiRect rect );
ImUiPos							ImUiRectGetCenter( ImUiRect rect );
float							ImUiRectGetRight( ImUiRect rect );
float							ImUiRectGetBottom( ImUiRect rect );

ImUiColor						ImUiColorCreate( uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha );
ImUiColor						ImUiColorCreateFloat( float red, float green, float blue, float alpha );
ImUiColor						ImUiColorCreateBlack();
ImUiColor						ImUiColorCreateBlackA( uint8_t alpha );
ImUiColor						ImUiColorCreateWhite();
ImUiColor						ImUiColorCreateWhiteA( uint8_t alpha );
ImUiColor						ImUiColorCreateGray( uint8_t gray );
ImUiColor						ImUiColorCreateGrayA( uint8_t gray, uint8_t alpha );
ImUiColor						ImUiColorCreateTransparentBlack();

#ifdef __cplusplus
}
#endif
