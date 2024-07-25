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
	ImUiToolboxColor_DropDownListItemText,
	ImUiToolboxColor_DropDownListItemHover,
	ImUiToolboxColor_DropDownListItemClicked,
	ImUiToolboxColor_DropDownListItemSelected,

	ImUiToolboxColor_PopupBackground,
	ImUiToolboxColor_Popup,

	ImUiToolboxColor_MAX
} ImUiToolboxColor;

typedef enum ImUiToolboxSkin
{
	ImUiToolboxSkin_Button,
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
	ImUiToolboxSkin_ListItemSelected,
	ImUiToolboxSkin_DropDown,
	ImUiToolboxSkin_DropDownList,
	ImUiToolboxSkin_DropDownListItem,
	ImUiToolboxSkin_Popup,

	ImUiToolboxSkin_MAX
} ImUiToolboxSkin;

typedef enum ImUiToolboxIcon
{
	ImUiToolboxIcon_CheckBoxChecked,
	ImUiToolboxIcon_DropDownOpenIcon,
	ImUiToolboxIcon_DropDownCloseIcon,

	ImUiToolboxIcon_MAX
} ImUiToolboxIcon;

typedef struct ImUiToolboxButtonConfig
{
	float			height;
	ImUiBorder		padding;
} ImUiToolboxButtonConfig;

typedef struct ImUiToolboxCheckBoxConfig
{
	ImUiSize		size;
	float			textSpacing;
} ImUiToolboxCheckBoxConfig;

typedef struct ImUiToolboxSliderConfig
{
	float			height;
	ImUiBorder		padding;
	ImUiSize		pivotSize;
} ImUiToolboxSliderConfig;

typedef struct ImUiToolboxTextEditConfig
{
	float			height;
	ImUiBorder		padding;
	ImUiSize		cursorSize;
	float			blinkTime;
} ImUiToolboxTextEditConfig;

typedef struct ImUiToolboxProgressBarConfig
{
	float			height;
	ImUiBorder		padding;
} ImUiToolboxProgressBarConfig;

typedef struct ImUiToolboxScrollAreaConfig
{
	float			barSize;
	float			barSpacing;
	float			barMinSize;
} ImUiToolboxScrollAreaConfig;

typedef struct ImUiToolboxListConfig
{
	float			itemSpacing;
} ImUiToolboxListConfig;

typedef struct ImUiToolboxDropDownConfig
{
	float			height;
	ImUiBorder		padding;

	uint32_t		listZOrder;
	uint32_t		maxListLength;

	ImUiBorder		itemPadding;
	float			itemSize;
	float			itemSpacing;
} ImUiToolboxDropDownConfig;

typedef struct ImUiToolboxPopupConfig
{
	uint32_t		zOrder;
	ImUiBorder		padding;

	float			buttonSpacing;
} ImUiToolboxPopupConfig;

typedef struct ImUiToolboxConfig
{
	ImUiColor						colors[ ImUiToolboxColor_MAX ];
	ImUiSkin						skins[ ImUiToolboxSkin_MAX ];
	ImUiImage						icons[ ImUiToolboxIcon_MAX ];

	ImUiFont*						font;

	ImUiToolboxButtonConfig			button;
	ImUiToolboxCheckBoxConfig		checkBox;
	ImUiToolboxSliderConfig			slider;
	ImUiToolboxTextEditConfig		textEdit;
	ImUiToolboxProgressBarConfig	progressBar;
	ImUiToolboxScrollAreaConfig		scrollArea;
	ImUiToolboxListConfig			list;
	ImUiToolboxDropDownConfig		dropDown;
	ImUiToolboxPopupConfig			popup;
} ImUiToolboxConfig;

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

	bool							changed;
} ImUiToolboxListContext;

typedef struct ImUiToolboxDropDownState ImUiToolboxDropDownState;
typedef struct ImUiToolboxDropDownContext
{
	ImUiWidget*						dropDown;

	ImUiToolboxDropDownState*		state;

	bool							changed;
} ImUiToolboxDropDownContext;

const ImUiToolboxConfig*	ImUiToolboxGetConfig();

void			ImUiToolboxFillDefaultConfig( ImUiToolboxConfig* config, ImUiFont* font );
void			ImUiToolboxSetConfig( const ImUiToolboxConfig* config );

void			ImUiToolboxSpacer( ImUiWindow* window, float width, float height );
void			ImUiToolboxStrecher( ImUiWindow* window, float horizontal, float vertical );

ImUiWidget*		ImUiToolboxButtonBegin( ImUiWindow* window );
bool			ImUiToolboxButtonEnd( ImUiWidget* button );
ImUiWidget*		ImUiToolboxButtonLabelBegin( ImUiWindow* window, const char* text );
ImUiWidget*		ImUiToolboxButtonLabelBeginFormat( ImUiWindow* window, const char* format, ... );
ImUiWidget*		ImUiToolboxButtonLabelBeginFormatArgs( ImUiWindow* window, const char* format, va_list args );
bool			ImUiToolboxButtonLabelEnd( ImUiWidget* button );
bool			ImUiToolboxButtonLabel( ImUiWindow* window, const char* text );
bool			ImUiToolboxButtonLabelFormat( ImUiWindow* window, const char* format, ... );
bool			ImUiToolboxButtonLabelFormatArgs( ImUiWindow* window, const char* format, va_list args );
ImUiWidget*		ImUiToolboxButtonIconBegin( ImUiWindow* window, ImUiImage icon, ImUiSize iconSize );
bool			ImUiToolboxButtonIconEnd( ImUiWidget* button );
bool			ImUiToolboxButtonIcon( ImUiWindow* window, ImUiImage icon );
bool			ImUiToolboxButtonIconSize( ImUiWindow* window, ImUiImage icon, ImUiSize iconSize );

ImUiWidget*		ImUiToolboxCheckBoxBegin( ImUiWindow* window );
bool			ImUiToolboxCheckBoxEnd( ImUiWidget* checkBox, bool* checked, const char* text );
bool			ImUiToolboxCheckBox( ImUiWindow* window, bool* checked, const char* text );
bool			ImUiToolboxCheckBoxState( ImUiWindow* window, const char* text );
bool			ImUiToolboxCheckBoxStateDefault( ImUiWindow* window, const char* text, bool defaultValue );

ImUiWidget*		ImUiToolboxLabelBegin( ImUiWindow* window, const char* text );
ImUiWidget*		ImUiToolboxLabelBeginFormat( ImUiWindow* window, const char* format, ... );
ImUiWidget*		ImUiToolboxLabelBeginFormatArgs( ImUiWindow* window, const char* format, va_list args );
void			ImUiToolboxLabelEnd( ImUiWidget* label );
void			ImUiToolboxLabel( ImUiWindow* window, const char* text );
void			ImUiToolboxLabelFormat( ImUiWindow* window, const char* format, ... );
void			ImUiToolboxLabelFormatArgs( ImUiWindow* window, const char* format, va_list args );

ImUiWidget*		ImUiToolboxImageBegin( ImUiWindow* window, ImUiSize imgSize );
void			ImUiToolboxImageEnd( ImUiWidget* imgWidget, const ImUiImage* img );
void			ImUiToolboxImage( ImUiWindow* window, const ImUiImage* img );
void			ImUiToolboxImageSize( ImUiWindow* window, const ImUiImage* img, ImUiSize imgSize );

ImUiWidget*		ImUiToolboxSliderBegin( ImUiWindow* window );
bool			ImUiToolboxSliderEnd( ImUiWidget* slider, float* value, float min, float max );
bool			ImUiToolboxSlider( ImUiWindow* window, float* value );								// value range is 0 to 1
bool			ImUiToolboxSliderMinMax (ImUiWindow* window, float* value, float min, float max);
float			ImUiToolboxSliderState( ImUiWindow* window );										// value range is 0 to 1
float			ImUiToolboxSliderStateDefault( ImUiWindow* window, float defaultValue );			// value range is 0 to 1
float			ImUiToolboxSliderStateMinMax( ImUiWindow* window, float min, float max );
float			ImUiToolboxSliderStateMinMaxDefault( ImUiWindow* window, float min, float max, float defaultValue );

ImUiWidget*		ImUiToolboxTextEditBegin( ImUiWindow* window );
bool			ImUiToolboxTextEditEnd( ImUiWidget* textEdit, char* buffer, size_t bufferSize, size_t* textLength );
bool			ImUiToolboxTextEdit( ImUiWindow* window, char* buffer, size_t bufferSize, size_t* textLength );
const char*		ImUiToolboxTextEditStateBuffer( ImUiWindow* window, size_t bufferSize );
const char*		ImUiToolboxTextEditStateBufferDefault( ImUiWindow* window, size_t bufferSize, const char* defaultValue );

void			ImUiToolboxProgressBar( ImUiWindow* window, float value ); // value range 0 to 1
void			ImUiToolboxProgressBarMinMax( ImUiWindow* window, float value, float min, float max );

void			ImUiToolboxScrollAreaBegin( ImUiToolboxScrollAreaContext* scrollArea, ImUiWindow* window );
void			ImUiToolboxScrollAreaEnableSpacing( ImUiToolboxScrollAreaContext* scrollArea, bool horizontal, bool vertical );
void			ImUiToolboxScrollAreaEnd( ImUiToolboxScrollAreaContext* scrollArea );

void			ImUiToolboxListBegin( ImUiToolboxListContext* list, ImUiWindow* window, float itemSize, size_t itemCount );
size_t			ImUiToolboxListGetBeginIndex( const ImUiToolboxListContext* list );
size_t			ImUiToolboxListGetEndIndex( const ImUiToolboxListContext* list );
size_t			ImUiToolboxListGetSelectedIndex( const ImUiToolboxListContext* list );
void			ImUiToolboxListSetSelectedIndex( ImUiToolboxListContext* list, size_t index );
ImUiWidget*		ImUiToolboxListNextItem( ImUiToolboxListContext* list );
bool			ImUiToolboxListEnd( ImUiToolboxListContext* list );

void			ImUiToolboxDropDownBegin( ImUiToolboxDropDownContext* dropDown, ImUiWindow* window, const char** items, size_t itemCount, size_t itemStride );
size_t			ImUiToolboxDropDownGetSelectedIndex( const ImUiToolboxDropDownContext* dropDown );
void			ImUiToolboxDropDownSetSelectedIndex( const ImUiToolboxDropDownContext* dropDown, size_t index );
bool			ImUiToolboxDropDownEnd( ImUiToolboxDropDownContext* dropDown );
size_t			ImUiToolboxDropDown( ImUiWindow* window, const char** items, size_t itemCount, size_t itemStride );

ImUiWindow*		ImUiToolboxPopupBegin( ImUiWindow* window );
ImUiWindow*		ImUiToolboxPopupBeginSurface( ImUiSurface* surface );
size_t			ImUiToolboxPopupEndButtons( ImUiWindow* popupWindow, const char** buttons, size_t buttonCount );
void			ImUiToolboxPopupEnd( ImUiWindow* popupWindow );

#ifdef __cplusplus
}
#endif
