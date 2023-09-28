#include "imui/imui_toolbox.h"

#include "imui_internal.h"
#include "imui_memory.h"
#include "imui_types.h"

#include <math.h>
#include <stdarg.h>
#include <stdio.h>

static ImUiToolboxConfig s_config;

typedef struct ImUiToolboxScrollState ImUiToolboxScrollState;
struct ImUiToolboxScrollState
{
	ImUiPos			offset;
};

static ImUiWidget*	ImUiToolboxCheckBoxBegin( ImUiWindow* window );
static bool			ImUiToolboxCheckBoxEnd( ImUiWindow* window, ImUiWidget* checkBoxFrame, ImUiStringView text, bool* checked );
static ImUiWidget*	ImUiToolboxSliderBegin( ImUiWindow* window );
static bool			ImUiToolboxSliderEnd( ImUiWindow* window, ImUiWidget* sliderFrame, float* value, float min, float max );
static ImUiWidget*	ImUiToolboxScrollAreaBegin( ImUiWindow* window, bool horizontal, bool vertical );

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
	config->colors[ ImUiToolboxColor_CheckBoxChecked ]			= backgroundColor;
	config->colors[ ImUiToolboxColor_CheckBoxCheckedHover ]		= textEditCursorColor;
	config->colors[ ImUiToolboxColor_CheckBoxCheckedClicked ]	= elementClickedColor;
	config->colors[ ImUiToolboxColor_SliderBackground ]			= backgroundColor;
	config->colors[ ImUiToolboxColor_SliderPivot ]				= elementColor;
	config->colors[ ImUiToolboxColor_SliderPivotHover ]			= elementHoverColor;
	config->colors[ ImUiToolboxColor_SliderPivotClicked ]		= elementClickedColor;
	config->colors[ ImUiToolboxColor_TextEditBackground ]		= backgroundColor;
	config->colors[ ImUiToolboxColor_TextEditText ]				= textColor;
	config->colors[ ImUiToolboxColor_TextEditCursor ]			= textEditCursorColor;
	config->colors[ ImUiToolboxColor_TextEditSelection ]		= elementColor;
	config->colors[ ImUiToolboxColor_ProgressBarBackground ]	= backgroundColor;
	config->colors[ ImUiToolboxColor_ProgressBarProgress ]		= elementColor;
	config->colors[ ImUiToolboxColor_ScrollAreaBarBackground ]	= backgroundColor;
	config->colors[ ImUiToolboxColor_ScrollAreaBarPivot ]		= elementColor;
	config->colors[ ImUiToolboxColor_ListItemBackground ]		= backgroundColor;
	config->colors[ ImUiToolboxColor_ListItemText ]				= textColor;
	_STATIC_ASSERT( ImUiToolboxColor_MAX == 25 );

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
	_STATIC_ASSERT( ImUiToolboxSkin_MAX == 11 );

	config->font					= font;

	config->button.height			= 25.0f;
	config->button.padding			= ImUiBorderCreateAll( 8.0f );

	config->checkBox.size			= ImUiSizeCreateAll( 25.0f );
	config->checkBox.textSpacing	= 8.0f;

	config->slider.height			= 25.0f;
	config->slider.padding			= ImUiBorderCreateHorizontalVertical( 5.0f, 0.0f );
	config->slider.pivotSize		= 10.0f;

	config->textEdit.height			= 25.0f;
	config->textEdit.padding			= ImUiBorderCreateAll( 2.0f );
	config->textEdit.cursorSize		= ImUiSizeCreate( 1.0f, 21.0f );
	config->textEdit.blinkTime		= 1.0f;

	config->progressBar.height		= 25.0f;
	config->progressBar.padding		= ImUiBorderCreateAll( 2.0f );
}

void ImUiToolboxSetConfig( const ImUiToolboxConfig* config )
{
	s_config = *config;
}

bool ImUiToolboxButtonLabel( ImUiWindow* window, ImUiStringView text )
{
	ImUiWidget* buttonFrame = ImUiWidgetBegin( window );
	ImUiWidgetSetPadding( buttonFrame, s_config.button.padding );

	ImUiInputWidgetState inputState;
	ImUiInputGetWidgetState( buttonFrame, &inputState );

	ImUiColor color = s_config.colors[ ImUiToolboxColor_Button ];
	if( inputState.isMouseDown )
	{
		color = s_config.colors[ ImUiToolboxColor_ButtonClicked ];
	}
	else if( inputState.isMouseOver )
	{
		color = s_config.colors[ ImUiToolboxColor_ButtonHover ];
	}

	ImUiDrawWidgetSkinColor( buttonFrame, s_config.skins[ ImUiToolboxSkin_Button ], color );

	ImUiWidget* buttonText = ImUiWidgetBegin( window );

	ImUiTextLayout* layout = ImUiTextLayoutCreateWidget( buttonText, s_config.font, text );
	const ImUiSize textSize = ImUiTextLayoutGetSize( layout );
	ImUiWidgetSetFixedSize( buttonText, textSize );

	if( layout )
	{
		ImUiDrawWidgetTextColor( buttonText, layout, s_config.colors[ ImUiToolboxColor_ButtonText ] );
	}

	ImUiWidgetEnd( buttonText );

	ImUiWidgetEnd( buttonFrame );

	return inputState.hasMouseReleased;
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

		const bool result = ImUiToolboxButtonLabel( window, ImUiStringViewCreateLength( headBuffer, length ) );

		ImUiMemoryFree( &window->imui->allocator, headBuffer );
		return result;
	}

	return ImUiToolboxButtonLabel( window, ImUiStringViewCreateLength( buffer, length ) );
}

static ImUiWidget* ImUiToolboxCheckBoxBegin( ImUiWindow* window )
{
	ImUiWidget* checkBoxFrame = ImUiWidgetBegin( window );
	ImUiWidgetSetPadding( checkBoxFrame, ImUiBorderCreate( 0.0f, s_config.checkBox.size.width + s_config.checkBox.textSpacing, 0.0f, 0.0f ) );
	ImUiWidgetSetFixedHeight( checkBoxFrame, s_config.checkBox.size.height );

	return checkBoxFrame;
}

static bool ImUiToolboxCheckBoxEnd( ImUiWindow* window, ImUiWidget* checkBoxFrame, ImUiStringView text, bool* checked )
{
	ImUiInputWidgetState inputState;
	ImUiInputGetWidgetState( checkBoxFrame, &inputState );

	ImUiColor color = s_config.colors[ *checked ? ImUiToolboxColor_CheckBoxChecked : ImUiToolboxColor_CheckBox ];
	if( inputState.isMouseDown )
	{
		color = s_config.colors[ *checked ? ImUiToolboxColor_CheckBoxCheckedClicked : ImUiToolboxColor_CheckBoxClicked ];
	}
	else if( inputState.isMouseOver )
	{
		color = s_config.colors[ *checked ? ImUiToolboxColor_CheckBoxCheckedHover : ImUiToolboxColor_CheckBoxHover ];
	}

	const ImUiRect rect = ImUiRectCreatePosSize( ImUiWidgetGetPos( checkBoxFrame ), s_config.checkBox.size );
	ImUiDrawSkinColor( checkBoxFrame, rect, s_config.skins[ *checked ? ImUiToolboxSkin_CheckBoxChecked : ImUiToolboxSkin_CheckBox ], color );

	ImUiWidget* checkBoxText = ImUiWidgetBegin( window );

	ImUiTextLayout* layout = ImUiTextLayoutCreateWidget( checkBoxText, s_config.font, text );
	const ImUiSize textSize = ImUiTextLayoutGetSize( layout );
	ImUiWidgetSetFixedSize( checkBoxText, textSize );
	ImUiWidgetSetVAlign( checkBoxText, ImUiVAlign_Center );

	if( layout )
	{
		ImUiDrawWidgetTextColor( checkBoxText, layout, s_config.colors[ ImUiToolboxColor_Text ] );
	}

	ImUiWidgetEnd( checkBoxText );

	ImUiWidgetEnd( checkBoxFrame );

	if( inputState.hasMouseReleased )
	{
		*checked = !*checked;
		return true;
	}

	return false;
}

bool ImUiToolboxCheckBox( ImUiWindow* window, bool* checked, ImUiStringView text )
{
	ImUiWidget* checkBoxFrame = ImUiToolboxCheckBoxBegin( window );
	return ImUiToolboxCheckBoxEnd( window, checkBoxFrame, text, checked );
}

bool ImUiToolboxCheckBoxState( ImUiWindow* window, ImUiStringView text )
{
	ImUiWidget* checkBoxFrame = ImUiToolboxCheckBoxBegin( window );

	bool* checked = ImUiWidgetAllocState( checkBoxFrame, sizeof( bool ) );
	return ImUiToolboxCheckBoxEnd( window, checkBoxFrame, text, checked );
}

void ImUiToolboxLabel( ImUiWindow* window, ImUiStringView text )
{
	ImUiWidget* label = ImUiWidgetBegin( window );

	ImUiTextLayout* layout = ImUiTextLayoutCreateWidget( label, s_config.font, text );
	const ImUiSize textSize = ImUiTextLayoutGetSize( layout );
	ImUiWidgetSetFixedSize( label, textSize );

	if( layout )
	{
		ImUiDrawWidgetTextColor( label, layout, s_config.colors[ ImUiToolboxColor_Text ] );
	}

	ImUiWidgetEnd( label );
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
	char buffer[ 256u ];

	int length = vsnprintf( buffer, sizeof( buffer ), format, args );
	if( length < 0 )
	{
		return;
	}
	else if( length >= sizeof( buffer ) )
	{
		char* headBuffer = ImUiMemoryAlloc( &window->imui->allocator, length + 1u );
		if( !headBuffer )
		{
			return;
		}

		length = vsnprintf( headBuffer, length + 1u, format, args );

		ImUiToolboxLabel( window, ImUiStringViewCreateLength( headBuffer, length ) );

		ImUiMemoryFree( &window->imui->allocator, headBuffer );
		return;
	}

	ImUiToolboxLabel( window, ImUiStringViewCreateLength( buffer, length ) );
}

static ImUiWidget* ImUiToolboxSliderBegin( ImUiWindow* window )
{
	ImUiWidget* sliderFrame = ImUiWidgetBegin( window );
	ImUiWidgetSetStretch( sliderFrame, ImUiSizeCreateHorizintal() );
	ImUiWidgetSetPadding( sliderFrame, s_config.slider.padding );
	ImUiWidgetSetFixedHeight( sliderFrame, s_config.slider.height );

	return sliderFrame;
}

static bool ImUiToolboxSliderEnd( ImUiWindow* window, ImUiWidget* sliderFrame, float* value, float min, float max )
{
	ImUiContext* imui = ImUiWindowGetContext( window );

	ImUiInputWidgetState frameInputState;
	ImUiInputGetWidgetState( sliderFrame, &frameInputState );

	ImUiDrawWidgetSkinColor( sliderFrame, s_config.skins[ ImUiToolboxSkin_SliderBackground ], s_config.colors[ ImUiToolboxColor_SliderBackground ] );

	ImUiWidget* sliderPivot = ImUiWidgetBegin( window );
	ImUiWidgetSetFixedSize( sliderPivot, ImUiSizeCreate( s_config.slider.pivotSize, s_config.slider.height ) );

	const ImUiRect sliderInnerRect = ImUiWidgetGetInnerRect( sliderFrame );
	const float normalizedValue		= (*value - min) / (max - min);
	const float sliderPivotX		= (normalizedValue * (sliderInnerRect.size.width - s_config.slider.pivotSize));
	const float sliderPivotOffset	= sliderPivotX < 0.0f ? 0.0f : sliderPivotX;;

	ImUiWidgetSetMargin( sliderPivot, ImUiBorderCreate( 0.0f, sliderPivotOffset, 0.0f, 0.0f ) );

	ImUiInputWidgetState inputState;
	ImUiInputGetWidgetState( sliderPivot, &inputState );

	ImUiColor color = s_config.colors[ ImUiToolboxColor_SliderPivot ];
	if( frameInputState.isMouseDown )
	{
		color = s_config.colors[ ImUiToolboxColor_SliderPivotClicked ];
	}
	else if( inputState.isMouseOver )
	{
		color = s_config.colors[ ImUiToolboxColor_SliderPivotHover ];
	}

	bool changed = false;

	if( frameInputState.isMouseDown )
	{
		const ImUiPos mousePos			= ImUiInputGetMousePos( imui );
		const float mouseValueNorm		= (mousePos.x - sliderInnerRect.pos.x) / (sliderInnerRect.size.width - s_config.slider.pivotSize);
		const float mouseValueNormClamp	= mouseValueNorm > 1.0f ? 1.0f : (mouseValueNorm < 0.0f ? 0.0f : mouseValueNorm);
		IMUI_ASSERT( mouseValueNormClamp >= 0.0f && mouseValueNormClamp <= 1.0f );
		const float mouseValue			= (mouseValueNormClamp * (max - min)) + min;

		*value = mouseValue;
		changed = true;
	}

	ImUiDrawWidgetSkinColor( sliderPivot, s_config.skins[ ImUiToolboxSkin_SliderPivot ], color );

	ImUiWidgetEnd( sliderPivot );

	ImUiWidgetEnd( sliderFrame );

	return changed;
}

bool ImUiToolboxSlider( ImUiWindow* window, float* value )
{
	return ImUiToolboxSliderMinMax( window, value, 0.0f, 1.0f );
}

bool ImUiToolboxSliderMinMax( ImUiWindow* window, float* value, float min, float max )
{
	ImUiWidget* sliderFrame = ImUiToolboxSliderBegin( window );
	return ImUiToolboxSliderEnd( window, sliderFrame, value, min, max );
}

float ImUiToolboxSliderState( ImUiWindow* window )
{
	return ImUiToolboxSliderStateMinMax( window, 0.0f, 1.0f );
}

float ImUiToolboxSliderStateMinMax( ImUiWindow* window, float min, float max )
{
	ImUiWidget* sliderFrame = ImUiToolboxSliderBegin( window );

	bool isNew;
	float* value = ImUiWidgetAllocStateNew( sliderFrame, sizeof( float ), &isNew );
	if( isNew )
	{
		*value = min;
	}

	ImUiToolboxSliderEnd( window, sliderFrame, value, min, max );
	return *value;
}

bool ImUiToolboxTextEdit( ImUiWindow* window, char* buffer, size_t bufferSize, size_t textLength )
{
	ImUiWidget* textEditFrame = ImUiWidgetBegin( window );
	ImUiWidgetSetStretch( textEditFrame, ImUiSizeCreateHorizintal() );
	ImUiWidgetSetPadding( textEditFrame, s_config.textEdit.padding );
	ImUiWidgetSetFixedHeight( textEditFrame, s_config.textEdit.height );

	ImUiInputWidgetState frameInputState;
	ImUiInputGetWidgetState( textEditFrame, &frameInputState );

	ImUiDrawWidgetSkinColor( textEditFrame, s_config.skins[ ImUiToolboxSkin_SliderBackground ], s_config.colors[ ImUiToolboxColor_SliderBackground ] );

	ImUiWidget* textEditText = ImUiWidgetBegin( window );
	ImUiWidgetSetFixedSize( textEditText, ImUiSizeCreate( s_config.slider.pivotSize, s_config.slider.height ) );

	//const ImUiRect sliderInnerRect = ImUiWidgetGetInnerRect( textEditFrame );
	//const float normalizedValue		= (*value - min) / (max - min);
	//const float sliderPivotX		= (normalizedValue * (sliderInnerRect.size.width - s_config.sliderPivotSize));
	//const float sliderPivotOffset	= sliderPivotX < 0.0f ? 0.0f : sliderPivotX;;

	//ImUiWidgetSetMargin( textEditText, ImUiBorderCreate( 0.0f, sliderPivotOffset, 0.0f, 0.0f ) );

	//ImUiInputWidgetState inputState;
	//ImUiInputGetWidgetState( textEditText, &inputState );

	//ImUiColor color = s_config.colors[ ImUiToolboxColor_SliderPivot ];
	//if( frameInputState.isMouseDown )
	//{
	//	color = s_config.colors[ ImUiToolboxColor_SliderPivotClicked ];
	//}
	//else if( inputState.isMouseOver )
	//{
	//	color = s_config.colors[ ImUiToolboxColor_SliderPivotHover ];
	//}

	bool changed = false;

	//if( frameInputState.isMouseDown )
	//{
	//	const ImUiPos mousePos		= ImUiInputGetMousePos( imui );
	//	const float mouseValueNorm		= (mousePos.x - sliderInnerRect.pos.x) / (sliderInnerRect.size.width - s_config.sliderPivotSize);
	//	const float mouseValueNormClamp	= mouseValueNorm > 1.0f ? 1.0f : (mouseValueNorm < 0.0f ? 0.0f : mouseValueNorm);
	//	IMUI_ASSERT( mouseValueNormClamp >= 0.0f && mouseValueNormClamp <= 1.0f );
	//	const float mouseValue			= (mouseValueNormClamp * (max - min)) + min;

	//	*value = mouseValue;
	//	changed = true;
	//}

	//ImUiDrawWidgetSkinColor( textEditText, s_config.skins[ ImUiToolboxSkin_SliderPivot ], color );

	ImUiWidgetEnd( textEditText );

	ImUiWidgetEnd( textEditFrame );

	return changed;
}

void ImUiToolboxProgressBar( ImUiWindow* window, float value )
{
	ImUiToolboxProgressBarMinMax( window, value, 0.0f, 1.0f );
}

void ImUiToolboxProgressBarMinMax( ImUiWindow* window, float value, float min, float max )
{
	ImUiWidget* progressBarFrame = ImUiWidgetBegin( window );
	ImUiWidgetSetStretch( progressBarFrame, ImUiSizeCreateHorizintal() );
	ImUiWidgetSetPadding( progressBarFrame, s_config.progressBar.padding );
	ImUiWidgetSetFixedHeight( progressBarFrame, s_config.progressBar.height );

	ImUiDrawWidgetSkinColor( progressBarFrame, s_config.skins[ ImUiToolboxSkin_ProgressBarBackground ], s_config.colors[ ImUiToolboxColor_ProgressBarBackground ] );

	ImUiWidget* progressBarProgress = ImUiWidgetBegin( window );

	if( value < min )
	{
		const float time		= ImUiWindowGetTime( window );
		const float fullWidth	= ImUiWidgetGetInnerSize( progressBarFrame ).width;
		const float cos			= (cosf( time * 8.0f ) * 0.15f) + 0.15f;
		const float sin			= (sinf( time * 4.0f ) * 0.5f) + 0.5f;
		const float width		= fullWidth * (0.1f + cos);
		const float margin		= sin * (fullWidth - width);
		ImUiWidgetSetMargin( progressBarProgress, ImUiBorderCreate( 0.0f, margin, 0.0f, 0.0f ) );
		ImUiWidgetSetFixedWidth( progressBarProgress, width );
		ImUiWidgetSetStretch( progressBarProgress, ImUiSizeCreateVertical() );
	}
	else
	{
		const float valueNorm = (value - min) / (max - min);
		ImUiWidgetSetStretch( progressBarProgress, ImUiSizeCreate( valueNorm, 1.0f ) );
	}

	ImUiDrawWidgetSkinColor( progressBarProgress, s_config.skins[ ImUiToolboxSkin_ProgressBarProgress ], s_config.colors[ ImUiToolboxColor_ProgressBarProgress ] );

	ImUiWidgetEnd( progressBarProgress );

	ImUiWidgetEnd( progressBarFrame );
}

static ImUiWidget* ImUiToolboxScrollAreaBegin( ImUiWindow* window, bool horizontal, bool vertical )
{
	ImUiWidget* scrollFrame = ImUiWidgetBeginNamed( window, IMUI_STR( "scroll_frame" ) );

	bool isNew;
	ImUiToolboxScrollState* state = ImUiWidgetAllocStateNew( scrollFrame, sizeof( *state ), &isNew );
	if( isNew )
	{
		state->offset = ImUiPosCreateZero();
	}

	state->offset.y = (sinf( ImUiWidgetGetTime( scrollFrame ) ) * 0.5f + 0.5f) * 400.0f;

	ImUiWidgetSetLayoutScroll( scrollFrame, state->offset );

	ImUiWidget* scrollArea = ImUiWidgetBeginNamed( window, IMUI_STR( "scroll_area" ) );

	const ImUiRect frameRect = ImUiWidgetGetRect( scrollFrame );
	const ImUiSize maxSize = ImUiSizeCreate(
		horizontal ? IMUI_FLOAT_MAX : frameRect.size.width,
		vertical ? IMUI_FLOAT_MAX : frameRect.size.height
	);
	ImUiWidgetSetMaxSize( scrollArea, maxSize );

	const ImUiSize areaSize		= ImUiWidgetGetSize( scrollArea );
	const float barMargin		= s_config.scrollArea.barSize + s_config.scrollArea.barSpacing;
	const bool hasHorizontalBar	= horizontal && areaSize.width > frameRect.size.width;
	const bool hasVerticalBar	= vertical && areaSize.height > frameRect.size.height;

	ImUiWidgetSetMargin( scrollArea, ImUiBorderCreate( 0.0f, 0.0f, hasHorizontalBar * barMargin, hasVerticalBar * barMargin ) );

	if( hasVerticalBar )
	{
		const ImUiRect barBackgroundRect = ImUiRectCreate(
			(frameRect.pos.x + frameRect.size.width) - s_config.scrollArea.barSize,
			frameRect.pos.y,
			s_config.scrollArea.barSize,
			frameRect.size.height
		);

		const float barSizeFactor	= frameRect.size.height / areaSize.height;
		const float barSize			= IMUI_MAX( frameRect.size.height * barSizeFactor, s_config.scrollArea.barMinSize );
		const float barOffset		= (state->offset.y / areaSize.height) * frameRect.size.height;

		const ImUiRect barPivotRect = ImUiRectCreate(
			barBackgroundRect.pos.x,
			frameRect.pos.y + barOffset,
			s_config.scrollArea.barSize,
			barSize
		);

		ImUiDrawSkinColor( scrollFrame, barBackgroundRect, s_config.skins[ ImUiToolboxSkin_ScrollAreaBarBackground ], s_config.colors[ ImUiToolboxColor_ScrollAreaBarBackground ] );
		ImUiDrawSkinColor( scrollFrame, barPivotRect, s_config.skins[ ImUiToolboxSkin_ScrollAreaBarPivot ], s_config.colors[ ImUiToolboxColor_ScrollAreaBarPivot ] );
	}

	return scrollFrame;
}

ImUiWidget* ImUiToolboxScrollAreaBeginHorizontal( ImUiWindow* window )
{
	return ImUiToolboxScrollAreaBegin( window, true, false );
}

ImUiWidget* ImUiToolboxScrollAreaBeginVertical( ImUiWindow* window )
{
	return ImUiToolboxScrollAreaBegin( window, false, true );
}

ImUiWidget* ImUiToolboxScrollAreaBeginBoth( ImUiWindow* window )
{
	return ImUiToolboxScrollAreaBegin( window, true, true );
}

void ImUiToolboxScrollAreaEnd( ImUiWidget* scroll )
{
	ImUiWidgetEnd( scroll->firstChild );
	ImUiWidgetEnd( scroll );
}
