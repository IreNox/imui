#include "imui/imui_toolbox.h"

#include "imui_internal.h"
#include "imui_memory.h"
#include "imui_types.h"

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static ImUiToolboxConfig s_config;

typedef struct ImUiToolboxScrollState ImUiToolboxScrollState;
struct ImUiToolboxScrollState
{
	ImUiPos			offset;
	bool			wasPressedX;
	bool			wasPressedY;
	ImUiPos			pressPoint;
};

typedef struct ImUiToolboxTextEditState ImUiToolboxTextEditState;
struct ImUiToolboxTextEditState
{
	bool			hasFocus;

	ImUiPos			offset;
	bool			wasPressedX;
	bool			wasPressedY;
	ImUiPos			pressPoint;

	uint32			selectionStart;
	uint32			selectionEnd;
	uint32			cursorPos;
};

typedef struct ImUiToolboxListState ImUiToolboxListState;
struct ImUiToolboxListState
{
	uintsize		selectedIndex;
};

typedef struct ImUiToolboxDropDownState ImUiToolboxDropDownState;
struct ImUiToolboxDropDownState
{
	bool			isOpen;

	uintsize		selectedIndex;
};

static void			ImUiToolboxListItemEndInternal( ImUiToolboxListContext* list );

void ImUiToolboxFillDefaultConfig( ImUiToolboxConfig* config, ImUiFont* font )
{
	const ImUiColor textColor			= ImUiColorCreateWhite();
	const ImUiColor elementColor		= ImUiColorCreateGray( 0xb2u );
	const ImUiColor elementHoverColor	= ImUiColorCreateGray( 0xe5u );
	const ImUiColor elementClickedColor	= ImUiColorCreateGray( 0x66u );
	const ImUiColor backgroundColor		= ImUiColorCreateGray( 0x4cu );
	const ImUiColor textEditCursorColor	= ImUiColorCreateBlack();

	config->colors[ ImUiToolboxColor_Text ]						= textColor;
	config->colors[ ImUiToolboxColor_Button ]					= elementColor;
	config->colors[ ImUiToolboxColor_ButtonHover ]				= elementHoverColor;
	config->colors[ ImUiToolboxColor_ButtonClicked ]			= elementClickedColor;
	config->colors[ ImUiToolboxColor_ButtonText ]				= textColor;
	config->colors[ ImUiToolboxColor_CheckBox ]					= elementColor;
	config->colors[ ImUiToolboxColor_CheckBoxHover ]			= elementHoverColor;
	config->colors[ ImUiToolboxColor_CheckBoxClicked ]			= elementClickedColor;
	config->colors[ ImUiToolboxColor_CheckBoxChecked ]			= textColor;
	config->colors[ ImUiToolboxColor_SliderBackground ]			= backgroundColor;
	config->colors[ ImUiToolboxColor_SliderPivot ]				= elementColor;
	config->colors[ ImUiToolboxColor_SliderPivotHover ]			= elementHoverColor;
	config->colors[ ImUiToolboxColor_SliderPivotClicked ]		= elementClickedColor;
	config->colors[ ImUiToolboxColor_TextEditBackground ]		= elementClickedColor;
	config->colors[ ImUiToolboxColor_TextEditText ]				= textColor;
	config->colors[ ImUiToolboxColor_TextEditCursor ]			= textEditCursorColor;
	config->colors[ ImUiToolboxColor_TextEditSelection ]		= elementColor;
	config->colors[ ImUiToolboxColor_ProgressBarBackground ]	= backgroundColor;
	config->colors[ ImUiToolboxColor_ProgressBarProgress ]		= elementColor;
	config->colors[ ImUiToolboxColor_ScrollAreaBarBackground ]	= backgroundColor;
	config->colors[ ImUiToolboxColor_ScrollAreaBarPivot ]		= elementColor;
	config->colors[ ImUiToolboxColor_ListItemHover ]			= elementHoverColor;
	config->colors[ ImUiToolboxColor_ListItemClicked ]			= elementClickedColor;
	config->colors[ ImUiToolboxColor_ListItemSelected ]			= elementColor;
	config->colors[ ImUiToolboxColor_DropDown ]					= elementClickedColor;
	config->colors[ ImUiToolboxColor_DropDownText ]				= textColor;
	config->colors[ ImUiToolboxColor_DropDownHover ]			= elementHoverColor;
	config->colors[ ImUiToolboxColor_DropDownClicked ]			= elementClickedColor;
	config->colors[ ImUiToolboxColor_DropDownOpen ]				= elementColor;
	config->colors[ ImUiToolboxColor_DropDownList ]				= backgroundColor;
	config->colors[ ImUiToolboxColor_DropDownListItemText ]		= textColor;
	config->colors[ ImUiToolboxColor_DropDownListItemHover ]	= elementHoverColor;
	config->colors[ ImUiToolboxColor_DropDownListItemClicked ]	= elementClickedColor;
	config->colors[ ImUiToolboxColor_DropDownListItemSelected ]	= elementColor;
	config->colors[ ImUiToolboxColor_PopupBackground ]			= ImUiColorCreateFloat( 0.0f, 0.0f, 0.0f, 0.2f );
	config->colors[ ImUiToolboxColor_Popup ]					= backgroundColor;
	_STATIC_ASSERT( ImUiToolboxColor_MAX == 36 );

	const ImUiSkin skin = { NULL };

	config->skins[ ImUiToolboxSkin_Button ]						= skin;
	config->skins[ ImUiToolboxSkin_CheckBox ]					= skin;
	config->skins[ ImUiToolboxSkin_CheckBoxChecked ]			= skin;
	config->skins[ ImUiToolboxSkin_SliderBackground ]			= skin;
	config->skins[ ImUiToolboxSkin_SliderPivot ]				= skin;
	config->skins[ ImUiToolboxSkin_TextEditBackground ]			= skin;
	config->skins[ ImUiToolboxSkin_ProgressBarBackground ]		= skin;
	config->skins[ ImUiToolboxSkin_ProgressBarProgress ]		= skin;
	config->skins[ ImUiToolboxSkin_ScrollAreaBarBackground ]	= skin;
	config->skins[ ImUiToolboxSkin_ScrollAreaBarPivot ]			= skin;
	config->skins[ ImUiToolboxSkin_ListItem ]					= skin;
	config->skins[ ImUiToolboxSkin_DropDown ]					= skin;
	config->skins[ ImUiToolboxSkin_DropDownList ]				= skin;
	config->skins[ ImUiToolboxSkin_DropDownListItem ]			= skin;
	config->skins[ ImUiToolboxSkin_Popup ]						= skin;
	_STATIC_ASSERT( ImUiToolboxSkin_MAX == 15 );

	const ImUiTexture image = { NULL, { 22.0f, 22.0f } };

	config->font					= font;

	config->button.height			= 25.0f;
	config->button.padding			= ImUiBorderCreateAll( 8.0f );

	config->checkBox.checkedIcon	= image;
	config->checkBox.size			= ImUiSizeCreateAll( 25.0f );
	config->checkBox.textSpacing	= 8.0f;

	config->slider.height			= 25.0f;
	config->slider.padding			= ImUiBorderCreateHorizontalVertical( 5.0f, 0.0f );
	config->slider.pivotSize		= 10.0f;

	config->textEdit.height			= 25.0f;
	config->textEdit.padding		= ImUiBorderCreateAll( 2.0f );
	config->textEdit.cursorSize		= ImUiSizeCreate( 1.0f, 21.0f );
	config->textEdit.blinkTime		= 0.53f;

	config->progressBar.height		= 25.0f;
	config->progressBar.padding		= ImUiBorderCreateAll( 2.0f );

	config->scrollArea.barSize		= 8.0f;
	config->scrollArea.barSpacing	= 8.0f;
	config->scrollArea.barMinSize	= 25.0f;

	config->list.itemSpacing		= 8.0f;

	config->dropDown.openIcon		= image;
	config->dropDown.closeIcon		= image;
	config->dropDown.height			= 25.0f;
	config->dropDown.padding		= ImUiBorderCreate( 0.0f, 4.0f, 0.0f, 0.0f );
	config->dropDown.listZOrder		= 20u;
	config->dropDown.maxListLength	= 12u;
	config->dropDown.itemPadding	= ImUiBorderCreate( 0.0f, 4.0f, 0.0f, 0.0f );
	config->dropDown.itemSize		= 25.0f;
	config->dropDown.itemSpacing	= 8.0f;

	config->popup.zOrder			= 10u;
	config->popup.padding			= ImUiBorderCreateAll( 8.0f );
	config->popup.buttonSpacing		= 4.0f;
}

void ImUiToolboxSetConfig( const ImUiToolboxConfig* config )
{
	s_config = *config;
}

void ImUiToolboxSpacer( ImUiWindow* window, float width, float height )
{
	ImUiWidget* widget = ImUiWidgetBegin( window );
	ImUiWidgetSetFixedSize( widget, ImUiSizeCreate( width, height ) );
	ImUiWidgetEnd( widget );
}

void ImUiToolboxStrecher( ImUiWindow* window, float horizontal, float vertical )
{
	ImUiWidget* widget = ImUiWidgetBegin( window );
	ImUiWidgetSetStretch( widget, ImUiSizeCreate( horizontal, vertical ) );
	ImUiWidgetEnd( widget );
}

ImUiWidget* ImUiToolboxButtonBegin( ImUiWindow* window )
{
	ImUiWidget* button = ImUiWidgetBegin( window );
	ImUiWidgetSetPadding( button, s_config.button.padding );

	ImUiWidgetInputState inputState;
	ImUiWidgetGetInputState( button, &inputState );

	ImUiColor color = s_config.colors[ ImUiToolboxColor_Button ];
	if( inputState.wasPressed && inputState.isMouseDown )
	{
		color = s_config.colors[ ImUiToolboxColor_ButtonClicked ];
	}
	else if( inputState.isMouseOver )
	{
		color = s_config.colors[ ImUiToolboxColor_ButtonHover ];
	}

	ImUiDrawWidgetSkinColor( button, s_config.skins[ ImUiToolboxSkin_Button ], color );

	return button;
}

bool ImUiToolboxButtonEnd( ImUiWidget* button )
{
	ImUiWidgetEnd( button );

	ImUiWidgetInputState inputState;
	ImUiWidgetGetInputState( button, &inputState );

	return inputState.wasPressed && inputState.hasMouseReleased;
}

ImUiWidget* ImUiToolboxButtonLabelBegin( ImUiWindow* window, ImUiStringView text )
{
	ImUiWidget* buttonFrame = ImUiToolboxButtonBegin( window );

	ImUiWidget* buttonText = ImUiWidgetBegin( window );

	ImUiTextLayout* layout = ImUiTextLayoutCreateWidget( buttonText, s_config.font, text );
	const ImUiSize textSize = ImUiTextLayoutGetSize( layout );
	ImUiWidgetSetAlign( buttonText, ImUiAlignCreateCenter() );
	ImUiWidgetSetFixedSize( buttonText, textSize );

	if( layout )
	{
		ImUiDrawWidgetTextColor( buttonText, layout, s_config.colors[ ImUiToolboxColor_ButtonText ] );
	}

	ImUiWidgetEnd( buttonText );

	return buttonFrame;
}

ImUiWidget* ImUiToolboxButtonLabelBeginFormat( ImUiWindow* window, const char* format, ... )
{
	va_list args;
	va_start( args, format );
	ImUiWidget* button = ImUiToolboxButtonLabelBeginFormatArgs( window, format, args );
	va_end( args );

	return button;
}

ImUiWidget* ImUiToolboxButtonLabelBeginFormatArgs( ImUiWindow* window, const char* format, va_list args )
{
	char buffer[ 256u ];

	int length = vsnprintf( buffer, sizeof( buffer ), format, args );
	if( length < 0 )
	{
		return false;
	}
	else if( length >= sizeof( buffer ) )
	{
		char* headBuffer = ImUiMemoryAlloc( &window->imui->allocator, length + 1u );
		if( !headBuffer )
		{
			return false;
		}

		length = vsnprintf( headBuffer, length + 1u, format, args );

		ImUiWidget* button = ImUiToolboxButtonLabelBegin( window, ImUiStringViewCreateLength( headBuffer, length ) );

		ImUiMemoryFree( &window->imui->allocator, headBuffer );
		return button;
	}

	return ImUiToolboxButtonLabelBegin( window, ImUiStringViewCreateLength( buffer, length ) );
}

bool ImUiToolboxButtonLabelEnd( ImUiWidget* button )
{
	return ImUiToolboxButtonEnd( button );
}

bool ImUiToolboxButtonLabel( ImUiWindow* window, ImUiStringView text )
{
	ImUiWidget* button = ImUiToolboxButtonLabelBegin( window, text );
	return ImUiToolboxButtonLabelEnd( button );
}

bool ImUiToolboxButtonLabelFormat( ImUiWindow* window, const char* format, ... )
{
	va_list args;
	va_start( args, format );
	const bool result = ImUiToolboxButtonLabelFormatArgs( window, format, args );
	va_end( args );

	return result;
}

bool ImUiToolboxButtonLabelFormatArgs( ImUiWindow* window, const char* format, va_list args )
{
	ImUiWidget* button = ImUiToolboxButtonLabelBeginFormatArgs( window, format, args );
	return ImUiToolboxButtonLabelEnd( button );
}

ImUiWidget* ImUiToolboxButtonIconBegin( ImUiWindow* window, ImUiTexture icon, ImUiSize iconSize )
{
	ImUiWidget* buttonFrame = ImUiToolboxButtonBegin( window );

	ImUiWidget* buttonIcon = ImUiWidgetBegin( window );
	ImUiWidgetSetAlign( buttonIcon, ImUiAlignCreateCenter() );
	ImUiWidgetSetFixedSize( buttonIcon, iconSize );

	ImUiDrawWidgetTexture( buttonIcon, icon );

	ImUiWidgetEnd( buttonIcon );

	return buttonFrame;
}

bool ImUiToolboxButtonIconEnd( ImUiWidget* button )
{
	return ImUiToolboxButtonEnd( button );
}

bool ImUiToolboxButtonIcon( ImUiWindow* window, ImUiTexture icon )
{
	return ImUiToolboxButtonIconSize( window, icon, icon.size );
}

bool ImUiToolboxButtonIconSize( ImUiWindow* window, ImUiTexture icon, ImUiSize iconSize )
{
	ImUiWidget* button = ImUiToolboxButtonIconBegin( window, icon, iconSize );
	return ImUiToolboxButtonIconEnd( button );
}

ImUiWidget* ImUiToolboxCheckBoxBegin( ImUiWindow* window )
{
	ImUiWidget* checkBoxFrame = ImUiWidgetBegin( window );
	ImUiWidgetSetPadding( checkBoxFrame, ImUiBorderCreate( 0.0f, s_config.checkBox.size.width + s_config.checkBox.textSpacing, 0.0f, 0.0f ) );
	ImUiWidgetSetFixedHeight( checkBoxFrame, s_config.checkBox.size.height );

	return checkBoxFrame;
}

bool ImUiToolboxCheckBoxEnd( ImUiWidget* checkBox, bool* checked, ImUiStringView text )
{
	ImUiWidgetInputState inputState;
	ImUiWidgetGetInputState( checkBox, &inputState );

	ImUiColor color = s_config.colors[ ImUiToolboxColor_CheckBox ];
	if( inputState.wasPressed && inputState.isMouseDown )
	{
		color = s_config.colors[ ImUiToolboxColor_CheckBoxClicked ];
	}
	else if( inputState.isMouseOver )
	{
		color = s_config.colors[ ImUiToolboxColor_CheckBoxHover ];
	}

	const ImUiRect checkBackgroundRect = ImUiRectCreatePosSize( ImUiWidgetGetPos( checkBox ), s_config.checkBox.size );
	ImUiDrawRectSkinColor( checkBox, checkBackgroundRect, s_config.skins[ ImUiToolboxSkin_CheckBox ], color );

	if( *checked )
	{
		const ImUiRect checkIconRect = ImUiRectCreateCenterPosSize( ImUiRectGetCenter( checkBackgroundRect ), s_config.checkBox.checkedIcon.size );
		ImUiDrawRectTextureColor( checkBox, checkIconRect, s_config.checkBox.checkedIcon, s_config.colors[ ImUiToolboxColor_CheckBoxChecked ] );
	}

	ImUiWidget* checkBoxText = ImUiWidgetBegin( ImUiWidgetGetWindow( checkBox ) );

	ImUiTextLayout* layout = ImUiTextLayoutCreateWidget( checkBoxText, s_config.font, text );
	const ImUiSize textSize = ImUiTextLayoutGetSize( layout );
	ImUiWidgetSetFixedSize( checkBoxText, textSize );
	ImUiWidgetSetVAlign( checkBoxText, 0.5f );

	if( layout )
	{
		ImUiDrawWidgetTextColor( checkBoxText, layout, s_config.colors[ ImUiToolboxColor_Text ] );
	}

	ImUiWidgetEnd( checkBoxText );

	ImUiWidgetEnd( checkBox );

	if( inputState.wasPressed && inputState.hasMouseReleased )
	{
		*checked = !*checked;
		return true;
	}

	return false;
}

bool ImUiToolboxCheckBox( ImUiWindow* window, bool* checked, ImUiStringView text )
{
	ImUiWidget* checkBox = ImUiToolboxCheckBoxBegin( window );
	return ImUiToolboxCheckBoxEnd( checkBox, checked, text);
}

bool ImUiToolboxCheckBoxState( ImUiWindow* window, ImUiStringView text )
{
	return ImUiToolboxCheckBoxStateDefault( window, text, false );
}

bool ImUiToolboxCheckBoxStateDefault( ImUiWindow* window, ImUiStringView text, bool defaultValue )
{
	ImUiWidget* checkBox = ImUiToolboxCheckBoxBegin( window );

	bool isNew;
	bool* checked = ImUiWidgetAllocStateNew( checkBox, sizeof( bool ), &isNew );
	if( isNew )
	{
		*checked = defaultValue;
	}

	ImUiToolboxCheckBoxEnd( checkBox, checked, text );
	return *checked;
}

ImUiWidget* ImUiToolboxLabelBegin( ImUiWindow* window, ImUiStringView text )
{
	ImUiWidget* label = ImUiWidgetBegin( window );

	ImUiTextLayout* layout = ImUiTextLayoutCreateWidget( label, s_config.font, text );
	const ImUiSize textSize = ImUiTextLayoutGetSize( layout );
	ImUiWidgetSetFixedSize( label, textSize );

	if( layout )
	{
		ImUiDrawWidgetTextColor( label, layout, s_config.colors[ ImUiToolboxColor_Text ] );
	}

	return label;
}

ImUiWidget* ImUiToolboxLabelBeginFormat( ImUiWindow* window, const char* format, ... )
{
	va_list args;
	va_start( args, format );
	ImUiWidget* label = ImUiToolboxLabelBeginFormatArgs( window, format, args );
	va_end( args );
	return label;
}

ImUiWidget* ImUiToolboxLabelBeginFormatArgs( ImUiWindow* window, const char* format, va_list args )
{
	char buffer[ 256u ];

	int length = vsnprintf( buffer, sizeof( buffer ), format, args );
	if( length < 0 )
	{
		return NULL;
	}
	else if( length >= sizeof( buffer ) )
	{
		char* headBuffer = ImUiMemoryAlloc( &window->imui->allocator, length + 1u );
		if( !headBuffer )
		{
			return NULL;
		}

		length = vsnprintf( headBuffer, length + 1u, format, args );

		ImUiWidget* label = ImUiToolboxLabelBegin( window, ImUiStringViewCreateLength( headBuffer, length ) );

		ImUiMemoryFree( &window->imui->allocator, headBuffer );
		return label;
	}

	return ImUiToolboxLabelBegin( window, ImUiStringViewCreateLength( buffer, length ) );
}

void ImUiToolboxLabelEnd( ImUiWidget* label )
{
	ImUiWidgetEnd( label );
}

void ImUiToolboxLabel( ImUiWindow* window, ImUiStringView text )
{
	ImUiWidget* label = ImUiToolboxLabelBegin( window, text );
	ImUiToolboxLabelEnd( label );
}

void ImUiToolboxLabelFormat( ImUiWindow* window, const char* format, ... )
{
	va_list args;
	va_start( args, format );
	ImUiToolboxLabelFormatArgs( window, format, args );
	va_end( args );
}

void ImUiToolboxLabelFormatArgs( ImUiWindow* window, const char* format, va_list args )
{
	ImUiWidget* label = ImUiToolboxLabelBeginFormatArgs( window, format, args );
	ImUiToolboxLabelEnd( label );
}

ImUiWidget* ImUiToolboxSliderBegin( ImUiWindow* window )
{
	ImUiWidget* slider = ImUiWidgetBegin( window );
	ImUiWidgetSetStretch( slider, ImUiSizeCreateHorizontal() );
	ImUiWidgetSetPadding( slider, s_config.slider.padding );
	ImUiWidgetSetFixedHeight( slider, s_config.slider.height );

	return slider;
}

bool ImUiToolboxSliderEnd( ImUiWidget* slider, float* value, float min, float max )
{
	ImUiWidgetInputState frameInputState;
	ImUiWidgetGetInputState( slider, &frameInputState );

	ImUiDrawWidgetSkinColor( slider, s_config.skins[ ImUiToolboxSkin_SliderBackground ], s_config.colors[ ImUiToolboxColor_SliderBackground ] );

	ImUiWidget* sliderPivot = ImUiWidgetBegin( ImUiWidgetGetWindow( slider ) );
	ImUiWidgetSetFixedSize( sliderPivot, ImUiSizeCreate( s_config.slider.pivotSize, s_config.slider.height ) );

	const ImUiRect sliderInnerRect = ImUiWidgetGetInnerRect( slider );
	const float normalizedValue		= (*value - min) / (max - min);
	//const float sliderPivotX		= (normalizedValue * (sliderInnerRect.size.width - s_config.slider.pivotSize));
	//const float sliderPivotOffset	= sliderPivotX < 0.0f ? 0.0f : roundf( sliderPivotX );

	ImUiWidgetSetHAlign( sliderPivot, normalizedValue ); // ImUiBorderCreate( 0.0f, sliderPivotOffset, 0.0f, 0.0f ) );

	ImUiWidgetInputState inputState;
	ImUiWidgetGetInputState( sliderPivot, &inputState );

	ImUiColor color = s_config.colors[ ImUiToolboxColor_SliderPivot ];
	if( frameInputState.wasPressed )
	{
		color = s_config.colors[ ImUiToolboxColor_SliderPivotClicked ];
	}
	else if( inputState.isMouseOver )
	{
		color = s_config.colors[ ImUiToolboxColor_SliderPivotHover ];
	}

	bool changed = false;

	if( frameInputState.wasPressed )
	{
		const float mouseValueNorm		= (frameInputState.relativeMousePos.x - (s_config.slider.pivotSize * 0.5f)) / (sliderInnerRect.size.width - s_config.slider.pivotSize);
		const float mouseValueNormClamp	= mouseValueNorm > 1.0f ? 1.0f : (mouseValueNorm < 0.0f ? 0.0f : mouseValueNorm);
		IMUI_ASSERT( mouseValueNormClamp >= 0.0f && mouseValueNormClamp <= 1.0f );
		const float mouseValue			= (mouseValueNormClamp * (max - min)) + min;

		*value = mouseValue;
		changed = true;
	}

	ImUiDrawWidgetSkinColor( sliderPivot, s_config.skins[ ImUiToolboxSkin_SliderPivot ], color );

	ImUiWidgetEnd( sliderPivot );

	ImUiWidgetEnd( slider );

	return changed;
}

bool ImUiToolboxSlider( ImUiWindow* window, float* value )
{
	return ImUiToolboxSliderMinMax( window, value, 0.0f, 1.0f );
}

bool ImUiToolboxSliderMinMax( ImUiWindow* window, float* value, float min, float max )
{
	ImUiWidget* sliderFrame = ImUiToolboxSliderBegin( window );
	return ImUiToolboxSliderEnd( sliderFrame, value, min, max );
}

float ImUiToolboxSliderState( ImUiWindow* window )
{
	return ImUiToolboxSliderStateMinMaxDefault( window, 0.0f, 1.0f, 0.0f );
}

float ImUiToolboxSliderStateDefault( ImUiWindow* window, float defaultValue )
{
	return ImUiToolboxSliderStateMinMaxDefault( window, 0.0f, 1.0f, defaultValue );
}

float ImUiToolboxSliderStateMinMax( ImUiWindow* window, float min, float max )
{
	return ImUiToolboxSliderStateMinMaxDefault( window, min, max, min );
}

float ImUiToolboxSliderStateMinMaxDefault( ImUiWindow* window, float min, float max, float defaultValue )
{
	IMUI_ASSERT( min <= max );
	IMUI_ASSERT( defaultValue >= min );
	IMUI_ASSERT( defaultValue <= max );

	ImUiWidget* sliderFrame = ImUiToolboxSliderBegin( window );

	bool isNew;
	float* value = ImUiWidgetAllocStateNew( sliderFrame, sizeof( float ), &isNew );
	if( isNew )
	{
		*value = defaultValue;
	}

	ImUiToolboxSliderEnd( sliderFrame, value, min, max );
	return *value;
}

ImUiWidget* ImUiToolboxTextEditBegin( ImUiWindow* window )
{
	ImUiWidget* textEditFrame = ImUiWidgetBegin( window );
	ImUiWidgetSetStretch( textEditFrame, ImUiSizeCreateHorizontal() );
	ImUiWidgetSetPadding( textEditFrame, s_config.textEdit.padding );
	ImUiWidgetSetFixedHeight( textEditFrame, s_config.textEdit.height );

	ImUiDrawWidgetSkinColor( textEditFrame, s_config.skins[ ImUiToolboxSkin_TextEditBackground ], s_config.colors[ ImUiToolboxColor_TextEditBackground ] );

	return textEditFrame;
}

bool ImUiToolboxTextEditEnd( ImUiWidget* textEdit, char* buffer, size_t bufferSize, size_t* textLength )
{
	IMUI_ASSERT( buffer );
	IMUI_ASSERT( bufferSize > 0u );

	ImUiContext* imui = ImUiWidgetGetContext( textEdit );

	uintsize textLengthInternal;
	if( textLength )
	{
		textLengthInternal = *textLength;
	}
	else
	{
		textLengthInternal = strlen( buffer );
	}

	ImUiWidgetInputState inputState;
	ImUiWidgetGetInputState( textEdit, &inputState );

	if( inputState.isMouseOver )
	{
		ImUiInputSetMouseCursor( imui, ImUiInputMouseCursor_IBeam );
	}

	ImUiWidget* text = ImUiWidgetBegin( ImUiWidgetGetWindow( textEdit ) );

	bool isNew;
	ImUiToolboxTextEditState* state = ImUiWidgetAllocStateNew( text, sizeof( *state ), &isNew );

	if( ImUiInputHasMouseButtonReleased( imui, ImUiInputMouseButton_Left ) )
	{
		state->hasFocus = inputState.hasMouseReleased;
		if( state->hasFocus )
		{
			state->cursorPos = (uint32)textLengthInternal;
		}
	}

	const ImUiRect textRect = ImUiWidgetGetRect( text );
	const ImUiRect textEditInnerRect = ImUiWidgetGetInnerRect( textEdit );

	ImUiTextLayout* layout = ImUiTextLayoutCreateWidget( text, s_config.font, ImUiStringViewCreate( buffer ) );
	const ImUiSize textSize = ImUiTextLayoutGetSize( layout );
	ImUiWidgetSetFixedSize( text, textSize );

	bool changed = false;
	if( state->hasFocus )
	{
		const uint32 mods = ImUiInputGetKeyModifiers( imui );

		const ImUiStringView textInput = ImUiInputGetText( imui );
		if( textInput.length > 0u )
		{
			const uintsize remainingSize	= bufferSize - textLengthInternal - 1u;
			const uintsize newSize			= IMUI_MIN( textInput.length, remainingSize );

			if( state->selectionStart != state->selectionEnd )
			{
				memmove( buffer + state->selectionStart, buffer + state->selectionEnd, state->selectionEnd - state->selectionStart );
				state->cursorPos = state->selectionStart;

				state->selectionStart	= 0u;
				state->selectionEnd		= 0u;
			}

			if( state->cursorPos != textLengthInternal )
			{
				memmove( buffer + state->cursorPos + newSize, buffer + state->cursorPos, textLengthInternal - state->cursorPos );
			}

			memcpy( buffer + state->cursorPos, textInput.data, newSize );
			textLengthInternal += newSize;
			buffer[ textLengthInternal ] = '\0';

			state->cursorPos += (uint32)newSize;

			changed = true;
		}

		if( ImUiInputHasKeyPressed( imui, ImUiInputKey_Backspace ) )
		{
			if( state->selectionStart != state->selectionEnd )
			{
				memmove( buffer + state->selectionStart, buffer + state->selectionEnd, state->selectionEnd - state->selectionStart );
				state->cursorPos = state->selectionStart;

				state->selectionStart	= 0u;
				state->selectionEnd		= 0u;

				changed = true;
			}
			else if( state->cursorPos > 0u )
			{
				for( uintsize i = state->cursorPos - 1u; i < textLengthInternal; ++i )
				{
					buffer[ i ] = buffer[ i + 1u ];
				}

				textLengthInternal--;
				state->cursorPos--;

				changed = true;
			}
		}
		else if( ImUiInputHasKeyPressed( imui, ImUiInputKey_Delete ) )
		{
			if( state->selectionStart != state->selectionEnd )
			{
				memmove( buffer + state->selectionStart, buffer + state->selectionEnd, state->selectionEnd - state->selectionStart );
				state->cursorPos = state->selectionStart;

				state->selectionStart	= 0u;
				state->selectionEnd		= 0u;

				changed = true;
			}
			else if( textLengthInternal > state->cursorPos )
			{
				for( uintsize i = state->cursorPos; i < textLengthInternal; ++i )
				{
					buffer[ i ] = buffer[ i + 1u ];
				}

				textLengthInternal--;

				changed = true;
			}
		}

		sint32 nextCursorPos = (sint32)state->cursorPos;
		if( ImUiInputHasKeyPressed( imui, ImUiInputKey_Left ) ||
			ImUiInputHasKeyPressed( imui, ImUiInputKey_Right ) )
		{
			const sint32 direction = ImUiInputHasKeyPressed( imui, ImUiInputKey_Left ) ? -1 : 1;
			if( mods & (ImUiInputModifier_LeftCtrl | ImUiInputModifier_RightCtrl) )
			{
				nextCursorPos += direction; \

					for( ; nextCursorPos > 0 && nextCursorPos <= (sint32)textLengthInternal; nextCursorPos += direction )
					{
						const char c = buffer[ nextCursorPos ];
						if( (c>= 'a' && c <= 'z') ||
							(c>= 'A' && c <= 'Z') ||
							(c>= '0' && c <= '9') )
						{
							continue;
						}

						break;
					}
			}
			else
			{
				nextCursorPos += direction;
			}
		}
		else if( ImUiInputHasKeyPressed( imui, ImUiInputKey_Home ) )
		{
			nextCursorPos = 0u;
		}
		else if( ImUiInputHasKeyPressed( imui, ImUiInputKey_End ) )
		{
			nextCursorPos = (sint32)textLengthInternal;
		}

		if( nextCursorPos != (sint32)state->cursorPos )
		{
			nextCursorPos = IMUI_MAX( nextCursorPos, 0 );
			nextCursorPos = IMUI_MIN( nextCursorPos, (sint32)textLengthInternal );

			const uint32 nextCursorPosU = (uint32)nextCursorPos;
			if( mods & (ImUiInputModifier_LeftShift | ImUiInputModifier_RightShift) )
			{
				if( state->selectionStart == state->selectionEnd )
				{
					state->selectionStart	= IMUI_MIN( state->cursorPos, nextCursorPosU );
					state->selectionEnd		= IMUI_MAX( state->cursorPos, nextCursorPosU );
				}
				else if( state->selectionStart == state->cursorPos )
				{
					if( nextCursorPosU > state->selectionEnd )
					{
						state->selectionStart	= state->selectionEnd;
						state->selectionEnd		= nextCursorPosU;
					}
					else
					{
						state->selectionStart	= nextCursorPosU;
					}
				}
				else
				{
					if( nextCursorPosU < state->selectionStart )
					{
						state->selectionEnd		= state->selectionStart;
						state->selectionStart	= nextCursorPosU;
					}
					else
					{
						state->selectionEnd		= nextCursorPosU;
					}
				}
			}
			else
			{
				state->selectionStart	= 0u;
				state->selectionEnd		= 0u;
			}

			state->cursorPos = nextCursorPosU;
		}

		if( state->selectionStart != state->selectionEnd )
		{
			const ImUiPos startPos			= ImUiTextLayoutGetGlyphPos( layout, state->selectionStart );
			const ImUiPos endPos			= ImUiTextLayoutGetGlyphPos( layout, state->selectionEnd );

			const ImUiRect selection = ImUiRectCreate(
				textRect.pos.x + startPos.x,
				textEditInnerRect.pos.y,
				endPos.x - startPos.x,
				textEditInnerRect.size.height
			);
			ImUiDrawRectColor( textEdit, selection, s_config.colors[ ImUiToolboxColor_TextEditSelection ] );
		}
	}

	if( layout )
	{
		ImUiDrawWidgetTextColor( text, layout, s_config.colors[ ImUiToolboxColor_Text ] );
	}

	if( state->hasFocus )
	{
		const float blinkValue	= fmodf( ImUiWidgetGetTime( textEdit ), s_config.textEdit.blinkTime * 2.0f );
		const bool blink		= blinkValue > s_config.textEdit.blinkTime;
		if( blink )
		{
			const ImUiPos cursorPos			= ImUiTextLayoutGetGlyphPos( layout, state->cursorPos );
			const ImUiPos cursorPosTop		= ImUiPosCreate( textRect.pos.x + cursorPos.x, textEditInnerRect.pos.y );
			const ImUiPos cursorPosBottom	= ImUiPosCreate( cursorPosTop.x, cursorPosTop.y + textEditInnerRect.size.height );
			ImUiDrawLine( textEdit, cursorPosTop, cursorPosBottom, s_config.colors[ ImUiToolboxColor_TextEditCursor ] );
		}
	}

	ImUiWidgetEnd( text );

	ImUiWidgetEnd( textEdit );

	if( textLength )
	{
		*textLength = textLengthInternal;
	}

	return changed;
}

bool ImUiToolboxTextEdit( ImUiWindow* window, char* buffer, size_t bufferSize, size_t* textLength )
{
	ImUiWidget* textEdit = ImUiToolboxTextEditBegin( window );
	return ImUiToolboxTextEditEnd( textEdit, buffer, bufferSize, textLength );
}

ImUiStringView ImUiToolboxTextEditStateBuffer( ImUiWindow* window, size_t bufferSize )
{
	return ImUiToolboxTextEditStateBufferDefault( window, bufferSize, ImUiStringViewCreateEmpty() );
}

ImUiStringView ImUiToolboxTextEditStateBufferDefault( ImUiWindow* window, size_t bufferSize, ImUiStringView defaultValue )
{
	ImUiWidget* textEdit = ImUiToolboxTextEditBegin( window );

	bool isNew;
	char* buffer = (char*)ImUiWidgetAllocStateNew( textEdit, bufferSize, &isNew );
	if( isNew )
	{
		const size_t length = IMUI_MIN( bufferSize - 1u, defaultValue.length );
		memcpy( buffer, defaultValue.data, length );
		buffer[ length ] = '\0';
	}

	ImUiToolboxTextEditEnd( textEdit, buffer, bufferSize, NULL );
	return ImUiStringViewCreate( buffer );
}

void ImUiToolboxProgressBar( ImUiWindow* window, float value )
{
	ImUiToolboxProgressBarMinMax( window, value, 0.0f, 1.0f );
}

void ImUiToolboxProgressBarMinMax( ImUiWindow* window, float value, float min, float max )
{
	ImUiWidget* progressBar = ImUiWidgetBeginNamed( window, IMUI_STR( "progress_bar" ));
	ImUiWidgetSetStretch( progressBar, ImUiSizeCreateHorizontal() );
	ImUiWidgetSetPadding( progressBar, s_config.progressBar.padding );
	ImUiWidgetSetFixedHeight( progressBar, s_config.progressBar.height );

	ImUiDrawWidgetSkinColor( progressBar, s_config.skins[ ImUiToolboxSkin_ProgressBarBackground ], s_config.colors[ ImUiToolboxColor_ProgressBarBackground ] );

	const ImUiRect barRect = ImUiWidgetGetInnerRect( progressBar );

	ImUiRect progressRect;
	if( value < min )
	{
		const float time		= ImUiWindowGetTime( window );
		const float cos			= (cosf( time * 8.0f ) * 0.15f) + 0.15f;
		const float sin			= (sinf( time * 4.0f ) * 0.5f) + 0.5f;
		const float width		= ceilf( barRect.size.width * (0.1f + cos) );
		const float margin		= floorf( sin * (barRect.size.width - width) );

		progressRect = ImUiRectCreate(
			barRect.pos.x + margin,
			barRect.pos.y,
			width,
			barRect.size.height
		);
	}
	else
	{
		const float valueNorm	= (value - min) / (max - min);
		const float width		= ceilf( barRect.size.width * valueNorm );

		progressRect = ImUiRectCreatePos(
			barRect.pos,
			width,
			barRect.size.height
		);
	}

	ImUiDrawRectSkinColor( progressBar, progressRect, s_config.skins[ ImUiToolboxSkin_ProgressBarProgress ], s_config.colors[ ImUiToolboxColor_ProgressBarProgress ] );

	ImUiWidgetEnd( progressBar );
}

ImUiWidget* ImUiToolboxScrollAreaBegin( ImUiWindow* window )
{
	ImUiWidget* scrollFrame = ImUiWidgetBegin( window );

	ImUiToolboxScrollState* state = ImUiWidgetAllocState( scrollFrame, sizeof( *state ) );
	ImUiWidgetSetLayoutScroll( scrollFrame, state->offset );

	//ImUiWidget* scrollArea = ImUiWidgetBeginNamed( window, IMUI_STR( "scroll_area" ) );

	//const ImUiRect frameRect = ImUiWidgetGetRect( scrollFrame );
	//const ImUiSize maxSize = ImUiSizeCreate(
	//	horizontal ? IMUI_FLOAT_MAX : frameRect.size.width,
	//	vertical ? IMUI_FLOAT_MAX : frameRect.size.height
	//);
	//ImUiWidgetSetMaxSize( scrollArea, maxSize );

	return scrollFrame;
}

void ImUiToolboxScrollAreaEnd( ImUiWidget* scroll )
{
	ImUiToolboxScrollState* state = (ImUiToolboxScrollState*)ImUiWidgetGetState( scroll );
	//ImUiWidget* scrollArea = ImUiWidgetGetFirstChild( scroll );
	ImUiWindow* window = ImUiWidgetGetWindow( scroll );

	const ImUiRect frameRect = ImUiWidgetGetRect( scroll );

	ImUiSize areaSize = ImUiSizeCreateZero();
	//const ImUiPos baseOffset = ImUiPosSubPos( state->offset, frameRect.pos );
	for( ImUiWidget* child = ImUiWidgetGetFirstChild( scroll ); child; child = ImUiWidgetGetNextSibling( child ) )
	{
		const ImUiRect childRect = ImUiWidgetGetRect( child );
		//const ImUiPos childBr = ImUiRectGetBottomRight( childRect );

		areaSize.width	= IMUI_MAX( areaSize.width, childRect.size.width );
		areaSize.height	= IMUI_MAX( areaSize.height, childRect.size.height );
	}

	ImUiWidgetInputState frameInputState;
	ImUiWidgetGetInputState( scroll, &frameInputState );

	const float barMargin		= s_config.scrollArea.barSize + s_config.scrollArea.barSpacing;
	const bool hasHorizontalBar	= areaSize.width > frameRect.size.width;
	const bool hasVerticalBar	= areaSize.height > frameRect.size.height;

	//ImUiWidgetSetPadding( scroll, ImUiBorderCreate( 0.0f, 0.0f, hasHorizontalBar * barMargin, hasVerticalBar * barMargin ) );

	//ImUiWidgetEnd( scrollArea );

	if( hasVerticalBar )
	{
		ImUiWidget* scrollBar = ImUiWidgetBeginNamed( window, IMUI_STR( "scroll_bar" ) );
		ImUiWidgetSetMargin( scrollBar, ImUiBorderCreate( state->offset.y, 0.0f, 0.0f, 0.0f ) );
		ImUiWidgetSetHAlign( scrollBar, 1.0f );
		ImUiWidgetSetFixedSize( scrollBar, ImUiSizeCreate( s_config.scrollArea.barSize, frameRect.size.height ) );

		const ImUiRect barRect		= ImUiWidgetGetRect( scrollBar );
		const float barSizeFactor	= frameRect.size.height / areaSize.height;
		const float barSize			= IMUI_MAX( frameRect.size.height * barSizeFactor, s_config.scrollArea.barMinSize );
		const float barOffset		= (state->offset.y / areaSize.height) * frameRect.size.height;

		const ImUiRect barPivotRect = ImUiRectCreate(
			barRect.pos.x,
			frameRect.pos.y + barOffset,
			s_config.scrollArea.barSize,
			barSize
		);

		ImUiWidgetInputState inputState;
		ImUiWidgetGetInputState( scrollBar, &inputState );

		if( inputState.wasPressed )
		{
			if( !state->wasPressedY &&
				ImUiRectIncludesPos( barPivotRect, ImUiInputGetMousePos( ImUiWidgetGetContext( scrollBar ) ) ) )
			{
				state->pressPoint.y	= inputState.relativeMousePos.y;
				state->pressPoint.y	-= barOffset;
				state->wasPressedY	= true;
			}

			if( state->wasPressedY )
			{
				const float newBarOffset	= inputState.relativeMousePos.y - state->pressPoint.y;
				const float newOffset		= (newBarOffset / barRect.size.height) * areaSize.height;

				state->offset.y = newOffset;
			}
		}
		else
		{
			state->wasPressedY = false;
		}

		ImUiDrawWidgetSkinColor( scrollBar, s_config.skins[ ImUiToolboxSkin_ScrollAreaBarBackground ], s_config.colors[ ImUiToolboxColor_ScrollAreaBarBackground ] );
		ImUiDrawRectSkinColor( scrollBar, barPivotRect, s_config.skins[ ImUiToolboxSkin_ScrollAreaBarPivot ], s_config.colors[ ImUiToolboxColor_ScrollAreaBarPivot ] );

		ImUiWidgetEnd( scrollBar );
	}

	if( frameInputState.isMouseOver )
	{
		state->offset = ImUiPosSubPos( state->offset, ImUiPosScale( ImUiInputGetMouseScrollDelta( ImUiWindowGetContext( window ) ), 80.0f ) );
	}

	state->offset = ImUiPosMax( ImUiPosCreateZero(), ImUiPosMin( state->offset, ImUiSizeToPos( ImUiSizeSubSize( areaSize, frameRect.size ) ) ) );

	ImUiWidgetEnd( scroll );
}

void ImUiToolboxListBegin( ImUiToolboxListContext* list, ImUiWindow* window, float itemSize, size_t itemCount )
{
	IMUI_ASSERT( list );

	const float totalItemSize = itemSize + s_config.list.itemSpacing;

	list->list = ImUiToolboxScrollAreaBegin( window );

	list->listLayout = ImUiWidgetBegin( window );
	ImUiWidgetSetStretch( list->listLayout, ImUiSizeCreate( 1.0f, 0.0f ) );
	ImUiWidgetSetLayoutVerticalSpacing( list->listLayout, s_config.list.itemSpacing );
	if( itemCount > 0u )
	{
		ImUiWidgetSetFixedHeight( list->listLayout, (totalItemSize * itemCount) - s_config.list.itemSpacing );
	}

	bool isNew;
	list->state = (ImUiToolboxListState*)ImUiWidgetAllocStateNew( list->listLayout, sizeof( ImUiToolboxListState ), &isNew );
	if( isNew )
	{
		list->state->selectedIndex = (uintsize)-1;
	}

	const ImUiRect listRect		= ImUiWidgetGetRect( list->list );
	const ImUiRect layoutRect	= ImUiWidgetGetRect( list->listLayout );

	list->itemSize		= itemSize;
	list->itemCount		= itemCount;
	list->beginIndex	= (uintsize)((listRect.pos.y - layoutRect.pos.y) / totalItemSize);
	list->endIndex		= list->beginIndex + (uintsize)ceilf( (listRect.size.height + totalItemSize) / totalItemSize );
	list->endIndex		= IMUI_MIN( list->endIndex, itemCount );

	list->item			= NULL;
	list->itemIndex		= list->beginIndex - 1u;

	list->changed		= false;
}

size_t ImUiToolboxListGetBeginIndex( const ImUiToolboxListContext* list )
{
	return list->beginIndex;
}

size_t ImUiToolboxListGetEndIndex( const ImUiToolboxListContext* list )
{
	return list->endIndex;
}

size_t ImUiToolboxListGetSelectedIndex( const ImUiToolboxListContext* list )
{
	return list->state->selectedIndex;
}

void ImUiToolboxListSetSelectedIndex( ImUiToolboxListContext* list, size_t index )
{
	list->state->selectedIndex = index;
}

static void ImUiToolboxListItemEndInternal( ImUiToolboxListContext* list )
{
	ImUiWidget* item = ImUiWidgetGetLastChild( list->listLayout );
	if( item )
	{
		ImUiWidgetEnd( item );
		list->item = NULL;
	}
}

ImUiWidget* ImUiToolboxListNextItem( ImUiToolboxListContext* list )
{
	ImUiToolboxListItemEndInternal( list );

	list->itemIndex++;

	ImUiWidget* item = ImUiWidgetBegin( ImUiWidgetGetWindow( list->list ) );
	ImUiWidgetSetStretch( item, ImUiSizeCreate( 1.0f, 0.0f ) );
	ImUiWidgetSetFixedHeight( item, list->itemSize );

	if( list->beginIndex > 0 &&
		list->itemIndex == list->beginIndex )
	{
		const float totalItemSize = list->itemSize + s_config.list.itemSpacing;
		ImUiWidgetSetMargin( item, ImUiBorderCreate( totalItemSize * list->beginIndex, 0.0f, 0.0f, 0.0f ) );
	}

	ImUiWidgetInputState inputState;
	ImUiWidgetGetInputState( item, &inputState );

	if( inputState.isMouseDown )
	{
		ImUiDrawWidgetSkinColor( item, s_config.skins[ ImUiToolboxSkin_ListItem ], s_config.colors[ ImUiToolboxColor_ListItemClicked ] );
	}
	else if( inputState.isMouseOver )
	{
		ImUiDrawWidgetSkinColor( item, s_config.skins[ ImUiToolboxSkin_ListItem ], s_config.colors[ ImUiToolboxColor_ListItemHover ] );
	}
	else if( list->itemIndex == list->state->selectedIndex )
	{
		ImUiDrawWidgetSkinColor( item, s_config.skins[ ImUiToolboxSkin_ListItem ], s_config.colors[ ImUiToolboxColor_ListItemSelected ] );
	}

	if( inputState.hasMouseReleased )
	{
		list->state->selectedIndex = list->itemIndex;
		list->changed = true;
	}

	return item;
}

bool ImUiToolboxListEnd( ImUiToolboxListContext* list )
{
	ImUiToolboxListItemEndInternal( list );

	ImUiWidgetEnd( list->listLayout );
	list->listLayout = NULL;

	ImUiToolboxScrollAreaEnd( list->list );
	list->list = NULL;

	return list->changed;
}

void ImUiToolboxDropDownBegin( ImUiToolboxDropDownContext* dropDown, ImUiWindow* window, const ImUiStringView* items, size_t itemCount )
{
	dropDown->dropDown = ImUiWidgetBegin( window );
	ImUiWidgetSetPadding( dropDown->dropDown, s_config.dropDown.padding );
	ImUiWidgetSetFixedHeight( dropDown->dropDown, s_config.dropDown.height );

	bool isNew;
	dropDown->state = (ImUiToolboxDropDownState*)ImUiWidgetAllocStateNew( dropDown->dropDown, sizeof( *dropDown->state ), &isNew );
	if( isNew )
	{
		dropDown->state->selectedIndex = (uintsize)-1;
	}

	ImUiWidgetInputState inputState;
	ImUiWidgetGetInputState( dropDown->dropDown, &inputState );

	ImUiColor color = s_config.colors[ ImUiToolboxColor_DropDown ];
	if( dropDown->state->isOpen )
	{
		color = s_config.colors[ ImUiToolboxColor_DropDownOpen ];
	}
	else if( inputState.isMouseDown )
	{
		color = s_config.colors[ ImUiToolboxColor_DropDownClicked ];
	}
	else if( inputState.isMouseOver )
	{
		color = s_config.colors[ ImUiToolboxColor_DropDownHover ];
	}

	ImUiDrawWidgetSkinColor( dropDown->dropDown, s_config.skins[ ImUiToolboxSkin_DropDown ], color );

	ImUiWidget* icon = ImUiWidgetBegin( window );
	const ImUiTexture iconImage = dropDown->state->isOpen ? s_config.dropDown.closeIcon : s_config.dropDown.openIcon;
	ImUiWidgetSetFixedSize( icon, iconImage.size );
	ImUiWidgetSetHAlign( icon, 1.0f );
	ImUiWidgetSetVAlign( icon, 0.5f );
	ImUiDrawWidgetTexture( icon, iconImage );
	ImUiWidgetEnd( icon );

	ImUiSize maxSize = ImUiSizeCreateZero();
	ImUiTextLayout* selectedTextLayout = NULL;
	for( uintsize i = 0; i < itemCount; ++i )
	{
		ImUiTextLayout* textLayout = ImUiTextLayoutCreateWidget( dropDown->dropDown, s_config.font, items[ i ] );

		maxSize = ImUiSizeMax( maxSize, ImUiTextLayoutGetSize( textLayout ) );

		if( i == dropDown->state->selectedIndex )
		{
			selectedTextLayout = textLayout;
		}
	}

	ImUiWidgetSetMinWidth( dropDown->dropDown, maxSize.width + ImUiBorderGetMinSize( s_config.dropDown.padding ).width + s_config.dropDown.padding.left + ImUiWidgetGetSize( icon ).width );

	ImUiWidget* text = ImUiWidgetBegin( window );
	ImUiWidgetSetFixedSize( text, maxSize );
	ImUiWidgetSetVAlign( text, 0.5f );

	if( selectedTextLayout )
	{
		ImUiDrawWidgetTextColor( text, selectedTextLayout, s_config.colors[ ImUiToolboxColor_DropDownText ] );
	}

	ImUiWidgetEnd( text );

	if( inputState.hasMousePressed )
	{
		dropDown->state->isOpen = !dropDown->state->isOpen;
	}

	if( dropDown->state->isOpen )
	{
		ImUiSurface* surface = ImUiWindowGetSurface( window );
		const ImUiSize surfaceSize = ImUiSurfaceGetSize( surface );

		const ImUiRect dropDownRect = ImUiWidgetGetRect( dropDown->dropDown );

		const float listHeight = (s_config.dropDown.itemSize + s_config.dropDown.itemSpacing) * IMUI_MIN( itemCount, s_config.dropDown.maxListLength );
		const float dropDownBottom = dropDownRect.pos.y + dropDownRect.size.height;
		ImUiRect listRect;
		if( dropDownBottom + listHeight > surfaceSize.height )
		{
			listRect = ImUiRectCreate( dropDownRect.pos.x, dropDownRect.pos.y - listHeight, dropDownRect.size.width, listHeight );
		}
		else
		{
			listRect = ImUiRectCreate( dropDownRect.pos.x, dropDownBottom, dropDownRect.size.width, listHeight  );
		}
		ImUiWindow* listWindow = ImUiWindowBegin( ImUiWindowGetSurface( window ), IMUI_STR( "dropDownList" ), listRect, s_config.dropDown.listZOrder );

		ImUiToolboxListContext list;
		ImUiToolboxListBegin( &list, listWindow, s_config.dropDown.itemSize, itemCount );
		ImUiWidgetSetStretch( list.list, ImUiSizeCreateOne() );

		ImUiWidgetInputState listInputState;
		ImUiWidgetGetInputState( list.list, &listInputState );

		ImUiDrawWidgetSkinColor( list.list, s_config.skins[ ImUiToolboxSkin_DropDownList ], s_config.colors[ ImUiToolboxColor_DropDownList ] );

		ImUiToolboxListSetSelectedIndex( &list, dropDown->state->selectedIndex );

		for( uintsize i = ImUiToolboxListGetBeginIndex( &list ); i < ImUiToolboxListGetEndIndex( &list ); ++i )
		{
			ImUiWidget* item = ImUiToolboxListNextItem( &list );
			ImUiWidgetSetPadding( item, s_config.dropDown.itemPadding );

			ImUiWidget* label = ImUiToolboxLabelBegin( listWindow, items[ i ] );
			ImUiWidgetSetAlign( label, ImUiAlignCreate( 0.0f, 0.5f ) );
			ImUiWidgetEnd( label );
		}

		dropDown->state->selectedIndex = ImUiToolboxListGetSelectedIndex( &list );

		if( ImUiToolboxListEnd( &list ) )
		{
			dropDown->state->isOpen = false;
			dropDown->changed = true;
		}
		else
		{
			dropDown->changed = false;
		}

		ImUiWindowEnd( listWindow );

		if( !inputState.wasPressed &&
			!listInputState.wasPressed &&
			ImUiInputHasMouseButtonReleased( ImUiWindowGetContext( window ), ImUiInputMouseButton_Left ) )
		{
			dropDown->state->isOpen = false;
		}
	}
}

size_t ImUiToolboxDropDownGetSelectedIndex( const ImUiToolboxDropDownContext* dropDown )
{
	return dropDown->state->selectedIndex;
}

void ImUiToolboxDropDownSetSelectedIndex( const ImUiToolboxDropDownContext* dropDown, size_t index )
{
	dropDown->state->selectedIndex = index;
}

bool ImUiToolboxDropDownEnd( ImUiToolboxDropDownContext* dropDown )
{
	ImUiWidgetEnd( dropDown->dropDown );

	return dropDown->changed;
}

size_t ImUiToolboxDropDown( ImUiWindow* window, const ImUiStringView* items, size_t itemCount )
{
	ImUiToolboxDropDownContext dropDown;
	ImUiToolboxDropDownBegin( &dropDown, window, items, itemCount );
	const size_t selectedIndex = ImUiToolboxDropDownGetSelectedIndex( &dropDown );
	ImUiToolboxDropDownEnd( &dropDown );
	return selectedIndex;
}

ImUiWindow* ImUiToolboxPopupBegin( ImUiWindow* window )
{
	return ImUiToolboxPopupBeginSurface( ImUiWindowGetSurface( window ) );
}

ImUiWindow* ImUiToolboxPopupBeginSurface( ImUiSurface* surface )
{
	const ImUiRect windowRect = ImUiRectCreatePosSize( ImUiPosCreateZero(), ImUiSurfaceGetSize( surface ) );
	ImUiWindow* popupWindow = ImUiWindowBegin( surface, IMUI_STR( "popup" ), windowRect, s_config.popup.zOrder );

	ImUiWidget* background = ImUiWidgetBegin( popupWindow );
	ImUiWidgetSetStretch( background, ImUiSizeCreateOne() );

	ImUiDrawWidgetColor( background, s_config.colors[ ImUiToolboxColor_PopupBackground ] );

	ImUiWidget* popup = ImUiWidgetBegin( popupWindow );
	ImUiWidgetSetAlign( popup, ImUiAlignCreateCenter() );
	ImUiWidgetSetPadding( popup, s_config.popup.padding );
	ImUiWidgetSetLayoutVertical( popup );

	ImUiDrawWidgetSkinColor( popup, s_config.skins[ ImUiToolboxSkin_Popup ], s_config.colors[ ImUiToolboxColor_Popup ] );

	return popupWindow;
}

size_t ImUiToolboxPopupEndButtons( ImUiWindow* popupWindow, const ImUiStringView* buttons, size_t buttonCount )
{
	ImUiWidget* buttonsLayout = ImUiWidgetBegin( popupWindow );
	ImUiWidgetSetHAlign( buttonsLayout, 1.0f );
	ImUiWidgetSetLayoutHorizontalSpacing( buttonsLayout, s_config.popup.buttonSpacing );

	uintsize clickedButton = (uintsize)-1;
	for( uintsize i = 0; i < buttonCount; ++i )
	{
		if( ImUiToolboxButtonLabel( popupWindow, buttons[ i ] ) )
		{
			clickedButton = i;
		}
	}

	ImUiWidgetEnd( buttonsLayout );

	ImUiToolboxPopupEnd( popupWindow );

	return clickedButton;
}

void ImUiToolboxPopupEnd( ImUiWindow* popupWindow )
{
	ImUiWidget* background = ImUiWindowGetFirstChild( popupWindow );
	ImUiWidget* popup = ImUiWidgetGetFirstChild( background );
	ImUiWidgetEnd( popup );
	ImUiWidgetEnd( background );
	ImUiWindowEnd( popupWindow );
}
