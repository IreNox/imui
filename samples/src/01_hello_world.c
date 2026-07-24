#include "00_samples.h"

#include "framework.h"

#include "imui/imui.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

typedef struct ImuiHelloWorldSampleContext
{
	ImuiFont*	font;
	ImuiImage	fontTexture;
} ImuiHelloWorldSampleContext;

static ImuiHelloWorldSampleContext s_helloWorldContext = { NULL };

void imuiHelloWorldSampleTick( ImuiWindow* window )
{
	ImuiContext* imui = imuiWindowGetContext( window );

	ImuiTextLayout* textLayout = imuiTextLayoutCreate( imui, s_helloWorldContext.font, u8"ΑΒΓΔ Hello World! ΦΧΨΩ" );

	const double time	= imuiWindowGetTime( window );
	const float timeSin = (float)sin( time / -2.0 ) * 0.5f + 0.5f;
	const float timeCos = (float)cos( time / -2.0 ) * 0.5f + 0.5f;

	const float timeLeft	= timeSin;
	const float timeRight	= 1.0f - timeSin;
	const float timeTop		= timeCos;
	const float timeBottom	= 1.0f - timeCos;

	const float timeR		= (timeSin * 0.5f) + 0.5f;
	const float timeG		= (timeCos * 0.5f) + 0.5f;
	const float timeB		= ((2.0f - (timeSin + timeCos)) * 0.25f) + 0.5f;

	ImuiWidget* vLayout = imuiWidgetBeginNamed( window, "vMain" );
	imuiWidgetSetStretchOne( vLayout );
	imuiWidgetSetLayoutVertical( vLayout );

	{
		ImuiWidget* vTop = imuiWidgetBeginNamed( window, "vTop" );
		imuiWidgetSetStretch( vTop, 1.0f, timeTop );
		imuiWidgetEnd( vTop );
	}

	{
		ImuiWidget* hLayout = imuiWidgetBeginNamed( window, "hMain" );
		imuiWidgetSetHStretch( hLayout, 1.0f );
		imuiWidgetSetLayoutHorizontal( hLayout );

		{
			ImuiWidget* hLeft = imuiWidgetBeginNamed( window, "hLeft" );
			imuiWidgetSetStretch( hLeft, timeLeft, 1.0f );
			imuiWidgetEnd( hLeft );

			ImuiWidget* hCenter = imuiWidgetBeginNamed( window, "hCenter" );
			imuiWidgetSetFixedSize( hCenter, imuiSizeExpandBorder( imuiTextLayoutGetSize( textLayout ), imuiBorderCreateAll( 50.0f ) ) );

			imuiWidgetDrawColor( hCenter, imuiColorCreateFloat( timeR, timeG, timeB, 1.0f ) );

			ImuiWidget* text = imuiWidgetBeginNamed( window, "centerText" );
			imuiWidgetSetFixedSize( text, imuiTextLayoutGetSize( textLayout ) );
			imuiWidgetSetAlign( text, timeLeft, timeTop );

			imuiWidgetDrawText( text, textLayout, imuiColorCreateWhite() );

			imuiWidgetEnd( text );

			imuiWidgetEnd( hCenter );

			ImuiWidget* hRight = imuiWidgetBeginNamed( window, "hRight" );
			imuiWidgetSetStretch( hRight, timeRight, 1.0f );
			imuiWidgetEnd( hRight );
		}

		imuiWidgetEnd( hLayout );
	}

	{
		ImuiWidget* vBottom = imuiWidgetBeginNamed( window, "vBottom" );
		imuiWidgetSetStretch( vBottom, 1.0f, timeBottom );
		imuiWidgetEnd( vBottom );
	}

	imuiWidgetEnd( vLayout );
}

bool imuiHelloWorldSampleInitialize( ImuiContext* imui )
{
	return imuiFrameworkFontCreate( &s_helloWorldContext.font, &s_helloWorldContext.fontTexture, "c:/windows/fonts/arial.ttf", 32.0f );
}

void imuiHelloWorldSampleShutdown( ImuiContext* imui )
{
	imuiFrameworkFontDestroy( &s_helloWorldContext.font, &s_helloWorldContext.fontTexture );
}
