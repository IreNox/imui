#include "00_samples.h"

#ifndef IMUI_NO_SAMPLE_FRAMEWORK
#	include "framework.h"
#endif

#include "imui/imui.h"
#include "imui/imui_toolbox.h"

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define IMUI_ARRAY_COUNT( arr ) (sizeof( arr ) / sizeof( *(arr) ))

typedef struct ImUiToolboxSampleContext
{
	float							sliderValue1;

#ifndef IMUI_NO_SAMPLE_FRAMEWORK
	ImUiFrameworkToolboxConfigData	configData;
#endif
} ImUiToolboxSampleContext;

static ImUiToolboxSampleContext s_toolboxContext =
{
	.sliderValue1 = 2.5f
};

static void			ImUiToolboxSampleButtonsAndCheckBoxes( ImUiWindow* window, ImUiWidget* vLayout );
static void			ImUiToolboxSampleSlidersAndProgressBars( ImUiWindow* window );
static void			ImUiToolboxSampleTextEdit( ImUiWindow* window );
static void			ImUiToolboxSampleDropDown( ImUiWindow* window );
static void			ImUiToolboxSamplePopup( ImUiWindow* window );
static void			ImUiToolboxSampleScrollAndList( ImUiWindow* window );
static void			ImUiToolboxSampleTabView( ImUiWindow* window );
static void			ImUiToolboxSampleTextView( ImUiWindow* window );

typedef struct ImUiTestPopupState ImUiTestPopupState;
struct ImUiTestPopupState
{
	bool isOpen;
};

void ImUiToolboxSampleTick( ImUiWindow* window )
{
#ifndef IMUI_NO_SAMPLE_FRAMEWORK
	ImUiFrameworkToolboxConfigDataApply( &s_toolboxContext.configData );
#endif

	ImUiWidget* hLayout = ImUiWidgetBeginNamed( window, "hMain" );
	ImUiWidgetSetStretchOne( hLayout );
	ImUiWidgetSetMargin( hLayout, ImUiBorderCreateAll( 25.0f ) );
	ImUiWidgetSetLayoutHorizontalSpacing( hLayout, 10.0f );

	{
		ImUiWidget* vLayout = ImUiWidgetBeginNamed( window, "vMain" );
		ImUiWidgetSetHStretch( vLayout, 1.0f );
		ImUiWidgetSetLayoutVerticalSpacing( vLayout, 10.0f );

		ImUiToolboxSampleButtonsAndCheckBoxes( window, vLayout );
		ImUiToolboxSampleSlidersAndProgressBars( window );
		ImUiToolboxSampleTextEdit( window );
		ImUiToolboxSampleDropDown( window );
		ImUiToolboxSamplePopup( window );

		ImUiWidgetEnd( vLayout );
	}

	{
		ImUiWidget* vLayout = ImUiWidgetBeginNamed( window, "vLayout" );
		ImUiWidgetSetHStretch( vLayout, 1.0f );
		ImUiWidgetSetLayoutVerticalSpacing( vLayout, 10.0f );

		ImUiToolboxSampleScrollAndList( window );
		ImUiToolboxSampleTabView( window );
		ImUiToolboxSampleTextView( window );

		ImUiWidgetEnd( vLayout );
	}

	{
		const ImUiPos mousePos = ImUiInputGetMousePos( ImUiWindowGetContext( window ) );
		ImUiWidget* mouseLabel = ImUiToolboxLabelBeginFormat( window, "X: %.0f\nY: %.0f", mousePos.x, mousePos.y );
		ImUiWidgetSetFixedWidth( mouseLabel, 100.0f );
		ImUiWidgetSetVAlign( mouseLabel, 0.0f );
		ImUiToolboxLabelEnd( mouseLabel );
	}

	ImUiWidgetEnd( hLayout );
}

static void ImUiToolboxSampleButtonsAndCheckBoxes( ImUiWindow* window, ImUiWidget* vLayout )
{
	bool isNewState;
	bool* checked = (bool*)ImUiWidgetAllocStateNew( vLayout, sizeof( bool ) * 3u, IMUI_ID_STR( "check" ), &isNewState);
	if( isNewState )
	{
		checked[ 1u ] = true;
	}

	{
		ImUiWidget* buttonsLayout = ImUiWidgetBeginNamed( window, "buttons" );
		ImUiWidgetSetLayoutHorizontalSpacing( buttonsLayout, 10.0f );

		if( ImUiToolboxButtonLabel( window, "Button 1" ) )
		{
			checked[ 0u ] = !checked[ 0u ];
		}

		if( ImUiToolboxButtonLabel( window, "Button 2" ) )
		{
			checked[ 1u ] = !checked[ 1u ];
		}

		if( ImUiToolboxButtonLabel( window, "Button 3" ) )
		{
			checked[ 2u ] = !checked[ 2u ];
		}

		ImUiWidgetEnd( buttonsLayout );
	}

	{
		ImUiWidget* checkLayout = ImUiWidgetBeginNamed( window, "checks" );
		ImUiWidgetSetHStretch( checkLayout, 1.0f );
		ImUiWidgetSetLayoutVerticalSpacing( checkLayout, 10.0f );

		ImUiToolboxCheckBox( window, &checked[ 0u ], "Check 1" );
		ImUiToolboxCheckBox( window, &checked[ 1u ], "Check 2" );
		ImUiToolboxCheckBox( window, &checked[ 2u ], "Check 3" );

		ImUiToolboxCheckBoxState( window, "Check State" );

		ImUiToolboxLabelFormat( window, "C1: %d, C2: %d, C3: %d", checked[ 0u ], checked[ 1u ], checked[ 2u ] );

		ImUiWidgetEnd( checkLayout );
	}
}

static void ImUiToolboxSampleSlidersAndProgressBars( ImUiWindow* window )
{
	ImUiWidget* sliderLayout = ImUiWidgetBeginNamed( window, "sliders" );
	ImUiWidgetSetHStretch( sliderLayout, 1.0f );
	ImUiWidgetSetLayoutVerticalSpacing( sliderLayout, 10.0f );

	ImUiToolboxSliderMinMax( window, &s_toolboxContext.sliderValue1, 1.0f, 5.0f );

	const float value2 = ImUiToolboxSliderStateMinMax( window, 1.0f, 5.0f );

	ImUiToolboxLabelFormat( window, "V1: %.2f, V2: %.2f", s_toolboxContext.sliderValue1, value2 );

	ImUiToolboxProgressBarMinMax( window, s_toolboxContext.sliderValue1, 1.0f, 5.0f );
	ImUiToolboxProgressBar( window, -1.0f );

	ImUiWidgetEnd( sliderLayout );
}

static void ImUiToolboxSampleTextEdit( ImUiWindow* window )
{
	ImUiToolboxTextEditStateBuffer( window, 128u );
}

static void ImUiToolboxSampleScrollAndList( ImUiWindow* window )
{
	ImUiToolboxLabel( window, "Item count:" );
	const float itemCount = ImUiToolboxSliderStateMinMaxDefault( window, 0.0f, 128.0f, 32.0f );

	const bool useList = ImUiToolboxCheckBoxStateDefault( window, "List", true );

	const size_t count = (size_t)itemCount;
	if( useList )
	{
		ImUiToolboxListContext list;
		ImUiToolboxListBegin( &list, window, 25.0f, count, true );
		ImUiWidgetSetMinSize( list.list, 200.0f, 200.0f );

		for( size_t i = ImUiToolboxListGetBeginIndex( &list ); i < ImUiToolboxListGetEndIndex( &list ); ++i )
		{
			ImUiWidget* item = ImUiToolboxListNextItem( &list );
			ImUiWidgetSetPadding( item, ImUiBorderCreateAll( 4.0f ) );

			ImUiWidget* label = ImUiToolboxLabelBeginFormat( window, "List Label %i", i );
			ImUiWidgetSetVAlign( label, 0.5f );
			ImUiToolboxLabelEnd( label );
		}

		ImUiToolboxListEnd( &list );
	}
	else
	{
		ImUiToolboxScrollAreaContext scrollArea;
		ImUiToolboxScrollAreaBegin( &scrollArea, window );
		ImUiWidgetSetMinSize( scrollArea.area, 200.0f, 200.0f );

		ImUiWidget* scrollLayout = ImUiWidgetBeginNamed( window, "scroll" );
		ImUiWidgetSetHStretch( scrollLayout, 1.0f );
		ImUiWidgetSetLayoutVerticalSpacing( scrollLayout, 4.0f );

		for( size_t i = 0; i < itemCount; ++i )
		{
			ImUiToolboxLabelFormat( window, "Scroll Label %i", i );
		}

		ImUiWidgetEnd( scrollLayout );
		ImUiToolboxScrollAreaEnd( &scrollArea );
	}
}

static void ImUiToolboxSampleDropDown( ImUiWindow* window )
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

	ImUiToolboxDropDown( window, items, IMUI_ARRAY_COUNT( items ), sizeof( const char* ) );
}

static void ImUiToolboxSamplePopup( ImUiWindow* window )
{
	ImUiWidget* button = ImUiToolboxButtonLabelBegin( window, "Open Popup" );

	ImUiTestPopupState* state = (ImUiTestPopupState*)ImUiWidgetAllocState( button, sizeof( *state ), IMUI_ID_TYPE( ImUiTestPopupState ) );

	if( ImUiToolboxButtonEnd( button ) )
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

		ImUiWindow* popup = ImUiToolboxPopupBegin( window );

		ImUiToolboxLabel( popup, "Hello from a popup window!" );

		const size_t clickedButton = ImUiToolboxPopupEndButtons( popup, buttons, IMUI_ARRAY_COUNT( buttons ) );
		if( clickedButton < IMUI_ARRAY_COUNT( buttons ) )
		{
			state->isOpen = false;
		}
	}
}

static void ImUiToolboxSampleTabView( ImUiWindow* window )
{
	ImUiToolboxTabViewContext tabView;
	ImUiToolboxTabViewBegin( &tabView, window );
	ImUiWidgetSetHStretch( tabView.view, 1.0f );

	char buffer[ 32u ];

	uintsize selectedTab = 0u;
	for( uintsize i = 0u; i < 4u; ++i )
	{
		snprintf( buffer, sizeof( buffer ), "Tab %zu", i + 1u );

		if( ImUiToolboxTabViewHeader( &tabView, buffer ) )
		{
			selectedTab = i;
		}
	}

	ImUiToolboxTabViewBodyBegin( &tabView );

	snprintf( buffer, sizeof( buffer ), "Hello from Tab %zu", selectedTab + 1u );
	ImUiToolboxLabel( window, buffer );

	ImUiToolboxTabViewBodyEnd( &tabView );
	ImUiToolboxTabViewEnd( &tabView );
}

static void ImUiToolboxSampleTextView( ImUiWindow* window )
{
	ImUiToolboxTextBuffer* textBuffer = ImUiToolboxTextBufferCreate( window, "Hello\nWorld\nLine 3\nLine 4\nLine 5\nLine 6\n" );

	ImUiToolboxTextBufferAppend( textBuffer, "Line 7" );
	ImUiToolboxTextBufferAppend( textBuffer, " and more for 7\n" );
	ImUiToolboxTextBufferAppend( textBuffer, "Line 8\n\nLine 10 after empty Line" );
	ImUiToolboxTextBufferAppend( textBuffer, "\nLine 11\nLine 12\n" );
	ImUiToolboxTextBufferAppend( textBuffer, "Line 13\nLine 14" );

	ImUiToolboxTextViewContext textViewContext;
	ImUiWidget* textView = ImUiToolboxTextViewBeginBuffer( &textViewContext, window, textBuffer );

	ImUiWidgetSetFixedWidth( textView, 250.0f );
	ImUiWidgetSetFixedHeight( textView, 150.0f );

	ImUiToolboxTextViewEnd( &textViewContext );


	ImUiToolboxTextBufferFree( textBuffer );
}

#ifndef IMUI_NO_SAMPLE_FRAMEWORK
bool ImUiToolboxSampleInitialize( ImUiContext* imui )
{
	return ImUiFrameworkToolboxConfigDataInitialize( &s_toolboxContext.configData, imui );
}

void ImUiToolboxSampleShutdown( ImUiContext* imui )
{
	ImUiFrameworkToolboxConfigDataShutdown( &s_toolboxContext.configData, imui );
}

#endif
