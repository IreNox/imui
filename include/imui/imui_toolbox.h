#pragma once

#include "imui.h"

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdarg.h>

typedef enum ImUiToolboxColor
{
	ImUiToolboxColor_Text,

	ImUiToolboxColor_Button,
	ImUiToolboxColor_ButtonHover,
	ImUiToolboxColor_ButtonClicked,
	ImUiToolboxColor_ButtonText,

	ImUiToolboxColor_CheckBox,
	ImUiToolboxColor_CheckBoxHover,
	ImUiToolboxColor_CheckBoxClicked,
	ImUiToolboxColor_CheckBoxUnchecked,
	ImUiToolboxColor_CheckBoxChecked,

	ImUiToolboxColor_SliderBackground,
	ImUiToolboxColor_SliderPivot,
	ImUiToolboxColor_SliderPivotHover,
	ImUiToolboxColor_SliderPivotClicked,

	ImUiToolboxColor_TextEditBackground,
	ImUiToolboxColor_TextEditText,
	ImUiToolboxColor_TextEditCursor,
	ImUiToolboxColor_TextEditSelection,

	ImUiToolboxColor_ProgressBarBackground,
	ImUiToolboxColor_ProgressBarProgress,

	ImUiToolboxColor_ScrollAreaBarBackground,
	ImUiToolboxColor_ScrollAreaBarPivot,

	ImUiToolboxColor_ListItemHover,
	ImUiToolboxColor_ListItemClicked,
	ImUiToolboxColor_ListItemSelected,

	ImUiToolboxColor_DropDown,
	ImUiToolboxColor_DropDownText,
	ImUiToolboxColor_DropDownIcon,
	ImUiToolboxColor_DropDownHover,
	ImUiToolboxColor_DropDownClicked,
	ImUiToolboxColor_DropDownOpen,
	ImUiToolboxColor_DropDownList,
	ImUiToolboxColor_DropDownItemText,
	ImUiToolboxColor_DropDownItemHover,
	ImUiToolboxColor_DropDownItemClicked,
	ImUiToolboxColor_DropDownItemSelected,

	ImUiToolboxColor_PopupBackground,
	ImUiToolboxColor_Popup,

	ImUiToolboxColor_TabViewHeadBackground,
	ImUiToolboxColor_TabViewHeaderActive,
	ImUiToolboxColor_TabViewHeaderInactive,
	ImUiToolboxColor_TabViewBody,

	ImUiToolboxColor_MAX
} ImUiToolboxColor;

typedef enum ImUiToolboxSkin
{
	ImUiToolboxSkin_Button,
	ImUiToolboxSkin_ButtonHover,
	ImUiToolboxSkin_ButtonClicked,
	ImUiToolboxSkin_CheckBox,
	ImUiToolboxSkin_CheckBoxChecked,
	ImUiToolboxSkin_SliderBackground,
	ImUiToolboxSkin_SliderPivot,
	ImUiToolboxSkin_TextEditBackground,
	ImUiToolboxSkin_ProgressBarBackground,
	ImUiToolboxSkin_ProgressBarProgress,
	ImUiToolboxSkin_ScrollAreaBarBackground,
	ImUiToolboxSkin_ScrollAreaBarPivot,
	ImUiToolboxSkin_ListItem,
	ImUiToolboxSkin_ItemSelected,
	ImUiToolboxSkin_DropDown,
	ImUiToolboxSkin_DropDownList,
	ImUiToolboxSkin_DropDownItem,
	ImUiToolboxSkin_Popup,
	ImUiToolboxSkin_TabViewHeadBackground,
	ImUiToolboxSkin_TabViewHeaderActive,
	ImUiToolboxSkin_TabViewHeaderInactive,
	ImUiToolboxSkin_TabViewBody,

	ImUiToolboxSkin_MAX
} ImUiToolboxSkin;

typedef enum ImUiToolboxIcon
{
	ImUiToolboxIcon_CheckBoxUnchecked,
	ImUiToolboxIcon_CheckBoxChecked,
	ImUiToolboxIcon_DropDownOpen,
	ImUiToolboxIcon_DropDownClose,

	ImUiToolboxIcon_MAX
} ImUiToolboxIcon;

typedef struct ImUiToolboxThemeButton
{
	float			height;
	ImUiBorder		padding;
} ImUiToolboxThemeButton;

typedef struct ImUiToolboxThemeCheckBox
{
	ImUiSize		size;
	float			textSpacing;
} ImUiToolboxThemeCheckBox;

typedef struct ImUiToolboxThemeSlider
{
	float			height;
	ImUiBorder		padding;
	ImUiSize		pivotSize;
} ImUiToolboxThemeSlider;

typedef struct ImUiToolboxThemeTextEdit
{
	float			height;
	ImUiBorder		padding;
	ImUiSize		cursorSize;
	double			blinkTime;
} ImUiToolboxThemeTextEdit;

typedef struct ImUiToolboxThemeProgressBar
{
	float			height;
	ImUiBorder		padding;
} ImUiToolboxThemeProgressBar;

typedef struct ImUiToolboxThemeScrollArea
{
	float			barSize;
	float			barSpacing;
	float			barMinSize;
} ImUiToolboxThemeScrollArea;

typedef struct ImUiToolboxThemeList
{
	float			itemSpacing;
} ImUiToolboxThemeList;

typedef struct ImUiToolboxThemeDropDown
{
	float			height;
	ImUiBorder		padding;

	uint32_t		listZOrder;
	ImUiBorder		listMargin;
	uint32_t		listMaxLength;

	ImUiBorder		itemPadding;
	float			itemSize;
	float			itemSpacing;
} ImUiToolboxThemeDropDown;

typedef struct ImUiToolboxThemePopup
{
	uint32_t		zOrder;
	ImUiBorder		padding;

	float			buttonSpacing;
} ImUiToolboxThemePopup;

typedef struct ImUiToolboxThemeTabView
{
	float			headerSpacing;
	float			headerCutLeft;
	float			headerCutRight;
	ImUiBorder		headerPadding;

	ImUiBorder		bodyPadding;

} ImUiToolboxThemeTabView;

typedef struct ImUiToolboxTheme
{
	ImUiColor						colors[ ImUiToolboxColor_MAX ];
	ImUiSkin						skins[ ImUiToolboxSkin_MAX ];
	ImUiImage						icons[ ImUiToolboxIcon_MAX ];

	ImUiFont*						font;

	ImUiToolboxThemeButton			button;
	ImUiToolboxThemeCheckBox		checkBox;
	ImUiToolboxThemeSlider			slider;
	ImUiToolboxThemeTextEdit		textEdit;
	ImUiToolboxThemeProgressBar		progressBar;
	ImUiToolboxThemeScrollArea		scrollArea;
	ImUiToolboxThemeList			list;
	ImUiToolboxThemeDropDown		dropDown;
	ImUiToolboxThemePopup			popup;
	ImUiToolboxThemeTabView			tabView;
} ImUiToolboxTheme;

typedef enum ImUiToolboxThemeReflectionType
{
	ImUiToolboxThemeReflectionType_Color,
	ImUiToolboxThemeReflectionType_Skin,
	ImUiToolboxThemeReflectionType_Image,
	ImUiToolboxThemeReflectionType_Font,
	ImUiToolboxThemeReflectionType_Size,
	ImUiToolboxThemeReflectionType_Border,
	ImUiToolboxThemeReflectionType_Float,
	ImUiToolboxThemeReflectionType_Double,
	ImUiToolboxThemeReflectionType_UInt32
} ImUiToolboxThemeReflectionType;

typedef struct ImUiToolboxThemeReflectionField
{
	const char*						name;
	ImUiToolboxThemeReflectionType	type;
	size_t							offset;
} ImUiToolboxThemeReflectionField;

typedef struct ImUiToolboxThemeReflection
{
	const ImUiToolboxThemeReflectionField*	fields;
	size_t									count;
} ImUiToolboxThemeReflection;

typedef struct ImUiToolboxTextBuffer ImUiToolboxTextBuffer;

typedef struct ImUiToolboxScrollAreaState ImUiToolboxScrollAreaState;

typedef struct ImUiToolboxScrollAreaContext
{
	bool							horizontalSpacing;
	bool							verticalSpacing;

	ImUiWidget*						area;
	ImUiWidget*						content;
	ImUiToolboxScrollAreaState*		state;
} ImUiToolboxScrollAreaContext;

typedef struct ImUiToolboxListState ImUiToolboxListState;

typedef struct ImUiToolboxListContext
{
	float							itemSize;
	size_t							itemCount;

	ImUiToolboxScrollAreaContext	scrollArea;

	ImUiWidget*						list;
	ImUiWidget*						listLayout;
	ImUiToolboxListState*			state;

	ImUiWidget*						item;
	size_t							itemIndex;

	size_t							beginIndex;
	size_t							endIndex;

	bool							selection;
	bool							changed;
} ImUiToolboxListContext;

typedef struct ImUiToolboxTextViewState ImUiToolboxTextViewState;

typedef struct ImUiToolboxTextViewContext
{
	ImUiToolboxListContext			list;

	bool							ownsBuffer;
	const ImUiToolboxTextBuffer*	textBuffer;
} ImUiToolboxTextViewContext;

typedef struct ImUiToolboxDropDownState ImUiToolboxDropDownState;

typedef struct ImUiToolboxDropDownContext
{
	ImUiWidget*						dropDown;

	ImUiToolboxDropDownState*		state;

	bool							changed;
} ImUiToolboxDropDownContext;

typedef struct ImUiToolboxTabViewState ImUiToolboxTabViewState;

typedef struct ImUiToolboxTabViewContext
{
	ImUiWidget*						view;
	ImUiWidget*						head;
	ImUiWidget*						body;

	size_t							headerCount;
	float							selectedHeaderOffset;
	float							selectedHeaderWidth;
	ImUiToolboxTabViewState*		state;
} ImUiToolboxTabViewContext;

ImUiToolboxThemeReflection	ImUiToolboxThemeReflectionGet();

ImUiToolboxTheme*		ImUiToolboxThemeGet();
void					ImUiToolboxThemeFillDefault( ImUiToolboxTheme* config, ImUiFont* font );
void					ImUiToolboxThemeSet( const ImUiToolboxTheme* config );

// TODO: add theme config scopes

void					ImUiToolboxSpacer( ImUiWindow* window, float width, float height );
void					ImUiToolboxStrecher( ImUiWindow* window, float horizontal, float vertical );

ImUiWidget*				ImUiToolboxButtonBegin( ImUiWindow* window );
bool					ImUiToolboxButtonEnd( ImUiWidget* button );
ImUiWidget*				ImUiToolboxButtonLabelBegin( ImUiWindow* window, const char* text );
ImUiWidget*				ImUiToolboxButtonLabelBeginFormat( ImUiWindow* window, const char* format, ... );
ImUiWidget*				ImUiToolboxButtonLabelBeginFormatArgs( ImUiWindow* window, const char* format, va_list args );
bool					ImUiToolboxButtonLabel( ImUiWindow* window, const char* text );
bool					ImUiToolboxButtonLabelFormat( ImUiWindow* window, const char* format, ... );
bool					ImUiToolboxButtonLabelFormatArgs( ImUiWindow* window, const char* format, va_list args );
ImUiWidget*				ImUiToolboxButtonIconBegin( ImUiWindow* window, const ImUiImage* icon, ImUiSize iconSize );
bool					ImUiToolboxButtonIcon( ImUiWindow* window, const ImUiImage* icon );
bool					ImUiToolboxButtonIconSize( ImUiWindow* window, const ImUiImage* icon, ImUiSize iconSize );

ImUiWidget*				ImUiToolboxCheckBoxBegin( ImUiWindow* window );
bool					ImUiToolboxCheckBoxEnd( ImUiWidget* checkBox, bool* checked, const char* text );
bool					ImUiToolboxCheckBox( ImUiWindow* window, bool* checked, const char* text );
bool					ImUiToolboxCheckBoxState( ImUiWindow* window, const char* text );
bool					ImUiToolboxCheckBoxStateDefault( ImUiWindow* window, const char* text, bool defaultValue );

ImUiWidget*				ImUiToolboxLabelBegin( ImUiWindow* window, const char* text );
ImUiWidget*				ImUiToolboxLabelBeginColor( ImUiWindow* window, const char* text, ImUiColor color );
ImUiWidget*				ImUiToolboxLabelBeginLength( ImUiWindow* window, const char* text, size_t length );
ImUiWidget*				ImUiToolboxLabelBeginLengthColor( ImUiWindow* window, const char* text, size_t length, ImUiColor color );
ImUiWidget*				ImUiToolboxLabelBeginFormat( ImUiWindow* window, const char* format, ... );
ImUiWidget*				ImUiToolboxLabelBeginFormatArgs( ImUiWindow* window, const char* format, va_list args );
void					ImUiToolboxLabelEnd( ImUiWidget* label );
void					ImUiToolboxLabel( ImUiWindow* window, const char* text );
void					ImUiToolboxLabelLength( ImUiWindow* window, const char* text, size_t length );
void					ImUiToolboxLabelColor( ImUiWindow* window, const char* text, ImUiColor color );
void					ImUiToolboxLabelFormat( ImUiWindow* window, const char* format, ... );
void					ImUiToolboxLabelFormatArgs( ImUiWindow* window, const char* format, va_list args );

ImUiWidget*				ImUiToolboxImageBegin( ImUiWindow* window, ImUiSize imgSize );
void					ImUiToolboxImageEnd( ImUiWidget* imgWidget, const ImUiImage* img );
void					ImUiToolboxImage( ImUiWindow* window, const ImUiImage* img );
void					ImUiToolboxImageSize( ImUiWindow* window, const ImUiImage* img, ImUiSize imgSize );

ImUiWidget*				ImUiToolboxSliderBegin( ImUiWindow* window );
bool					ImUiToolboxSliderEnd( ImUiWidget* slider, float* value, float min, float max );
bool					ImUiToolboxSlider( ImUiWindow* window, float* value );								// value range is 0 to 1
bool					ImUiToolboxSliderMinMax (ImUiWindow* window, float* value, float min, float max);
float					ImUiToolboxSliderState( ImUiWindow* window );										// value range is 0 to 1
float					ImUiToolboxSliderStateDefault( ImUiWindow* window, float defaultValue );			// value range is 0 to 1
float					ImUiToolboxSliderStateMinMax( ImUiWindow* window, float min, float max );
float					ImUiToolboxSliderStateMinMaxDefault( ImUiWindow* window, float min, float max, float defaultValue );

ImUiToolboxTextBuffer*	ImUiToolboxTextBufferCreate( ImUiContext* imui );
ImUiToolboxTextBuffer*	ImUiToolboxTextBufferCreateText( ImUiContext* imui, const char* text );
void					ImUiToolboxTextBufferFree( ImUiToolboxTextBuffer* textBuffer );
void					ImUiToolboxTextBufferSet( ImUiToolboxTextBuffer* textBuffer, const char* text );
void					ImUiToolboxTextBufferAppend( ImUiToolboxTextBuffer* textBuffer, const char* text );
void					ImUiToolboxTextBufferAppendLength( ImUiToolboxTextBuffer* textBuffer, const char* text, size_t textLength );
const char*				ImUiToolboxTextBufferGetData( const ImUiToolboxTextBuffer* textBuffer );
size_t					ImUiToolboxTextBufferGetLength( const ImUiToolboxTextBuffer* textBuffer );

ImUiWidget*				ImUiToolboxTextEditBegin( ImUiWindow* window );
bool					ImUiToolboxTextEditEnd( ImUiWidget* textEdit, char* buffer, size_t bufferSize, size_t* textLength );
bool					ImUiToolboxTextEdit( ImUiWindow* window, char* buffer, size_t bufferSize, size_t* textLength );
const char*				ImUiToolboxTextEditStateBuffer( ImUiWindow* window, size_t bufferSize );
const char*				ImUiToolboxTextEditStateBufferDefault( ImUiWindow* window, size_t bufferSize, const char* defaultValue );

ImUiWidget*				ImUiToolboxTextViewBegin( ImUiToolboxTextViewContext* textView, ImUiWindow* window, const char* text );
ImUiWidget*				ImUiToolboxTextViewBeginBuffer( ImUiToolboxTextViewContext* textView, ImUiWindow* window, const ImUiToolboxTextBuffer* textBuffer );
void					ImUiToolboxTextViewEnd( ImUiToolboxTextViewContext* textView );
void					ImUiToolboxTextView( ImUiWindow* window, const char* text );
void					ImUiToolboxTextViewBuffer( ImUiWindow* window, const ImUiToolboxTextBuffer* textBuffer );

void					ImUiToolboxProgressBar( ImUiWindow* window, float value ); // value range 0 to 1
void					ImUiToolboxProgressBarMinMax( ImUiWindow* window, float value, float min, float max );

ImUiWidget*				ImUiToolboxScrollAreaBegin( ImUiToolboxScrollAreaContext* scrollArea, ImUiWindow* window );
void					ImUiToolboxScrollAreaEnableSpacing( ImUiToolboxScrollAreaContext* scrollArea, bool horizontal, bool vertical );
void					ImUiToolboxScrollAreaSetOffset( ImUiToolboxScrollAreaContext* scrollArea, float offsetX, float offsetY );
void					ImUiToolboxScrollAreaMoveOffset( ImUiToolboxScrollAreaContext* scrollArea, float offsetX, float offsetY );
void					ImUiToolboxScrollAreaOffsetTo( ImUiToolboxScrollAreaContext* scrollArea, const ImUiWidget* widgetToScrollTo );
void					ImUiToolboxScrollAreaEnd( ImUiToolboxScrollAreaContext* scrollArea );

ImUiWidget*				ImUiToolboxListBegin( ImUiToolboxListContext* list, ImUiWindow* window, float itemSize, size_t itemCount, bool selection );
size_t					ImUiToolboxListGetBeginIndex( const ImUiToolboxListContext* list );
size_t					ImUiToolboxListGetEndIndex( const ImUiToolboxListContext* list );
size_t					ImUiToolboxListGetSelectedIndex( const ImUiToolboxListContext* list );
void					ImUiToolboxListSetSelectedIndex( ImUiToolboxListContext* list, size_t index );
ImUiWidget*				ImUiToolboxListNextItem( ImUiToolboxListContext* list );
ImUiWidget*				ImUiToolboxListNextItemId( ImUiToolboxListContext* list, ImUiId id );
bool					ImUiToolboxListEnd( ImUiToolboxListContext* list );

ImUiWidget*				ImUiToolboxDropDownBegin( ImUiToolboxDropDownContext* dropDown, ImUiWindow* window, const char** items, size_t itemCount, size_t itemStride );
size_t					ImUiToolboxDropDownGetSelectedIndex( const ImUiToolboxDropDownContext* dropDown );
void					ImUiToolboxDropDownSetSelectedIndex( const ImUiToolboxDropDownContext* dropDown, size_t index );
bool					ImUiToolboxDropDownEnd( ImUiToolboxDropDownContext* dropDown );
size_t					ImUiToolboxDropDown( ImUiWindow* window, const char** items, size_t itemCount, size_t itemStride );

ImUiWindow*				ImUiToolboxPopupBegin( ImUiWindow* window );
ImUiWindow*				ImUiToolboxPopupBeginSurface( ImUiSurface* surface );
size_t					ImUiToolboxPopupEndButtons( ImUiWindow* popupWindow, const char** buttons, size_t buttonCount );
void					ImUiToolboxPopupEnd( ImUiWindow* popupWindow );

ImUiWidget*				ImUiToolboxTabViewBegin( ImUiToolboxTabViewContext* tabView, ImUiWindow* window );
size_t					ImUiToolboxTabViewGetSelectedIndex( const ImUiToolboxTabViewContext* tabView );
void					ImUiToolboxTabViewSetSelectedIndex( ImUiToolboxTabViewContext* tabView, size_t index );
bool					ImUiToolboxTabViewHeader( ImUiToolboxTabViewContext* tabView, const char* text );
ImUiWidget*				ImUiToolboxTabViewHeaderBegin( ImUiToolboxTabViewContext* tabView );
bool					ImUiToolboxTabViewHeaderEnd( ImUiToolboxTabViewContext* tabView, ImUiWidget* tabHeader );
ImUiWidget*				ImUiToolboxTabViewBodyBegin( ImUiToolboxTabViewContext* tabView );
void					ImUiToolboxTabViewBodyEnd( ImUiToolboxTabViewContext* tabView );
void					ImUiToolboxTabViewEnd( ImUiToolboxTabViewContext* tabView );

#ifdef __cplusplus
}
#endif
