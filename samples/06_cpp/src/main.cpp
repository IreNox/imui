#include "../../framework/framework.h"

#include "imui/imui_cpp.h"

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

using namespace imui;
using namespace imui::toolbox;

static ImUiTexture	s_fontTexture		= { NULL };
static ImUiFont*	s_font				= NULL;

static ImUiSkin		s_skinRect			= { NULL };
static ImUiTexture	s_skinRectTexture	= { NULL };
static ImUiSkin		s_skinLine			= { NULL };
static ImUiTexture	s_skinLineTexture	= { NULL };

static void			ImUiTestSetConfig();

static void			ImUiTestDoButtonsAndCheckBoxes( UiWindow& window, UiWidget& vLayout );
static void			ImUiTestDoSlidersAndProgressBars( UiWindow& window );
static void			ImUiTestDoTextEdit( UiWindow& window );
static void			ImUiTestDoScrollAndList( UiWindow& window );

static float		s_sliderValue1 = 2.5f;

struct ImUiTestCheckBoxState
{
	bool checked[ 3u ];
};

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

	UiWidget hLayout( window, "vMain" );
	hLayout.setStretch( UiSize::One );
	hLayout.setMargin( UiBorder( 25.0f ) );
	hLayout.setLayoutHorizontalSpacing( 10.0f );

	{
		UiWidget vLayout( window, "vMain" );
		vLayout.setStretch( UiSize::Horizontal );
		vLayout.setLayoutVerticalSpacing( 10.0f );

		ImUiTestDoButtonsAndCheckBoxes( window, vLayout );
		ImUiTestDoSlidersAndProgressBars( window );
		ImUiTestDoTextEdit( window );
	}

	ImUiTestDoScrollAndList( window );

	{
		const ImUiPos mousePos = ImUiInputGetMousePos( ImUiWindowGetContext( window.getInternal() ) );
		UiToolboxLabel mouseLabel;
		mouseLabel.beginFormat( window, "X: %.0f, Y: %.0f", mousePos.x, mousePos.y );
		mouseLabel.setMinWidth( 100.0f );
	}

	//ImUiDrawRectTexture( vLayout, ImUiRectCreateSize( 50.0f, 50.0f, s_fontTexture.size ), s_fontTexture );
}

static void ImUiTestDoButtonsAndCheckBoxes( UiWindow& window, UiWidget& vLayout )
{
	bool isNewState;
	ImUiTestCheckBoxState* state = vLayout.newState< ImUiTestCheckBoxState >( isNewState );
	if( isNewState )
	{
		state->checked[ 1u ] = true;
	}

	{
		UiWidget buttonsLayout( window, "buttons" );
		buttonsLayout.setStretch( UiSize::Zero );
		buttonsLayout.setLayoutHorizontalSpacing( 10.0f );

		if( toolbox::buttonLabel( window, "Button 1" ) )
		{
			state->checked[ 0u ] = !state->checked[ 0u ];
		}

		if( toolbox::buttonLabel( window, "Button 3" ) )
		{
			state->checked[ 1u ] = !state->checked[ 1u ];
		}

		if( toolbox::buttonLabel( window, "Button 2" ) )
		{
			state->checked[ 2u ] = !state->checked[ 2u ];
		}
	}

	{
		UiWidget checkLayout( window, "checks" );
		checkLayout.setStretch( UiSize::Horizontal );
		checkLayout.setLayoutVerticalSpacing( 10.0f );

		toolbox::checkBox( window, state->checked[ 0u ], "Check 1" );
		toolbox::checkBox( window, state->checked[ 1u ], "Check 2" );
		toolbox::checkBox( window, state->checked[ 2u ], "Check 3" );

		toolbox::checkBoxState( window, "Check State" );

		toolbox::labelFormat( window, "C1: %d, C2: %d, C3: %d", state->checked[ 0u ], state->checked[ 1u ], state->checked[ 2u ] );
	}
}

static void ImUiTestDoSlidersAndProgressBars( UiWindow& window )
{
	UiWidget sliderLayout( window, "sliders" );
	sliderLayout.setStretch( UiSize::Horizontal );
	sliderLayout.setLayoutVerticalSpacing( 10.0f );

	toolbox::slider( window, s_sliderValue1, 1.0f, 5.0f );

	const float value2 = toolbox::sliderState( window, 1.0f, 5.0f );

	toolbox::labelFormat( window, "V1: %.2f, V2: %.2f", s_sliderValue1, value2 );

	toolbox::progressBar( window, s_sliderValue1, 0.0f, 5.0f );
	toolbox::progressBar( window, -1.0f );
}

static void ImUiTestDoTextEdit( UiWindow& window )
{
	toolbox::textEditState( window, 128u );
}

static void ImUiTestDoScrollAndList( UiWindow& window )
{
	UiWidget vLayout( window, "vLayout" );
	vLayout.setStretch( UiSize::Horizontal );
	vLayout.setLayoutVerticalSpacing( 10.0f );

	toolbox::label( window, "Item count:" );
	const float itemCount = toolbox::sliderState( window, 0.0f, 128.0f, 32.0f );

	const bool useList = toolbox::checkBoxState( window, "List", true );

	const size_t count = (size_t)itemCount;
	if( useList )
	{
		UiToolboxList list( window, 25.0f, count );
		list.setMinSize( UiSize( 200.0f ) );

		for( size_t i = list.getBeginIndex(); i < list.getEndIndex(); ++i )
		{
			ImUiWidget* item = list.nextItem();
			ImUiWidgetSetPadding( item, ImUiBorderCreateAll( 4.0f ) );

			UiToolboxLabel label;
			label.beginFormat( window, "List Label %i", i );
			label.setVAlign( 0.5f );
		}
	}
	else
	{
		UiToolboxScrollArea scrollArea( window );
		scrollArea.setMinSize( UiSize( 200.0f ) );

		UiWidget scrollLayout( window, "scroll" );
		scrollLayout.setStretch( UiSize::Horizontal );
		scrollLayout.setLayoutVerticalSpacing( 4.0f );

		for( size_t i = 0; i < itemCount; ++i )
		{
			toolbox::labelFormat( window, "Scroll Label %i", i );
		}
	}
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
	const ImUiColor textEditCursorColor	= ImUiColorCreateWhite();

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
	config.colors[ ImUiToolboxColor_ListItemHover ]				= elementHoverColor;
	config.colors[ ImUiToolboxColor_ListItemClicked ]			= elementClickedColor;
	config.colors[ ImUiToolboxColor_ListItemSelected ]			= elementColor;
	_STATIC_ASSERT( ImUiToolboxColor_MAX == 26 );

	config.skins[ ImUiToolboxSkin_Button ]						= s_skinRect;
	config.skins[ ImUiToolboxSkin_CheckBox ]					= s_skinRect;
	config.skins[ ImUiToolboxSkin_CheckBoxChecked ]				= s_skinRect;
	config.skins[ ImUiToolboxSkin_SliderBackground ]			= s_skinLine;
	config.skins[ ImUiToolboxSkin_SliderPivot ]					= s_skinRect;
	config.skins[ ImUiToolboxSkin_TextEditBackground ]			= s_skinRect;
	config.skins[ ImUiToolboxSkin_ProgressBarBackground ]		= s_skinLine;
	config.skins[ ImUiToolboxSkin_ProgressBarProgress ]			= s_skinRect;
	config.skins[ ImUiToolboxSkin_ScrollAreaBarBackground ]		= s_skinRect;
	config.skins[ ImUiToolboxSkin_ScrollAreaBarPivot ]			= s_skinRect;
	config.skins[ ImUiToolboxSkin_ListItem ]					= s_skinRect;
	_STATIC_ASSERT( ImUiToolboxSkin_MAX == 11 );

	config.font						= s_font;

	config.button.padding			= ImUiBorderCreateAll( 8.0f );

	config.checkBox.size			= ImUiSizeCreateAll( 20.0f );
	config.checkBox.textSpacing		= 5.0f;

	config.slider.padding			= ImUiBorderCreateHorizontalVertical( 0.0f, 8.0f );
	config.slider.pivotSize			= 12.0f;
	config.slider.height			= 20.0f;

	config.textEdit.height			= 25.0f;
	config.textEdit.padding			= ImUiBorderCreateAll( 4.0f );
	config.textEdit.cursorSize		= ImUiSizeCreate( 1.0f, 12.0f );
	config.textEdit.blinkTime		= 0.53f;

	config.progressBar.height		= 20.0f;
	config.progressBar.padding		= ImUiBorderCreateHorizontalVertical( 0.0f, 4.0f );

	config.scrollArea.barSize		= 8.0f;
	config.scrollArea.barSpacing	= 4.0f;
	config.scrollArea.barMinSize	= 20.0f;

	config.list.itemSpacing			= 4.0f;

	ImUiToolboxSetConfig( &config );
}
