#include "00_samples.h"

#include "framework.h"

#include "imui/imui.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

typedef struct ImUiHelloWorldSampleContext
{
	ImUiFont*	font;
	ImUiImage	fontTexture;
} ImUiHelloWorldSampleContext;

static ImUiHelloWorldSampleContext s_helloWorldContext = { NULL };

void ImUiHelloWorldSampleTick( ImUiSurface* surface )
{
	ImUiContext* imui = ImUiSurfaceGetContext( surface );

	ImUiTextLayout* textLayout = ImUiTextLayoutCreate( imui, s_helloWorldContext.font, u8"ΑΒΓΔ Hello World! ΦΧΨΩ" );

	const ImUiSize surfaceSize = ImUiSurfaceGetSize( surface );
	ImUiWindow* window = ImUiWindowBegin( surface, "main", ImUiRectCreate( 0.0f, 0.0f, surfaceSize.width, surfaceSize.height ), 1 );

	const double time	= ImUiWindowGetTime( window );
	const float timeSin = (float)sin( time / -2.0 ) * 0.5f + 0.5f;
	const float timeCos = (float)cos( time / -2.0 ) * 0.5f + 0.5f;

	const float timeLeft	= timeSin;
	const float timeRight	= 1.0f - timeSin;
	const float timeTop		= timeCos;
	const float timeBottom	= 1.0f - timeCos;

	const float timeR		= (timeSin * 0.5f) + 0.5f;
	const float timeG		= (timeCos * 0.5f) + 0.5f;
	const float timeB		= ((2.0f - (timeSin + timeCos)) * 0.25f) + 0.5f;

	ImUiWidget* vLayout = ImUiWidgetBeginNamed( window, "vMain" );
	ImUiWidgetSetStretchOne( vLayout );
	ImUiWidgetSetLayoutVertical( vLayout );

	{
		ImUiWidget* vTop = ImUiWidgetBeginNamed( window, "vTop" );
		ImUiWidgetSetStretch( vTop, 1.0f, timeTop );
		ImUiWidgetEnd( vTop );
	}

	{
		ImUiWidget* hLayout = ImUiWidgetBeginNamed( window, "hMain" );
		ImUiWidgetSetHStretch( hLayout, 1.0f );
		ImUiWidgetSetLayoutHorizontal( hLayout );

		{
			ImUiWidget* hLeft = ImUiWidgetBeginNamed( window, "hLeft" );
			ImUiWidgetSetStretch( hLeft, timeLeft, 1.0f );
			ImUiWidgetEnd( hLeft );

			ImUiWidget* hCenter = ImUiWidgetBeginNamed( window, "hCenter" );
			ImUiWidgetSetFixedSize( hCenter, ImUiSizeExpandBorder( ImUiTextLayoutGetSize( textLayout ), ImUiBorderCreateAll( 50.0f ) ) );

			ImUiWidgetDrawColor( hCenter, ImUiColorCreateFloat( timeR, timeG, timeB, 1.0f ) );

			ImUiWidget* text = ImUiWidgetBeginNamed( window, "centerText" );
			ImUiWidgetSetFixedSize( text, ImUiTextLayoutGetSize( textLayout ) );
			ImUiWidgetSetAlign( text, timeLeft, timeTop );

			ImUiWidgetDrawText( text, textLayout, ImUiColorCreateWhite() );

			ImUiWidgetEnd( text );

			ImUiWidgetEnd( hCenter );

			ImUiWidget* hRight = ImUiWidgetBeginNamed( window, "hRight" );
			ImUiWidgetSetStretch( hRight, timeRight, 1.0f );
			ImUiWidgetEnd( hRight );
		}

		ImUiWidgetEnd( hLayout );
	}

	{
		ImUiWidget* vBottom = ImUiWidgetBeginNamed( window, "vBottom" );
		ImUiWidgetSetStretch( vBottom, 1.0f, timeBottom );
		ImUiWidgetEnd( vBottom );
	}

	ImUiWidgetEnd( vLayout );

	ImUiWindowEnd( window );
}

bool ImUiHelloWorldSampleInitialize( ImUiContext* imui )
{
	return ImUiFrameworkFontCreate( &s_helloWorldContext.font, &s_helloWorldContext.fontTexture, "c:/windows/fonts/arial.ttf", 32.0f );
}

void ImUiHelloWorldSampleShutdown( ImUiContext* imui )
{
	ImUiFrameworkFontDestroy( &s_helloWorldContext.font, &s_helloWorldContext.fontTexture );
}
