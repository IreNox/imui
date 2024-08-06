#include "imui/imui_toolbox.h"

#include "imui_internal.h"
#include "imui_memory.h"
#include "imui_types.h"

#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static ImUiToolboxConfig s_config;

struct ImUiToolboxScrollAreaState
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
	config->colors[ ImUiToolboxColor_DropDownIcon ]				= textColor;
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
	static_assert( ImUiToolboxColor_MAX == 37, "more colors" );

	const ImUiSkin skin = { { NULL } };

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
	config->skins[ ImUiToolboxSkin_ListItemSelected ]			= skin;
	config->skins[ ImUiToolboxSkin_DropDown ]					= skin;
	config->skins[ ImUiToolboxSkin_DropDownList ]				= skin;
	config->skins[ ImUiToolboxSkin_DropDownListItem ]			= skin;
	config->skins[ ImUiToolboxSkin_Popup ]						= skin;
	static_assert( ImUiToolboxSkin_MAX == 16, "more skins" );

	const ImUiImage image = { NULL, 22u, 22u, { 0.0f, 0.0f, 1.0f, 1.0f } };

	config->icons[ ImUiToolboxIcon_CheckBoxChecked ] = image;
	config->icons[ ImUiToolboxIcon_DropDownOpenIcon ] = image;
	config->icons[ ImUiToolboxIcon_DropDownCloseIcon ] = image;

	config->font					= font;

	config->button.height			= 25.0f;
	config->button.padding			= ImUiBorderCreate( 0.0f, 8.0f, 0.0f, 8.0f );

	config->checkBox.size			= ImUiSizeCreateAll( 25.0f );
	config->checkBox.textSpacing	= 8.0f;

	config->slider.height			= 25.0f;
	config->slider.padding			= ImUiBorderCreateHorizontalVertical( 5.0f, 0.0f );
	config->slider.pivotSize		= ImUiSizeCreate( 10.0f, 25.0f );

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

const ImUiToolboxConfig* ImUiToolboxGetConfig()
{
	return &s_config;
}

void ImUiToolboxSpacer( ImUiWindow* window, float width, float height )
{
	ImUiWidget* widget = ImUiWidgetBegin( window );
	ImUiWidgetSetFixedSizeFloat( widget, width, height );
	ImUiWidgetEnd( widget );
}

void ImUiToolboxStrecher( ImUiWindow* window, float horizontal, float vertical )
{
	ImUiWidget* widget = ImUiWidgetBegin( window );
	ImUiWidgetSetStretch( widget, horizontal, vertical );
	ImUiWidgetEnd( widget );
}

ImUiWidget* ImUiToolboxButtonBegin( ImUiWindow* window )
{
	ImUiWidget* button = ImUiWidgetBegin( window );
	ImUiWidgetSetFixedHeight( button, s_config.button.height );
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

	ImUiWidgetDrawSkin( button, &s_config.skins[ ImUiToolboxSkin_Button ], color );

	return button;
}

bool ImUiToolboxButtonEnd( ImUiWidget* button )
{
	ImUiWidgetEnd( button );

	ImUiWidgetInputState inputState;
	ImUiWidgetGetInputState( button, &inputState );

	return inputState.wasPressed && inputState.hasMouseReleased;
}

ImUiWidget* ImUiToolboxButtonLabelBegin( ImUiWindow* window, const char* text )
{
	ImUiWidget* buttonFrame = ImUiToolboxButtonBegin( window );

	ImUiWidget* buttonText = ImUiWidgetBegin( window );

	ImUiTextLayout* layout = ImUiTextLayoutCreateWidget( buttonText, s_config.font, text );
	const ImUiSize textSize = ImUiTextLayoutGetSize( layout );
	ImUiWidgetSetAlign( buttonText, 0.5f, 0.5f );
	ImUiWidgetSetFixedSize( buttonText, textSize );

	if( layout )
	{
		ImUiWidgetDrawText( buttonText, layout, s_config.colors[ ImUiToolboxColor_ButtonText ] );
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
		char* headBuffer = (char*)ImUiMemoryAlloc( &window->context->allocator, length + 1u );
		if( !headBuffer )
		{
			return false;
		}

		length = vsnprintf( headBuffer, length + 1u, format, args );

		ImUiWidget* button = ImUiToolboxButtonLabelBegin( window, headBuffer );

		ImUiMemoryFree( &window->context->allocator, headBuffer );
		return button;
	}

	return ImUiToolboxButtonLabelBegin( window, buffer );
}

bool ImUiToolboxButtonLabelEnd( ImUiWidget* button )
{
	return ImUiToolboxButtonEnd( button );
}

bool ImUiToolboxButtonLabel( ImUiWindow* window, const char* text )
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

ImUiWidget* ImUiToolboxButtonIconBegin( ImUiWindow* window, ImUiImage icon, ImUiSize iconSize )
{
	ImUiWidget* buttonFrame = ImUiToolboxButtonBegin( window );

	ImUiWidget* buttonIcon = ImUiWidgetBegin( window );
	ImUiWidgetSetAlign( buttonIcon, 0.5f, 0.5f );
	ImUiWidgetSetFixedSize( buttonIcon, iconSize );

	ImUiWidgetDrawImage( buttonIcon, &icon );

	ImUiWidgetEnd( buttonIcon );

	return buttonFrame;
}

bool ImUiToolboxButtonIconEnd( ImUiWidget* button )
{
	return ImUiToolboxButtonEnd( button );
}

bool ImUiToolboxButtonIcon( ImUiWindow* window, ImUiImage icon )
{
	return ImUiToolboxButtonIconSize( window, icon, ImUiSizeCreateImage( &icon ) );
}

bool ImUiToolboxButtonIconSize( ImUiWindow* window, ImUiImage icon, ImUiSize iconSize )
{
	ImUiWidget* button = ImUiToolboxButtonIconBegin( window, icon, iconSize );
	return ImUiToolboxButtonIconEnd( button );
}

ImUiWidget* ImUiToolboxCheckBoxBegin( ImUiWindow* window )
{
	ImUiWidget* checkBoxFrame = ImUiWidgetBegin( window );
	ImUiWidgetSetPadding( checkBoxFrame, ImUiBorderCreate( 0.0f, s_config.checkBox.size.width + s_config.checkBox.textSpacing, 0.0f, 0.0f ) );
	ImUiWidgetSetFixedHeight( checkBoxFrame, s_config.checkBox.size.height );
	ImUiWidgetSetVAlign( checkBoxFrame, 0.5f );

	return checkBoxFrame;
}

bool ImUiToolboxCheckBoxEnd( ImUiWidget* checkBox, bool* checked, const char* text )
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

	const ImUiRect checkBackgroundRect = ImUiRectCreatePosSize( ImUiPosCreateZero(), s_config.checkBox.size );
	ImUiWidgetDrawPartialSkin( checkBox, checkBackgroundRect, &s_config.skins[ ImUiToolboxSkin_CheckBox ], color );

	if( *checked )
	{
		const ImUiRect checkIconRect = ImUiRectCreateCenterPosSize( ImUiRectGetCenter( checkBackgroundRect ), ImUiSizeCreateImage( &s_config.icons[ ImUiToolboxIcon_CheckBoxChecked ] ) );
		ImUiWidgetDrawPartialImageColor( checkBox, checkIconRect, &s_config.icons[ ImUiToolboxIcon_CheckBoxChecked ], s_config.colors[ ImUiToolboxColor_CheckBoxChecked ] );
	}

	ImUiWidget* checkBoxText = ImUiWidgetBegin( ImUiWidgetGetWindow( checkBox ) );

	ImUiTextLayout* layout = ImUiTextLayoutCreateWidget( checkBoxText, s_config.font, text );
	const ImUiSize textSize = ImUiTextLayoutGetSize( layout );
	ImUiWidgetSetFixedSize( checkBoxText, textSize );
	ImUiWidgetSetVAlign( checkBoxText, 0.5f );

	if( layout )
	{
		ImUiWidgetDrawText( checkBoxText, layout, s_config.colors[ ImUiToolboxColor_Text ] );
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

bool ImUiToolboxCheckBox( ImUiWindow* window, bool* checked, const char* text )
{
	ImUiWidget* checkBox = ImUiToolboxCheckBoxBegin( window );
	return ImUiToolboxCheckBoxEnd( checkBox, checked, text);
}

bool ImUiToolboxCheckBoxState( ImUiWindow* window, const char* text )
{
	return ImUiToolboxCheckBoxStateDefault( window, text, false );
}

bool ImUiToolboxCheckBoxStateDefault( ImUiWindow* window, const char* text, bool defaultValue )
{
	ImUiWidget* checkBox = ImUiToolboxCheckBoxBegin( window );

	bool isNew;
	bool* checked = (bool*)ImUiWidgetAllocStateNew( checkBox, sizeof( bool ), IMUI_ID_STR( "check box" ), &isNew);
	if( isNew )
	{
		*checked = defaultValue;
	}

	ImUiToolboxCheckBoxEnd( checkBox, checked, text );
	return *checked;
}

ImUiWidget* ImUiToolboxLabelBegin( ImUiWindow* window, const char* text )
{
	ImUiWidget* label = ImUiWidgetBegin( window );

	ImUiTextLayout* layout = ImUiTextLayoutCreateWidget( label, s_config.font, text );
	const ImUiSize textSize = ImUiTextLayoutGetSize( layout );
	ImUiWidgetSetFixedSize( label, textSize );
	ImUiWidgetSetVAlign( label, 0.5f );

	if( layout )
	{
		ImUiWidgetDrawText( label, layout, s_config.colors[ ImUiToolboxColor_Text ] );
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
		char* headBuffer = (char*)ImUiMemoryAlloc( &window->context->allocator, length + 1u );
		if( !headBuffer )
		{
			return NULL;
		}

		length = vsnprintf( headBuffer, length + 1u, format, args );

		ImUiWidget* label = ImUiToolboxLabelBegin( window, headBuffer );

		ImUiMemoryFree( &window->context->allocator, headBuffer );
		return label;
	}

	return ImUiToolboxLabelBegin( window, buffer );
}

void ImUiToolboxLabelEnd( ImUiWidget* label )
{
	ImUiWidgetEnd( label );
}

void ImUiToolboxLabel( ImUiWindow* window, const char* text )
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

ImUiWidget* ImUiToolboxImageBegin( ImUiWindow* window, ImUiSize imgSize )
{
	ImUiWidget* imgWidget = ImUiWidgetBegin( window );
	ImUiWidgetSetFixedSize( imgWidget, imgSize );

	return imgWidget;
}

void ImUiToolboxImageEnd( ImUiWidget* imgWidget, const ImUiImage* img )
{
	ImUiWidgetDrawImage( imgWidget, img );

	ImUiWidgetEnd( imgWidget );
}

void ImUiToolboxImage( ImUiWindow* window, const ImUiImage* img )
{
	ImUiToolboxImageSize( window, img, ImUiSizeCreateImage( img ) );
}

void ImUiToolboxImageSize( ImUiWindow* window, const ImUiImage* img, ImUiSize imgSize )
{
	ImUiWidget* imgWidget = ImUiToolboxImageBegin( window, imgSize );
	ImUiToolboxImageEnd( imgWidget, img );
}

ImUiWidget* ImUiToolboxSliderBegin( ImUiWindow* window )
{
	ImUiWidget* slider = ImUiWidgetBegin( window );
	ImUiWidgetSetHStretch( slider, 1.0f );
	ImUiWidgetSetPadding( slider, s_config.slider.padding );
	ImUiWidgetSetFixedHeight( slider, s_config.slider.height );

	return slider;
}

bool ImUiToolboxSliderEnd( ImUiWidget* slider, float* value, float min, float max )
{
	ImUiWidgetInputState frameInputState;
	ImUiWidgetGetInputState( slider, &frameInputState );

	ImUiWidgetDrawSkin( slider, &s_config.skins[ ImUiToolboxSkin_SliderBackground ], s_config.colors[ ImUiToolboxColor_SliderBackground ] );

	ImUiWidget* sliderPivot = ImUiWidgetBegin( ImUiWidgetGetWindow( slider ) );
	ImUiWidgetSetFixedSize( sliderPivot, s_config.slider.pivotSize );

	const float normalizedValue = (*value - min) / (max - min);
	ImUiWidgetSetHAlign( sliderPivot, normalizedValue );

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
		const ImUiRect sliderInnerRect = ImUiWidgetGetInnerRect( slider );

		const float mouseValueNorm		= (frameInputState.relativeMousePos.x - s_config.slider.pivotSize.width) / (sliderInnerRect.size.width - s_config.slider.pivotSize.width);
		const float mouseValueNormClamp	= mouseValueNorm > 1.0f ? 1.0f : (mouseValueNorm < 0.0f ? 0.0f : mouseValueNorm);
		IMUI_ASSERT( mouseValueNormClamp >= 0.0f && mouseValueNormClamp <= 1.0f );
		const float mouseValue			= (mouseValueNormClamp * (max - min)) + min;

		*value = mouseValue;
		changed = true;
	}

	ImUiWidgetDrawSkin( sliderPivot, &s_config.skins[ ImUiToolboxSkin_SliderPivot ], color );

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
	float* value = (float*)ImUiWidgetAllocStateNew( sliderFrame, sizeof( float ), IMUI_ID_STR( "slider" ), &isNew );
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
	ImUiWidgetSetHStretch( textEditFrame, 1.0f );
	ImUiWidgetSetPadding( textEditFrame, s_config.textEdit.padding );
	ImUiWidgetSetFixedHeight( textEditFrame, s_config.textEdit.height );

	ImUiWidgetDrawSkin( textEditFrame, &s_config.skins[ ImUiToolboxSkin_TextEditBackground ], s_config.colors[ ImUiToolboxColor_TextEditBackground ] );

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
	ImUiWidgetSetVAlign( text, 0.5f );

	bool isNew;
	ImUiToolboxTextEditState* state = (ImUiToolboxTextEditState*)ImUiWidgetAllocStateNew( text, sizeof( *state ), IMUI_ID_STR( "text edit" ), &isNew );

	const ImUiRect textEditRect = ImUiWidgetGetRect( textEdit );
	const ImUiRect textEditInnerRect = ImUiWidgetGetInnerRect( textEdit );

	ImUiTextLayout* layout = ImUiTextLayoutCreateWidget( text, s_config.font, buffer );
	const ImUiSize textSize = ImUiTextLayoutGetSize( layout );
	ImUiWidgetSetFixedSize( text, textSize );

	if( ImUiInputHasMouseButtonPressed( imui, ImUiInputMouseButton_Left ) )
	{
		state->hasFocus = inputState.hasMousePressed;
	}

	bool changed = false;
	if( state->hasFocus )
	{
		const ImUiInputShortcut shortcut = ImUiInputGetShortcut( imui );
		const uint32 mods = ImUiInputGetKeyModifiers( imui );

		if( shortcut == ImUiInputShortcut_SelectAll )
		{
			state->selectionStart	= 0u;
			state->selectionEnd		= (uint32)textLengthInternal;
		}

		const char* textInput = ImUiInputGetText( imui );
		if( shortcut == ImUiInputShortcut_Paste )
		{
			textInput = ImUiInputGetPasteText( imui );
		}

		if( textInput )
		{
			const uintsize remainingSize	= bufferSize - textLengthInternal - 1u;
			const uintsize inputLength		= strlen( textInput );
			const uintsize newSize			= IMUI_MIN( inputLength, remainingSize );

			if( state->selectionStart != state->selectionEnd )
			{
				memmove( buffer + state->selectionStart, buffer + state->selectionEnd, textLengthInternal - state->selectionEnd );
				state->cursorPos = state->selectionStart;

				textLengthInternal -= state->selectionEnd - state->selectionStart;

				state->selectionStart	= 0u;
				state->selectionEnd		= 0u;
			}

			if( state->cursorPos != textLengthInternal )
			{
				memmove( buffer + state->cursorPos + newSize, buffer + state->cursorPos, textLengthInternal - state->cursorPos );
			}

			memcpy( buffer + state->cursorPos, textInput, newSize );
			textLengthInternal += newSize;
			buffer[ textLengthInternal ] = '\0';

			state->cursorPos += (uint32)newSize;

			changed = true;
		}

		if( ImUiInputHasKeyPressed( imui, ImUiInputKey_Backspace ) )
		{
			if( state->selectionStart != state->selectionEnd )
			{
				memmove( buffer + state->selectionStart, buffer + state->selectionEnd, textLengthInternal - state->selectionEnd );
				state->cursorPos = state->selectionStart;

				textLengthInternal -= state->selectionEnd - state->selectionStart;
				buffer[ textLengthInternal ] = '\0';

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
				memmove( buffer + state->selectionStart, buffer + state->selectionEnd, textLengthInternal - state->selectionEnd );
				state->cursorPos = state->selectionStart;

				textLengthInternal -= state->selectionEnd - state->selectionStart;
				buffer[ textLengthInternal ] = '\0';

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
		const bool leftPressed = ImUiInputHasKeyPressed( imui, ImUiInputKey_Left );
		const bool shiftPressed = (mods & (ImUiInputModifier_LeftShift | ImUiInputModifier_RightShift)) != 0;
		if( leftPressed ||
			ImUiInputHasKeyPressed( imui, ImUiInputKey_Right ) )
		{
			const sint32 direction = leftPressed ? -1 : 1;
			if( mods & (ImUiInputModifier_LeftCtrl | ImUiInputModifier_RightCtrl) )
			{
				nextCursorPos += direction;

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
			else if( !shiftPressed &&
					 state->selectionStart != state->selectionEnd )
			{
				nextCursorPos			= leftPressed ? state->selectionStart : state->selectionEnd;
				state->selectionStart	= 0u;
				state->selectionEnd		= 0u;
			}
			else
			{
				nextCursorPos += direction;
			}
		}
		else if( shortcut == ImUiInputShortcut_Home )
		{
			nextCursorPos = 0u;
		}
		else if( shortcut == ImUiInputShortcut_End )
		{
			nextCursorPos = (sint32)textLengthInternal;
		}

		if( inputState.wasPressed )
		{
			ImUiWidgetInputState textInputState;
			ImUiWidgetGetInputState( text, &textInputState );
			nextCursorPos = (sint32)ImUiTextLayoutFindGlyphIndex( layout, textInputState.relativeMousePos );

			if( inputState.hasMousePressed )
			{
				state->selectionStart	= 0u;
				state->selectionEnd		= 0u;
				state->cursorPos		= nextCursorPos;
			}
		}

		if( nextCursorPos != (sint32)state->cursorPos )
		{
			nextCursorPos = IMUI_MAX( nextCursorPos, 0 );
			nextCursorPos = IMUI_MIN( nextCursorPos, (sint32)textLengthInternal );

			const uint32 nextCursorPosU = (uint32)nextCursorPos;
			if( shiftPressed ||
				inputState.wasPressed )
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
			if( shortcut == ImUiInputShortcut_Copy )
			{
				ImUiInputSetCopyText( imui, buffer + state->selectionStart, state->selectionEnd - state->selectionStart );
			}

			const ImUiPos startPos			= ImUiTextLayoutGetGlyphPos( layout, state->selectionStart );
			const ImUiPos endPos			= ImUiTextLayoutGetGlyphPos( layout, state->selectionEnd );

			const ImUiRect selection = ImUiRectCreate(
				(textEditInnerRect.pos.x - textEditRect.pos.x) + startPos.x,
				textEditInnerRect.pos.y - textEditRect.pos.y,
				endPos.x - startPos.x,
				textEditInnerRect.size.height
			);
			ImUiWidgetDrawPartialColor( textEdit, selection, s_config.colors[ ImUiToolboxColor_TextEditSelection ] );
		}
	}

	if( layout )
	{
		ImUiWidgetDrawText( text, layout, s_config.colors[ ImUiToolboxColor_Text ] );
	}

	if( state->hasFocus )
	{
		const double blinkValue	= fmod( ImUiWidgetGetTime( textEdit ), s_config.textEdit.blinkTime * 2.0 );
		const bool blink		= blinkValue > s_config.textEdit.blinkTime;
		if( blink )
		{
			const ImUiPos cursorPos			= ImUiTextLayoutGetGlyphPos( layout, state->cursorPos );
			const ImUiPos cursorPosTop		= ImUiPosCreate( (textEditInnerRect.pos.x - textEditRect.pos.x) + cursorPos.x, s_config.textEdit.padding.top );
			const ImUiPos cursorPosBottom	= ImUiPosCreate( cursorPosTop.x, cursorPosTop.y + textEditInnerRect.size.height - s_config.textEdit.padding.bottom );
			ImUiWidgetDrawLine( textEdit, cursorPosTop, cursorPosBottom, s_config.colors[ ImUiToolboxColor_TextEditCursor ] );
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

const char* ImUiToolboxTextEditStateBuffer( ImUiWindow* window, size_t bufferSize )
{
	return ImUiToolboxTextEditStateBufferDefault( window, bufferSize, NULL );
}

const char* ImUiToolboxTextEditStateBufferDefault( ImUiWindow* window, size_t bufferSize, const char* defaultValue )
{
	ImUiWidget* textEdit = ImUiToolboxTextEditBegin( window );

	bool isNew;
	char* buffer = (char*)ImUiWidgetAllocStateNew( textEdit, bufferSize, IMUI_ID_STR( "text buffer" ), &isNew );
	if( isNew && defaultValue )
	{
		strncpy( buffer, defaultValue, bufferSize );
	}

	ImUiToolboxTextEditEnd( textEdit, buffer, bufferSize, NULL );
	return buffer;
}

void ImUiToolboxProgressBar( ImUiWindow* window, float value )
{
	ImUiToolboxProgressBarMinMax( window, value, 0.0f, 1.0f );
}

void ImUiToolboxProgressBarMinMax( ImUiWindow* window, float value, float min, float max )
{
	ImUiWidget* progressBar = ImUiWidgetBegin( window );
	ImUiWidgetSetHStretch( progressBar, 1.0f );
	ImUiWidgetSetPadding( progressBar, s_config.progressBar.padding );
	ImUiWidgetSetFixedHeight( progressBar, s_config.progressBar.height );

	ImUiWidgetDrawSkin( progressBar, &s_config.skins[ ImUiToolboxSkin_ProgressBarBackground ], s_config.colors[ ImUiToolboxColor_ProgressBarBackground ] );

	const ImUiRect barRect = ImUiWidgetGetInnerRect( progressBar );

	ImUiRect progressRect;
	if( value < min )
	{
		const double time		= ImUiWindowGetTime( window );
		const float cosv		= ((float)cos( time * 8.0 ) * 0.15f) + 0.15f;
		const float sinv		= ((float)sin( time * 4.0 ) * 0.5f) + 0.5f;
		const float width		= ceilf( barRect.size.width * (0.1f + cosv) );
		const float margin		= floorf( sinv * (barRect.size.width - width) );

		progressRect = ImUiRectCreate(
			margin,
			s_config.progressBar.padding.top,
			width,
			barRect.size.height
		);
	}
	else
	{
		const float valueNorm	= (value - min) / (max - min);
		const float width		= ceilf( barRect.size.width * valueNorm );

		progressRect = ImUiRectCreate(
			s_config.progressBar.padding.left,
			s_config.progressBar.padding.top,
			width,
			barRect.size.height
		);
	}

	ImUiWidgetDrawPartialSkin( progressBar, progressRect, &s_config.skins[ ImUiToolboxSkin_ProgressBarProgress ], s_config.colors[ ImUiToolboxColor_ProgressBarProgress ] );

	ImUiWidgetEnd( progressBar );
}

void ImUiToolboxScrollAreaBegin( ImUiToolboxScrollAreaContext* scrollArea, ImUiWindow* window )
{
	scrollArea->horizontalSpacing	= false;
	scrollArea->verticalSpacing		= false;
	scrollArea->area				= ImUiWidgetBegin( window );
	scrollArea->state				= (ImUiToolboxScrollAreaState*)ImUiWidgetAllocState( scrollArea->area, sizeof( *scrollArea->state ), IMUI_ID_STR( "scroll area" ) );
	scrollArea->content				= ImUiWidgetBegin( window );

	ImUiWidgetSetStretch( scrollArea->content, 1.0f, 1.0f );
	ImUiWidgetSetLayoutScroll( scrollArea->content, scrollArea->state->offset.x, scrollArea->state->offset.y );
}

void ImUiToolboxScrollAreaEnableSpacing( ImUiToolboxScrollAreaContext* scrollArea, bool horizontal, bool vertical )
{
	scrollArea->horizontalSpacing	= horizontal;
	scrollArea->verticalSpacing		= vertical;
}

void ImUiToolboxScrollAreaEnd( ImUiToolboxScrollAreaContext* scrollArea )
{
	ImUiWindow* window = ImUiWidgetGetWindow( scrollArea->area );
	ImUiToolboxScrollAreaState* state = scrollArea->state;

	const ImUiRect frameRect = ImUiWidgetGetRect( scrollArea->area );

	ImUiSize areaSize = ImUiSizeCreateZero();
	for( ImUiWidget* child = ImUiWidgetGetFirstChild( scrollArea->content ); child; child = ImUiWidgetGetNextSibling( child ) )
	{
		const ImUiRect childRect = ImUiWidgetGetRect( child );

		areaSize.width	= IMUI_MAX( areaSize.width, childRect.size.width );
		areaSize.height	= IMUI_MAX( areaSize.height, childRect.size.height );
	}

	ImUiWidgetInputState areaInputState;
	ImUiWidgetGetInputState( scrollArea->area, &areaInputState );

	const bool hasHorizontalBar	= areaSize.width > frameRect.size.width;
	const bool hasVerticalBar	= areaSize.height > frameRect.size.height;

	ImUiBorder margin = ImUiBorderCreateZero();
	margin.right = (hasVerticalBar ? s_config.scrollArea.barSize + (scrollArea->horizontalSpacing ? s_config.scrollArea.barSpacing : 0.0f) : 0.0f) ;
	margin.bottom = (hasHorizontalBar ? s_config.scrollArea.barSize + (scrollArea->verticalSpacing ? s_config.scrollArea.barSpacing : 0.0f) : 0.0f) ;

	const ImUiSize frameAreaSize = ImUiSizeMax( ImUiSizeCreateZero(), ImUiSizeShrinkBorder( frameRect.size, margin ) );

	ImUiWidgetSetMargin( scrollArea->content, margin );
	ImUiWidgetEnd( scrollArea->content );
	scrollArea->content = NULL;

	if( hasHorizontalBar )
	{
		float barWidth = frameRect.size.width - (hasVerticalBar ? s_config.scrollArea.barSize : 0.0f);
		barWidth = IMUI_MAX( 0.0f, barWidth );

		ImUiWidget* scrollBar = ImUiWidgetBegin( window );
		ImUiWidgetSetVAlign( scrollBar, 1.0f );
		ImUiWidgetSetFixedHeight( scrollBar, s_config.scrollArea.barSize );
		ImUiWidgetSetHStretch( scrollBar, frameRect.size.width > 0.0f ? barWidth / frameRect.size.width : 1.0f );

		const ImUiRect barRect		= ImUiWidgetGetRect( scrollBar );
		const float pivotSizeFactor	= frameAreaSize.width / areaSize.width;
		const float pivotSize		= barWidth * pivotSizeFactor;
		const float pivotSizeFinal	= IMUI_MAX( pivotSize, s_config.scrollArea.barMinSize );
		const float pivotOffset		= (state->offset.x / areaSize.width) * (barWidth - (pivotSizeFinal - pivotSize));

		const ImUiRect barPivotRect = ImUiRectCreate(
			pivotOffset,
			0.0f,
			pivotSizeFinal,
			s_config.scrollArea.barSize
		);

		ImUiWidgetInputState inputState;
		ImUiWidgetGetInputState( scrollBar, &inputState );

		if( inputState.wasPressed )
		{
			if( !state->wasPressedX &&
				ImUiRectIncludesPos( barPivotRect, inputState.relativeMousePos ) )
			{
				state->pressPoint.x	= inputState.relativeMousePos.x;
				state->pressPoint.x	-= pivotOffset;
				state->wasPressedX	= true;
			}

			if( state->wasPressedX )
			{
				const float newBarOffset	= inputState.relativeMousePos.x - state->pressPoint.x;
				const float newOffset		= (newBarOffset / barRect.size.width) * areaSize.width;

				state->offset.x = newOffset;
			}
		}
		else
		{
			state->wasPressedX = false;
		}

		ImUiWidgetDrawSkin( scrollBar, &s_config.skins[ ImUiToolboxSkin_ScrollAreaBarBackground ], s_config.colors[ ImUiToolboxColor_ScrollAreaBarBackground ] );
		ImUiWidgetDrawPartialSkin( scrollBar, barPivotRect, &s_config.skins[ ImUiToolboxSkin_ScrollAreaBarPivot ], s_config.colors[ ImUiToolboxColor_ScrollAreaBarPivot ] );

		ImUiWidgetEnd( scrollBar );
	}

	if( hasVerticalBar )
	{
		float barHeight = frameRect.size.height - (hasHorizontalBar ? s_config.scrollArea.barSize : 0.0f);
		barHeight = IMUI_MAX( 0.0f, barHeight );

		ImUiWidget* scrollBar = ImUiWidgetBegin( window );
		ImUiWidgetSetHAlign( scrollBar, 1.0f );
		ImUiWidgetSetFixedWidth( scrollBar, s_config.scrollArea.barSize );
		ImUiWidgetSetVStretch( scrollBar, frameRect.size.height > 0.0f ? barHeight / frameRect.size.height : 1.0f );

		const ImUiRect barRect		= ImUiWidgetGetRect( scrollBar );
		const float pivotSizeFactor	= frameAreaSize.height / areaSize.height;
		const float pivotSize		= barHeight * pivotSizeFactor;
		const float pivotSizeFinal	= IMUI_MAX( pivotSize, s_config.scrollArea.barMinSize );
		const float pivotOffset		= (state->offset.y / areaSize.height) * (barHeight - (pivotSizeFinal - pivotSize));

		const ImUiRect barPivotRect = ImUiRectCreate(
			0.0f,
			pivotOffset,
			s_config.scrollArea.barSize,
			pivotSizeFinal
		);

		ImUiWidgetInputState inputState;
		ImUiWidgetGetInputState( scrollBar, &inputState );

		if( inputState.wasPressed )
		{
			if( !state->wasPressedY &&
				ImUiRectIncludesPos( barPivotRect, inputState.relativeMousePos ) )
			{
				state->pressPoint.y	= inputState.relativeMousePos.y;
				state->pressPoint.y	-= pivotOffset;
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

		ImUiWidgetDrawSkin( scrollBar, &s_config.skins[ ImUiToolboxSkin_ScrollAreaBarBackground ], s_config.colors[ ImUiToolboxColor_ScrollAreaBarBackground ] );
		ImUiWidgetDrawPartialSkin( scrollBar, barPivotRect, &s_config.skins[ ImUiToolboxSkin_ScrollAreaBarPivot ], s_config.colors[ ImUiToolboxColor_ScrollAreaBarPivot ] );

		ImUiWidgetEnd( scrollBar );
	}

	if( areaInputState.isMouseOver )
	{
		state->offset = ImUiPosSubPos( state->offset, ImUiPosScale( ImUiInputGetMouseScrollDelta( ImUiWindowGetContext( window ) ), 80.0f ) );
	}

	state->offset = ImUiPosMax( ImUiPosCreateZero(), ImUiPosMin( state->offset, ImUiSizeToPos( ImUiSizeSubSize( areaSize, frameAreaSize ) ) ) );

	ImUiWidgetEnd( scrollArea->area );
	scrollArea->area = NULL;
	scrollArea->state = NULL;
}

void ImUiToolboxListBegin( ImUiToolboxListContext* list, ImUiWindow* window, float itemSize, size_t itemCount )
{
	IMUI_ASSERT( list );

	const float totalItemSize = itemSize + s_config.list.itemSpacing;

	ImUiToolboxScrollAreaBegin( &list->scrollArea, window );
	list->list = list->scrollArea.area;

	ImUiToolboxScrollAreaEnableSpacing( &list->scrollArea, true, false );

	list->listLayout = ImUiWidgetBegin( window );
	ImUiWidgetSetHStretch( list->listLayout, 1.0f );
	ImUiWidgetSetLayoutVerticalSpacing( list->listLayout, s_config.list.itemSpacing );
	if( itemCount > 0u )
	{
		ImUiWidgetSetFixedHeight( list->listLayout, (totalItemSize * itemCount) - s_config.list.itemSpacing );
	}

	bool isNew;
	list->state = (ImUiToolboxListState*)ImUiWidgetAllocStateNew( list->listLayout, sizeof( ImUiToolboxListState ), IMUI_ID_STR( "list" ), &isNew );
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
	ImUiWidgetSetHStretch( item, 1.0f );
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
		ImUiWidgetDrawSkin( item, &s_config.skins[ ImUiToolboxSkin_ListItemSelected ], s_config.colors[ ImUiToolboxColor_ListItemClicked ] );
	}
	else if( inputState.isMouseOver )
	{
		ImUiWidgetDrawSkin( item, &s_config.skins[ ImUiToolboxSkin_ListItem ], s_config.colors[ ImUiToolboxColor_ListItemHover ] );
	}
	else if( list->itemIndex == list->state->selectedIndex )
	{
		ImUiWidgetDrawSkin( item, &s_config.skins[ ImUiToolboxSkin_ListItemSelected ], s_config.colors[ ImUiToolboxColor_ListItemSelected ] );
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

	ImUiToolboxScrollAreaEnd( &list->scrollArea );
	list->list = NULL;

	return list->changed;
}

void ImUiToolboxDropDownBegin( ImUiToolboxDropDownContext* dropDown, ImUiWindow* window, const char** items, size_t itemCount, size_t itemStride )
{
	if( itemStride == 0u )
	{
		itemStride = sizeof( const char* );
	}

	dropDown->dropDown = ImUiWidgetBegin( window );
	ImUiWidgetSetPadding( dropDown->dropDown, s_config.dropDown.padding );
	ImUiWidgetSetFixedHeight( dropDown->dropDown, s_config.dropDown.height );

	bool isNew;
	dropDown->state = (ImUiToolboxDropDownState*)ImUiWidgetAllocStateNew( dropDown->dropDown, sizeof( *dropDown->state ), IMUI_ID_STR( "drop down" ), &isNew );
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

	ImUiWidgetDrawSkin( dropDown->dropDown, &s_config.skins[ ImUiToolboxSkin_DropDown ], color );

	ImUiWidget* icon = ImUiWidgetBegin( window );
	const ImUiImage iconImage = s_config.icons[ dropDown->state->isOpen ? ImUiToolboxIcon_DropDownCloseIcon : ImUiToolboxIcon_DropDownOpenIcon ];
	ImUiWidgetSetFixedSize( icon, ImUiSizeCreateImage( &iconImage ) );
	ImUiWidgetSetHAlign( icon, 1.0f );
	ImUiWidgetSetVAlign( icon, 0.5f );
	ImUiWidgetDrawImageColor( icon, &iconImage, s_config.colors[ ImUiToolboxColor_DropDownIcon ] );
	ImUiWidgetEnd( icon );

	ImUiSize maxSize = ImUiSizeCreateZero();
	ImUiTextLayout* selectedTextLayout = NULL;
	const uint8* itemsBytes = (const byte*)items;
	for( uintsize i = 0; i < itemCount; ++i )
	{
		const char* itemText = *(const char**)itemsBytes;
		ImUiTextLayout* textLayout = ImUiTextLayoutCreateWidget( dropDown->dropDown, s_config.font, itemText );

		maxSize = ImUiSizeMax( maxSize, ImUiTextLayoutGetSize( textLayout ) );

		if( i == dropDown->state->selectedIndex )
		{
			selectedTextLayout = textLayout;
		}

		itemsBytes += itemStride;
	}

	ImUiWidgetSetMinWidth( dropDown->dropDown, maxSize.width + ImUiBorderGetMinSize( s_config.dropDown.padding ).width + s_config.dropDown.padding.left + ImUiWidgetGetSize( icon ).width );

	ImUiWidget* text = ImUiWidgetBegin( window );
	ImUiWidgetSetFixedSize( text, maxSize );
	ImUiWidgetSetVAlign( text, 0.5f );

	if( selectedTextLayout )
	{
		ImUiWidgetDrawText( text, selectedTextLayout, s_config.colors[ ImUiToolboxColor_DropDownText ] );
	}

	ImUiWidgetEnd( text );

	if( inputState.hasMousePressed )
	{
		dropDown->state->isOpen = !dropDown->state->isOpen;
	}

	if( dropDown->state->isOpen && itemCount > 0u )
	{
		ImUiSurface* surface = ImUiWindowGetSurface( window );
		const ImUiSize surfaceSize = ImUiSurfaceGetSize( surface );

		const ImUiRect dropDownRect = ImUiWidgetGetRect( dropDown->dropDown );

		const float listHeight = ((s_config.dropDown.itemSize + s_config.dropDown.itemSpacing) * IMUI_MIN( itemCount, s_config.dropDown.maxListLength )) - s_config.dropDown.itemSpacing;
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
		ImUiWindow* listWindow = ImUiWindowBegin( ImUiWindowGetSurface( window ), "dropDownList", listRect, s_config.dropDown.listZOrder );

		ImUiToolboxListContext list;
		ImUiToolboxListBegin( &list, listWindow, s_config.dropDown.itemSize, itemCount );
		ImUiWidgetSetStretch( list.list, 1.0f, 1.0f );

		ImUiWidgetInputState listInputState;
		ImUiWidgetGetInputState( list.list, &listInputState );

		ImUiWidgetDrawSkin( list.list, &s_config.skins[ ImUiToolboxSkin_DropDownList ], s_config.colors[ ImUiToolboxColor_DropDownList ] );

		ImUiToolboxListSetSelectedIndex( &list, dropDown->state->selectedIndex );

		itemsBytes = (const byte*)items;
		for( uintsize i = ImUiToolboxListGetBeginIndex( &list ); i < ImUiToolboxListGetEndIndex( &list ); ++i )
		{
			const char* itemText = *(const char**)(itemsBytes + (i * itemStride));

			ImUiWidget* item = ImUiToolboxListNextItem( &list );
			ImUiWidgetSetPadding( item, s_config.dropDown.itemPadding );

			ImUiWidget* label = ImUiToolboxLabelBegin( listWindow, itemText );
			ImUiWidgetSetVAlign( label, 0.5f );
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

size_t ImUiToolboxDropDown( ImUiWindow* window, const char** items, size_t itemCount, size_t itemStride )
{
	ImUiToolboxDropDownContext dropDown;
	ImUiToolboxDropDownBegin( &dropDown, window, items, itemCount, itemStride );
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
	ImUiWindow* popupWindow = ImUiWindowBegin( surface, "popup", windowRect, s_config.popup.zOrder );

	ImUiWidget* background = ImUiWidgetBegin( popupWindow );
	ImUiWidgetSetStretch( background, 1.0f, 1.0f );

	ImUiWidgetDrawColor( background, s_config.colors[ ImUiToolboxColor_PopupBackground ] );

	ImUiWidget* popup = ImUiWidgetBegin( popupWindow );
	ImUiWidgetSetAlign( popup, 0.5f, 0.5f );
	ImUiWidgetSetPadding( popup, s_config.popup.padding );
	ImUiWidgetSetLayoutVertical( popup );

	ImUiWidgetDrawSkin( popup, &s_config.skins[ ImUiToolboxSkin_Popup ], s_config.colors[ ImUiToolboxColor_Popup ] );

	return popupWindow;
}

size_t ImUiToolboxPopupEndButtons( ImUiWindow* popupWindow, const char** buttons, size_t buttonCount )
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
