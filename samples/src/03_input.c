#include "00_samples.h"

#include "imui/imui.h"

#include <stdlib.h>
#include <stdio.h>

static void imuiInputSampleElement( ImuiWindow* window );

void imuiInputSampleTick( ImuiWindow* window )
{
	ImuiContext* imui = imuiWindowGetContext( window );

	ImuiWidget* cLayout = imuiWidgetBeginNamed( window, "hMain" );
	imuiWidgetSetStretch( cLayout, 0.5f, 0.25f );
	imuiWidgetSetAlign( cLayout, 0.5f, 0.5f );

	imuiWidgetDrawColor( cLayout, imuiColorCreateWhite() );

	ImuiWidget* hLayout = imuiWidgetBeginNamed( window, "vMain" );
	imuiWidgetSetStretchOne( hLayout );
	imuiWidgetSetPadding( hLayout, imuiBorderCreateAll( 20.0f ) );
	imuiWidgetSetLayoutHorizontalSpacing( hLayout, 20.0f );

	for( size_t i = 0u; i < 5u; ++i )
	{
		imuiInputSampleElement( window );
	}

	imuiWidgetEnd( hLayout );

	imuiWidgetEnd( cLayout );
}

static void imuiInputSampleElement( ImuiWindow* window )
{
	ImuiWidget* widget = imuiWidgetBegin( window );
	imuiWidgetSetStretchOne( widget );

	ImuiColor color = imuiColorCreateFloat( 0.1f, 0.5f, 0.8f, 1.0f );

	ImuiWidgetInputState inputState;
	imuiWidgetGetInputState( widget, &inputState );

	if( inputState.hasMouseReleased )
	{
		color = imuiColorCreateFloat( 1.0f, 0.3f, 0.2f, 1.0f );
	}
	else if( inputState.isMouseDown )
	{
		color = imuiColorCreateFloat( 0.0f, 0.3f, 0.7f, 1.0f );
	}
	else if( inputState.isMouseOver )
	{
		color = imuiColorCreateFloat( 0.5f, 0.9f, 1.0f, 1.0f );
	}

	imuiWidgetDrawColor( widget, color );

	imuiWidgetEnd( widget );
}

bool imuiInputSampleInitialize( ImuiContext* imui )
{
	return true;
}

void imuiInputSampleShutdown( ImuiContext* imui )
{
}
