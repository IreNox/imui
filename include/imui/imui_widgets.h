#pragma once

#include "imui.h"

typedef enum ImUiWidgetsColor ImUiWidgetsColor;
enum ImUiWidgetsColor
{
	ImUiWidgetsColor_Text,

	ImUiWidgetsColor_Button,
	ImUiWidgetsColor_ButtonHover,
	ImUiWidgetsColor_ButtonClicked,
	ImUiWidgetsColor_ButtonText,

	ImUiWidgetsColor_CheckBox,
	ImUiWidgetsColor_CheckBoxHover,
	ImUiWidgetsColor_CheckBoxClicked,
	ImUiWidgetsColor_CheckBoxChecked,
	ImUiWidgetsColor_CheckBoxCheckedHover,
	ImUiWidgetsColor_CheckBoxCheckedClicked,

	ImUiWidgetsColor_SliderBackground,
	ImUiWidgetsColor_SliderPivot,
	ImUiWidgetsColor_SliderPivotHover,
	ImUiWidgetsColor_SliderPivotClicked,

	ImUiWidgetsColor_MAX
};

typedef enum ImUiWidgetsSkin ImUiWidgetsSkin;
enum ImUiWidgetsSkin
{
	ImUiWidgetsSkin_Button,
	ImUiWidgetsSkin_CheckBox,
	ImUiWidgetsSkin_CheckBoxChecked,
	ImUiWidgetsSkin_SliderBackground,
	ImUiWidgetsSkin_SliderPivot,

	ImUiWidgetsSkin_MAX
};

typedef struct ImUiWidgetsConfig ImUiWidgetsConfig;
struct ImUiWidgetsConfig
{
	ImUiColor		colors[ ImUiWidgetsColor_MAX ];
	ImUiSkin		skins[ ImUiWidgetsSkin_MAX ];

	ImUiFont*		font;

	ImUiBorder	buttonPadding;

	ImUiSize		checkBoxSize;
	float			checkBoxTextSpacing;

	ImUiBorder	sliderPadding;
	float			sliderHeight;
	float			sliderPivotSize;
};

void			ImUiWidgetsSetConfig( const ImUiWidgetsConfig* config );

bool			ImUiWidgetsButton( ImUiWindow* window, ImUiStringView text );

bool			ImUiWidgetsCheckBox( ImUiWindow* window, ImUiStringView text, bool* checked );

void			ImUiWidgetsLabel( ImUiWindow* window, ImUiStringView text );

bool			ImUiWidgetsSlider( ImUiWindow* window, float* value ); // value range is 0 to 1
bool			ImUiWidgetsSliderMinMax (ImUiWindow* window, float* value, float min, float max);
