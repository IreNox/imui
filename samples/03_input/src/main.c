#include "../../framework/framework.h"

#include "imui/imui.h"

#include <stdlib.h>
#include <stdio.h>

static void HwInputElement( ImUiWindow* window );

void ImUiFrameworkTick( ImUiSurface* surface )
{
	ImUiContext* imui = ImUiSurfaceGetContext( surface );

	const ImUiSize surfaceSize = ImUiSurfaceGetSize( surface );
	ImUiWindow* window = ImUiWindowBegin( surface, ImUiStringViewCreate( "main" ), ImUiRectangleCreate( 0.0f, 0.0f, surfaceSize.width, surfaceSize.height ), 1 );

	ImUiWidget* cLayout = ImUiWidgetBeginNamed( window, ImUiStringViewCreate( "hMain" ) );
	ImUiWidgetSetStretch( cLayout, ImUiSizeCreate( 0.5f, 0.25f ) );
	ImUiWidgetSetAlignment( cLayout, ImUiAlignmentCreateCenter() );

	ImUiDrawWidgetColor( cLayout, ImUiColorCreateWhite( 1.0f ) );

	ImUiWidget* hLayout = ImUiWidgetBeginNamed( window, ImUiStringViewCreate( "vMain" ) );
	ImUiWidgetSetStretch( hLayout, ImUiSizeCreateOne() );
	ImUiWidgetSetPadding( hLayout, ImUiThicknessCreateAll( 20.0f ) );
	ImUiWidgetSetLayoutHorizontalSpacing( hLayout, 20.0f );

	for( size_t i = 0u; i < 5u; ++i )
	{
		HwInputElement( window );
	}

	ImUiWidgetEnd( hLayout );

	ImUiWidgetEnd( cLayout );

	ImUiWindowEnd( window );
}

static void HwInputElement( ImUiWindow* window )
{
	ImUiWidget* widget = ImUiWidgetBegin( window );
	ImUiWidgetSetStretch( widget, ImUiSizeCreateOne() );

	ImUiColor color = ImUiColorCreate( 0.1f, 0.5f, 0.8f, 1.0f );

	ImUiInputWidgetState inputState;
	ImUiInputGetWidgetState( widget, &inputState );

	if( inputState.hasMouseReleased )
	{
		color = ImUiColorCreate( 1.0f, 0.3f, 0.2f, 1.0f );
	}
	else if( inputState.isMouseDown )
	{
		color = ImUiColorCreate( 0.0f, 0.3f, 0.7f, 1.0f );
	}
	else if( inputState.isMouseOver )
	{
		color = ImUiColorCreate( 0.5f, 0.9f, 1.0f, 1.0f );
	}

	ImUiDrawWidgetColor( widget, color );

	ImUiWidgetEnd( widget );
}

bool ImUiFrameworkInitialize( ImUiContext* imui )
{
	return true;
}

void ImUiFrameworkShutdown( ImUiContext* imui )
{
}
