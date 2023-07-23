#include "../../framework/framework.h"

#include "imui/imui.h"

static HwStack( ImUiWindow* window );
static HwHorizontal( ImUiWindow* window );
static HwVertical( ImUiWindow* window );

void ImUiFrameworkTick( ImUiSurface* surface )
{
	const ImUiSize surfaceSize = ImUiSurfaceGetSize( surface );
	ImUiWindow* window = ImUiWindowBegin( surface, ImUiStringViewCreate( "main" ), ImUiRectangleCreate( 0.0f, 0.0f, surfaceSize.width, surfaceSize.height ), 1 );

	//ImUiDraw

	ImUiWidget* hLayout = ImUiWidgetBeginNamed( window, ImUiStringViewCreate( "hMain" ) );
	ImUiWidgetSetMargin( hLayout, ImUiThicknessCreateAll( 50.0f ) );
	ImUiWidgetSetStretch( hLayout, ImUiSizeCreateOne() );
	ImUiWidgetSetLayoutHorizontalSpacing( hLayout, 50.0f );

	ImUiWidget* vLayout = ImUiWidgetBeginNamed( window, ImUiStringViewCreate( "vMain" ) );
	ImUiWidgetSetStretch( vLayout, ImUiSizeCreateOne() );
	ImUiWidgetSetLayoutVerticalSpacing( vLayout, 50.0f );


	HwStack( window );
	HwHorizontal( window );

	ImUiWidgetEnd( vLayout );

	HwVertical( window );

	ImUiWidgetEnd( hLayout );

	ImUiWindowEnd( window );
}

static HwStack( ImUiWindow* window )
{
	ImUiWidget* widget = ImUiWidgetBeginNamed( window, ImUiStringViewCreate( "stack" ) );
	//ImUiWidgetSetMargin( widget, ImUiThicknessCreateAll( 50.0f ) );
	ImUiWidgetSetPadding( widget, ImUiThicknessCreateAll( 20.0f ) );
	ImUiWidgetSetStretch( widget, ImUiSizeCreateOne() );
	//ImUiWidgetSetFixedSize( widget, ImUiSizeCreate( 400.0f, 150.0f ) );

	ImUiDrawRectangleColor( widget, ImUiWidgetGetRectangle( widget ), ImUiColorCreateWhite( 1.0f ) );

	{
		ImUiWidget* widget2 = ImUiWidgetBegin( window );
		ImUiWidgetSetMargin( widget2, ImUiThicknessCreateAll( 10.0f ) );
		ImUiWidgetSetStretch( widget2, ImUiSizeCreate( 0.5f, 0.5f ) );

		ImUiDrawRectangleColor( widget2, ImUiWidgetGetRectangle( widget2 ), ImUiColorCreate( 0.0f, 0.0f, 1.0f, 1.0f ) );

		ImUiWidgetEnd( widget2 );
	}

	{
		ImUiWidget* widget2 = ImUiWidgetBegin( window );
		ImUiWidgetSetMargin( widget2, ImUiThicknessCreateAll( 10.0f ) );
		ImUiWidgetSetStretch( widget2, ImUiSizeCreate( 0.5f, 0.5f ) );
		ImUiWidgetSetAlignment( widget2, ImUiAlignmentCreate( ImUiHorizintalAlignment_Right, ImUiVerticalAlignment_Bottom ) );

		ImUiDrawRectangleColor( widget2, ImUiWidgetGetRectangle( widget2 ), ImUiColorCreate( 0.0f, 1.0f, 0.0f, 1.0f ) );

		ImUiWidgetEnd( widget2 );
	}

	if( 1 )
	{
		ImUiWidget* widget2 = ImUiWidgetBegin( window );
		ImUiWidgetSetMargin( widget2, ImUiThicknessCreateAll( 10.0f ) );
		ImUiWidgetSetStretch( widget2, ImUiSizeCreate( 0.5f, 0.5f ) );
		ImUiWidgetSetAlignment( widget2, ImUiAlignmentCreate( ImUiHorizintalAlignment_Center, ImUiVerticalAlignment_Center ) );

		ImUiDrawRectangleColor( widget2, ImUiWidgetGetRectangle( widget2 ), ImUiColorCreate( 1.0f, 0.0f, 0.0f, 1.0f ) );

		ImUiWidgetEnd( widget2 );
	}

	ImUiWidgetEnd( widget );
}

static HwHorizontal( ImUiWindow* window )
{
	ImUiWidget* widget = ImUiWidgetBeginNamed( window, ImUiStringViewCreate( "horizontal" ) );
	//ImUiWidgetSetMargin( widget, ImUiThicknessCreate( 250.0f, 50.0f, 0.0f, 0.0f ) );
	ImUiWidgetSetPadding( widget, ImUiThicknessCreateAll( 20.0f ) );
	ImUiWidgetSetStretch( widget, ImUiSizeCreateOne() );
	//ImUiWidgetSetFixedSize( widget, ImUiSizeCreate( 400.0f, 150.0f ) );
	ImUiWidgetSetLayoutHorizontalSpacing( widget, 10.0f );

	ImUiDrawRectangleColor( widget, ImUiWidgetGetRectangle( widget ), ImUiColorCreateWhite( 1.0f ) );

	{
		ImUiWidget* widget2 = ImUiWidgetBegin( window );
		ImUiWidgetSetMargin( widget2, ImUiThicknessCreateAll( 10.0f ) );
		ImUiWidgetSetStretch( widget2, ImUiSizeCreate( 1.0f, 1.0f ) );

		ImUiDrawRectangleColor( widget2, ImUiWidgetGetRectangle( widget2 ), ImUiColorCreate( 0.0f, 0.0f, 1.0f, 1.0f ) );

		ImUiWidgetEnd( widget2 );
	}

	{
		ImUiWidget* widget2 = ImUiWidgetBegin( window );
		ImUiWidgetSetMargin( widget2, ImUiThicknessCreateAll( 10.0f ) );
		ImUiWidgetSetStretch( widget2, ImUiSizeCreate( 2.0f, 2.0f ) );

		ImUiDrawRectangleColor( widget2, ImUiWidgetGetRectangle( widget2 ), ImUiColorCreate( 1.0f, 0.0f, 0.0f, 1.0f ) );

		ImUiWidgetEnd( widget2 );
	}

	if( 1 )
	{
		ImUiWidget* widget2 = ImUiWidgetBegin( window );
		ImUiWidgetSetMargin( widget2, ImUiThicknessCreateAll( 10.0f ) );
		ImUiWidgetSetStretch( widget2, ImUiSizeCreate( 1.0f, 1.0f ) );
		ImUiWidgetSetVerticalAlignment( widget2, ImUiVerticalAlignment_Center );

		ImUiDrawRectangleColor( widget2, ImUiWidgetGetRectangle( widget2 ), ImUiColorCreate( 0.0f, 1.0f, 0.0f, 1.0f ) );

		ImUiWidgetEnd( widget2 );
	}

	ImUiWidgetEnd( widget );
}

static HwVertical( ImUiWindow* window )
{
	ImUiWidget* widget = ImUiWidgetBeginNamed( window, ImUiStringViewCreate( "vertical" ) );
	//ImUiWidgetSetMargin( widget, ImUiThicknessCreate( 50.0f, 500.0f, 0.0f, 0.0f ) );
	ImUiWidgetSetPadding( widget, ImUiThicknessCreateAll( 20.0f ) );
	ImUiWidgetSetStretch( widget, ImUiSizeCreateOne() );
	//ImUiWidgetSetFixedSize( widget, ImUiSizeCreate( 150.0f, 400.0f ) );
	ImUiWidgetSetLayoutVerticalSpacing( widget, 10.0f );

	ImUiDrawRectangleColor( widget, ImUiWidgetGetRectangle( widget ), ImUiColorCreateWhite( 1.0f ) );

	{
		ImUiWidget* widget2 = ImUiWidgetBegin( window );
		ImUiWidgetSetMargin( widget2, ImUiThicknessCreateAll( 10.0f ) );
		ImUiWidgetSetStretch( widget2, ImUiSizeCreate( 1.0f, 1.0f ) );

		ImUiDrawRectangleColor( widget2, ImUiWidgetGetRectangle( widget2 ), ImUiColorCreate( 0.0f, 0.0f, 1.0f, 1.0f ) );

		ImUiWidgetEnd( widget2 );
	}

	{
		ImUiWidget* widget2 = ImUiWidgetBegin( window );
		ImUiWidgetSetMargin( widget2, ImUiThicknessCreateAll( 10.0f ) );
		ImUiWidgetSetStretch( widget2, ImUiSizeCreate( 2.0f, 2.0f ) );

		ImUiDrawRectangleColor( widget2, ImUiWidgetGetRectangle( widget2 ), ImUiColorCreate( 1.0f, 0.0f, 0.0f, 1.0f ) );

		ImUiWidgetEnd( widget2 );
	}

	if( 1 )
	{
		ImUiWidget* widget2 = ImUiWidgetBegin( window );
		ImUiWidgetSetMargin( widget2, ImUiThicknessCreateAll( 10.0f ) );
		ImUiWidgetSetStretch( widget2, ImUiSizeCreate( 1.0f, 1.0f ) );
		ImUiWidgetSetHorizintalAlignment( widget2, ImUiHorizintalAlignment_Center );

		ImUiDrawRectangleColor( widget2, ImUiWidgetGetRectangle( widget2 ), ImUiColorCreate( 0.0f, 1.0f, 0.0f, 1.0f ) );

		ImUiWidgetEnd( widget2 );
	}

	ImUiWidgetEnd( widget );
}