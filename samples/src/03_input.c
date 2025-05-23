﻿#include "00_samples.h"

#include "imui/imui.h"

#include <stdlib.h>
#include <stdio.h>

static void ImUiInputSampleElement( ImUiWindow* window );

void ImUiInputSampleTick( ImUiWindow* window )
{
	ImUiContext* imui = ImUiWindowGetContext( window );

	ImUiWidget* cLayout = ImUiWidgetBeginNamed( window, "hMain" );
	ImUiWidgetSetStretch( cLayout, 0.5f, 0.25f );
	ImUiWidgetSetAlign( cLayout, 0.5f, 0.5f );

	ImUiWidgetDrawColor( cLayout, ImUiColorCreateWhite() );

	ImUiWidget* hLayout = ImUiWidgetBeginNamed( window, "vMain" );
	ImUiWidgetSetStretchOne( hLayout );
	ImUiWidgetSetPadding( hLayout, ImUiBorderCreateAll( 20.0f ) );
	ImUiWidgetSetLayoutHorizontalSpacing( hLayout, 20.0f );

	for( size_t i = 0u; i < 5u; ++i )
	{
		ImUiInputSampleElement( window );
	}

	ImUiWidgetEnd( hLayout );

	ImUiWidgetEnd( cLayout );
}

static void ImUiInputSampleElement( ImUiWindow* window )
{
	ImUiWidget* widget = ImUiWidgetBegin( window );
	ImUiWidgetSetStretchOne( widget );

	ImUiColor color = ImUiColorCreateFloat( 0.1f, 0.5f, 0.8f, 1.0f );

	ImUiWidgetInputState inputState;
	ImUiWidgetGetInputState( widget, &inputState );

	if( inputState.hasMouseReleased )
	{
		color = ImUiColorCreateFloat( 1.0f, 0.3f, 0.2f, 1.0f );
	}
	else if( inputState.isMouseDown )
	{
		color = ImUiColorCreateFloat( 0.0f, 0.3f, 0.7f, 1.0f );
	}
	else if( inputState.isMouseOver )
	{
		color = ImUiColorCreateFloat( 0.5f, 0.9f, 1.0f, 1.0f );
	}

	ImUiWidgetDrawColor( widget, color );

	ImUiWidgetEnd( widget );
}

bool ImUiInputSampleInitialize( ImUiContext* imui )
{
	return true;
}

void ImUiInputSampleShutdown( ImUiContext* imui )
{
}
