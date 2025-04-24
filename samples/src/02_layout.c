#include "00_samples.h"

#include "imui/imui.h"

#include <stdlib.h>
#include <stdio.h>

static void ImUiLayoutSampleMinSizeHorizontal( ImUiWindow* window );
static void ImUiLayoutSampleMinSizeVertical( ImUiWindow* window );
static void ImUiLayoutSampleMinSizeElement( ImUiWindow* window );

static void ImUiLayoutSampleStretchStack( ImUiWindow* window );
static void ImUiLayoutSampleStretchHorizontal( ImUiWindow* window );
static void ImUiLayoutSampleStretchVertical( ImUiWindow* window );
static void ImUiLayoutSampleStretchGrid( ImUiWindow* window );
static void ImUiLayoutSampleStretchElements( ImUiWindow* window, ImUiSize stretch1, ImUiSize stretch2, ImUiSize stretch3 );

void ImUiLayoutSampleTick( ImUiWindow* window )
{
	ImUiContext* imui = ImUiWindowGetContext( window );

	ImUiWidget* vMain = ImUiWidgetBeginNamed( window, "vMain" );
	ImUiWidgetSetStretchOne( vMain );
	ImUiWidgetSetLayoutVerticalSpacing( vMain, 0.0f );

	{
		ImUiWidget* hLayout = ImUiWidgetBeginNamed( window, "hMain" );
		ImUiWidgetSetMargin( hLayout, ImUiBorderCreateAll( 50.0f ) );
		ImUiWidgetSetStretchOne( hLayout );
		ImUiWidgetSetLayoutHorizontalSpacing( hLayout, 50.0f );

		{
			ImUiWidget* vLayout = ImUiWidgetBeginNamed( window, "vLeft" );
			ImUiWidgetSetStretchOne( vLayout );
			ImUiWidgetSetLayoutVerticalSpacing( vLayout, 50.0f );

			ImUiLayoutSampleMinSizeHorizontal( window );
			ImUiLayoutSampleStretchStack( window );
			ImUiLayoutSampleStretchHorizontal( window );

			ImUiWidgetEnd( vLayout );
		}

		ImUiLayoutSampleStretchVertical( window );
		ImUiLayoutSampleMinSizeVertical( window );

		ImUiWidgetEnd( hLayout );
	}

	ImUiLayoutSampleStretchGrid( window );

	ImUiWidgetEnd( vMain );
}

static void ImUiLayoutSampleMinSizeHorizontal( ImUiWindow* window )
{
	ImUiWidget* layout = ImUiWidgetBeginNamed( window, "min_horizontal" );
	ImUiWidgetSetPadding( layout, ImUiBorderCreateAll( 20.0f ) );
	ImUiWidgetSetLayoutHorizontalSpacing( layout, 10.0f );

	ImUiWidgetDrawColor( layout, ImUiColorCreateWhite() );

	ImUiLayoutSampleMinSizeElement( window );
	ImUiLayoutSampleMinSizeElement( window );
	ImUiLayoutSampleMinSizeElement( window );

	ImUiWidgetEnd( layout );
}

static void ImUiLayoutSampleMinSizeVertical( ImUiWindow* window )
{
	ImUiWidget* layout = ImUiWidgetBeginNamed( window, "min_vertical" );
	ImUiWidgetSetPadding( layout, ImUiBorderCreateAll( 20.0f ) );
	ImUiWidgetSetLayoutVerticalSpacing( layout, 10.0f );

	ImUiWidgetDrawColor( layout, ImUiColorCreateWhite() );

	ImUiLayoutSampleMinSizeElement( window );
	ImUiLayoutSampleMinSizeElement( window );
	ImUiLayoutSampleMinSizeElement( window );

	ImUiWidgetEnd( layout );
}

static void ImUiLayoutSampleMinSizeElement( ImUiWindow* window )
{
	ImUiWidget* widget = ImUiWidgetBegin( window );
	ImUiWidgetSetMargin( widget, ImUiBorderCreateAll( 10.0f ) );
	ImUiWidgetSetFixedSize( widget, ImUiSizeCreateAll( 20.0f ) );

	ImUiWidgetDrawColor( widget, ImUiColorCreate( 0u, 0xffu, 0xffu, 0xffu ) );

	ImUiWidgetEnd( widget );
}

static void ImUiLayoutSampleStretchStack( ImUiWindow* window )
{
	ImUiWidget* layout = ImUiWidgetBeginNamed( window, "stack" );
	ImUiWidgetSetPadding( layout, ImUiBorderCreateAll( 20.0f ) );
	ImUiWidgetSetStretchOne( layout );

	ImUiWidgetDrawColor( layout, ImUiColorCreateWhite() );

	{
		ImUiWidget* widget2 = ImUiWidgetBegin( window );
		ImUiWidgetSetMargin( widget2, ImUiBorderCreateAll( 10.0f ) );
		ImUiWidgetSetStretch( widget2, 0.5f, 0.5f );

		ImUiWidgetDrawColor( widget2, ImUiColorCreate( 0u, 0u, 0xffu, 0xffu ) );

		ImUiWidgetEnd( widget2 );
	}

	{
		ImUiWidget* widget2 = ImUiWidgetBegin( window );
		ImUiWidgetSetMargin( widget2, ImUiBorderCreateAll( 10.0f ) );
		ImUiWidgetSetStretch( widget2, 0.5f, 0.5f );
		ImUiWidgetSetAlign( widget2, 1.0f, 1.0f );

		ImUiWidgetDrawColor( widget2, ImUiColorCreate( 0u, 0xffu, 0u, 0xffu ) );

		ImUiWidgetEnd( widget2 );
	}

	{
		ImUiWidget* widget2 = ImUiWidgetBegin( window );
		ImUiWidgetSetMargin( widget2, ImUiBorderCreateAll( 10.0f ) );
		ImUiWidgetSetStretch( widget2, 0.5f, 0.5f );
		ImUiWidgetSetAlign( widget2, 0.5f, 0.5f );

		ImUiWidgetDrawColor( widget2, ImUiColorCreate( 0xffu, 0u, 0u, 0xffu ) );

		ImUiWidgetEnd( widget2 );
	}

	ImUiWidgetEnd( layout );
}

static void ImUiLayoutSampleStretchHorizontal( ImUiWindow* window )
{
	ImUiWidget* layout = ImUiWidgetBeginNamed( window, "horizontal" );
	ImUiWidgetSetPadding( layout, ImUiBorderCreateAll( 20.0f ) );
	ImUiWidgetSetStretchOne( layout );
	ImUiWidgetSetLayoutHorizontalSpacing( layout, 10.0f );

	ImUiWidgetDrawColor( layout, ImUiColorCreateWhite() );

	ImUiLayoutSampleStretchElements( window, ImUiSizeCreate( 0.5f, 1.0f ), ImUiSizeCreate( 2.0f, 2.0f ), ImUiSizeCreate( 1.0f, 1.0f ) );

	ImUiWidgetEnd( layout );
}

static void ImUiLayoutSampleStretchVertical( ImUiWindow* window )
{
	ImUiWidget* layout = ImUiWidgetBeginNamed( window, "vertical" );
	ImUiWidgetSetPadding( layout, ImUiBorderCreateAll( 20.0f ) );
	ImUiWidgetSetStretchOne( layout );
	ImUiWidgetSetLayoutVerticalSpacing( layout, 10.0f );

	ImUiWidgetDrawColor( layout, ImUiColorCreateWhite() );

	ImUiLayoutSampleStretchElements( window, ImUiSizeCreate( 1.0f, 0.5f ), ImUiSizeCreate( 2.0f, 2.0f ), ImUiSizeCreate( 1.0f, 1.0f ) );

	ImUiWidgetEnd( layout );
}

static void ImUiLayoutSampleStretchGrid( ImUiWindow* window )
{
	ImUiWidget* layout = ImUiWidgetBeginNamed( window, "grid" );
	ImUiWidgetSetPadding( layout, ImUiBorderCreateAll( 20.0f ) );
	ImUiWidgetSetStretchOne( layout );
	ImUiWidgetSetLayoutGrid( layout, 4u, 10.0f, 20.0f );

	ImUiWidgetDrawColor( layout, ImUiColorCreateWhite() );

	ImUiLayoutSampleStretchElements( window, ImUiSizeCreate( 1.0f, 0.5f ), ImUiSizeCreate( 2.0f, 2.0f ), ImUiSizeCreate( 1.0f, 1.0f ) );
	ImUiLayoutSampleStretchElements( window, ImUiSizeCreate( 1.0f, 0.5f ), ImUiSizeCreate( 2.0f, 2.0f ), ImUiSizeCreate( 1.0f, 1.0f ) );
	ImUiLayoutSampleStretchElements( window, ImUiSizeCreate( 1.0f, 0.5f ), ImUiSizeCreate( 2.0f, 2.0f ), ImUiSizeCreate( 1.0f, 1.0f ) );
	ImUiLayoutSampleStretchElements( window, ImUiSizeCreate( 1.0f, 0.5f ), ImUiSizeCreate( 2.0f, 2.0f ), ImUiSizeCreate( 1.0f, 1.0f ) );

	ImUiWidgetEnd( layout );
}

static void ImUiLayoutSampleStretchElements( ImUiWindow* window, ImUiSize stretch1, ImUiSize stretch2, ImUiSize stretch3 )
{
	{
		ImUiWidget* widget2 = ImUiWidgetBegin( window );
		ImUiWidgetSetMargin( widget2, ImUiBorderCreateAll( 10.0f ) );
		ImUiWidgetSetStretch( widget2, stretch1.width, stretch1.height );

		ImUiWidgetDrawColor( widget2, ImUiColorCreate( 0u, 0u, 0xffu, 0xffu ) );

		ImUiWidgetEnd( widget2 );
	}

	{
		ImUiWidget* widget2 = ImUiWidgetBegin( window );
		ImUiWidgetSetMargin( widget2, ImUiBorderCreateAll( 10.0f ) );
		ImUiWidgetSetStretch( widget2, stretch2.width, stretch2.height );

		ImUiWidgetDrawColor( widget2, ImUiColorCreate( 0xffu, 0u, 0u, 0xffu ) );

		ImUiWidgetEnd( widget2 );
	}

	{
		ImUiWidget* widget2 = ImUiWidgetBegin( window );
		ImUiWidgetSetMargin( widget2, ImUiBorderCreateAll( 10.0f ) );
		ImUiWidgetSetStretch( widget2, stretch3.width, stretch3.height );
		ImUiWidgetSetAlign( widget2, 0.5f, 0.5f );

		ImUiWidgetDrawColor( widget2, ImUiColorCreate( 0u, 0xffu, 0u, 0xffu ) );

		ImUiWidgetEnd( widget2 );
	}
}

bool ImUiLayoutSampleInitialize( ImUiContext* imui )
{
	return true;
}

void ImUiLayoutSampleShutdown( ImUiContext* imui )
{
}
