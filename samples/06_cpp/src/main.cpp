#include "../../framework/framework.h"

#include "imui/imui_cpp.h"

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

using namespace imui;

void ImUiFrameworkTick( ImUiSurface* surface )
{
#if 0
	ImUiFrameworkShutdown( ImUiSurfaceGetContext( surface ) );
	ImUiFrameworkInitialize( ImUiSurfaceGetContext( surface ) );
#endif
#if 1
	ImUiTestSetConfig();
#endif

	const UiSize surfaceSize = UiSize( ImUiSurfaceGetSize( surface ) );
	UiWindow window( surface, "main", UiRect( UiPos::Zero, surfaceSize ), 1 );

	UiWidget vLayout( window, "vMain" );
	vLayout.setStretch( UiSize::Zero );
	vLayout.setMargin( UiBorder( 25.0f ) );
	vLayout.setLayoutVerticalSpacing( 10.0f );

	static bool checked[ 3u ] = { false, true, false };
	{
		UiWidget buttonsLayout( window, "buttons" );
		buttonsLayout.setStretch( UiSize::Horizontal );
		buttonsLayout.setLayoutHorizontalSpacing( 10.0f );

		if( toolbox::buttonLabel( window, "Button 1" ) )
		{
			checked[ 0u ] = !checked[ 0u ];
		}

		if( toolbox::buttonLabel( window, "Button 2" ) )
		{
			checked[ 1u ] = !checked[ 1u ];
		}

		if( toolbox::buttonLabel( window, "Button 3" ) )
		{
			checked[ 2u ] = !checked[ 2u ];
		}
	}

	{
		UiWidget checkLayout( window, "checks" );
		checkLayout.setStretch( UiSize::Horizontal );
		checkLayout.setLayoutVerticalSpacing( 10.0f );

		toolbox::checkBox( window, checked[ 0u ], "Check 1" );
		toolbox::checkBox( window, checked[ 1u ], "Check 2" );
		toolbox::checkBox( window, checked[ 2u ], "Check 3" );

		toolbox::labelFormat( window, "C1: %d, C2: %d, C3: %d", checked[ 0u ], checked[ 1u ], checked[ 2u ] );
	}

	{
		UiWidget sliderLayout( window, "sliders" );
		sliderLayout.setStretch( UiSize::Horizontal );
		sliderLayout.setLayoutVerticalSpacing( 10.0f );

		static float value = 2.5f;
		toolbox::sliderMinMax( window, value, 0.0f, 5.0f );

		toolbox::labelFormat( window, "Value: %.2f", value );
	}

	//ImUiDrawRectTexture( vLayout, ImUiRectCreateSize( 50.0f, 50.0f, s_fontTexture.size ), s_fontTexture );
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
	ImUiToolboxConfig config;
	config.colors[ ImUiToolboxColor_Text ]						= ImUiColorCreate( 1.0f, 1.0f, 1.0f, 1.0f );
	config.colors[ ImUiToolboxColor_Button ]					= ImUiColorCreate( 0.1f, 0.5f, 0.7f, 1.0f );
	config.colors[ ImUiToolboxColor_ButtonHover ]				= ImUiColorCreate( 0.3f, 0.7f, 0.9f, 1.0f );
	config.colors[ ImUiToolboxColor_ButtonClicked ]				= ImUiColorCreate( 0.0f, 0.3f, 0.5f, 1.0f );
	config.colors[ ImUiToolboxColor_ButtonText ]				= ImUiColorCreate( 1.0f, 1.0f, 1.0f, 1.0f );
	config.colors[ ImUiToolboxColor_CheckBox ]					= ImUiColorCreate( 0.1f, 0.5f, 0.7f, 1.0f );
	config.colors[ ImUiToolboxColor_CheckBoxHover ]				= ImUiColorCreate( 0.3f, 0.7f, 0.9f, 1.0f );
	config.colors[ ImUiToolboxColor_CheckBoxClicked ]			= ImUiColorCreate( 0.0f, 0.3f, 0.5f, 1.0f );
	config.colors[ ImUiToolboxColor_CheckBoxChecked ]			= ImUiColorCreate( 1.0f, 0.5f, 0.7f, 1.0f );
	config.colors[ ImUiToolboxColor_CheckBoxCheckedHover ]		= ImUiColorCreate( 1.0f, 0.7f, 0.9f, 1.0f );
	config.colors[ ImUiToolboxColor_CheckBoxCheckedClicked ]	= ImUiColorCreate( 1.0f, 0.3f, 0.5f, 1.0f );
	config.colors[ ImUiToolboxColor_SliderBackground ]			= ImUiColorCreate( 0.0f, 0.3f, 0.5f, 1.0f );
	config.colors[ ImUiToolboxColor_SliderPivot ]				= ImUiColorCreate( 0.1f, 0.5f, 0.7f, 1.0f );
	config.colors[ ImUiToolboxColor_SliderPivotHover ]			= ImUiColorCreate( 0.3f, 0.7f, 0.9f, 1.0f );
	config.colors[ ImUiToolboxColor_SliderPivotClicked ]		= ImUiColorCreate( 1.0f, 0.3f, 0.5f, 1.0f );

	config.skins[ ImUiToolboxSkin_Button ]						= s_skinRect;
	config.skins[ ImUiToolboxSkin_CheckBox ]					= s_skinRect;
	config.skins[ ImUiToolboxSkin_CheckBoxChecked ]				= s_skinRect;
	config.skins[ ImUiToolboxSkin_SliderBackground ]			= s_skinLine;
	config.skins[ ImUiToolboxSkin_SliderPivot ]					= s_skinRect;

	config.font					= s_font;

	config.buttonPadding		= ImUiBorderCreateAll( 8.0f );

	config.checkBoxSize			= ImUiSizeCreateAll( 20.0f );
	config.checkBoxTextSpacing	= 5.0f;

	config.sliderPadding		= ImUiBorderCreateHorizontalVertical( 0.0f, 8.0f );
	config.sliderPivotSize		= 12.0f;
	config.sliderHeight			= 20.0f;

	ImUiToolboxSetConfig( &config );
}
