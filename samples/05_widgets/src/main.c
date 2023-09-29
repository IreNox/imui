#include "../../framework/framework.h"

#include "imui/imui.h"
#include "imui/imui_toolbox.h"

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static ImUiTexture	s_fontTexture		= { NULL };
static ImUiFont*	s_font				= NULL;

static ImUiSkin		s_skinRect			= { NULL };
static ImUiTexture	s_skinRectTexture	= { NULL };
static ImUiSkin		s_skinLine			= { NULL };
static ImUiTexture	s_skinLineTexture	= { NULL };

static void			ImUiTestSetConfig();

static void			ImUiTestDoButtonsAndCheckBoxes( ImUiWindow* window, ImUiWidget* vLayout );
static void			ImUiTestDoSlidersAndProgressBars( ImUiWindow* window );
static void			ImUiTestDoTextEdit( ImUiWindow* window );
static void			ImUiTestDoScrollArea( ImUiWindow* window );

static float		s_sliderValue1 = 2.5f;

void ImUiFrameworkTick( ImUiSurface* surface )
{
#if 0
	ImUiFrameworkShutdown( ImUiSurfaceGetContext( surface ) );
	ImUiFrameworkInitialize( ImUiSurfaceGetContext( surface ) );
#endif
#if 1
	ImUiTestSetConfig();
#endif

	const ImUiSize surfaceSize = ImUiSurfaceGetSize( surface );
	ImUiWindow* window = ImUiWindowBegin( surface, ImUiStringViewCreate( "main" ), ImUiRectCreate( 0.0f, 0.0f, surfaceSize.width, surfaceSize.height ), 1 );

	ImUiWidget* hLayout = ImUiWidgetBeginNamed( window, ImUiStringViewCreate( "hMain" ) );
	ImUiWidgetSetStretch( hLayout, ImUiSizeCreateOne() );
	ImUiWidgetSetMargin( hLayout, ImUiBorderCreateAll( 25.0f ) );
	ImUiWidgetSetLayoutHorizontalSpacing( hLayout, 10.0f );

	ImUiWidget* vLayout = ImUiWidgetBeginNamed( window, ImUiStringViewCreate( "vMain" ) );
	ImUiWidgetSetStretch( vLayout, ImUiSizeCreateHorizintal() );
	ImUiWidgetSetLayoutVerticalSpacing( vLayout, 10.0f );

	ImUiTestDoButtonsAndCheckBoxes( window, vLayout );
	ImUiTestDoSlidersAndProgressBars( window );
	ImUiTestDoTextEdit( window );

	ImUiWidgetEnd( vLayout );

	ImUiTestDoScrollArea( window );

	const ImUiPos mousePos = ImUiInputGetMousePos( ImUiWindowGetContext( window ) );
	ImUiWidget* mouseLabel = ImUiToolboxLabelBeginFormat( window, "X: %.0f, Y: %.0f", mousePos.x, mousePos.y );
	ImUiWidgetSetMinWidth( mouseLabel, 100.0f );
	ImUiToolboxLabelEnd( mouseLabel );

	//ImUiDrawRectTexture( vLayout, ImUiRectCreateSize( 50.0f, 50.0f, s_fontTexture.size ), s_fontTexture );


	ImUiWidgetEnd( hLayout );

	ImUiWindowEnd( window );
}

static void ImUiTestDoButtonsAndCheckBoxes( ImUiWindow* window, ImUiWidget* vLayout )
{
	bool isNewState;
	bool* checked = (bool*)ImUiWidgetAllocStateNew( vLayout, sizeof( bool ) * 3u, &isNewState );
	if( isNewState )
	{
		checked[ 1u ] = true;
	}

	{
		ImUiWidget* buttonsLayout = ImUiWidgetBeginNamed( window, ImUiStringViewCreate( "buttons" ) );
		ImUiWidgetSetStretch( buttonsLayout, ImUiSizeCreateZero() );
		ImUiWidgetSetLayoutHorizontalSpacing( buttonsLayout, 10.0f );

		if( ImUiToolboxButtonLabel( window, IMUI_STR( "Button 1" ) ) )
		{
			checked[ 0u ] = !checked[ 0u ];
		}

		if( ImUiToolboxButtonLabel( window, IMUI_STR( "Button 2" ) ) )
		{
			checked[ 1u ] = !checked[ 1u ];
		}

		if( ImUiToolboxButtonLabel( window, IMUI_STR( "Button 3" ) ) )
		{
			checked[ 2u ] = !checked[ 2u ];
		}

		ImUiWidgetEnd( buttonsLayout );
	}

	{
		ImUiWidget* checkLayout = ImUiWidgetBeginNamed( window, ImUiStringViewCreate( "checks" ) );
		ImUiWidgetSetStretch( checkLayout, ImUiSizeCreateHorizintal() );
		ImUiWidgetSetLayoutVerticalSpacing( checkLayout, 10.0f );

		ImUiToolboxCheckBox( window, &checked[ 0u ], IMUI_STR( "Check 1" ) );
		ImUiToolboxCheckBox( window, &checked[ 1u ], IMUI_STR( "Check 2" ) );
		ImUiToolboxCheckBox( window, &checked[ 2u ], IMUI_STR( "Check 3" ) );

		ImUiToolboxCheckBoxState( window, IMUI_STR( "Check State" ) );

		ImUiToolboxLabelFormat( window, "C1: %d, C2: %d, C3: %d", checked[ 0u ], checked[ 1u ], checked[ 2u ] );

		ImUiWidgetEnd( checkLayout );
	}
}

static void ImUiTestDoSlidersAndProgressBars( ImUiWindow* window )
{
	ImUiWidget* sliderLayout = ImUiWidgetBeginNamed( window, ImUiStringViewCreate( "sliders" ) );
	ImUiWidgetSetStretch( sliderLayout, ImUiSizeCreate( 1.0f, 0.0f ) );
	ImUiWidgetSetLayoutVerticalSpacing( sliderLayout, 10.0f );

	ImUiToolboxSliderMinMax( window, &s_sliderValue1, 1.0f, 5.0f );

	const float value2 = ImUiToolboxSliderStateMinMax( window, 1.0f, 5.0f );

	ImUiToolboxLabelFormat( window, "V1: %.2f, V2: %.2f", s_sliderValue1, value2 );

	ImUiToolboxProgressBarMinMax( window, s_sliderValue1, 0.0f, 5.0f );
	ImUiToolboxProgressBar( window, -1.0f );

	ImUiWidgetEnd( sliderLayout );
}

static void ImUiTestDoTextEdit( ImUiWindow* window )
{
	//imuitoolboxtextedit
}

static void ImUiTestDoScrollArea( ImUiWindow* window )
{
	ImUiWidget* vLayout = ImUiWidgetBeginNamed( window, ImUiStringViewCreate( "vLayout" ) );
	ImUiWidgetSetStretch( vLayout, ImUiSizeCreate( 1.0f, 0.0f ) );
	ImUiWidgetSetLayoutVerticalSpacing( vLayout, 10.0f );

	ImUiToolboxLabel( window, IMUI_STR( "Item count:" ) );
	const float itemCount = ImUiToolboxSliderStateMinMaxDefault( window, 0.0f, 128.0f, 32.0f );

	ImUiWidget* scrollArea = ImUiToolboxScrollAreaBeginVertical( window );
	ImUiWidgetSetMinSize( scrollArea, ImUiSizeCreateAll( 200.0f ) );

	ImUiWidget* scrollLayout = ImUiWidgetBeginNamed( window, IMUI_STR( "scroll" ));
	ImUiWidgetSetStretch( scrollLayout, ImUiSizeCreateHorizintal() );
	ImUiWidgetSetLayoutVerticalSpacing( scrollLayout, 4.0f );

	const size_t count = (size_t)itemCount;
	for( size_t i = 0; i < itemCount; ++i )
	{
		ImUiToolboxLabelFormat( window, "Scroll Label %i", i );
	}

	ImUiWidgetEnd( scrollLayout );
	ImUiToolboxScrollAreaEnd( scrollArea );

	ImUiWidgetEnd( vLayout );
}

bool ImUiFrameworkInitialize( ImUiContext* imui )
{
	if( !ImUiFrameworkFontCreate( &s_font, &s_fontTexture, "c:/windows/fonts/arial.ttf", 15.0f ) )
	{
		return false;
	}

	if( !ImUiFrameworkSkinCreate( &s_skinRect, &s_skinRectTexture, 32u, 8.0f, 128.0f, false ) )
	{
		return false;
	}

	if( !ImUiFrameworkSkinCreate( &s_skinLine, &s_skinLineTexture, 32u, 6.0f, 64.0f, true ) )
	{
		return false;
	}

	ImUiTestSetConfig();

	return true;
}

void ImUiFrameworkShutdown( ImUiContext* imui )
{
	ImUiFrameworkFontDestroy( &s_font, &s_fontTexture );
	ImUiFrameworkSkinDestroy( &s_skinRect, &s_skinRectTexture );
	ImUiFrameworkSkinDestroy( &s_skinLine, &s_skinLineTexture );
}

static void ImUiTestSetConfig()
{
	const ImUiColor textColor			= ImUiColorCreateWhite();
	const ImUiColor elementColor		= ImUiColorCreateFloat( 0.1f, 0.5f, 0.7f, 1.0f );
	const ImUiColor elementHoverColor	= ImUiColorCreateFloat( 0.3f, 0.7f, 0.9f, 1.0f );
	const ImUiColor elementClickedColor	= ImUiColorCreateFloat( 0.0f, 0.3f, 0.5f, 1.0f );
	const ImUiColor backgroundColor		= ImUiColorCreateFloat( 0.0f, 0.3f, 0.5f, 1.0f );
	const ImUiColor checkedColor		= ImUiColorCreateFloat( 1.0f, 0.5f, 0.7f, 1.0f );
	const ImUiColor textEditCursorColor	= ImUiColorCreateBlack();

	ImUiToolboxConfig config;
	config.colors[ ImUiToolboxColor_Text ]						= ImUiColorCreateFloat( 1.0f, 1.0f, 1.0f, 1.0f );
	config.colors[ ImUiToolboxColor_Button ]					= elementColor;
	config.colors[ ImUiToolboxColor_ButtonHover ]				= elementHoverColor;
	config.colors[ ImUiToolboxColor_ButtonClicked ]				= elementClickedColor;
	config.colors[ ImUiToolboxColor_ButtonText ]				= textColor;
	config.colors[ ImUiToolboxColor_CheckBox ]					= elementColor;
	config.colors[ ImUiToolboxColor_CheckBoxHover ]				= elementHoverColor;
	config.colors[ ImUiToolboxColor_CheckBoxClicked ]			= elementClickedColor;
	config.colors[ ImUiToolboxColor_CheckBoxChecked ]			= checkedColor;
	config.colors[ ImUiToolboxColor_CheckBoxCheckedHover ]		= ImUiColorCreateFloat( 1.0f, 0.7f, 0.9f, 1.0f );
	config.colors[ ImUiToolboxColor_CheckBoxCheckedClicked ]	= ImUiColorCreateFloat( 1.0f, 0.3f, 0.5f, 1.0f );
	config.colors[ ImUiToolboxColor_SliderBackground ]			= backgroundColor;
	config.colors[ ImUiToolboxColor_SliderPivot ]				= elementColor;
	config.colors[ ImUiToolboxColor_SliderPivotHover ]			= elementHoverColor;
	config.colors[ ImUiToolboxColor_SliderPivotClicked ]		= checkedColor;
	config.colors[ ImUiToolboxColor_TextEditBackground ]		= backgroundColor;
	config.colors[ ImUiToolboxColor_TextEditText ]				= textColor;
	config.colors[ ImUiToolboxColor_TextEditCursor ]			= textEditCursorColor;
	config.colors[ ImUiToolboxColor_TextEditSelection ]			= elementColor;
	config.colors[ ImUiToolboxColor_ProgressBarBackground ]		= backgroundColor;
	config.colors[ ImUiToolboxColor_ProgressBarProgress ]		= elementColor;
	config.colors[ ImUiToolboxColor_ScrollAreaBarBackground ]	= backgroundColor;
	config.colors[ ImUiToolboxColor_ScrollAreaBarPivot ]		= elementColor;
	config.colors[ ImUiToolboxColor_ListItemBackground ]		= backgroundColor;
	config.colors[ ImUiToolboxColor_ListItemText ]				= textColor;
	_STATIC_ASSERT( ImUiToolboxColor_MAX == 25 );

	config.skins[ ImUiToolboxSkin_Button ]						= s_skinRect;
	config.skins[ ImUiToolboxSkin_CheckBox ]					= s_skinRect;
	config.skins[ ImUiToolboxSkin_CheckBoxChecked ]				= s_skinRect;
	config.skins[ ImUiToolboxSkin_SliderBackground ]			= s_skinLine;
	config.skins[ ImUiToolboxSkin_SliderPivot ]					= s_skinRect;
	config.skins[ ImUiToolboxSkin_TextEditBackground ]			= s_skinLine;
	config.skins[ ImUiToolboxSkin_ProgressBarBackground ]		= s_skinLine;
	config.skins[ ImUiToolboxSkin_ProgressBarProgress ]			= s_skinRect;
	config.skins[ ImUiToolboxSkin_ScrollAreaBarBackground ]		= s_skinRect;
	config.skins[ ImUiToolboxSkin_ScrollAreaBarPivot ]			= s_skinRect;
	config.skins[ ImUiToolboxSkin_ListItem ]					= s_skinLine;
	_STATIC_ASSERT( ImUiToolboxSkin_MAX == 11 );

	config.font						= s_font;

	config.button.padding			= ImUiBorderCreateAll( 8.0f );

	config.checkBox.size			= ImUiSizeCreateAll( 20.0f );
	config.checkBox.textSpacing		= 5.0f;

	config.slider.padding			= ImUiBorderCreateHorizontalVertical( 0.0f, 8.0f );
	config.slider.pivotSize			= 12.0f;
	config.slider.height			= 20.0f;

	config.textEdit.height			= 20.0f;
	config.textEdit.padding			= ImUiBorderCreateAll( 4.0f );
	config.textEdit.cursorSize		= ImUiSizeCreate( 1.0f, 12.0f );
	config.textEdit.blinkTime		= 1.0f;

	config.progressBar.height		= 20.0f;
	config.progressBar.padding		= ImUiBorderCreateHorizontalVertical( 0.0f, 4.0f );

	config.scrollArea.barSize		= 8.0f;
	config.scrollArea.barSpacing	= 4.0f;
	config.scrollArea.barMinSize	= 20.0f;

	ImUiToolboxSetConfig( &config );
}
