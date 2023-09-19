#include "../../framework/framework.h"

#include "imui/imui.h"
#include "imui/imui_widgets.h"

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

	ImUiWidget* vLayout = ImUiWidgetBeginNamed( window, ImUiStringViewCreate( "vMain" ) );
	ImUiWidgetSetStretch( vLayout, ImUiSizeCreateZero() );
	ImUiWidgetSetMargin( vLayout, ImUiBorderCreateAll( 25.0f ) );
	ImUiWidgetSetLayoutVerticalSpacing( vLayout, 10.0f );

	bool isNewState;
	bool* checked = (bool*)ImUiWidgetAllocStateNew( vLayout, sizeof( bool ) * 3u, &isNewState );
	if( isNewState )
	{
		checked[ 1u ] = true;
	}

	{
		ImUiWidget* buttonsLayout = ImUiWidgetBeginNamed( window, ImUiStringViewCreate( "buttons" ) );
		ImUiWidgetSetStretch( buttonsLayout, ImUiSizeCreateHorizintal() );
		ImUiWidgetSetLayoutHorizontalSpacing( buttonsLayout, 10.0f );

		if( ImUiWidgetsButtonLabel( window, IMUI_STR( "Button 1" ) ) )
		{
			checked[ 0u ] = !checked[ 0u ];
		}

		if( ImUiWidgetsButtonLabel( window, IMUI_STR( "Button 2" ) ) )
		{
			checked[ 1u ] = !checked[ 1u ];
		}

		if( ImUiWidgetsButtonLabel( window, IMUI_STR( "Button 3" ) ) )
		{
			checked[ 2u ] = !checked[ 2u ];
		}

		ImUiWidgetEnd( buttonsLayout );
	}

	{
		ImUiWidget* checkLayout = ImUiWidgetBeginNamed( window, ImUiStringViewCreate( "checks" ) );
		ImUiWidgetSetStretch( checkLayout, ImUiSizeCreateHorizintal() );
		ImUiWidgetSetLayoutVerticalSpacing( checkLayout, 10.0f );

		ImUiWidgetsCheckBox( window, &checked[ 0u ], IMUI_STR( "Check 1" ) );
		ImUiWidgetsCheckBox( window, &checked[ 1u ], IMUI_STR( "Check 2" ) );
		ImUiWidgetsCheckBox( window, &checked[ 2u ], IMUI_STR( "Check 3" ) );

		ImUiWidgetsCheckBoxState( window, IMUI_STR( "Check State" ) );

		ImUiWidgetsLabelFormat( window, "C1: %d, C2: %d, C3: %d", checked[ 0u ], checked[ 1u ], checked[ 2u ] );

		ImUiWidgetEnd( checkLayout );
	}

	{
		ImUiWidget* sliderLayout = ImUiWidgetBeginNamed( window, ImUiStringViewCreate( "sliders" ) );
		ImUiWidgetSetStretch( sliderLayout, ImUiSizeCreateHorizintal() );
		ImUiWidgetSetLayoutVerticalSpacing( sliderLayout, 10.0f );

		static float value1 = 2.5f;
		ImUiWidgetsSliderMinMax( window, &value1, 1.0f, 5.0f );

		const float value2 = ImUiWidgetsSliderStateMinMax( window, 1.0f, 5.0f );

		ImUiWidgetsLabelFormat( window, "V1: %.2f, V2: %.2f", value1, value2 );

		ImUiWidgetsProgressBarMinMax( window, value1, 0.0f, 5.0f );
		ImUiWidgetsProgressBar( window, -1.0f );

		ImUiWidgetEnd( sliderLayout );
	}

	//ImUiDrawRectTexture( vLayout, ImUiRectCreateSize( 50.0f, 50.0f, s_fontTexture.size ), s_fontTexture );

	ImUiWidgetEnd( vLayout );

	ImUiWindowEnd( window );
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
	ImUiWidgetsConfig config;
	config.colors[ ImUiWidgetsColor_Text ]						= ImUiColorCreate( 1.0f, 1.0f, 1.0f, 1.0f );
	config.colors[ ImUiWidgetsColor_Button ]					= ImUiColorCreate( 0.1f, 0.5f, 0.7f, 1.0f );
	config.colors[ ImUiWidgetsColor_ButtonHover ]				= ImUiColorCreate( 0.3f, 0.7f, 0.9f, 1.0f );
	config.colors[ ImUiWidgetsColor_ButtonClicked ]				= ImUiColorCreate( 0.0f, 0.3f, 0.5f, 1.0f );
	config.colors[ ImUiWidgetsColor_ButtonText ]				= ImUiColorCreate( 1.0f, 1.0f, 1.0f, 1.0f );
	config.colors[ ImUiWidgetsColor_CheckBox ]					= ImUiColorCreate( 0.1f, 0.5f, 0.7f, 1.0f );
	config.colors[ ImUiWidgetsColor_CheckBoxHover ]				= ImUiColorCreate( 0.3f, 0.7f, 0.9f, 1.0f );
	config.colors[ ImUiWidgetsColor_CheckBoxClicked ]			= ImUiColorCreate( 0.0f, 0.3f, 0.5f, 1.0f );
	config.colors[ ImUiWidgetsColor_CheckBoxChecked ]			= ImUiColorCreate( 1.0f, 0.5f, 0.7f, 1.0f );
	config.colors[ ImUiWidgetsColor_CheckBoxCheckedHover ]		= ImUiColorCreate( 1.0f, 0.7f, 0.9f, 1.0f );
	config.colors[ ImUiWidgetsColor_CheckBoxCheckedClicked ]	= ImUiColorCreate( 1.0f, 0.3f, 0.5f, 1.0f );
	config.colors[ ImUiWidgetsColor_SliderBackground ]			= ImUiColorCreate( 0.0f, 0.3f, 0.5f, 1.0f );
	config.colors[ ImUiWidgetsColor_SliderPivot ]				= ImUiColorCreate( 0.1f, 0.5f, 0.7f, 1.0f );
	config.colors[ ImUiWidgetsColor_SliderPivotHover ]			= ImUiColorCreate( 0.3f, 0.7f, 0.9f, 1.0f );
	config.colors[ ImUiWidgetsColor_SliderPivotClicked ]		= ImUiColorCreate( 1.0f, 0.3f, 0.5f, 1.0f );
	config.colors[ ImUiWidgetsColor_TextEditBackground ]		= ImUiColorCreate( 0.7f, 0.7f, 0.9f, 1.0f );
	config.colors[ ImUiWidgetsColor_TextEditText ]				= ImUiColorCreate( 1.0f, 1.0f, 1.0f, 1.0f );
	config.colors[ ImUiWidgetsColor_TextEditCursor ]			= ImUiColorCreate( 1.0f, 1.0f, 1.0f, 1.0f );
	config.colors[ ImUiWidgetsColor_TextEditSelection ]			= ImUiColorCreate( 0.5f, 0.5f, 0.7f, 1.0f );
	config.colors[ ImUiWidgetsColor_ProgressBarBackground ]		= ImUiColorCreate( 0.0f, 0.3f, 0.5f, 1.0f );
	config.colors[ ImUiWidgetsColor_ProgressBarProgress ]		= ImUiColorCreate( 0.1f, 0.5f, 0.7f, 1.0f );

	config.skins[ ImUiWidgetsSkin_Button ]						= s_skinRect;
	config.skins[ ImUiWidgetsSkin_CheckBox ]					= s_skinRect;
	config.skins[ ImUiWidgetsSkin_CheckBoxChecked ]				= s_skinRect;
	config.skins[ ImUiWidgetsSkin_SliderBackground ]			= s_skinLine;
	config.skins[ ImUiWidgetsSkin_SliderPivot ]					= s_skinRect;
	config.skins[ ImUiWidgetsSkin_TextEditBackground ]			= s_skinLine;
	config.skins[ ImUiWidgetsSkin_ProgressBarBackground ]		= s_skinLine;
	config.skins[ ImUiWidgetsSkin_ProgressBarProgress ]			= s_skinRect;

	config.font					= s_font;

	config.buttonPadding		= ImUiBorderCreateAll( 8.0f );

	config.checkBoxSize			= ImUiSizeCreateAll( 20.0f );
	config.checkBoxTextSpacing	= 5.0f;

	config.sliderPadding		= ImUiBorderCreateHorizontalVertical( 0.0f, 8.0f );
	config.sliderPivotSize		= 12.0f;
	config.sliderHeight			= 20.0f;

	config.textEditHeight		= 20.0f;
	config.textEditPadding		= ImUiBorderCreateAll( 4.0f );
	config.textEditCursorSize	= ImUiSizeCreate( 1.0f, 12.0f );
	config.textEditBlinkTime	= 1.0f;

	config.progressBarHeight	= 20.0f;
	config.progressBarPadding	= ImUiBorderCreateHorizontalVertical( 0.0f, 4.0f );

	ImUiWidgetsSetConfig( &config );
}
