#include "imui/imui_widgets.h"

#include "imui_types.h"

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

	if( layout )
	{
		ImUiDrawWidgetTextColor( buttonText, layout, s_config.colors[ ImUiWidgetsColor_ButtonText ] );
	}

	ImUiWidgetEnd( buttonText );

	ImUiWidgetEnd( buttonFrame );

	return inputState.hasMouseReleased;
}

bool ImUiWidgetsCheckBox( ImUiWindow* window, ImUiStringView text, bool* checked )
{
	ImUiWidget* checkBoxFrame = ImUiWidgetBegin( window );
	ImUiWidgetSetPadding( checkBoxFrame, ImUiBorderCreate( 0.0f, s_config.checkBoxSize.width + s_config.checkBoxTextSpacing, 0.0f, 0.0f ) );
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

	const ImUiRect rect = ImUiRectCreatePosSize( ImUiWidgetGetPos( checkBoxFrame ), s_config.checkBoxSize );
	ImUiDrawSkinColor( checkBoxFrame, rect, s_config.skins[ *checked ? ImUiWidgetsSkin_CheckBoxChecked : ImUiWidgetsSkin_CheckBox ], color );

	ImUiWidget* checkBoxText = ImUiWidgetBegin( window );

	ImUiTextLayout* layout = ImUiTextLayoutCreateWidget( checkBoxText, s_config.font, text );
	const ImUiSize textSize = ImUiTextLayoutGetSize( layout );
	ImUiWidgetSetFixedSize( checkBoxText, textSize );
	ImUiWidgetSetVAlign( checkBoxText, ImUiVAlign_Center );

	if( layout )
	{
		ImUiDrawWidgetTextColor( checkBoxText, layout, s_config.colors[ ImUiWidgetsColor_Text ] );
	}

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

	if( layout )
	{
		ImUiDrawWidgetTextColor( label, layout, s_config.colors[ ImUiWidgetsColor_Text ] );
	}

	ImUiWidgetEnd( label );
}

bool ImUiWidgetsSlider( ImUiWindow* window, float* value )
{
	return ImUiWidgetsSliderMinMax( window, value, 0.0f, 1.0f );
}

bool ImUiWidgetsSliderMinMax( ImUiWindow* window, float* value, float min, float max )
{
	ImUiContext* imui = ImUiWindowGetContext( window );

	ImUiWidget* sliderFrame = ImUiWidgetBegin( window );
	ImUiWidgetSetStretch( sliderFrame, ImUiSizeCreateHorizintal() );
	ImUiWidgetSetPadding( sliderFrame, s_config.sliderPadding );
	ImUiWidgetSetFixedHeight( sliderFrame, s_config.sliderHeight );

	ImUiInputWidgetState frameInputState;
	ImUiInputGetWidgetState( sliderFrame, &frameInputState );

	ImUiDrawWidgetSkinColor( sliderFrame, s_config.skins[ ImUiWidgetsSkin_SliderBackground ], s_config.colors[ ImUiWidgetsColor_SliderBackground ] );

	ImUiWidget* sliderPivot = ImUiWidgetBegin( window );
	ImUiWidgetSetFixedSize( sliderPivot, ImUiSizeCreate( s_config.sliderPivotSize, s_config.sliderHeight ) );

	const ImUiRect sliderInnerRect = ImUiWidgetGetInnerRect( sliderFrame );
	const float normalizedValue		= (*value - min) / (max - min);
	const float sliderPivotX		= (normalizedValue * (sliderInnerRect.size.width - s_config.sliderPivotSize));
	const float sliderPivotOffset	= sliderPivotX < 0.0f ? 0.0f : sliderPivotX;;

	ImUiWidgetSetMargin( sliderPivot, ImUiBorderCreate( 0.0f, sliderPivotOffset, 0.0f, 0.0f ) );

	ImUiInputWidgetState inputState;
	ImUiInputGetWidgetState( sliderPivot, &inputState );

	ImUiColor color = s_config.colors[ ImUiWidgetsColor_SliderPivot ];
	if( frameInputState.isMouseDown )
	{
		color = s_config.colors[ ImUiWidgetsColor_SliderPivotClicked ];
	}
	else if( inputState.isMouseOver )
	{
		color = s_config.colors[ ImUiWidgetsColor_SliderPivotHover ];
	}

	bool changed = false;

	if( frameInputState.isMouseDown )
	{
		const ImUiPos mousePos		= ImUiInputGetMousePos( imui );
		const float mouseValueNorm		= (mousePos.x - sliderInnerRect.pos.x) / (sliderInnerRect.size.width - s_config.sliderPivotSize);
		const float mouseValueNormClamp	= mouseValueNorm > 1.0f ? 1.0f : (mouseValueNorm < 0.0f ? 0.0f : mouseValueNorm);
		IMUI_ASSERT( mouseValueNormClamp >= 0.0f && mouseValueNormClamp <= 1.0f );
		const float mouseValue			= (mouseValueNormClamp * (max - min)) + min;

		*value = mouseValue;
		changed = true;
	}

	ImUiDrawWidgetSkinColor( sliderPivot, s_config.skins[ ImUiWidgetsSkin_SliderPivot ], color );

	ImUiWidgetEnd( sliderPivot );

	ImUiWidgetEnd( sliderFrame );

	return changed;
}
