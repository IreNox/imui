﻿#include "../../framework/framework.h"

#include "imui/imui.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

static ImUiTexture s_fontTexture;
static ImUiFont* s_font = NULL;

static float s_time = 0.0f;

void ImUiFrameworkTick( ImUiSurface* surface )
{
	s_time += (1.0f / 60.0f);

	ImUiContext* imui = ImUiSurfaceGetContext( surface );

	ImUiTextLayout* text = ImUiTextLayoutCreate( imui, s_font, ImUiStringViewCreate( u8"ΑΒΓΔ Hello World! ΦΧΨΩ" ) );

	const ImUiSize surfaceSize = ImUiSurfaceGetSize( surface );
	ImUiWindow* window = ImUiWindowBegin( surface, ImUiStringViewCreate( "main" ), ImUiRectCreate( 0.0f, 0.0f, surfaceSize.width, surfaceSize.height ), 1 );

	const float timeSin = sinf( s_time / -2.0f ) * 0.5f + 0.5f;
	const float timeCos = cosf( s_time / -2.0f ) * 0.5f + 0.5f;

	const float timeLeft	= timeSin;
	const float timeRight	= 1.0f - timeSin;
	const float timeTop		= timeCos;
	const float timeBottom	= 1.0f - timeCos;

	const float timeR		= (timeSin * 0.5f) + 0.5f;
	const float timeG		= (timeCos * 0.5f) + 0.5f;
	const float timeB		= ((2.0f - (timeSin + timeCos)) * 0.25f) + 0.5f;

	ImUiWidget* vLayout = ImUiWidgetBeginNamed( window, ImUiStringViewCreate( "vMain" ) );
	ImUiWidgetSetStretch( vLayout, ImUiSizeCreateOne() );
	ImUiWidgetSetLayoutVerical( vLayout );

	{
		ImUiWidget* vTop = ImUiWidgetBeginNamed( window, ImUiStringViewCreate( "vTop" ) );
		ImUiWidgetSetStretch( vTop, ImUiSizeCreate( 1.0f, timeTop ) );
		ImUiWidgetEnd( vTop );
	}

	{
		ImUiWidget* hLayout = ImUiWidgetBeginNamed( window, ImUiStringViewCreate( "hMain" ) );
		ImUiWidgetSetStretch( hLayout, ImUiSizeCreate( 1.0f, 0.0f ) );
		ImUiWidgetSetLayoutHorizontal( hLayout );

		{
			ImUiWidget* hLeft = ImUiWidgetBeginNamed( window, ImUiStringViewCreate( "hLeft" ) );
			ImUiWidgetSetStretch( hLeft, ImUiSizeCreate( timeLeft, 1.0f ) );
			ImUiWidgetEnd( hLeft );

			ImUiWidget* hCenter = ImUiWidgetBeginNamed( window, ImUiStringViewCreate( "hCenter" ) );
			ImUiWidgetSetStretch( hCenter, ImUiSizeCreateZero() );
			ImUiWidgetSetPadding( hCenter, ImUiBorderCreateAll( 50.0f ) );

			ImUiDrawWidgetColor( hCenter, ImUiColorCreate( timeR, timeG, timeB, 1.0f ) );

			ImUiWidget* centerText = ImUiWidgetBeginNamed( window, ImUiStringViewCreate( "centerText" ) );
			ImUiWidgetSetFixedSize( centerText, ImUiTextLayoutGetSize( text ) );

			ImUiDrawText( centerText, ImUiWidgetGetPos( centerText ), text );

			ImUiWidgetEnd( centerText );

			ImUiWidgetEnd( hCenter );

			ImUiWidget* hRight = ImUiWidgetBeginNamed( window, ImUiStringViewCreate( "hRight" ) );
			ImUiWidgetSetStretch( hRight, ImUiSizeCreate( timeRight, 1.0f ) );
			ImUiWidgetEnd( hRight );
		}

		ImUiWidgetEnd( hLayout );
	}

	{
		ImUiWidget* vBottom = ImUiWidgetBeginNamed( window, ImUiStringViewCreate( "vBottom" ) );
		ImUiWidgetSetStretch( vBottom, ImUiSizeCreate( 1.0f, timeBottom ) );
		ImUiWidgetEnd( vBottom );
	}

	ImUiWidgetEnd( vLayout );

	ImUiWindowEnd( window );
}

bool ImUiFrameworkInitialize( ImUiContext* imui )
{
	return ImUiFrameworkFontCreate( &s_font, &s_fontTexture, "c:/windows/fonts/arial.ttf", 32.0f );
}

void ImUiFrameworkShutdown( ImUiContext* imui )
{
	ImUiFrameworkFontDestroy( &s_font, &s_fontTexture );
}
