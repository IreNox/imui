#include "../../framework/framework.h"

#include "imui/imui.h"

#include <stdlib.h>
#include <stdio.h>

static void HwMinSizeHorizontal( ImUiWindow* window );
static void HwMinSizeVertical( ImUiWindow* window );
static void HwMinSizeElement( ImUiWindow* window );

static void HwStretchStack( ImUiWindow* window );
static void HwStretchHorizontal( ImUiWindow* window );
static void HwStretchVertical( ImUiWindow* window );
static void HwStretchElements( ImUiWindow* window, ImUiSize stretch1, ImUiSize stretch2, ImUiSize stretch3 );

void ImUiFrameworkTick( ImUiSurface* surface )
{
	ImUiContext* imui = ImUiSurfaceGetContext( surface );

	const ImUiSize surfaceSize = ImUiSurfaceGetSize( surface );
	ImUiWindow* window = ImUiWindowBegin( surface, ImUiStringViewCreate( "main" ), ImUiRectangleCreate( 0.0f, 0.0f, surfaceSize.width, surfaceSize.height ), 1 );

	ImUiWidget* hLayout = ImUiWidgetBeginNamed( window, ImUiStringViewCreate( "hMain" ) );
	ImUiWidgetSetMargin( hLayout, ImUiThicknessCreateAll( 50.0f ) );
	ImUiWidgetSetStretch( hLayout, ImUiSizeCreateOne() );
	ImUiWidgetSetLayoutHorizontalSpacing( hLayout, 50.0f );

	ImUiWidget* vLayout = ImUiWidgetBeginNamed( window, ImUiStringViewCreate( "vMain" ) );
	ImUiWidgetSetStretch( vLayout, ImUiSizeCreateOne() );
	ImUiWidgetSetLayoutVerticalSpacing( vLayout, 50.0f );

	HwMinSizeHorizontal( window );
	HwStretchStack( window );
	HwStretchHorizontal( window );

	ImUiWidgetEnd( vLayout );

	HwStretchVertical( window );
	HwMinSizeVertical( window );

	ImUiWidgetEnd( hLayout );

	ImUiWindowEnd( window );
}

static void HwMinSizeHorizontal( ImUiWindow* window )
{
	ImUiWidget* layout = ImUiWidgetBeginNamed( window, ImUiStringViewCreate( "min_horizontal" ) );
	ImUiWidgetSetPadding( layout, ImUiThicknessCreateAll( 20.0f ) );
	ImUiWidgetSetLayoutHorizontalSpacing( layout, 10.0f );

	ImUiDrawWidgetColor( layout, ImUiColorCreateWhite( 1.0f ) );

	HwMinSizeElement( window );
	HwMinSizeElement( window );
	HwMinSizeElement( window );

	ImUiWidgetEnd( layout );
}

static void HwMinSizeVertical( ImUiWindow* window )
{
	ImUiWidget* layout = ImUiWidgetBeginNamed( window, ImUiStringViewCreate( "min_vertical" ) );
	ImUiWidgetSetPadding( layout, ImUiThicknessCreateAll( 20.0f ) );
	ImUiWidgetSetLayoutVerticalSpacing( layout, 10.0f );

	ImUiDrawWidgetColor( layout, ImUiColorCreateWhite( 1.0f ) );

	HwMinSizeElement( window );
	HwMinSizeElement( window );
	HwMinSizeElement( window );

	ImUiWidgetEnd( layout );
}

static void HwMinSizeElement( ImUiWindow* window )
{
	ImUiWidget* widget = ImUiWidgetBegin( window );
	ImUiWidgetSetMargin( widget, ImUiThicknessCreateAll( 10.0f ) );
	ImUiWidgetSetFixedSize( widget, ImUiSizeCreateAll( 20.0f ) );

	ImUiDrawWidgetColor( widget, ImUiColorCreate( 0.0f, 1.0f, 1.0f, 1.0f ) );

	ImUiWidgetEnd( widget );
}

static void HwStretchStack( ImUiWindow* window )
{
	ImUiWidget* layout = ImUiWidgetBeginNamed( window, ImUiStringViewCreate( "stack" ) );
	ImUiWidgetSetPadding( layout, ImUiThicknessCreateAll( 20.0f ) );
	ImUiWidgetSetStretch( layout, ImUiSizeCreateOne() );

	ImUiDrawWidgetColor( layout, ImUiColorCreateWhite( 1.0f ) );

	{
		ImUiWidget* widget2 = ImUiWidgetBegin( window );
		ImUiWidgetSetMargin( widget2, ImUiThicknessCreateAll( 10.0f ) );
		ImUiWidgetSetStretch( widget2, ImUiSizeCreate( 0.5f, 0.5f ) );

		ImUiDrawWidgetColor( widget2, ImUiColorCreate( 0.0f, 0.0f, 1.0f, 1.0f ) );

		ImUiWidgetEnd( widget2 );
	}

	{
		ImUiWidget* widget2 = ImUiWidgetBegin( window );
		ImUiWidgetSetMargin( widget2, ImUiThicknessCreateAll( 10.0f ) );
		ImUiWidgetSetStretch( widget2, ImUiSizeCreate( 0.5f, 0.5f ) );
		ImUiWidgetSetAlignment( widget2, ImUiAlignmentCreate( ImUiHorizintalAlignment_Right, ImUiVerticalAlignment_Bottom ) );

		ImUiDrawWidgetColor( widget2, ImUiColorCreate( 0.0f, 1.0f, 0.0f, 1.0f ) );

		ImUiWidgetEnd( widget2 );
	}

	{
		ImUiWidget* widget2 = ImUiWidgetBegin( window );
		ImUiWidgetSetMargin( widget2, ImUiThicknessCreateAll( 10.0f ) );
		ImUiWidgetSetStretch( widget2, ImUiSizeCreate( 0.5f, 0.5f ) );
		ImUiWidgetSetAlignment( widget2, ImUiAlignmentCreate( ImUiHorizintalAlignment_Center, ImUiVerticalAlignment_Center ) );

		ImUiDrawWidgetColor( widget2, ImUiColorCreate( 1.0f, 0.0f, 0.0f, 1.0f ) );

		ImUiWidgetEnd( widget2 );
	}

	ImUiWidgetEnd( layout );
}

static void HwStretchHorizontal( ImUiWindow* window )
{
	ImUiWidget* layout = ImUiWidgetBeginNamed( window, ImUiStringViewCreate( "horizontal" ) );
	ImUiWidgetSetPadding( layout, ImUiThicknessCreateAll( 20.0f ) );
	ImUiWidgetSetStretch( layout, ImUiSizeCreateOne() );
	ImUiWidgetSetLayoutHorizontalSpacing( layout, 10.0f );

	ImUiDrawWidgetColor( layout, ImUiColorCreateWhite( 1.0f ) );

	HwStretchElements( window, ImUiSizeCreate( 1.0f, 1.0f ), ImUiSizeCreate( 2.0f, 2.0f ), ImUiSizeCreate( 1.0f, 1.0f ) );

	ImUiWidgetEnd( layout );
}

static void HwStretchVertical( ImUiWindow* window )
{
	ImUiWidget* layout = ImUiWidgetBeginNamed( window, ImUiStringViewCreate( "vertical" ) );
	ImUiWidgetSetPadding( layout, ImUiThicknessCreateAll( 20.0f ) );
	ImUiWidgetSetStretch( layout, ImUiSizeCreateOne() );
	ImUiWidgetSetLayoutVerticalSpacing( layout, 10.0f );

	ImUiDrawWidgetColor( layout, ImUiColorCreateWhite( 1.0f ) );

	HwStretchElements( window, ImUiSizeCreate( 1.0f, 1.0f ), ImUiSizeCreate( 2.0f, 2.0f ), ImUiSizeCreate( 1.0f, 1.0f ) );

	ImUiWidgetEnd( layout );
}

static void HwStretchElements( ImUiWindow* window, ImUiSize stretch1, ImUiSize stretch2, ImUiSize stretch3 )
{
	{
		ImUiWidget* widget2 = ImUiWidgetBegin( window );
		ImUiWidgetSetMargin( widget2, ImUiThicknessCreateAll( 10.0f ) );
		ImUiWidgetSetStretch( widget2, stretch1 );

		ImUiDrawWidgetColor( widget2, ImUiColorCreate( 0.0f, 0.0f, 1.0f, 1.0f ) );

		ImUiWidgetEnd( widget2 );
	}

	{
		ImUiWidget* widget2 = ImUiWidgetBegin( window );
		ImUiWidgetSetMargin( widget2, ImUiThicknessCreateAll( 10.0f ) );
		ImUiWidgetSetStretch( widget2, stretch2 );

		ImUiDrawWidgetColor( widget2, ImUiColorCreate( 1.0f, 0.0f, 0.0f, 1.0f ) );

		ImUiWidgetEnd( widget2 );
	}

	{
		ImUiWidget* widget2 = ImUiWidgetBegin( window );
		ImUiWidgetSetMargin( widget2, ImUiThicknessCreateAll( 10.0f ) );
		ImUiWidgetSetStretch( widget2, stretch3 );
		ImUiWidgetSetVerticalAlignment( widget2, ImUiVerticalAlignment_Center );
		ImUiWidgetSetHorizintalAlignment( widget2, ImUiHorizintalAlignment_Center );

		ImUiDrawWidgetColor( widget2, ImUiColorCreate( 0.0f, 1.0f, 0.0f, 1.0f ) );

		ImUiWidgetEnd( widget2 );
	}
}

bool ImUiFrameworkInitialize( ImUiContext* imui )
{
	return true;
}

void ImUiFrameworkShutdown( ImUiContext* imui )
{
}
