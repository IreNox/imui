#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define IMUI_DEFINES

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef IMUI_DEFINES
#	define IMUI_STR( s ) ImUiStringViewCreate( s )
#endif

#if !defined( IMUI_DEBUG ) && defined( _DEBUG )
#	define IMUI_DEBUG 1
#endif

typedef struct ImUiContext ImUiContext;
typedef struct ImUiSurface ImUiSurface;
typedef struct ImUiWindow ImUiWindow;
typedef struct ImUiWidget ImUiWidget;
typedef struct ImUiInput ImUiInput;
typedef struct ImUiDraw ImUiDraw;

typedef uint32_t ImUiId;
typedef uint32_t ImUiHash;

typedef void*(*ImUiAllocatorMallocFunc)(size_t size, void* userData);
typedef void*(*ImUiAllocatorReallocFunc)(void* memory, size_t oldSize, size_t newSize, void* userData);
typedef void( *ImUiAllocatorFreeFunc )(void* memory, void* userData);

typedef struct ImUiStringView ImUiStringView;
struct ImUiStringView
{
	const char*					data;
	size_t						length;
};

typedef struct ImUiAllocator ImUiAllocator;
struct ImUiAllocator
{
	ImUiAllocatorMallocFunc		mallocFunc;
	ImUiAllocatorReallocFunc	reallocFunc;	// can be NULL
	ImUiAllocatorFreeFunc		freeFunc;
	void*						userData;
	void*						internalData;	// internal use only
#if IMUI_DEBUG
	size_t						allocationCount;
	size_t						maxAllocationCount;
	size_t						allocationSize;
	size_t						maxAllocationSize;
#endif
};

typedef enum ImUiVertexElementType ImUiVertexElementType;
enum ImUiVertexElementType
{
	ImUiVertexElementType_Invalid,

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
};

typedef enum ImUiVertexElementSemantic ImUiVertexElementSemantic;
enum ImUiVertexElementSemantic
{
	ImUiVertexElementSemantic_None,
	ImUiVertexElementSemantic_PositionScreenSpace,
	ImUiVertexElementSemantic_PositionClipSpace,
	ImUiVertexElementSemantic_TextureCoordinate,
	ImUiVertexElementSemantic_Color
};

typedef struct ImUiVertexElement ImUiVertexElement;
struct ImUiVertexElement
{
	uint32_t					alignment;
	ImUiVertexElementType		type;
	ImUiVertexElementSemantic	semantic;
};

typedef struct ImUiVertexFormat ImUiVertexFormat;
struct ImUiVertexFormat
{
	const ImUiVertexElement*	elements;
	size_t						elementCount;
};

typedef enum ImUiVertexType ImUiVertexType;
enum ImUiVertexType
{
	ImUiVertexType_VertexList,
	ImUiVertexType_VertexStrip,
	ImUiVertexType_IndexedVertexList,
	ImUiVertexType_IndexedVertexStrip
};

typedef struct ImUiParameters ImUiParameters;
struct ImUiParameters							// Fill with zero for default parameters
{
	ImUiAllocator			allocator;			// Override memory Allocator. Default use malloc/free
	ImUiVertexFormat		vertexFormat;		// Override vertex format. Default: float2 pos ss, float2 uv, float4 color
	ImUiVertexType			vertexType;			// Override vertex type, Default: ImUiVertexType_VertexList
};

typedef enum ImUiDrawTopology ImUiDrawTopology;
enum ImUiDrawTopology
{
	ImUiDrawTopology_LineList,
	ImUiDrawTopology_TriangleList,
	ImUiDrawTopology_TriangleStrip,
	ImUiDrawTopology_IndexedTriangleList,
	ImUiDrawTopology_IndexedTriangleStrip,

	ImUiDrawTopology_MAX
};

typedef struct ImUiDrawCommand ImUiDrawCommand;
struct ImUiDrawCommand
{
	ImUiDrawTopology		topology;
	void*					texture;
	//ImUiRectangle			clipRect;
	size_t					offset;				// index offset if index buffer is used otherwise vertex offset
	size_t					count;				// same as offset but count
};

typedef struct ImUiDrawData ImUiDrawData;
struct ImUiDrawData
{
	const void*				vertexData;
	size_t					vertexDataSize;
	const uint32_t*			indexData;
	size_t					indexCount;

	const ImUiDrawCommand*	commands;
	size_t					commandCount;
};

typedef struct ImUiFrame ImUiFrame;
struct ImUiFrame
{
	ImUiContext*			imui;
	ImUiInput*				input;
	ImUiDraw*				draw;
};

ImUiContext*				ImUiCreate( const ImUiParameters* parameters );
void						ImUiDestroy( ImUiContext* imui );

ImUiFrame*					ImUiBegin( ImUiContext* imui );
void						ImUiEnd( ImUiFrame* frame );

//////////////////////////////////////////////////////////////////////////
// Types

typedef struct ImUiColor ImUiColor;
struct ImUiColor
{
	float					red;
	float					green;
	float					blue;
	float					alpha;
};

typedef struct ImUiThickness ImUiThickness;
struct ImUiThickness
{
	float					top;
	float					left;
	float					bottom;
	float					right;
};

typedef enum ImUiHorizontalAlignment ImUiHorizontalAlignment;
enum ImUiHorizontalAlignment
{
	ImUiHorizintalAlignment_Left,
	ImUiHorizintalAlignment_Center,
	ImUiHorizintalAlignment_Right
};

typedef enum ImUiVerticalAlignment ImUiVerticalAlignment;
enum ImUiVerticalAlignment
{
	ImUiVerticalAlignment_Top,
	ImUiVerticalAlignment_Center,
	ImUiVerticalAlignment_Bottom
};

typedef struct ImUiAlignment ImUiAlignment;
struct ImUiAlignment
{
	ImUiHorizontalAlignment	horizontal;
	ImUiVerticalAlignment	vertical;
};

typedef struct ImUiPosition ImUiPosition;
struct ImUiPosition
{
	float					x;
	float					y;
};

typedef struct ImUiSize ImUiSize;
struct ImUiSize
{
	float					width;
	float					height;
};

typedef struct ImUiRectangle ImUiRectangle;
struct ImUiRectangle
{
	ImUiPosition			position;
	ImUiSize				size;
};

typedef struct ImUiTextureCooridinate ImUiTextureCooridinate;
struct ImUiTextureCooridinate
{
	float					u0;
	float					v0;
	float					u1;
	float					v1;
};

typedef struct ImUiTexture ImUiTexture;
struct ImUiTexture
{
	void*					data;
	ImUiSize				size;
};

typedef struct ImUiSkin ImUiSkin;
struct ImUiSkin
{
	ImUiTexture				texture;
	ImUiThickness			border;
};

typedef enum ImUiLayout ImUiLayout;
enum ImUiLayout
{
	ImUiLayout_Stack,
	ImUiLayout_Scroll,
	ImUiLayout_Horizontal,
	ImUiLayout_Vertical,
	ImUiLayout_Grid
};

//////////////////////////////////////////////////////////////////////////
// Surface - Presents a OS window or a screen

ImUiSurface*				ImUiSurfaceBegin( ImUiFrame* frame, ImUiStringView name, ImUiSize size, float dpiScale );
const ImUiDrawData*			ImUiSurfaceEnd( ImUiSurface* surface );

ImUiSize					ImUiSurfaceGetSize( const ImUiSurface* surface );
float						ImUiSurfaceGetDpiScale( const ImUiSurface* surface );

//////////////////////////////////////////////////////////////////////////
// Window - A part of a Surface with z ordering

ImUiWindow*					ImUiWindowBegin( ImUiSurface* surface, ImUiStringView name, ImUiRectangle rectangle, uint32_t zOrder );
void						ImUiWindowEnd( ImUiWindow* window );

//////////////////////////////////////////////////////////////////////////
// Widget - todo

ImUiWidget*					ImUiWidgetBegin( ImUiWindow* window );
ImUiWidget*					ImUiWidgetBeginId( ImUiWindow* window, ImUiId id );
ImUiWidget*					ImUiWidgetBeginNamed( ImUiWindow* window, ImUiStringView name );
void						ImUiWidgetEnd( ImUiWidget* widget );

ImUiLayout					ImUiWidgetGetLayout( const ImUiWidget* widget );
void						ImUiWidgetSetLayoutStack( ImUiWidget* widget );							// default
void						ImUiWidgetSetLayoutScroll( ImUiWidget* widget, ImUiPosition offset );
void						ImUiWidgetSetLayoutHorizontal( ImUiWidget* widget );
void						ImUiWidgetSetLayoutHorizontalSpacing( ImUiWidget* widget, float spacing );
void						ImUiWidgetSetLayoutVerical( ImUiWidget* widget );
void						ImUiWidgetSetLayoutVericalSpacing( ImUiWidget* widget, float spacing );
void						ImUiWidgetSetLayoutGrid( ImUiWidget* widget, size_t columnCount );

ImUiThickness				ImUiWidgetGetMargin( const ImUiWidget* widget );
void						ImUiWidgetSetMargin( ImUiWidget* widget, ImUiThickness margin );
ImUiThickness				ImUiWidgetGetPadding( const ImUiWidget* widget );
void						ImUiWidgetSetPadding( ImUiWidget* widget, ImUiThickness padding );

ImUiSize					ImUiWidgetGetMinSize( const ImUiWidget* widget );
void						ImUiWidgetSetMinSize( ImUiWidget* widget, ImUiSize size );
ImUiSize					ImUiWidgetGetMaxSize( const ImUiWidget* widget );
void						ImUiWidgetSetMaxSize( ImUiWidget* widget, ImUiSize size );
ImUiSize					ImUiWidgetGetPrefSize( const ImUiWidget* widget );
void						ImUiWidgetSetPrefSize( ImUiWidget* widget, ImUiSize size );
void						ImUiWidgetSetFixedSize( ImUiWidget* widget, ImUiSize size );

ImUiSize					ImUiWidgetGetStretch( const ImUiWidget* widget );
void						ImUiWidgetSetStretch( ImUiWidget* widget, ImUiSize stretch );

ImUiAlignment				ImUiWidgetGetAlignment( const ImUiWidget* widget );
void						ImUiWidgetSetAlignment( ImUiWidget* widget, ImUiAlignment alignment );
void						ImUiWidgetSetHorizintalAlignment( ImUiWidget* widget, ImUiHorizontalAlignment alignment );
void						ImUiWidgetSetVerticalAlignment( ImUiWidget* widget, ImUiVerticalAlignment alignment );

ImUiRectangle				ImUiWidgetGetRectangle( const ImUiWidget* widget );

//////////////////////////////////////////////////////////////////////////
// Widget Draw
// see imui_draw.c

void						ImUiDrawLine( ImUiWidget* widget, ImUiPosition p0, ImUiPosition p1, ImUiColor color );
void						ImUiDrawRectangleColor( ImUiWidget* widget, ImUiRectangle rect, ImUiColor color );
void						ImUiDrawRectangleTexture( ImUiWidget* widget, ImUiRectangle rect, ImUiTexture texture );
void						ImUiDrawRectangleTextureUv( ImUiWidget* widget, ImUiRectangle rect, ImUiTexture texture, ImUiTextureCooridinate uv );
void						ImUiDrawRectangleTextureColor( ImUiWidget* widget, ImUiRectangle rect, ImUiTexture texture, ImUiColor color );
void						ImUiDrawRectangleTextureColorUv( ImUiWidget* widget, ImUiRectangle rect, ImUiTexture texture, ImUiColor color, ImUiTextureCooridinate uv );

//////////////////////////////////////////////////////////////////////////
// Input
// see imui_input.c

typedef enum ImUiInputMouseButton ImUiInputMouseButton;
enum ImUiInputMouseButton
{
	ImUiInputMouseButton_Left,
	ImUiInputMouseButton_Right,
	ImUiInputMouseButton_Middle,

	ImUiInputMouseButton_X1,
	ImUiInputMouseButton_X2,

	ImUiInputMouseButton_MAX
};

typedef enum ImUiInputKey ImUiInputKey;
enum ImUiInputKey
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

	ImUiInputKey_MAX
};

typedef enum ImUiInputModifier ImUiInputModifier;
enum ImUiInputModifier
{
	ImUiInputModifier_LeftShift		= 1u << 0u,
	ImUiInputModifier_RightShift	= 1u << 1u,
	ImUiInputModifier_LeftCtrl		= 1u << 2u,
	ImUiInputModifier_RightCtrl		= 1u << 3u,
	ImUiInputModifier_LeftAlt		= 1u << 4u,
	ImUiInputModifier_RightAlt		= 1u << 5u
};

// Push
ImUiInput*						ImUiInputBegin( ImUiContext* imui );
void							ImUiInputEnd( ImUiContext* imui );

void							ImUiInputPushKeyDown( ImUiInput* input, ImUiInputKey key );
void							ImUiInputPushKeyUp( ImUiInput* input, ImUiInputKey key );
void							ImUiInputPushKeyRepeate( ImUiInput* input, ImUiInputKey key );
void							ImUiInputPushText( ImUiInput* input, const char* text );
void							ImUiInputPushTextChar( ImUiInput* input, char c );

void							ImUiInputPushMouseDown( ImUiInput* input, ImUiInputMouseButton button );
void							ImUiInputPushMouseUp( ImUiInput* input, ImUiInputMouseButton button );
void							ImUiInputPushMouseMove( ImUiInput* input, float x, float y );
void							ImUiInputPushMouseMoveDelta( ImUiInput* input, float deltaX, float deltaY );
void							ImUiInputPushMouseScroll( ImUiInput* input, float horizontalOffset, float verticalOffset );
void							ImUiInputPushMouseScrollDelta( ImUiInput* input, float horizontalDelta, float verticalDelta );

// Read
uint32_t						ImUiInputGetKeyModifiers( ImUiInput* input );	// returns ImUiInputModifier
bool							ImUiInputIsKeyDown( ImUiInput* input, ImUiInputKey key );
bool							ImUiInputIsKeyUp( ImUiInput* input, ImUiInputKey key );
bool							ImUiInputHasKeyPressed( ImUiInput* input, ImUiInputKey key );
bool							ImUiInputHasKeyReleased( ImUiInput* input, ImUiInputKey key );
ImUiInputKey					ImUiInputGetKeyRepeate( ImUiInput* input, size_t index );
size_t							ImUiInputGetKeyRepeateCount( ImUiInput* input );

bool							ImUiInputIsMouseInRectangle( ImUiInput* input, ImUiRectangle rectangle );
bool							ImUiInputIsMouseButtonDown( ImUiInput* input, ImUiInputMouseButton button );
bool							ImUiInputIsMouseButtonUp( ImUiInput* input, ImUiInputMouseButton button );
bool							ImUiInputHasMouseButtonPressed( ImUiInput* input, ImUiInputMouseButton button );
bool							ImUiInputHasMouseButtonReleased( ImUiInput* input, ImUiInputMouseButton button );

//////////////////////////////////////////////////////////////////////////
// Data Type Functions
// see imui_helper.c

ImUiStringView					ImUiStringViewCreate( const char* str );
bool							ImUiStringViewIsEquals( ImUiStringView string1, ImUiStringView string2 );

ImUiHash						ImUiHashCreate( const void* data, size_t dataSize, ImUiHash seed );
ImUiHash						ImUiHashString( ImUiStringView string, ImUiHash seed );
ImUiHash						ImUiHashMix( ImUiHash hash1, ImUiHash hash2 );

ImUiAlignment					ImUiAlignmentCreate( ImUiHorizontalAlignment horizintal, ImUiVerticalAlignment vertical );

ImUiPosition					ImUiPositionCreate( float x, float y );
ImUiPosition					ImUiPositionAdd( ImUiPosition pos, float x, float y );
ImUiPosition					ImUiPositionAddPos( ImUiPosition pos, ImUiPosition add );
ImUiPosition					ImUiPositionSub( ImUiPosition pos, float x, float y );
ImUiPosition					ImUiPositionSubPos( ImUiPosition pos, ImUiPosition add );
ImUiPosition					ImUiPositionScale( ImUiPosition pos, float factor );

ImUiSize						ImUiSizeCreate( float width, float height );
ImUiSize						ImUiSizeAdd( ImUiSize size, float width, float height );
ImUiSize						ImUiSizeAddSize( ImUiSize size, ImUiSize add );
ImUiSize						ImUiSizeSub( ImUiSize size, float width, float height );
ImUiSize						ImUiSizeSubSize( ImUiSize size, ImUiSize sub );
ImUiSize						ImUiSizeLerp( ImUiSize a, ImUiSize b, float t );
ImUiSize						ImUiSizeLerp2( ImUiSize a, ImUiSize b, float widthT, float heightT );
ImUiSize						ImUiSizeShrinkThickness( ImUiSize size, ImUiThickness thickness );

ImUiThickness					ImUiThicknessCreate( float top, float left, float bottom, float right );
ImUiThickness					ImUiThicknessCreateAll( float all );
ImUiThickness					ImUiThicknessCreateVerticalHorizontal( float vertical, float horizontal );

ImUiRectangle					ImUiRectangleCreate( float x, float y, float width, float height );
ImUiRectangle					ImUiRectangleCreatePos( ImUiPosition pos, float width, float height );
ImUiRectangle					ImUiRectangleCreateSize( float x, float y, ImUiSize size );
ImUiRectangle					ImUiRectangleCreatePosSize( ImUiPosition pos, ImUiSize size );
ImUiRectangle					ImUiRectangleCreateCenter( float x, float y, float width, float height );
ImUiRectangle					ImUiRectangleCreateCenterPos( ImUiPosition pos, float width, float height );
ImUiRectangle					ImUiRectangleCreateCenterSize( float x, float y, ImUiSize size );
ImUiRectangle					ImUiRectangleCreateCenterPosSize( ImUiPosition pos, ImUiSize size );
ImUiRectangle					ImUiRectangleShrinkThickness( ImUiRectangle rect, ImUiThickness thickness );
bool							ImUiRectangleIncludesPosition( ImUiRectangle rect, ImUiPosition position );
bool							ImUiRectangleIntersectsRectangle( ImUiRectangle rect1, ImUiRectangle rect2 );
ImUiPosition					ImUiRectangleGetTopLeft( ImUiRectangle rect );
ImUiPosition					ImUiRectangleGetTopRight( ImUiRectangle rect );
ImUiPosition					ImUiRectangleGetBottomLeft( ImUiRectangle rect );
ImUiPosition					ImUiRectangleGetBottomRight( ImUiRectangle rect );

ImUiColor						ImUiColorCreate( float red, float green, float blue, float alpha );
ImUiColor						ImUiColorCreateBlack( float alpha );
ImUiColor						ImUiColorCreateWhite( float alpha );
ImUiColor						ImUiColorCreateTransparentBlack();

#ifdef __cplusplus
}
#endif
