#pragma once

#include "imui.h"

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

	ImUiToolboxSkin_MAX
};

typedef struct ImUiToolboxConfig ImUiToolboxConfig;
struct ImUiToolboxConfig
{
	ImUiColor		colors[ ImUiToolboxColor_MAX ];
	ImUiSkin		skins[ ImUiToolboxSkin_MAX ];

	ImUiFont*		font;

	ImUiBorder		buttonPadding;

	ImUiSize		checkBoxSize;
	float			checkBoxTextSpacing;

	float			sliderHeight;
	ImUiBorder		sliderPadding;
	float			sliderPivotSize;

	float			textEditHeight;
	ImUiBorder		textEditPadding;
	ImUiSize		textEditCursorSize;
	float			textEditBlinkTime;

	float			progressBarHeight;
	ImUiBorder		progressBarPadding;
};

void				ImUiToolboxFillDefaultConfig( ImUiToolboxConfig* config, ImUiFont* font );
void				ImUiToolboxSetConfig( const ImUiToolboxConfig* config );

bool				ImUiToolboxButtonLabel( ImUiWindow* window, ImUiStringView text );
bool				ImUiToolboxButtonLabelFormat( ImUiWindow* window, const char* format, ... );

bool				ImUiToolboxCheckBox( ImUiWindow* window, bool* checked, ImUiStringView text );
bool				ImUiToolboxCheckBoxState( ImUiWindow* window, ImUiStringView text );

void				ImUiToolboxLabel( ImUiWindow* window, ImUiStringView text );
void				ImUiToolboxLabelFormat( ImUiWindow* window, const char* format, ... );

bool				ImUiToolboxSlider( ImUiWindow* window, float* value );								// value range is 0 to 1
bool				ImUiToolboxSliderMinMax (ImUiWindow* window, float* value, float min, float max);
float				ImUiToolboxSliderState( ImUiWindow* window );										// value range is 0 to 1
float				ImUiToolboxSliderStateMinMax( ImUiWindow* window, float min, float max );

bool				ImUiToolboxTextEdit( ImUiWindow* window, char* buffer, size_t bufferSize, size_t textLength );

void				ImUiToolboxProgressBar( ImUiWindow* window, float value ); // value range 0 to 1
void				ImUiToolboxProgressBarMinMax( ImUiWindow* window, float value, float min, float max );
