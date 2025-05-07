#include "00_samples.h"

#include "framework.h"

#include "imui/imui_cpp.h"

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define IMUI_ARRAY_COUNT( arr ) (sizeof( arr ) / sizeof( *(arr) ))

using namespace imui;
using namespace imui::toolbox;

typedef struct ImUiToolboxCppSampleContext
{
	float							sliderValue1;

#ifndef IMUI_NO_SAMPLE_FRAMEWORK
	ImUiFrameworkToolboxConfigData	configData;
#endif
} ImUiToolboxCppSampleContext;

static ImUiToolboxCppSampleContext s_toolboxCppContext = { 2.5f };

static void			ImUiToolboxCppSampleButtonsAndCheckBoxes( UiToolboxWindow& window, UiWidget& vLayout );
static void			ImUiToolboxCppSampleSlidersAndProgressBars( UiToolboxWindow& window );
static void			ImUiToolboxCppSampleTextEdit( UiToolboxWindow& window );
static void			ImUiToolboxCppSampleDropDown( UiToolboxWindow& window );
static void			ImUiToolboxCppSamplePopup( UiToolboxWindow& window );
static void			ImUiToolboxCppSampleScrollAndList( UiToolboxWindow& window );

struct ImUiTestCheckBoxState
{
	bool checked[ 3u ];
};

struct ImUiTestPopupState
{
	bool isOpen;
};

void ImUiToolboxCppSampleTick( ImUiWindow* cWindow )
{
#ifndef IMUI_NO_SAMPLE_FRAMEWORK
	ImUiFrameworkToolboxConfigDataApply( &s_toolboxCppContext.configData );
#endif

	UiToolboxWindow window( cWindow );

	UiWidget hLayout( window, "vMain" );
	hLayout.setStretchOne();
	hLayout.setMargin( UiBorder( 25.0f ) );
	hLayout.setLayoutHorizontal( 10.0f );

	{
		UiWidget vLayout( window, "vMain" );
		vLayout.setHStretch( 1.0f );
		vLayout.setLayoutVertical( 10.0f );

		ImUiToolboxCppSampleButtonsAndCheckBoxes( window, vLayout );
		ImUiToolboxCppSampleSlidersAndProgressBars( window );
		ImUiToolboxCppSampleTextEdit( window );
		ImUiToolboxCppSampleDropDown( window );
		ImUiToolboxCppSamplePopup( window );
	}

	ImUiToolboxCppSampleScrollAndList( window );

	{
		const ImUiPos mousePos = ImUiInputGetMousePos( ImUiWindowGetContext( window.getInternal() ) );
		UiToolboxLabel mouseLabel;
		mouseLabel.beginFormat( window, "X: %.0f, Y: %.0f", mousePos.x, mousePos.y );
		mouseLabel.setMinWidth( 100.0f );
		mouseLabel.setVAlign( 0.0f );
	}

	//ImUiDrawRectTexture( vLayout, ImUiRectCreateSize( 50.0f, 50.0f, s_toolboxCppContext.fontTexture.size ), s_toolboxCppContext.fontTexture );
}

static void ImUiToolboxCppSampleButtonsAndCheckBoxes( UiToolboxWindow& window, UiWidget& vLayout )
{
	bool isNewState;
	ImUiTestCheckBoxState* state = vLayout.newState< ImUiTestCheckBoxState >( isNewState );
	if( isNewState )
	{
		state->checked[ 1u ] = true;
	}

	{
		UiWidget buttonsLayout( window, "buttons" );
		buttonsLayout.setLayoutHorizontal( 10.0f );

		if( window.buttonLabel( "Button 1" ) )
		{
			state->checked[ 0u ] = !state->checked[ 0u ];
		}

		if( window.buttonLabel( "Button 3" ) )
		{
			state->checked[ 1u ] = !state->checked[ 1u ];
		}

		if( window.buttonLabel( "Button 2" ) )
		{
			state->checked[ 2u ] = !state->checked[ 2u ];
		}
	}

	{
		UiWidget checkLayout( window, "checks" );
		checkLayout.setHStretch( 1.0f );
		checkLayout.setLayoutVertical( 10.0f );

		window.checkBox( state->checked[ 0u ], "Check 1" );
		window.checkBox( state->checked[ 1u ], "Check 2" );
		window.checkBox( state->checked[ 2u ], "Check 3" );

		window.checkBoxState( "Check State" );

		window.labelFormat( "C1: %d, C2: %d, C3: %d", state->checked[ 0u ], state->checked[ 1u ], state->checked[ 2u ] );
	}
}

static void ImUiToolboxCppSampleSlidersAndProgressBars( UiToolboxWindow& window )
{
	UiWidget sliderLayout( window, "sliders" );
	sliderLayout.setHStretch( 1.0f );
	sliderLayout.setLayoutVertical( 10.0f );

	window.slider( s_toolboxCppContext.sliderValue1, 1.0f, 5.0f );

	const float value2 = window.sliderState( 1.0f, 5.0f );

	window.labelFormat( "V1: %.2f, V2: %.2f", s_toolboxCppContext.sliderValue1, value2 );

	window.progressBar( s_toolboxCppContext.sliderValue1, 0.0f, 5.0f );
	window.progressBar( -1.0f );
}

static void ImUiToolboxCppSampleTextEdit( UiToolboxWindow& window )
{
	window.textEditState( 128u );
}

static void ImUiToolboxCppSampleScrollAndList( UiToolboxWindow& window )
{
	UiWidget vLayout( window, "vLayout" );
	vLayout.setHStretch( 1.0f );
	vLayout.setLayoutVertical( 10.0f );

	window.label( "Item count:" );
	const float itemCount = window.sliderState( 0.0f, 128.0f, 32.0f );

	const bool useList = window.checkBoxState( "List", true );

	const size_t count = (size_t)itemCount;
	if( useList )
	{
		UiToolboxList list( window, 25.0f, count, true );
		list.setMinSize( UiSize( 200.0f ) );

		for( size_t i = list.getBeginIndex(); i < list.getEndIndex(); ++i )
		{
			UiWidget item;
			list.nextItem( &item );
			item.setPadding( UiBorder( 4.0f ) );

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
		scrollLayout.setHStretch( 1.0f );
		scrollLayout.setLayoutVertical( 4.0f );

		for( size_t i = 0; i < itemCount; ++i )
		{
			window.labelFormat( "Scroll Label %i", i );
		}
	}
}

static void ImUiToolboxCppSampleDropDown( UiToolboxWindow& window )
{
	const char* items[] =
	{
		"Item 1",
		"Item 2",
		"Item 3",
		"Item 4",
		"Item 5",
		"Item 6",
		"Item 7",
		"Item 8",
		"Item 9",
		"Item 10",
		"Item 11",
		"Item 12",
		"Item 13",
		"Item 14"
	};

	window.dropDown( items, IMUI_ARRAY_COUNT( items ) );
}

static void ImUiToolboxCppSamplePopup( UiToolboxWindow& window )
{
	UiToolboxButtonLabel button( window, "Open Popup" );
	ImUiTestPopupState* state = button.newState< ImUiTestPopupState >();

	if( button.end() )
	{
		state->isOpen = !state->isOpen;
	}

	if( state->isOpen )
	{
		const char* buttons[] =
		{
			"Ok",
			"Cancel"
		};

		UiToolboxPopup popup( window );

		popup.label( "Hello from a popup window!" );

		const size_t clickedButton = popup.end( buttons, IMUI_ARRAY_COUNT( buttons ) );
		if( clickedButton < IMUI_ARRAY_COUNT( buttons ) )
		{
			state->isOpen = false;
		}
	}
}

bool ImUiToolboxCppSampleInitialize( ImUiContext* imui )
{
	return ImUiFrameworkToolboxConfigDataInitialize( &s_toolboxCppContext.configData, imui );
}

void ImUiToolboxCppSampleShutdown( ImUiContext* imui )
{
	ImUiFrameworkToolboxConfigDataShutdown( &s_toolboxCppContext.configData, imui );
}
