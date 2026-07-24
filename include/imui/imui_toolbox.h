#pragma once

#include "imui.h"

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdarg.h>

typedef enum ImuiToolboxColor
{
	ImuiToolboxColor_Text,

	ImuiToolboxColor_Button,
	ImuiToolboxColor_ButtonHover,
	ImuiToolboxColor_ButtonClicked,
	ImuiToolboxColor_ButtonText,

	ImuiToolboxColor_CheckBox,
	ImuiToolboxColor_CheckBoxHover,
	ImuiToolboxColor_CheckBoxClicked,
	ImuiToolboxColor_CheckBoxUnchecked,
	ImuiToolboxColor_CheckBoxChecked,

	ImuiToolboxColor_SliderBackground,
	ImuiToolboxColor_SliderPivot,
	ImuiToolboxColor_SliderPivotHover,
	ImuiToolboxColor_SliderPivotClicked,

	ImuiToolboxColor_TextEditBackground,
	ImuiToolboxColor_TextEditText,
	ImuiToolboxColor_TextEditCursor,
	ImuiToolboxColor_TextEditSelection,

	ImuiToolboxColor_ProgressBarBackground,
	ImuiToolboxColor_ProgressBarProgress,

	ImuiToolboxColor_ScrollAreaBarBackground,
	ImuiToolboxColor_ScrollAreaBarPivot,

	ImuiToolboxColor_ListItemHover,
	ImuiToolboxColor_ListItemClicked,
	ImuiToolboxColor_ListItemSelected,

	ImuiToolboxColor_DropDown,
	ImuiToolboxColor_DropDownText,
	ImuiToolboxColor_DropDownIcon,
	ImuiToolboxColor_DropDownHover,
	ImuiToolboxColor_DropDownClicked,
	ImuiToolboxColor_DropDownOpen,
	ImuiToolboxColor_DropDownList,
	ImuiToolboxColor_DropDownItemText,
	ImuiToolboxColor_DropDownItemHover,
	ImuiToolboxColor_DropDownItemClicked,
	ImuiToolboxColor_DropDownItemSelected,

	ImuiToolboxColor_PopupBackground,
	ImuiToolboxColor_Popup,

	ImuiToolboxColor_TabViewHeadBackground,
	ImuiToolboxColor_TabViewHeaderActive,
	ImuiToolboxColor_TabViewHeaderInactive,
	ImuiToolboxColor_TabViewBody,

	ImuiToolboxColor_MAX
} ImuiToolboxColor;

typedef enum ImuiToolboxSkin
{
	ImuiToolboxSkin_Button,
	ImuiToolboxSkin_ButtonHover,
	ImuiToolboxSkin_ButtonClicked,
	ImuiToolboxSkin_CheckBox,
	ImuiToolboxSkin_CheckBoxChecked,
	ImuiToolboxSkin_SliderBackground,
	ImuiToolboxSkin_SliderPivot,
	ImuiToolboxSkin_TextEditBackground,
	ImuiToolboxSkin_ProgressBarBackground,
	ImuiToolboxSkin_ProgressBarProgress,
	ImuiToolboxSkin_ScrollAreaBarBackground,
	ImuiToolboxSkin_ScrollAreaBarPivot,
	ImuiToolboxSkin_ListItem,
	ImuiToolboxSkin_ItemSelected,
	ImuiToolboxSkin_DropDown,
	ImuiToolboxSkin_DropDownList,
	ImuiToolboxSkin_DropDownItem,
	ImuiToolboxSkin_Popup,
	ImuiToolboxSkin_TabViewHeadBackground,
	ImuiToolboxSkin_TabViewHeaderActive,
	ImuiToolboxSkin_TabViewHeaderInactive,
	ImuiToolboxSkin_TabViewBody,

	ImuiToolboxSkin_MAX
} ImuiToolboxSkin;

typedef enum ImuiToolboxIcon
{
	ImuiToolboxIcon_CheckBoxUnchecked,
	ImuiToolboxIcon_CheckBoxChecked,
	ImuiToolboxIcon_DropDownOpen,
	ImuiToolboxIcon_DropDownClose,

	ImuiToolboxIcon_MAX
} ImuiToolboxIcon;

typedef struct ImuiToolboxThemeButton
{
	float			height;
	ImuiBorder		padding;
} ImuiToolboxThemeButton;

typedef struct ImuiToolboxThemeCheckBox
{
	ImuiSize		size;
	float			textSpacing;
} ImuiToolboxThemeCheckBox;

typedef struct ImuiToolboxThemeSlider
{
	float			height;
	ImuiBorder		padding;
	ImuiSize		pivotSize;
} ImuiToolboxThemeSlider;

typedef struct ImuiToolboxThemeTextEdit
{
	float			height;
	ImuiBorder		padding;
	ImuiSize		cursorSize;
	double			blinkTime;
} ImuiToolboxThemeTextEdit;

typedef struct ImuiToolboxThemeProgressBar
{
	float			height;
	ImuiBorder		padding;
} ImuiToolboxThemeProgressBar;

typedef struct ImuiToolboxThemeScrollArea
{
	float			barSize;
	float			barSpacing;
	float			barMinSize;
} ImuiToolboxThemeScrollArea;

typedef struct ImuiToolboxThemeList
{
	float			itemSpacing;
} ImuiToolboxThemeList;

typedef struct ImuiToolboxThemeDropDown
{
	float			height;
	ImuiBorder		padding;

	uint32_t		listZOrder;
	ImuiBorder		listMargin;
	uint32_t		listMaxLength;

	ImuiBorder		itemPadding;
	float			itemSize;
	float			itemSpacing;
} ImuiToolboxThemeDropDown;

typedef struct ImuiToolboxThemePopup
{
	uint32_t		zOrder;
	ImuiBorder		padding;

	float			buttonSpacing;
} ImuiToolboxThemePopup;

typedef struct ImuiToolboxThemeTabView
{
	float			headerSpacing;
	float			headerCutLeft;
	float			headerCutRight;
	ImuiBorder		headerPadding;

	ImuiBorder		bodyPadding;

} ImuiToolboxThemeTabView;

typedef struct ImuiToolboxTheme
{
	ImuiColor						colors[ ImuiToolboxColor_MAX ];
	ImuiSkin						skins[ ImuiToolboxSkin_MAX ];
	ImuiImage						icons[ ImuiToolboxIcon_MAX ];

	ImuiFont*						font;

	ImuiToolboxThemeButton			button;
	ImuiToolboxThemeCheckBox		checkBox;
	ImuiToolboxThemeSlider			slider;
	ImuiToolboxThemeTextEdit		textEdit;
	ImuiToolboxThemeProgressBar		progressBar;
	ImuiToolboxThemeScrollArea		scrollArea;
	ImuiToolboxThemeList			list;
	ImuiToolboxThemeDropDown		dropDown;
	ImuiToolboxThemePopup			popup;
	ImuiToolboxThemeTabView			tabView;
} ImuiToolboxTheme;

typedef enum ImuiToolboxThemeReflectionType
{
	ImuiToolboxThemeReflectionType_Color,
	ImuiToolboxThemeReflectionType_Skin,
	ImuiToolboxThemeReflectionType_Image,
	ImuiToolboxThemeReflectionType_Font,
	ImuiToolboxThemeReflectionType_Size,
	ImuiToolboxThemeReflectionType_Border,
	ImuiToolboxThemeReflectionType_Float,
	ImuiToolboxThemeReflectionType_Double,
	ImuiToolboxThemeReflectionType_UInt32
} ImuiToolboxThemeReflectionType;

typedef struct ImuiToolboxThemeReflectionField
{
	const char*						name;
	ImuiToolboxThemeReflectionType	type;
	size_t							offset;
} ImuiToolboxThemeReflectionField;

typedef struct ImuiToolboxThemeReflection
{
	const ImuiToolboxThemeReflectionField*	fields;
	size_t									count;
} ImuiToolboxThemeReflection;

typedef struct ImuiToolboxTextBuffer ImuiToolboxTextBuffer;

typedef struct ImuiToolboxScrollAreaState ImuiToolboxScrollAreaState;

typedef struct ImuiToolboxScrollAreaContext
{
	bool							horizontalSpacing;
	bool							verticalSpacing;

	ImuiWidget*						area;
	ImuiWidget*						content;
	ImuiToolboxScrollAreaState*		state;
} ImuiToolboxScrollAreaContext;

typedef struct ImuiToolboxListState ImuiToolboxListState;

typedef struct ImuiToolboxListContext
{
	float							itemSize;
	size_t							itemCount;

	ImuiToolboxScrollAreaContext	scrollArea;

	ImuiWidget*						list;
	ImuiWidget*						listLayout;
	ImuiToolboxListState*			state;

	ImuiWidget*						item;
	size_t							itemIndex;

	size_t							beginIndex;
	size_t							endIndex;

	bool							selection;
	bool							changed;
} ImuiToolboxListContext;

typedef struct ImuiToolboxTextViewState ImuiToolboxTextViewState;

typedef struct ImuiToolboxTextViewContext
{
	ImuiToolboxListContext			list;

	bool							ownsBuffer;
	const ImuiToolboxTextBuffer*	textBuffer;
} ImuiToolboxTextViewContext;

typedef struct ImuiToolboxDropDownState ImuiToolboxDropDownState;

typedef struct ImuiToolboxDropDownContext
{
	ImuiWidget*						dropDown;

	ImuiToolboxDropDownState*		state;

	bool							changed;
} ImuiToolboxDropDownContext;

typedef struct ImuiToolboxTabViewState ImuiToolboxTabViewState;

typedef struct ImuiToolboxTabViewContext
{
	ImuiWidget*						view;
	ImuiWidget*						head;
	ImuiWidget*						body;

	size_t							headerCount;
	float							selectedHeaderOffset;
	float							selectedHeaderWidth;
	ImuiToolboxTabViewState*		state;
} ImuiToolboxTabViewContext;

ImuiToolboxThemeReflection	imuiToolboxThemeReflectionGet();

ImuiToolboxTheme*		imuiToolboxThemeGet();
void					imuiToolboxThemeFillDefault( ImuiToolboxTheme* config, ImuiFont* font );
void					imuiToolboxThemeSet( const ImuiToolboxTheme* config );

// TODO: add theme config scopes

void					imuiToolboxSpacer( ImuiWindow* window, float width, float height );
void					imuiToolboxStrecher( ImuiWindow* window, float horizontal, float vertical );

ImuiWidget*				imuiToolboxButtonBegin( ImuiWindow* window );
bool					imuiToolboxButtonEnd( ImuiWidget* button );
ImuiWidget*				imuiToolboxButtonLabelBegin( ImuiWindow* window, const char* text );
ImuiWidget*				imuiToolboxButtonLabelBeginFormat( ImuiWindow* window, const char* format, ... );
ImuiWidget*				imuiToolboxButtonLabelBeginFormatArgs( ImuiWindow* window, const char* format, va_list args );
bool					imuiToolboxButtonLabel( ImuiWindow* window, const char* text );
bool					imuiToolboxButtonLabelFormat( ImuiWindow* window, const char* format, ... );
bool					imuiToolboxButtonLabelFormatArgs( ImuiWindow* window, const char* format, va_list args );
ImuiWidget*				imuiToolboxButtonIconBegin( ImuiWindow* window, const ImuiImage* icon, ImuiSize iconSize );
bool					imuiToolboxButtonIcon( ImuiWindow* window, const ImuiImage* icon );
bool					imuiToolboxButtonIconSize( ImuiWindow* window, const ImuiImage* icon, ImuiSize iconSize );

ImuiWidget*				imuiToolboxCheckBoxBegin( ImuiWindow* window );
bool					imuiToolboxCheckBoxEnd( ImuiWidget* checkBox, bool* checked, const char* text );
bool					imuiToolboxCheckBox( ImuiWindow* window, bool* checked, const char* text );
bool					imuiToolboxCheckBoxState( ImuiWindow* window, const char* text );
bool					imuiToolboxCheckBoxStateDefault( ImuiWindow* window, const char* text, bool defaultValue );

ImuiWidget*				imuiToolboxLabelBegin( ImuiWindow* window, const char* text );
ImuiWidget*				imuiToolboxLabelBeginColor( ImuiWindow* window, const char* text, ImuiColor color );
ImuiWidget*				imuiToolboxLabelBeginLength( ImuiWindow* window, const char* text, size_t length );
ImuiWidget*				imuiToolboxLabelBeginLengthColor( ImuiWindow* window, const char* text, size_t length, ImuiColor color );
ImuiWidget*				imuiToolboxLabelBeginFormat( ImuiWindow* window, const char* format, ... );
ImuiWidget*				imuiToolboxLabelBeginFormatArgs( ImuiWindow* window, const char* format, va_list args );
void					imuiToolboxLabelEnd( ImuiWidget* label );
void					imuiToolboxLabel( ImuiWindow* window, const char* text );
void					imuiToolboxLabelLength( ImuiWindow* window, const char* text, size_t length );
void					imuiToolboxLabelColor( ImuiWindow* window, const char* text, ImuiColor color );
void					imuiToolboxLabelFormat( ImuiWindow* window, const char* format, ... );
void					imuiToolboxLabelFormatArgs( ImuiWindow* window, const char* format, va_list args );

ImuiWidget*				imuiToolboxImageBegin( ImuiWindow* window, ImuiSize imgSize );
void					imuiToolboxImageEnd( ImuiWidget* imgWidget, const ImuiImage* img );
void					imuiToolboxImage( ImuiWindow* window, const ImuiImage* img );
void					imuiToolboxImageSize( ImuiWindow* window, const ImuiImage* img, ImuiSize imgSize );

ImuiWidget*				imuiToolboxSliderBegin( ImuiWindow* window );
bool					imuiToolboxSliderEnd( ImuiWidget* slider, float* value, float min, float max );
bool					imuiToolboxSlider( ImuiWindow* window, float* value );								// value range is 0 to 1
bool					imuiToolboxSliderMinMax (ImuiWindow* window, float* value, float min, float max);
float					imuiToolboxSliderState( ImuiWindow* window );										// value range is 0 to 1
float					imuiToolboxSliderStateDefault( ImuiWindow* window, float defaultValue );			// value range is 0 to 1
float					imuiToolboxSliderStateMinMax( ImuiWindow* window, float min, float max );
float					imuiToolboxSliderStateMinMaxDefault( ImuiWindow* window, float min, float max, float defaultValue );

ImuiToolboxTextBuffer*	imuiToolboxTextBufferCreate( ImuiContext* imui );
ImuiToolboxTextBuffer*	imuiToolboxTextBufferCreateText( ImuiContext* imui, const char* text );
void					imuiToolboxTextBufferFree( ImuiToolboxTextBuffer* textBuffer );
void					imuiToolboxTextBufferSet( ImuiToolboxTextBuffer* textBuffer, const char* text );
void					imuiToolboxTextBufferAppend( ImuiToolboxTextBuffer* textBuffer, const char* text );
void					imuiToolboxTextBufferAppendLength( ImuiToolboxTextBuffer* textBuffer, const char* text, size_t textLength );
const char*				imuiToolboxTextBufferGetData( const ImuiToolboxTextBuffer* textBuffer );
size_t					imuiToolboxTextBufferGetLength( const ImuiToolboxTextBuffer* textBuffer );

ImuiWidget*				imuiToolboxTextEditBegin( ImuiWindow* window );
bool					imuiToolboxTextEditEnd( ImuiWidget* textEdit, char* buffer, size_t bufferSize, size_t* textLength );
bool					imuiToolboxTextEdit( ImuiWindow* window, char* buffer, size_t bufferSize, size_t* textLength );
const char*				imuiToolboxTextEditStateBuffer( ImuiWindow* window, size_t bufferSize );
const char*				imuiToolboxTextEditStateBufferDefault( ImuiWindow* window, size_t bufferSize, const char* defaultValue );

ImuiWidget*				imuiToolboxTextViewBegin( ImuiToolboxTextViewContext* textView, ImuiWindow* window, const char* text );
ImuiWidget*				imuiToolboxTextViewBeginBuffer( ImuiToolboxTextViewContext* textView, ImuiWindow* window, const ImuiToolboxTextBuffer* textBuffer );
void					imuiToolboxTextViewEnd( ImuiToolboxTextViewContext* textView );
void					imuiToolboxTextView( ImuiWindow* window, const char* text );
void					imuiToolboxTextViewBuffer( ImuiWindow* window, const ImuiToolboxTextBuffer* textBuffer );

void					imuiToolboxProgressBar( ImuiWindow* window, float value ); // value range 0 to 1
void					imuiToolboxProgressBarMinMax( ImuiWindow* window, float value, float min, float max );

ImuiWidget*				imuiToolboxScrollAreaBegin( ImuiToolboxScrollAreaContext* scrollArea, ImuiWindow* window );
void					imuiToolboxScrollAreaEnableSpacing( ImuiToolboxScrollAreaContext* scrollArea, bool horizontal, bool vertical );
void					imuiToolboxScrollAreaSetOffset( ImuiToolboxScrollAreaContext* scrollArea, float offsetX, float offsetY );
void					imuiToolboxScrollAreaMoveOffset( ImuiToolboxScrollAreaContext* scrollArea, float offsetX, float offsetY );
void					imuiToolboxScrollAreaOffsetTo( ImuiToolboxScrollAreaContext* scrollArea, const ImuiWidget* widgetToScrollTo );
void					imuiToolboxScrollAreaEnd( ImuiToolboxScrollAreaContext* scrollArea );

ImuiWidget*				imuiToolboxListBegin( ImuiToolboxListContext* list, ImuiWindow* window, float itemSize, size_t itemCount, bool selection );
size_t					imuiToolboxListGetBeginIndex( const ImuiToolboxListContext* list );
size_t					imuiToolboxListGetEndIndex( const ImuiToolboxListContext* list );
size_t					imuiToolboxListGetSelectedIndex( const ImuiToolboxListContext* list );
void					imuiToolboxListSetSelectedIndex( ImuiToolboxListContext* list, size_t index );
ImuiWidget*				imuiToolboxListNextItem( ImuiToolboxListContext* list );
ImuiWidget*				imuiToolboxListNextItemId( ImuiToolboxListContext* list, ImuiId id );
bool					imuiToolboxListEnd( ImuiToolboxListContext* list );

ImuiWidget*				imuiToolboxDropDownBegin( ImuiToolboxDropDownContext* dropDown, ImuiWindow* window, const char** items, size_t itemCount, size_t itemStride );
size_t					imuiToolboxDropDownGetSelectedIndex( const ImuiToolboxDropDownContext* dropDown );
void					imuiToolboxDropDownSetSelectedIndex( const ImuiToolboxDropDownContext* dropDown, size_t index );
bool					imuiToolboxDropDownEnd( ImuiToolboxDropDownContext* dropDown );
size_t					imuiToolboxDropDown( ImuiWindow* window, const char** items, size_t itemCount, size_t itemStride );

ImuiWindow*				imuiToolboxPopupBegin( ImuiWindow* window );
ImuiWindow*				imuiToolboxPopupBeginSurface( ImuiSurface* surface );
size_t					imuiToolboxPopupEndButtons( ImuiWindow* popupWindow, const char** buttons, size_t buttonCount );
void					imuiToolboxPopupEnd( ImuiWindow* popupWindow );

ImuiWidget*				imuiToolboxTabViewBegin( ImuiToolboxTabViewContext* tabView, ImuiWindow* window );
size_t					imuiToolboxTabViewGetSelectedIndex( const ImuiToolboxTabViewContext* tabView );
void					imuiToolboxTabViewSetSelectedIndex( ImuiToolboxTabViewContext* tabView, size_t index );
bool					imuiToolboxTabViewHeader( ImuiToolboxTabViewContext* tabView, const char* text );
ImuiWidget*				imuiToolboxTabViewHeaderBegin( ImuiToolboxTabViewContext* tabView );
bool					imuiToolboxTabViewHeaderEnd( ImuiToolboxTabViewContext* tabView, ImuiWidget* tabHeader );
ImuiWidget*				imuiToolboxTabViewBodyBegin( ImuiToolboxTabViewContext* tabView );
void					imuiToolboxTabViewBodyEnd( ImuiToolboxTabViewContext* tabView );
void					imuiToolboxTabViewEnd( ImuiToolboxTabViewContext* tabView );

#ifdef __cplusplus
}
#endif
