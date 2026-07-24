#include "00_samples.h"

#include "imui/imui.h"
#include "imui/imui_toolbox.h"

#include "framework.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

static void imuiLayoutSampleMinSizeHorizontal( ImuiWindow* window );
static void imuiLayoutSampleMinSizeVertical( ImuiWindow* window );
static void imuiLayoutSampleMinSizeElement( ImuiWindow* window );

static void imuiLayoutSampleStretchStack( ImuiWindow* window );
static void imuiLayoutSampleStretchHorizontal( ImuiWindow* window );
static void imuiLayoutSampleStretchVertical( ImuiWindow* window );
static void imuiLayoutSampleStretchGrid( ImuiWindow* window );
static void imuiLayoutSampleStretchElements( ImuiWindow* window, ImuiSize stretch1, ImuiSize stretch2, ImuiSize stretch3 );

static void imuiLayoutSampleFocus( ImuiWidget* widget );

static ImuiFrameworkToolboxConfigData s_configData;

void imuiLayoutSampleTick( ImuiWindow* window )
{
	ImuiContext* imui = imuiWindowGetContext( window );
	imuiWindowSetFocus( window, 0.5f, false );

	ImuiPos focusPoint = imuiPosCreateZero();
	const ImuiWidget* focusWidget = imuiWindowGetFocusWidget( window );
	if( focusWidget )
	{
		focusPoint = imuiRectGetCenter( imuiWidgetGetRect( focusWidget ) );
	}

	ImuiWidget* vMain = imuiWidgetBeginNamed( window, "vMain" );
	imuiWidgetSetStretchOne( vMain );
	imuiWidgetSetLayoutVerticalSpacing( vMain, 0.0f );

	{
		ImuiWidget* hLayout = imuiWidgetBeginNamed( window, "hMain" );
		imuiWidgetSetMargin( hLayout, imuiBorderCreateAll( 50.0f ) );
		imuiWidgetSetStretch( hLayout, 1.0f, 2.0f );
		imuiWidgetSetLayoutHorizontalSpacing( hLayout, 50.0f );

		{
			ImuiWidget* vLayout = imuiWidgetBeginNamed( window, "vLeft" );
			imuiWidgetSetStretchOne( vLayout );
			imuiWidgetSetLayoutVerticalSpacing( vLayout, 50.0f );

			imuiLayoutSampleMinSizeHorizontal( window );
			imuiLayoutSampleStretchStack( window );
			imuiLayoutSampleStretchHorizontal( window );

			imuiWidgetEnd( vLayout );
		}

		imuiLayoutSampleStretchVertical( window );
		imuiLayoutSampleMinSizeVertical( window );

		imuiWidgetEnd( hLayout );
	}

	imuiLayoutSampleStretchGrid( window );

	if( focusWidget )
	{
		const ImuiRect focusRect = imuiRectShrinkBorder( imuiWidgetGetRect( focusWidget ), imuiBorderCreateAll( 4.0f ) );
		const float gray = (sinf( (float)imuiWidgetGetTime( vMain ) ) / 2.0f + 0.5f) * 255.0f;
		imuiWidgetDrawPartialColor( vMain, focusRect, imuiColorCreateGray( (uint8_t)gray ) );
	}
	const ImuiWidget* peekFocusWidget = imuiWindowPeekFocusWidget( window );
	if( peekFocusWidget )
	{
		const ImuiRect focusRect = imuiRectShrinkBorder( imuiWidgetGetRect( peekFocusWidget ), imuiBorderCreateAll( 4.0f ) );
		const float gray = (cosf( (float)imuiWidgetGetTime( vMain ) ) / 2.0f + 0.5f) * 255.0f;
		imuiWidgetDrawPartialColor( vMain, focusRect, imuiColorCreateGray( (uint8_t)gray ) );
	}
	const ImuiPos p1 = imuiPosAddPos( focusPoint, imuiPosScale( imuiInputGetDirection( imuiWidgetGetInput( vMain ) ), 10000.0f ) );
	imuiWidgetDrawLine( vMain, focusPoint, p1, imuiColorCreateBlack() );

	imuiWidgetEnd( vMain );
}

static void imuiLayoutSampleMinSizeHorizontal( ImuiWindow* window )
{
	ImuiWidget* layout = imuiWidgetBeginNamed( window, "min_horizontal" );
	imuiWidgetSetPadding( layout, imuiBorderCreateAll( 20.0f ) );
	imuiWidgetSetLayoutHorizontalSpacing( layout, 10.0f );

	imuiWidgetDrawColor( layout, imuiColorCreateWhite() );

	imuiLayoutSampleMinSizeElement( window );
	imuiLayoutSampleMinSizeElement( window );
	imuiLayoutSampleMinSizeElement( window );

	imuiWidgetEnd( layout );
}

static void imuiLayoutSampleMinSizeVertical( ImuiWindow* window )
{
	ImuiWidget* layout = imuiWidgetBeginNamed( window, "min_vertical" );
	imuiWidgetSetPadding( layout, imuiBorderCreateAll( 20.0f ) );
	imuiWidgetSetLayoutVerticalSpacing( layout, 10.0f );

	imuiWidgetDrawColor( layout, imuiColorCreateWhite() );

	imuiLayoutSampleMinSizeElement( window );
	imuiLayoutSampleMinSizeElement( window );
	imuiLayoutSampleMinSizeElement( window );

	imuiWidgetEnd( layout );
}

static void imuiLayoutSampleMinSizeElement( ImuiWindow* window )
{
	ImuiWidget* widget = imuiWidgetBegin( window );
	imuiWidgetSetMargin( widget, imuiBorderCreateAll( 10.0f ) );
	imuiWidgetSetFixedSize( widget, imuiSizeCreateAll( 20.0f ) );
	imuiLayoutSampleFocus( widget );

	imuiWidgetDrawColor( widget, imuiColorCreate( 0u, 0xffu, 0xffu, 0xffu ) );

	imuiWidgetEnd( widget );
}

static void imuiLayoutSampleStretchStack( ImuiWindow* window )
{
	ImuiWidget* layout = imuiWidgetBeginNamed( window, "stack" );
	imuiWidgetSetPadding( layout, imuiBorderCreateAll( 20.0f ) );
	imuiWidgetSetStretchOne( layout );

	imuiWidgetDrawColor( layout, imuiColorCreateWhite() );

	{
		ImuiWidget* widget2 = imuiWidgetBegin( window );
		imuiWidgetSetMargin( widget2, imuiBorderCreateAll( 10.0f ) );
		imuiWidgetSetStretch( widget2, 0.5f, 0.5f );
		imuiLayoutSampleFocus( widget2 );

		imuiWidgetDrawColor( widget2, imuiColorCreate( 0u, 0u, 0xffu, 0xffu ) );

		imuiWidgetEnd( widget2 );
	}

	{
		ImuiWidget* widget2 = imuiWidgetBegin( window );
		imuiWidgetSetMargin( widget2, imuiBorderCreateAll( 10.0f ) );
		imuiWidgetSetStretch( widget2, 0.5f, 0.5f );
		imuiWidgetSetAlign( widget2, 1.0f, 1.0f );
		imuiLayoutSampleFocus( widget2 );

		imuiWidgetDrawColor( widget2, imuiColorCreate( 0u, 0xffu, 0u, 0xffu ) );

		imuiWidgetEnd( widget2 );
	}

	{
		ImuiWidget* widget2 = imuiWidgetBegin( window );
		imuiWidgetSetMargin( widget2, imuiBorderCreateAll( 10.0f ) );
		imuiWidgetSetStretch( widget2, 0.5f, 0.5f );
		imuiWidgetSetAlign( widget2, 0.5f, 0.5f );
		imuiLayoutSampleFocus( widget2 );

		imuiWidgetDrawColor( widget2, imuiColorCreate( 0xffu, 0u, 0u, 0xffu ) );

		imuiWidgetEnd( widget2 );
	}

	imuiWidgetEnd( layout );
}

static void imuiLayoutSampleStretchHorizontal( ImuiWindow* window )
{
	ImuiWidget* layout = imuiWidgetBeginNamed( window, "horizontal" );
	imuiWidgetSetPadding( layout, imuiBorderCreateAll( 20.0f ) );
	imuiWidgetSetStretchOne( layout );
	imuiWidgetSetLayoutHorizontalSpacing( layout, 10.0f );

	imuiWidgetDrawColor( layout, imuiColorCreateWhite() );

	const ImuiSize stretches[] =
	{
		{ 0.5f, 1.0f },
		{ 2.0f, 2.0f },
		{ 1.0f, 1.0f }
	};

	const ImuiSize childSizes[] =
	{
		{ 100.0f, 20.0f },
		{ 20.0f, 20.0f },
		{ 75.0f, 20.0f }
	};

	const ImuiColor colors[] =
	{
		{ 0u, 0u, 0xffu, 0xffu },
		{ 0xffu, 0u, 0u, 0xffu },
		{ 0u, 0xffu, 0u, 0xffu }
	};

	const char* names[] =
	{
		"1blue",
		"2red",
		"3green"
	};

	for( size_t i = 0; i < 3u; ++i )
	{
		ImuiWidget* widget = imuiWidgetBeginNamed( window, names[ i ] );
		imuiWidgetSetMargin( widget, imuiBorderCreateAll( 10.0f ) );
		imuiWidgetSetStretch( widget, stretches[ i ].width, stretches[ i ].height);
		imuiLayoutSampleFocus( widget );

		if( i == 2 )
		{
			imuiWidgetSetAlign( widget, 0.5f, 0.5f );
		}

		imuiWidgetDrawColor( widget, colors[ i ] );

		ImuiWidget* childWidget = imuiWidgetBegin( window );
		imuiWidgetSetAlign( childWidget, 0.5f, 0.5f );
		imuiWidgetSetFixedSize( childWidget, childSizes[ i ] );
		imuiWidgetDrawColor( childWidget, colors[ (i + 1) % 3 ] );
		imuiWidgetEnd( childWidget );

		imuiWidgetEnd( widget );
	}

	imuiWidgetEnd( layout );
}

static void imuiLayoutSampleStretchVertical( ImuiWindow* window )
{
	ImuiWidget* layout = imuiWidgetBeginNamed( window, "vertical" );
	imuiWidgetSetPadding( layout, imuiBorderCreateAll( 20.0f ) );
	imuiWidgetSetStretchOne( layout );
	imuiWidgetSetLayoutVerticalSpacing( layout, 10.0f );

	imuiWidgetDrawColor( layout, imuiColorCreateWhite() );

	imuiLayoutSampleStretchElements( window, imuiSizeCreate( 1.0f, 0.5f ), imuiSizeCreate( 2.0f, 2.0f ), imuiSizeCreate( 1.0f, 1.0f ) );

	imuiWidgetEnd( layout );
}

static void imuiLayoutSampleStretchGrid( ImuiWindow* window )
{
	ImuiWidget* layout = imuiWidgetBeginNamed( window, "grid" );
	imuiWidgetSetPadding( layout, imuiBorderCreateAll( 20.0f ) );
	imuiWidgetSetStretchOne( layout );
	imuiWidgetSetLayoutGrid( layout, 4u, 10.0f, 20.0f );

	imuiWidgetDrawColor( layout, imuiColorCreateWhite() );

	imuiLayoutSampleStretchElements( window, imuiSizeCreate( 1.0f, 0.5f ), imuiSizeCreate( 2.0f, 2.0f ), imuiSizeCreate( 1.0f, 1.0f ) );
	imuiLayoutSampleStretchElements( window, imuiSizeCreate( 1.0f, 0.5f ), imuiSizeCreate( 2.0f, 2.0f ), imuiSizeCreate( 1.0f, 1.0f ) );
	imuiLayoutSampleStretchElements( window, imuiSizeCreate( 1.0f, 0.5f ), imuiSizeCreate( 2.0f, 2.0f ), imuiSizeCreate( 1.0f, 1.0f ) );
	imuiLayoutSampleStretchElements( window, imuiSizeCreate( 1.0f, 0.5f ), imuiSizeCreate( 2.0f, 2.0f ), imuiSizeCreate( 1.0f, 1.0f ) );

	imuiWidgetEnd( layout );
}

static void imuiLayoutSampleStretchElements( ImuiWindow* window, ImuiSize stretch1, ImuiSize stretch2, ImuiSize stretch3 )
{
	{
		ImuiWidget* widget2 = imuiWidgetBegin( window );
		imuiWidgetSetMargin( widget2, imuiBorderCreateAll( 10.0f ) );
		imuiWidgetSetStretch( widget2, stretch1.width, stretch1.height );
		imuiLayoutSampleFocus( widget2 );

		imuiWidgetDrawColor( widget2, imuiColorCreate( 0u, 0u, 0xffu, 0xffu ) );

		imuiWidgetEnd( widget2 );
	}

	{
		ImuiWidget* widget2 = imuiWidgetBegin( window );
		imuiWidgetSetMargin( widget2, imuiBorderCreateAll( 10.0f ) );
		imuiWidgetSetStretch( widget2, stretch2.width, stretch2.height );
		imuiLayoutSampleFocus( widget2 );

		imuiWidgetDrawColor( widget2, imuiColorCreate( 0xffu, 0u, 0u, 0xffu ) );

		imuiWidgetEnd( widget2 );
	}

	{
		ImuiWidget* widget2 = imuiWidgetBegin( window );
		imuiWidgetSetMargin( widget2, imuiBorderCreateAll( 10.0f ) );
		imuiWidgetSetStretch( widget2, stretch3.width, stretch3.height );
		imuiWidgetSetAlign( widget2, 0.5f, 0.5f );
		imuiLayoutSampleFocus( widget2 );

		imuiWidgetDrawColor( widget2, imuiColorCreate( 0u, 0xffu, 0u, 0xffu ) );

		imuiWidgetEnd( widget2 );
	}
}

static void imuiLayoutSampleFocus( ImuiWidget* widget )
{
	imuiWidgetSetCanHaveFocus( widget );
}

bool imuiLayoutSampleInitialize( ImuiContext* imui )
{
	return imuiFrameworkToolboxConfigDataInitialize( &s_configData, imui );
}

void imuiLayoutSampleShutdown( ImuiContext* imui )
{
	imuiFrameworkToolboxConfigDataShutdown( &s_configData, imui );
}
