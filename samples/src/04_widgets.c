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

typedef struct ImuiToolboxSampleContext
{
	float							sliderValue1;

#ifndef IMUI_NO_SAMPLE_FRAMEWORK
	ImuiFrameworkToolboxConfigData	configData;
#endif
} ImuiToolboxSampleContext;

static ImuiToolboxSampleContext s_toolboxContext =
{
	.sliderValue1 = 2.5f
};

static void			imuiToolboxSampleButtonsAndCheckBoxes( ImuiWindow* window, ImuiWidget* vLayout );
static void			imuiToolboxSampleSlidersAndProgressBars( ImuiWindow* window );
static void			imuiToolboxSampleTextEdit( ImuiWindow* window );
static void			imuiToolboxSampleDropDown( ImuiWindow* window );
static void			imuiToolboxSamplePopup( ImuiWindow* window );
static void			imuiToolboxSampleScrollAndList( ImuiWindow* window );
static void			imuiToolboxSampleTabView( ImuiWindow* window );
static void			imuiToolboxSampleTextView( ImuiWindow* window );

typedef struct ImuiTestPopupState ImuiTestPopupState;
struct ImuiTestPopupState
{
	bool isOpen;
};

void imuiToolboxSampleTick( ImuiWindow* window )
{
#ifndef IMUI_NO_SAMPLE_FRAMEWORK
	imuiFrameworkToolboxConfigDataApply( &s_toolboxContext.configData );
#endif

	ImuiWidget* hLayout = imuiWidgetBeginNamed( window, "hMain" );
	imuiWidgetSetStretchOne( hLayout );
	imuiWidgetSetMargin( hLayout, imuiBorderCreateAll( 25.0f ) );
	imuiWidgetSetLayoutHorizontalSpacing( hLayout, 10.0f );

	{
		ImuiWidget* vLayout = imuiWidgetBeginNamed( window, "vMain" );
		imuiWidgetSetHStretch( vLayout, 1.0f );
		imuiWidgetSetLayoutVerticalSpacing( vLayout, 10.0f );

		imuiToolboxSampleButtonsAndCheckBoxes( window, vLayout );
		imuiToolboxSampleSlidersAndProgressBars( window );
		imuiToolboxSampleTextEdit( window );
		imuiToolboxSampleDropDown( window );
		imuiToolboxSamplePopup( window );

		imuiWidgetEnd( vLayout );
	}

	{
		ImuiWidget* vLayout = imuiWidgetBeginNamed( window, "vLayout" );
		imuiWidgetSetHStretch( vLayout, 1.0f );
		imuiWidgetSetLayoutVerticalSpacing( vLayout, 10.0f );

		imuiToolboxSampleScrollAndList( window );
		imuiToolboxSampleTabView( window );
		imuiToolboxSampleTextView( window );

		imuiWidgetEnd( vLayout );
	}

	{
		const ImuiPos mousePos = imuiInputGetMousePos( imuiWindowGetInput( window ) );
		ImuiWidget* mouseLabel = imuiToolboxLabelBeginFormat( window, "X: %.0f\nY: %.0f", mousePos.x, mousePos.y );
		imuiWidgetSetFixedWidth( mouseLabel, 100.0f );
		imuiWidgetSetVAlign( mouseLabel, 0.0f );
		imuiToolboxLabelEnd( mouseLabel );
	}

	imuiWidgetEnd( hLayout );
}

static void imuiToolboxSampleButtonsAndCheckBoxes( ImuiWindow* window, ImuiWidget* vLayout )
{
	bool isNewState;
	bool* checked = (bool*)imuiWidgetAllocStateNew( vLayout, sizeof( bool ) * 3u, IMUI_ID_STR( "check" ), &isNewState);
	if( isNewState )
	{
		checked[ 1u ] = true;
	}

	{
		ImuiWidget* buttonsLayout = imuiWidgetBeginNamed( window, "buttons" );
		imuiWidgetSetLayoutHorizontalSpacing( buttonsLayout, 10.0f );

		if( imuiToolboxButtonLabel( window, "Button 1" ) )
		{
			checked[ 0u ] = !checked[ 0u ];
		}

		if( imuiToolboxButtonLabel( window, "Button 2" ) )
		{
			checked[ 1u ] = !checked[ 1u ];
		}

		if( imuiToolboxButtonLabel( window, "Button 3" ) )
		{
			checked[ 2u ] = !checked[ 2u ];
		}

		imuiWidgetEnd( buttonsLayout );
	}

	{
		ImuiWidget* checkLayout = imuiWidgetBeginNamed( window, "checks" );
		imuiWidgetSetHStretch( checkLayout, 1.0f );
		imuiWidgetSetLayoutVerticalSpacing( checkLayout, 10.0f );

		imuiToolboxCheckBox( window, &checked[ 0u ], "Check 1" );
		imuiToolboxCheckBox( window, &checked[ 1u ], "Check 2" );
		imuiToolboxCheckBox( window, &checked[ 2u ], "Check 3" );

		imuiToolboxCheckBoxState( window, "Check State" );

		imuiToolboxLabelFormat( window, "C1: %d, C2: %d, C3: %d", checked[ 0u ], checked[ 1u ], checked[ 2u ] );

		imuiWidgetEnd( checkLayout );
	}
}

static void imuiToolboxSampleSlidersAndProgressBars( ImuiWindow* window )
{
	ImuiWidget* sliderLayout = imuiWidgetBeginNamed( window, "sliders" );
	imuiWidgetSetHStretch( sliderLayout, 1.0f );
	imuiWidgetSetLayoutVerticalSpacing( sliderLayout, 10.0f );

	imuiToolboxSliderMinMax( window, &s_toolboxContext.sliderValue1, 1.0f, 5.0f );

	const float value2 = imuiToolboxSliderStateMinMax( window, 1.0f, 5.0f );

	imuiToolboxLabelFormat( window, "V1: %.2f, V2: %.2f", s_toolboxContext.sliderValue1, value2 );

	imuiToolboxProgressBarMinMax( window, s_toolboxContext.sliderValue1, 1.0f, 5.0f );
	imuiToolboxProgressBar( window, -1.0f );

	imuiWidgetEnd( sliderLayout );
}

static void imuiToolboxSampleTextEdit( ImuiWindow* window )
{
	imuiToolboxTextEditStateBuffer( window, 128u );
}

static void imuiToolboxSampleScrollAndList( ImuiWindow* window )
{
	imuiToolboxLabel( window, "Item count:" );
	const float itemCount = imuiToolboxSliderStateMinMaxDefault( window, 0.0f, 128.0f, 32.0f );

	const bool useList = imuiToolboxCheckBoxStateDefault( window, "List", false );

	const size_t count = (size_t)itemCount;
	if( useList )
	{
		ImuiToolboxListContext list;
		imuiToolboxListBegin( &list, window, 25.0f, count, true );
		imuiWidgetSetMinSizeFloat( list.list, 200.0f, 200.0f );

		for( size_t i = imuiToolboxListGetBeginIndex( &list ); i < imuiToolboxListGetEndIndex( &list ); ++i )
		{
			ImuiWidget* item = imuiToolboxListNextItem( &list );
			imuiWidgetSetPadding( item, imuiBorderCreateAll( 4.0f ) );

			ImuiWidget* label = imuiToolboxLabelBeginFormat( window, "List Label %i", i );
			imuiWidgetSetVAlign( label, 0.5f );
			imuiToolboxLabelEnd( label );
		}

		imuiToolboxListEnd( &list );
	}
	else
	{
		ImuiToolboxScrollAreaContext scrollArea;
		imuiToolboxScrollAreaBegin( &scrollArea, window );
		imuiWidgetSetMinSizeFloat( scrollArea.area, 200.0f, 200.0f );

		ImuiWidget* scrollLayout = imuiWidgetBeginNamed( window, "scroll" );
		imuiWidgetSetHStretch( scrollLayout, 1.0f );
		imuiWidgetSetLayoutVerticalSpacing( scrollLayout, 4.0f );

		size_t scrollIndices[ 3 ];
		ImuiWidget* scrollWidgets[ 3 ];
		scrollIndices[ 0 ] = 0;
		scrollIndices[ 1 ] = count / 2;
		scrollIndices[ 2 ] = count - 1;

		for( size_t i = 0; i < count; ++i )
		{
			ImuiWidget* itemWidget = imuiToolboxLabelBeginFormat( window, "Scroll Label %i", i );
			imuiToolboxLabelEnd( itemWidget );

			for( size_t j = 0; j < IMUI_ARRAY_COUNT( scrollIndices ); ++j )
			{
				if( i == scrollIndices[ j ] )
				{
					scrollWidgets[ j ] = itemWidget;
				}
			}
		}

		imuiWidgetEnd( scrollLayout );
		imuiToolboxScrollAreaEnd( &scrollArea );

		if( count > 0 )
		{
			ImuiWidget* scrollToLayout = imuiWidgetBegin( window );
			imuiWidgetSetHStretch( scrollToLayout, 1.0f );
			imuiWidgetSetLayoutHorizontalSpacing( scrollToLayout, 4.0f );

			for( size_t i = 0; i < IMUI_ARRAY_COUNT( scrollWidgets ); ++i )
			{
				if( imuiToolboxButtonLabelFormat( window, "Scroll to %d", scrollIndices[ i ] ) )
				{
					imuiToolboxScrollAreaOffsetTo( &scrollArea, scrollWidgets[ i ] );
				}
			}

			imuiWidgetEnd( scrollToLayout );
		}
	}
}

static void imuiToolboxSampleDropDown( ImuiWindow* window )
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

	imuiToolboxDropDown( window, items, IMUI_ARRAY_COUNT( items ), sizeof( const char* ) );
}

static void imuiToolboxSamplePopup( ImuiWindow* window )
{
	ImuiWidget* button = imuiToolboxButtonLabelBegin( window, "Open Popup" );

	ImuiTestPopupState* state = (ImuiTestPopupState*)imuiWidgetAllocState( button, sizeof( *state ), IMUI_ID_TYPE( ImuiTestPopupState ) );

	if( imuiToolboxButtonEnd( button ) )
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

		ImuiWindow* popup = imuiToolboxPopupBegin( window );

		imuiToolboxLabel( popup, "Hello from a popup window!" );

		const size_t clickedButton = imuiToolboxPopupEndButtons( popup, buttons, IMUI_ARRAY_COUNT( buttons ) );
		if( clickedButton < IMUI_ARRAY_COUNT( buttons ) )
		{
			state->isOpen = false;
		}
	}
}

static void imuiToolboxSampleTabView( ImuiWindow* window )
{
	ImuiToolboxTabViewContext tabView;
	imuiToolboxTabViewBegin( &tabView, window );
	imuiWidgetSetHStretch( tabView.view, 1.0f );

	char buffer[ 32u ];

	size_t selectedTab = 0u;
	for( size_t i = 0u; i < 4u; ++i )
	{
		snprintf( buffer, sizeof( buffer ), "Tab %zu", i + 1u );

		if( imuiToolboxTabViewHeader( &tabView, buffer ) )
		{
			selectedTab = i;
		}
	}

	imuiToolboxTabViewBodyBegin( &tabView );

	snprintf( buffer, sizeof( buffer ), "Hello from Tab %zu", selectedTab + 1u );
	imuiToolboxLabel( window, buffer );

	imuiToolboxTabViewBodyEnd( &tabView );
	imuiToolboxTabViewEnd( &tabView );
}

static void imuiToolboxSampleTextView( ImuiWindow* window )
{
	ImuiToolboxTextBuffer* textBuffer = imuiToolboxTextBufferCreateText( imuiWindowGetContext( window ), "Hello\nWorld\nLine 3\nLine 4\nLine 5\nLine 6\n" );

	imuiToolboxTextBufferAppend( textBuffer, "Line 7" );
	imuiToolboxTextBufferAppend( textBuffer, " and more for 7\n" );
	imuiToolboxTextBufferAppend( textBuffer, "Line 8\n\nLine 10 after empty Line" );
	imuiToolboxTextBufferAppend( textBuffer, "\nLine 11\nLine 12\n" );
	imuiToolboxTextBufferAppend( textBuffer, "Line 13\nLine 14" );

	ImuiToolboxTextViewContext textViewContext;
	ImuiWidget* textView = imuiToolboxTextViewBeginBuffer( &textViewContext, window, textBuffer );

	imuiWidgetSetFixedWidth( textView, 250.0f );
	imuiWidgetSetFixedHeight( textView, 150.0f );

	imuiToolboxTextViewEnd( &textViewContext );


	imuiToolboxTextBufferFree( textBuffer );
}

#ifndef IMUI_NO_SAMPLE_FRAMEWORK
bool imuiToolboxSampleInitialize( ImuiContext* imui )
{
	return imuiFrameworkToolboxConfigDataInitialize( &s_toolboxContext.configData, imui );
}

void imuiToolboxSampleShutdown( ImuiContext* imui )
{
	imuiFrameworkToolboxConfigDataShutdown( &s_toolboxContext.configData, imui );
}
#endif
