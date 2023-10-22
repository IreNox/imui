﻿#include "../../framework/framework.h"

#include "imui/imui.h"

#include <stdlib.h>
#include <stdio.h>

static void HwMinSizeHorizontal( ImUiWindow* window );
static void HwMinSizeVertical( ImUiWindow* window );
static void HwMinSizeElement( ImUiWindow* window );

static void HwStretchStack( ImUiWindow* window );
static void HwStretchHorizontal( ImUiWindow* window );
static void HwStretchVertical( ImUiWindow* window );
static void HwStretchGrid( ImUiWindow* window );
static void HwStretchElements( ImUiWindow* window, ImUiSize stretch1, ImUiSize stretch2, ImUiSize stretch3 );

void ImUiFrameworkTick( ImUiSurface* surface )
{
	ImUiContext* imui = ImUiSurfaceGetContext( surface );

	const ImUiSize surfaceSize = ImUiSurfaceGetSize( surface );
	ImUiWindow* window = ImUiWindowBegin( surface, ImUiStringViewCreate( "main" ), ImUiRectCreate( 0.0f, 0.0f, surfaceSize.width, surfaceSize.height ), 1 );

	ImUiWidget* vMain = ImUiWidgetBeginNamed( window, ImUiStringViewCreate( "vMain" ) );
	ImUiWidgetSetStretch( vMain, ImUiSizeCreateOne() );
	ImUiWidgetSetLayoutVerticalSpacing( vMain, 0.0f );

	{
		ImUiWidget* hLayout = ImUiWidgetBeginNamed( window, ImUiStringViewCreate( "hMain" ) );
		ImUiWidgetSetMargin( hLayout, ImUiBorderCreateAll( 50.0f ) );
		ImUiWidgetSetStretch( hLayout, ImUiSizeCreateOne() );
		ImUiWidgetSetLayoutHorizontalSpacing( hLayout, 50.0f );

		{
			ImUiWidget* vLayout = ImUiWidgetBeginNamed( window, ImUiStringViewCreate( "vLeft" ) );
			ImUiWidgetSetStretch( vLayout, ImUiSizeCreateOne() );
			ImUiWidgetSetLayoutVerticalSpacing( vLayout, 50.0f );

			HwMinSizeHorizontal( window );
			HwStretchStack( window );
			HwStretchHorizontal( window );

			ImUiWidgetEnd( vLayout );
		}

		HwStretchVertical( window );
		HwMinSizeVertical( window );

		ImUiWidgetEnd( hLayout );
	}

	HwStretchGrid( window );

	ImUiWidgetEnd( vMain );

	ImUiWindowEnd( window );
}

static void HwMinSizeHorizontal( ImUiWindow* window )
{
	ImUiWidget* layout = ImUiWidgetBeginNamed( window, ImUiStringViewCreate( "min_horizontal" ) );
	ImUiWidgetSetPadding( layout, ImUiBorderCreateAll( 20.0f ) );
	ImUiWidgetSetLayoutHorizontalSpacing( layout, 10.0f );

	ImUiWidgetDrawColor( layout, ImUiColorCreateWhite() );

	HwMinSizeElement( window );
	HwMinSizeElement( window );
	HwMinSizeElement( window );

	ImUiWidgetEnd( layout );
}

static void HwMinSizeVertical( ImUiWindow* window )
{
	ImUiWidget* layout = ImUiWidgetBeginNamed( window, ImUiStringViewCreate( "min_vertical" ) );
	ImUiWidgetSetPadding( layout, ImUiBorderCreateAll( 20.0f ) );
	ImUiWidgetSetLayoutVerticalSpacing( layout, 10.0f );

	ImUiWidgetDrawColor( layout, ImUiColorCreateWhite() );

	HwMinSizeElement( window );
	HwMinSizeElement( window );
	HwMinSizeElement( window );

	ImUiWidgetEnd( layout );
}

static void HwMinSizeElement( ImUiWindow* window )
{
	ImUiWidget* widget = ImUiWidgetBegin( window );
	ImUiWidgetSetMargin( widget, ImUiBorderCreateAll( 10.0f ) );
	ImUiWidgetSetFixedSize( widget, ImUiSizeCreateAll( 20.0f ) );

	ImUiWidgetDrawColor( widget, ImUiColorCreate( 0u, 0xffu, 0xffu, 0xffu ) );

	ImUiWidgetEnd( widget );
}

static void HwStretchStack( ImUiWindow* window )
{
	ImUiWidget* layout = ImUiWidgetBeginNamed( window, ImUiStringViewCreate( "stack" ) );
	ImUiWidgetSetPadding( layout, ImUiBorderCreateAll( 20.0f ) );
	ImUiWidgetSetStretch( layout, ImUiSizeCreateOne() );

	ImUiWidgetDrawColor( layout, ImUiColorCreateWhite() );

	{
		ImUiWidget* widget2 = ImUiWidgetBegin( window );
		ImUiWidgetSetMargin( widget2, ImUiBorderCreateAll( 10.0f ) );
		ImUiWidgetSetStretch( widget2, ImUiSizeCreate( 0.5f, 0.5f ) );

		ImUiWidgetDrawColor( widget2, ImUiColorCreate( 0u, 0u, 0xffu, 0xffu ) );

		ImUiWidgetEnd( widget2 );
	}

	{
		ImUiWidget* widget2 = ImUiWidgetBegin( window );
		ImUiWidgetSetMargin( widget2, ImUiBorderCreateAll( 10.0f ) );
		ImUiWidgetSetStretch( widget2, ImUiSizeCreate( 0.5f, 0.5f ) );
		ImUiWidgetSetAlign( widget2, ImUiAlignCreate( 1.0f, 1.0f ) );

		ImUiWidgetDrawColor( widget2, ImUiColorCreate( 0u, 0xffu, 0u, 0xffu ) );

		ImUiWidgetEnd( widget2 );
	}

	{
		ImUiWidget* widget2 = ImUiWidgetBegin( window );
		ImUiWidgetSetMargin( widget2, ImUiBorderCreateAll( 10.0f ) );
		ImUiWidgetSetStretch( widget2, ImUiSizeCreate( 0.5f, 0.5f ) );
		ImUiWidgetSetAlign( widget2, ImUiAlignCreateCenter() );

		ImUiWidgetDrawColor( widget2, ImUiColorCreate( 0xffu, 0u, 0u, 0xffu ) );

		ImUiWidgetEnd( widget2 );
	}

	ImUiWidgetEnd( layout );
}

static void HwStretchHorizontal( ImUiWindow* window )
{
	ImUiWidget* layout = ImUiWidgetBeginNamed( window, ImUiStringViewCreate( "horizontal" ) );
	ImUiWidgetSetPadding( layout, ImUiBorderCreateAll( 20.0f ) );
	ImUiWidgetSetStretch( layout, ImUiSizeCreateOne() );
	ImUiWidgetSetLayoutHorizontalSpacing( layout, 10.0f );

	ImUiWidgetDrawColor( layout, ImUiColorCreateWhite() );

	HwStretchElements( window, ImUiSizeCreate( 0.5f, 1.0f ), ImUiSizeCreate( 2.0f, 2.0f ), ImUiSizeCreate( 1.0f, 1.0f ) );

	ImUiWidgetEnd( layout );
}

static void HwStretchVertical( ImUiWindow* window )
{
	ImUiWidget* layout = ImUiWidgetBeginNamed( window, ImUiStringViewCreate( "vertical" ) );
	ImUiWidgetSetPadding( layout, ImUiBorderCreateAll( 20.0f ) );
	ImUiWidgetSetStretch( layout, ImUiSizeCreateOne() );
	ImUiWidgetSetLayoutVerticalSpacing( layout, 10.0f );

	ImUiWidgetDrawColor( layout, ImUiColorCreateWhite() );

	HwStretchElements( window, ImUiSizeCreate( 1.0f, 0.5f ), ImUiSizeCreate( 2.0f, 2.0f ), ImUiSizeCreate( 1.0f, 1.0f ) );

	ImUiWidgetEnd( layout );
}

static void HwStretchGrid( ImUiWindow* window )
{
	ImUiWidget* layout = ImUiWidgetBeginNamed( window, ImUiStringViewCreate( "grid" ) );
	//ImUiWidgetSetMargin( layout, ImUiBorderCreateAll( 50.0f ) );
	ImUiWidgetSetPadding( layout, ImUiBorderCreateAll( 20.0f ) );
	ImUiWidgetSetStretch( layout, ImUiSizeCreate( 1.0f, 1.0f ) );
	ImUiWidgetSetLayoutGrid( layout, 4u, 10.0f, 20.0f );

	ImUiWidgetDrawColor( layout, ImUiColorCreateWhite() );

	HwStretchElements( window, ImUiSizeCreate( 1.0f, 0.5f ), ImUiSizeCreate( 2.0f, 2.0f ), ImUiSizeCreate( 1.0f, 1.0f ) );
	HwStretchElements( window, ImUiSizeCreate( 1.0f, 0.5f ), ImUiSizeCreate( 2.0f, 2.0f ), ImUiSizeCreate( 1.0f, 1.0f ) );
	HwStretchElements( window, ImUiSizeCreate( 1.0f, 0.5f ), ImUiSizeCreate( 2.0f, 2.0f ), ImUiSizeCreate( 1.0f, 1.0f ) );
	HwStretchElements( window, ImUiSizeCreate( 1.0f, 0.5f ), ImUiSizeCreate( 2.0f, 2.0f ), ImUiSizeCreate( 1.0f, 1.0f ) );

	ImUiWidgetEnd( layout );
}

static void HwStretchElements( ImUiWindow* window, ImUiSize stretch1, ImUiSize stretch2, ImUiSize stretch3 )
{
	{
		ImUiWidget* widget2 = ImUiWidgetBegin( window );
		ImUiWidgetSetMargin( widget2, ImUiBorderCreateAll( 10.0f ) );
		ImUiWidgetSetStretch( widget2, stretch1 );

		ImUiWidgetDrawColor( widget2, ImUiColorCreate( 0u, 0u, 0xffu, 0xffu ) );

		ImUiWidgetEnd( widget2 );
	}

	{
		ImUiWidget* widget2 = ImUiWidgetBegin( window );
		ImUiWidgetSetMargin( widget2, ImUiBorderCreateAll( 10.0f ) );
		ImUiWidgetSetStretch( widget2, stretch2 );

		ImUiWidgetDrawColor( widget2, ImUiColorCreate( 0xffu, 0u, 0u, 0xffu ) );

		ImUiWidgetEnd( widget2 );
	}

	{
		ImUiWidget* widget2 = ImUiWidgetBegin( window );
		ImUiWidgetSetMargin( widget2, ImUiBorderCreateAll( 10.0f ) );
		ImUiWidgetSetStretch( widget2, stretch3 );
		ImUiWidgetSetAlign( widget2, ImUiAlignCreateCenter() );

		ImUiWidgetDrawColor( widget2, ImUiColorCreate( 0u, 0xffu, 0u, 0xffu ) );

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
