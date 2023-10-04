#pragma once

#include "imui.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum ImUiToolboxColor ImUiToolboxColor;
enum ImUiToolboxColor
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
	ImUiToolboxColor_CheckBoxCheckedHover,
	ImUiToolboxColor_CheckBoxCheckedClicked,

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

	ImUiToolboxColor_MAX
};

typedef enum ImUiToolboxSkin ImUiToolboxSkin;
enum ImUiToolboxSkin
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

	ImUiToolboxSkin_MAX
};

typedef struct ImUiToolboxButtonConfig ImUiToolboxButtonConfig;
struct ImUiToolboxButtonConfig
{
	float			height;
	ImUiBorder		padding;
};

typedef struct ImUiToolboxCheckBoxConfig ImUiToolboxCheckBoxConfig;
struct ImUiToolboxCheckBoxConfig
{
	ImUiSize		size;
	float			textSpacing;
};

typedef struct ImUiToolboxSliderConfig ImUiToolboxSliderConfig;
struct ImUiToolboxSliderConfig
{
	float			height;
	ImUiBorder		padding;
	float			pivotSize;
};

typedef struct ImUiToolboxTextEditConfig ImUiToolboxTextEditConfig;
struct ImUiToolboxTextEditConfig
{
	float			height;
	ImUiBorder		padding;
	ImUiSize		cursorSize;
	float			blinkTime;
};

typedef struct ImUiToolboxProgressBarConfig ImUiToolboxProgressBarConfig;
struct ImUiToolboxProgressBarConfig
{
	float			height;
	ImUiBorder		padding;
};

typedef struct ImUiToolboxScrollAreaConfig ImUiToolboxScrollAreaConfig;
struct ImUiToolboxScrollAreaConfig
{
	float			barSize;
	float			barSpacing;
	float			barMinSize;
};

typedef struct ImUiToolboxListConfig ImUiToolboxListConfig;
struct ImUiToolboxListConfig
{
	float			itemSpacing;
};

typedef struct ImUiToolboxConfig ImUiToolboxConfig;
struct ImUiToolboxConfig
{
	ImUiColor						colors[ ImUiToolboxColor_MAX ];
	ImUiSkin						skins[ ImUiToolboxSkin_MAX ];

	ImUiFont*						font;

	ImUiToolboxButtonConfig			button;
	ImUiToolboxCheckBoxConfig		checkBox;
	ImUiToolboxSliderConfig			slider;
	ImUiToolboxTextEditConfig		textEdit;
	ImUiToolboxProgressBarConfig	progressBar;
	ImUiToolboxScrollAreaConfig		scrollArea;
	ImUiToolboxListConfig			list;
};

typedef struct ImUiToolboxListState ImUiToolboxListState;
typedef struct ImUiToolboxListContext ImUiToolboxListContext;
struct ImUiToolboxListContext
{
	float					itemSize;
	size_t					itemCount;

	ImUiWidget*				list;
	ImUiWidget*				listLayout;
	ImUiToolboxListState*	state;

	ImUiWidget*				item;
	size_t					itemIndex;

	size_t					beginIndex;
	size_t					endIndex;
};

void				ImUiToolboxFillDefaultConfig( ImUiToolboxConfig* config, ImUiFont* font );
void				ImUiToolboxSetConfig( const ImUiToolboxConfig* config );

ImUiWidget*			ImUiToolboxButtonLabelBegin( ImUiWindow* window, ImUiStringView text );
bool				ImUiToolboxButtonLabelEnd( ImUiWidget* button );
bool				ImUiToolboxButtonLabel( ImUiWindow* window, ImUiStringView text );
bool				ImUiToolboxButtonLabelFormat( ImUiWindow* window, const char* format, ... );
bool				ImUiToolboxButtonLabelFormatArgs( ImUiWindow* window, const char* format, va_list args );

ImUiWidget*			ImUiToolboxCheckBoxBegin( ImUiWindow* window );
bool				ImUiToolboxCheckBoxEnd( ImUiWidget* checkBox, bool* checked, ImUiStringView text );
bool				ImUiToolboxCheckBox( ImUiWindow* window, bool* checked, ImUiStringView text );
bool				ImUiToolboxCheckBoxState( ImUiWindow* window, ImUiStringView text );
bool				ImUiToolboxCheckBoxStateDefault( ImUiWindow* window, ImUiStringView text, bool defaultValue );

ImUiWidget*			ImUiToolboxLabelBegin( ImUiWindow* window, ImUiStringView text );
ImUiWidget*			ImUiToolboxLabelBeginFormat( ImUiWindow* window, const char* format, ... );
ImUiWidget*			ImUiToolboxLabelBeginFormatArgs( ImUiWindow* window, const char* format, va_list args );
void				ImUiToolboxLabelEnd( ImUiWidget* label );
void				ImUiToolboxLabel( ImUiWindow* window, ImUiStringView text );
void				ImUiToolboxLabelFormat( ImUiWindow* window, const char* format, ... );
void				ImUiToolboxLabelFormatArgs( ImUiWindow* window, const char* format, va_list args );

ImUiWidget*			ImUiToolboxSliderBegin( ImUiWindow* window );
bool				ImUiToolboxSliderEnd( ImUiWidget* slider, float* value, float min, float max );
bool				ImUiToolboxSlider( ImUiWindow* window, float* value );								// value range is 0 to 1
bool				ImUiToolboxSliderMinMax (ImUiWindow* window, float* value, float min, float max);
float				ImUiToolboxSliderState( ImUiWindow* window );										// value range is 0 to 1
float				ImUiToolboxSliderStateDefault( ImUiWindow* window, float defaultValue );			// value range is 0 to 1
float				ImUiToolboxSliderStateMinMax( ImUiWindow* window, float min, float max );
float				ImUiToolboxSliderStateMinMaxDefault( ImUiWindow* window, float min, float max, float defaultValue );

ImUiWidget*			ImUiToolboxTextEditBegin( ImUiWindow* window );
bool				ImUiToolboxTextEditEnd( ImUiWidget* textEdit, char* buffer, size_t bufferSize, size_t* textLength );
bool				ImUiToolboxTextEdit( ImUiWindow* window, char* buffer, size_t bufferSize, size_t* textLength );
ImUiStringView		ImUiToolboxTextEditStateBuffer( ImUiWindow* window, size_t bufferSize );

void				ImUiToolboxProgressBar( ImUiWindow* window, float value ); // value range 0 to 1
void				ImUiToolboxProgressBarMinMax( ImUiWindow* window, float value, float min, float max );

ImUiWidget*			ImUiToolboxScrollAreaBegin( ImUiWindow* window );
void				ImUiToolboxScrollAreaEnd( ImUiWidget* scroll );

void				ImUiToolboxListBeginHorizontal( ImUiToolboxListContext* list, ImUiWindow* window, float itemSize, size_t itemCount );
void				ImUiToolboxListBeginVertical( ImUiToolboxListContext* list, ImUiWindow* window, float itemSize, size_t itemCount );
size_t				ImUiToolboxListGetBeginIndex( const ImUiToolboxListContext* list );
size_t				ImUiToolboxListGetEndIndex( const ImUiToolboxListContext* list );
size_t				ImUiToolboxListGetSelectedIndex( const ImUiToolboxListContext* list );
ImUiWidget*			ImUiToolboxListNextItem( ImUiToolboxListContext* list );
void				ImUiToolboxListEnd( ImUiToolboxListContext* list );

#ifdef __cplusplus
}
#endif
