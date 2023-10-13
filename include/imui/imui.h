#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define IMUI_DEFINES

#ifdef IMUI_DEFINES
#	define IMUI_STR( s ) ImUiStringViewCreateLength( s, sizeof( s ) - 1u )
#endif

#if !defined( IMUI_DEBUG )
#	if defined( _DEBUG ) || defined( __DEBUG__ )
#		define IMUI_DEBUG 1
#	else
#		define IMUI_DEBUG 0
#	endif
#endif

typedef struct ImUiContext ImUiContext;
typedef struct ImUiDraw ImUiDraw;
typedef struct ImUiFrame ImUiFrame;
typedef struct ImUiInput ImUiInput;
typedef struct ImUiSurface ImUiSurface;
typedef struct ImUiTextLayout ImUiTextLayout;
typedef struct ImUiWidget ImUiWidget;
typedef struct ImUiWindow ImUiWindow;

typedef uint32_t ImUiId;
typedef uint32_t ImUiHash;

typedef void*(*ImUiAllocatorMallocFunc)(size_t size, void* userData);
typedef void*(*ImUiAllocatorReallocFunc)(void* memory, size_t oldSize, size_t newSize, void* userData);
typedef void( *ImUiAllocatorFreeFunc )(void* memory, void* userData);

typedef struct ImUiAllocator ImUiAllocator;
struct ImUiAllocator
{
	ImUiAllocatorMallocFunc		mallocFunc;		// set to NULL to use default malloc/free
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

typedef struct ImUiStringView ImUiStringView;
struct ImUiStringView
{
	const char*					data;
	size_t						length;
};

typedef enum ImUiVertexElementType ImUiVertexElementType;
enum ImUiVertexElementType
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
};

typedef enum ImUiVertexElementSemantic ImUiVertexElementSemantic;
enum ImUiVertexElementSemantic
{
	ImUiVertexElementSemantic_None,
	ImUiVertexElementSemantic_PositionScreenSpace,
	ImUiVertexElementSemantic_PositionClipSpace,
	ImUiVertexElementSemantic_TextureCoordinate,
	ImUiVertexElementSemantic_ColorRGBA,
	ImUiVertexElementSemantic_ColorABGR
};

typedef struct ImUiVertexElement ImUiVertexElement;
struct ImUiVertexElement
{
	uint32_t					align;
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
	ImUiVertexFormat		vertexFormat;		// Override vertex format. Default: float2 pos screen-space, float2 uv, float4 color
	ImUiVertexType			vertexType;			// Override vertex type, Default: ImUiVertexType_VertexList
};

ImUiContext*				ImUiCreate( const ImUiParameters* parameters );
void						ImUiDestroy( ImUiContext* imui );

ImUiFrame*					ImUiBegin( ImUiContext* imui, float timeInSeconds );
void						ImUiEnd( ImUiFrame* frame );

//////////////////////////////////////////////////////////////////////////
// Types

typedef struct ImUiAlign ImUiAlign;
struct ImUiAlign
{
	float					horizontal;
	float					vertical;
};

typedef struct ImUiBorder ImUiBorder;
struct ImUiBorder
{
	float					top;
	float					left;
	float					bottom;
	float					right;
};

typedef struct ImUiColor ImUiColor;
struct ImUiColor
{
	uint8_t					red;
	uint8_t					green;
	uint8_t					blue;
	uint8_t					alpha;
};

typedef struct ImUiPos ImUiPos;
struct ImUiPos
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

typedef struct ImUiRect ImUiRect;
struct ImUiRect
{
	ImUiPos					pos;
	ImUiSize				size;
};

typedef struct ImUiTexCoord ImUiTexCoord;
struct ImUiTexCoord
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
	ImUiBorder				border;
	ImUiTexCoord			uv;
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
	ImUiRect				clipRect;
	size_t					count;				// index count if index buffer is used otherwise vertex count
};

typedef struct ImUiDrawData ImUiDrawData;
struct ImUiDrawData
{
	const void*				vertexData;
	size_t					vertexDataSize;
	const uint32_t*			indexData;
	size_t					indexDataSize;

	const ImUiDrawCommand*	commands;
	size_t					commandCount;
};

//////////////////////////////////////////////////////////////////////////
// Surface - Presents a OS window or a screen

ImUiSurface*				ImUiSurfaceBegin( ImUiFrame* frame, ImUiStringView name, ImUiSize size, float dpiScale );
const ImUiDrawData*			ImUiSurfaceEnd( ImUiSurface* surface );

ImUiContext*				ImUiSurfaceGetContext( const ImUiSurface* surface );

float						ImUiSurfaceGetTime( const ImUiSurface* surface );

ImUiSize					ImUiSurfaceGetSize( const ImUiSurface* surface );
float						ImUiSurfaceGetDpiScale( const ImUiSurface* surface );

//////////////////////////////////////////////////////////////////////////
// Window - A part of a Surface with z ordering

ImUiWindow*					ImUiWindowBegin( ImUiSurface* surface, ImUiStringView name, ImUiRect rect, uint32_t zOrder );
void						ImUiWindowEnd( ImUiWindow* window );

ImUiContext*				ImUiWindowGetContext( const ImUiWindow* window );
ImUiSurface*				ImUiWindowGetSurface( const ImUiWindow* window );

float						ImUiWindowGetTime( const ImUiWindow* window );

ImUiWidget*					ImUiWindowGetFirstChild( const ImUiWindow* window );
ImUiWidget*					ImUiWindowGetLastChild( const ImUiWindow* window );

//////////////////////////////////////////////////////////////////////////
// Widget - A layout element in the tree

typedef void(*ImUiStateDestructFunc)( void* state );

typedef struct ImUiWidgetInputState ImUiWidgetInputState;
struct ImUiWidgetInputState
{
	ImUiPos						relativeMousePos;

	bool						wasPressed;
	bool						wasMouseOver;
	bool						isMouseOver;
	bool						isMouseDown;
	bool						hasMousePressed;
	bool						hasMouseReleased;
};

ImUiWidget*					ImUiWidgetBegin( ImUiWindow* window );
ImUiWidget*					ImUiWidgetBeginId( ImUiWindow* window, ImUiId id );
ImUiWidget*					ImUiWidgetBeginNamed( ImUiWindow* window, ImUiStringView name );
void						ImUiWidgetEnd( ImUiWidget* widget );

ImUiContext*				ImUiWidgetGetContext( const ImUiWidget* widget );
ImUiSurface*				ImUiWidgetGetSurface( const ImUiWidget* widget );
ImUiWindow*					ImUiWidgetGetWindow( const ImUiWidget* widget );

ImUiWidget*					ImUiWidgetGetParent( const ImUiWidget* widget );
ImUiWidget*					ImUiWidgetGetFirstChild( const ImUiWidget* widget );
ImUiWidget*					ImUiWidgetGetLastChild( const ImUiWidget* widget );
ImUiWidget*					ImUiWidgetGetPrevSibling( const ImUiWidget* widget );
ImUiWidget*					ImUiWidgetGetNextSibling( const ImUiWidget* widget );

float						ImUiWidgetGetTime( const ImUiWidget* widget );

void*						ImUiWidgetGetState( ImUiWidget* widget );
void*						ImUiWidgetAllocState( ImUiWidget* widget, size_t size );
void*						ImUiWidgetAllocStateNew( ImUiWidget* widget, size_t size, bool* isNew );
void*						ImUiWidgetAllocStateNewDestruct( ImUiWidget* widget, size_t size, bool* isNew, ImUiStateDestructFunc destructFunc );

ImUiLayout					ImUiWidgetGetLayout( const ImUiWidget* widget );
void						ImUiWidgetSetLayoutStack( ImUiWidget* widget );							// default
void						ImUiWidgetSetLayoutScroll( ImUiWidget* widget, ImUiPos offset );
void						ImUiWidgetSetLayoutHorizontal( ImUiWidget* widget );
void						ImUiWidgetSetLayoutHorizontalSpacing( ImUiWidget* widget, float spacing );
void						ImUiWidgetSetLayoutVertical( ImUiWidget* widget );
void						ImUiWidgetSetLayoutVerticalSpacing( ImUiWidget* widget, float spacing );
void						ImUiWidgetSetLayoutGrid( ImUiWidget* widget, size_t columnCount );

ImUiBorder					ImUiWidgetGetMargin( const ImUiWidget* widget );
void						ImUiWidgetSetMargin( ImUiWidget* widget, ImUiBorder margin );
ImUiBorder					ImUiWidgetGetPadding( const ImUiWidget* widget );
void						ImUiWidgetSetPadding( ImUiWidget* widget, ImUiBorder padding );

ImUiSize					ImUiWidgetGetMinSize( const ImUiWidget* widget );
void						ImUiWidgetSetMinWidth( ImUiWidget* widget, float value );
void						ImUiWidgetSetMinHeight( ImUiWidget* widget, float value );
void						ImUiWidgetSetMinSize( ImUiWidget* widget, ImUiSize size );
ImUiSize					ImUiWidgetGetMaxSize( const ImUiWidget* widget );
void						ImUiWidgetSetMaxWidth( ImUiWidget* widget, float value );
void						ImUiWidgetSetMaxHeight( ImUiWidget* widget, float value );
void						ImUiWidgetSetMaxSize( ImUiWidget* widget, ImUiSize size );

void						ImUiWidgetSetFixedWidth( ImUiWidget* widget, float value );
void						ImUiWidgetSetFixedHeight( ImUiWidget* widget, float value );
void						ImUiWidgetSetFixedSize( ImUiWidget* widget, ImUiSize size );

ImUiSize					ImUiWidgetGetStretch( const ImUiWidget* widget );
void						ImUiWidgetSetStretch( ImUiWidget* widget, ImUiSize stretch );
void						ImUiWidgetSetHStretch( ImUiWidget* widget, float stretch );
void						ImUiWidgetSetVStretch( ImUiWidget* widget, float stretch );

ImUiAlign					ImUiWidgetGetAlign( const ImUiWidget* widget );
void						ImUiWidgetSetAlign( ImUiWidget* widget, ImUiAlign align );
void						ImUiWidgetSetHAlign( ImUiWidget* widget, float align );
void						ImUiWidgetSetVAlign( ImUiWidget* widget, float align );

ImUiPos						ImUiWidgetGetPos( const ImUiWidget* widget );
ImUiSize					ImUiWidgetGetSize( const ImUiWidget* widget );
ImUiRect					ImUiWidgetGetRect( const ImUiWidget* widget );
ImUiSize					ImUiWidgetGetInnerSize( const ImUiWidget* widget );
ImUiRect					ImUiWidgetGetInnerRect( const ImUiWidget* widget );

void						ImUiWidgetGetInputState( ImUiWidget* widget, ImUiWidgetInputState* target );

//////////////////////////////////////////////////////////////////////////
// Widget Draw
// see imui_draw.c

void						ImUiDrawLine( ImUiWidget* widget, ImUiPos p0, ImUiPos p1, ImUiColor color );
void						ImUiDrawWidgetColor( ImUiWidget* widget, ImUiColor color );
void						ImUiDrawWidgetTexture( ImUiWidget* widget, ImUiTexture texture );
void						ImUiDrawWidgetTextureColor( ImUiWidget* widget, ImUiTexture texture, ImUiColor color );
void						ImUiDrawWidgetSkin( ImUiWidget* widget, ImUiSkin skin );
void						ImUiDrawWidgetSkinColor( ImUiWidget* widget, ImUiSkin skin, ImUiColor color );
void						ImUiDrawWidgetText( ImUiWidget* widget, ImUiTextLayout* layout );
void						ImUiDrawWidgetTextColor( ImUiWidget* widget, ImUiTextLayout* layout, ImUiColor color );
void						ImUiDrawRectColor( ImUiWidget* widget, ImUiRect rect, ImUiColor color );
void						ImUiDrawRectTexture( ImUiWidget* widget, ImUiRect rect, ImUiTexture texture );
void						ImUiDrawRectTextureUv( ImUiWidget* widget, ImUiRect rect, ImUiTexture texture, ImUiTexCoord uv );
void						ImUiDrawRectTextureColor( ImUiWidget* widget, ImUiRect rect, ImUiTexture texture, ImUiColor color );
void						ImUiDrawRectTextureColorUv( ImUiWidget* widget, ImUiRect rect, ImUiTexture texture, ImUiColor color, ImUiTexCoord uv );
void						ImUiDrawRectSkin( ImUiWidget* widget, ImUiRect rect, ImUiSkin skin );
void						ImUiDrawRectSkinColor( ImUiWidget* widget, ImUiRect rect, ImUiSkin skin, ImUiColor color );
void						ImUiDrawText( ImUiWidget* widget, ImUiPos pos, ImUiTextLayout* layout );
void						ImUiDrawTextColor( ImUiWidget* widget, ImUiPos pos, ImUiTextLayout* layout, ImUiColor color );

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

typedef enum ImUiInputMouseCursor ImUiInputMouseCursor;
enum ImUiInputMouseCursor
{
	ImUiInputMouseCursor_Arrow,
	ImUiInputMouseCursor_Wait,
	ImUiInputMouseCursor_WaitArrow,
	ImUiInputMouseCursor_IBeam,
	ImUiInputMouseCursor_Crooshair,
	ImUiInputMouseCursor_Hand,
	ImUiInputMouseCursor_ResizeNorthwestSoutheast,
	ImUiInputMouseCursor_ResizeNortheastSouthwest,
	ImUiInputMouseCursor_ResizeWestEast,
	ImUiInputMouseCursor_ResizeNorthSouth,
	ImUiInputMouseCursor_Move,

	ImUiInputMouseCursor_MAX
};

// Push

ImUiInput*						ImUiInputBegin( ImUiContext* imui );
void							ImUiInputEnd( ImUiContext* imui );

void							ImUiInputSetMouseCursor( ImUiContext* imui, ImUiInputMouseCursor cursor );

void							ImUiInputPushKeyDown( ImUiInput* input, ImUiInputKey key );
void							ImUiInputPushKeyUp( ImUiInput* input, ImUiInputKey key );
void							ImUiInputPushKeyRepeat( ImUiInput* input, ImUiInputKey key );
void							ImUiInputPushText( ImUiInput* input, const char* text );
void							ImUiInputPushTextChar( ImUiInput* input, char c );

void							ImUiInputPushMouseDown( ImUiInput* input, ImUiInputMouseButton button );
void							ImUiInputPushMouseUp( ImUiInput* input, ImUiInputMouseButton button );
void							ImUiInputPushMouseMove( ImUiInput* input, float x, float y );
void							ImUiInputPushMouseMoveDelta( ImUiInput* input, float deltaX, float deltaY );
void							ImUiInputPushMouseScroll( ImUiInput* input, float horizontalOffset, float verticalOffset );
void							ImUiInputPushMouseScrollDelta( ImUiInput* input, float horizontalDelta, float verticalDelta );

// Read

uint32_t						ImUiInputGetKeyModifiers( const ImUiContext* imui );	// returns ImUiInputModifier
bool							ImUiInputIsKeyDown( const ImUiContext* imui, ImUiInputKey key );
bool							ImUiInputIsKeyUp( const ImUiContext* imui, ImUiInputKey key );
bool							ImUiInputHasKeyPressed( const ImUiContext* imui, ImUiInputKey key );
bool							ImUiInputHasKeyReleased( const ImUiContext* imui, ImUiInputKey key );

ImUiStringView					ImUiInputGetText( const ImUiContext* imui );

ImUiPos							ImUiInputGetMousePos( const ImUiContext* imui );
ImUiInputMouseCursor			ImUiInputGetMouseCursor( ImUiContext* imui );
bool							ImUiInputIsMouseInRect( const ImUiContext* imui, ImUiRect rect );
bool							ImUiInputIsMouseButtonDown( const ImUiContext* imui, ImUiInputMouseButton button );
bool							ImUiInputIsMouseButtonUp( const ImUiContext* imui, ImUiInputMouseButton button );
bool							ImUiInputHasMouseButtonPressed( const ImUiContext* imui, ImUiInputMouseButton button );
bool							ImUiInputHasMouseButtonReleased( const ImUiContext* imui, ImUiInputMouseButton button );
ImUiPos							ImUiInputGetMouseScrollDelta( const ImUiContext* imui );

//////////////////////////////////////////////////////////////////////////
// Font
// see imui_font.c

typedef struct ImUiFont ImUiFont;
typedef struct ImUiFontTrueTypeData ImUiFontTrueTypeData;
typedef struct ImUiFontTrueTypeImage ImUiFontTrueTypeImage;

typedef struct ImUiFontCodepoint ImUiFontCodepoint;
struct ImUiFontCodepoint
{
	uint32_t					codepoint;
	float						width;
	float						height;
	float						advance;
	float						ascentOffset;
	ImUiTexCoord		uv;
};

typedef struct ImUiFontParameters ImUiFontParameters;
struct ImUiFontParameters
{
	ImUiTexture					texture;
	const ImUiFontCodepoint*	codepoints;
	size_t						codepointCount;

	float						fontSize;
	float						lineGap;
};

ImUiFont*						ImUiFontCreate( ImUiContext* imui, const ImUiFontParameters* parameters );
ImUiFont*						ImUiFontCreateTrueType( ImUiContext* imui, ImUiFontTrueTypeImage* ttfImage, ImUiTexture texture );
void							ImUiFontDestroy( ImUiContext* imui, ImUiFont* font );

ImUiFontTrueTypeData*			ImUiFontTrueTypeDataCreate( ImUiContext* imui, const void* data, size_t dataSize  ); // data must stay valid
ImUiFontTrueTypeData*			ImUiFontTrueTypeDataCreateCopy( ImUiContext* imui, const void* data, size_t dataSize ); // data copied into an internal buffer
void							ImUiFontTrueTypeDataDestroy( ImUiFontTrueTypeData* ttf );
bool							ImUiFontTrueTypeDataAddCodepoints( ImUiFontTrueTypeData* ttf, const uint32_t* codepoints, size_t codepointCount );
bool							ImUiFontTrueTypeDataAddCodepointRange( ImUiFontTrueTypeData* ttf, uint32_t firstCodepoint, uint32_t lastCodepoint );
void							ImUiFontTrueTypeDataCalculateMinTextureSize( ImUiFontTrueTypeData* ttf, float fontSizeInPixel, uint32_t* targetWidth, uint32_t* targetHeight );
ImUiFontTrueTypeImage*			ImUiFontTrueTypeDataGenerateTextureData( ImUiFontTrueTypeData* ttf, float fontSizeInPixel, void* targetData, size_t targetDataSize, uint32_t width, uint32_t height );

void							ImUiFontTrueTypeImageDestroy( ImUiFontTrueTypeImage* ttfImage );

//////////////////////////////////////////////////////////////////////////
// Text
// see imui_text.c

ImUiTextLayout*					ImUiTextLayoutCreate( ImUiContext* imui, ImUiFont* font, ImUiStringView text );
ImUiTextLayout*					ImUiTextLayoutCreateWidget( ImUiWidget* widget, ImUiFont* font, ImUiStringView text );

ImUiSize						ImUiTextLayoutGetSize( const ImUiTextLayout* layout );
ImUiPos							ImUiTextLayoutGetGlyphPos( const ImUiTextLayout* layout, size_t glyphIndex );

//////////////////////////////////////////////////////////////////////////
// Data Type Functions
// see imui_data_types.c

ImUiStringView					ImUiStringViewCreate( const char* str );
ImUiStringView					ImUiStringViewCreateLength( const char* str, size_t length );
ImUiStringView					ImUiStringViewCreateEmpty();
bool							ImUiStringViewIsEquals( ImUiStringView string1, ImUiStringView string2 );

ImUiHash						ImUiHashCreate( const void* data, size_t dataSize, ImUiHash seed );
ImUiHash						ImUiHashString( ImUiStringView string, ImUiHash seed );
ImUiHash						ImUiHashMix( ImUiHash hash1, ImUiHash hash2 );

ImUiAlign						ImUiAlignCreate( float horizontal, float vertical );
ImUiAlign						ImUiAlignCreateCenter();

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
ImUiSize						ImUiSizeCreateHorizontal();				// x = 1, y = 0
ImUiSize						ImUiSizeCreateVertical();				// x = 0, y = 1
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
