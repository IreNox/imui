#include "imui/imui_widgets.h"

static ImUiWidgetsConfig s_config;

void ImUiWidgetsSetConfig( const ImUiWidgetsConfig* config )
{
	s_config = *config;
}

bool ImUiWidgetsButton( ImUiWindow* window, ImUiStringView text )
{
	ImUiWidget* buttonFrame = ImUiWidgetBegin( window );
	ImUiWidgetSetPadding( buttonFrame, s_config.buttonPadding );

	ImUiInputWidgetState inputState;
	ImUiInputGetWidgetState( buttonFrame, &inputState );

	ImUiColor color = s_config.colors[ ImUiWidgetsColor_Button ];
	if( inputState.isMouseDown )
	{
		color = s_config.colors[ ImUiWidgetsColor_ButtonClicked ];
	}
	else if( inputState.isMouseOver )
	{
		color = s_config.colors[ ImUiWidgetsColor_ButtonHover ];
	}

	ImUiDrawWidgetSkinColor( buttonFrame, s_config.skins[ ImUiWidgetsSkin_Button ], color );

	ImUiWidget* buttonText = ImUiWidgetBegin( window );

	ImUiTextLayout* layout = ImUiTextLayoutCreateWidget( buttonText, s_config.font, text );
	const ImUiSize textSize = ImUiTextLayoutGetSize( layout );
	ImUiWidgetSetFixedSize( buttonText, textSize );

	ImUiDrawWidgetTextColor( buttonText, layout, s_config.colors[ ImUiWidgetsColor_ButtonText ] );

	ImUiWidgetEnd( buttonText );

	ImUiWidgetEnd( buttonFrame );

	return inputState.hasMouseReleased;
}

bool ImUiWidgetsCheckBox( ImUiWindow* window, ImUiStringView text, bool* checked )
{
	ImUiWidget* checkBoxFrame = ImUiWidgetBegin( window );
	ImUiWidgetSetPadding( checkBoxFrame, ImUiThicknessCreate( 0.0f, s_config.checkBoxSize.width + s_config.checkBoxTextSpacing, 0.0f, 0.0f ) );
	ImUiWidgetSetFixedHeight( checkBoxFrame, s_config.checkBoxSize.height );

	ImUiInputWidgetState inputState;
	ImUiInputGetWidgetState( checkBoxFrame, &inputState );

	ImUiColor color = s_config.colors[ *checked ? ImUiWidgetsColor_CheckBoxChecked : ImUiWidgetsColor_CheckBox ];
	if( inputState.isMouseDown )
	{
		color = s_config.colors[ *checked ? ImUiWidgetsColor_CheckBoxCheckedClicked : ImUiWidgetsColor_CheckBoxClicked ];
	}
	else if( inputState.isMouseOver )
	{
		color = s_config.colors[ *checked ? ImUiWidgetsColor_CheckBoxCheckedHover : ImUiWidgetsColor_CheckBoxHover ];
	}

	const ImUiRectangle rect = ImUiRectangleCreatePosSize( ImUiWidgetGetPosition( checkBoxFrame ), s_config.checkBoxSize );
	ImUiDrawSkinColor( checkBoxFrame, rect, s_config.skins[ *checked ? ImUiWidgetsSkin_CheckBoxChecked : ImUiWidgetsSkin_CheckBox ], color );

	ImUiWidget* checkBoxText = ImUiWidgetBegin( window );

	ImUiTextLayout* layout = ImUiTextLayoutCreateWidget( checkBoxText, s_config.font, text );
	const ImUiSize textSize = ImUiTextLayoutGetSize( layout );
	ImUiWidgetSetFixedSize( checkBoxText, textSize );
	ImUiWidgetSetVerticalAlignment( checkBoxText, ImUiVerticalAlignment_Center );

	ImUiDrawWidgetTextColor( checkBoxText, layout, s_config.colors[ ImUiWidgetsColor_Text ] );

	ImUiWidgetEnd( checkBoxText );

	ImUiWidgetEnd( checkBoxFrame );

	if( inputState.hasMouseReleased )
	{
		*checked = !*checked;
	}

	return inputState.hasMouseReleased;
}

void ImUiWidgetsLabel( ImUiWindow* window, ImUiStringView text )
{
	ImUiWidget* label = ImUiWidgetBegin( window );

	ImUiTextLayout* layout = ImUiTextLayoutCreateWidget( label, s_config.font, text );
	const ImUiSize textSize = ImUiTextLayoutGetSize( layout );
	ImUiWidgetSetFixedSize( label, textSize );

	ImUiDrawWidgetTextColor( label, layout, s_config.colors[ ImUiWidgetsColor_Text ] );

	ImUiWidgetEnd( label );
}
