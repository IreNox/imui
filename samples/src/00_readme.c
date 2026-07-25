#include "00_samples.h"

#include "imui/imui.h"
#include "imui/imui_toolbox.h"

#include <stdlib.h>
#include <stdio.h>

static int windowWidth = 1280u;
static int windowHeight = 720u;

void doUi( ImuiContext* imui, float time )
{
	ImuiInput* input = imuiInputBegin( imui, NULL );
	// write input state
	const ImuiInputState* inputState = imuiInputEnd( imui );

	ImuiFrame* frame = imuiBegin( imui, time );
	ImuiSurface* surface = imuiSurfaceBegin( frame, "main", imuiSizeCreate( (float)windowWidth, (float)windowHeight ), inputState, 1.0f );
	ImuiWindow* window = imuiWindowBegin( surface, "main", imuiRectCreateSize( 0.0f, 0.0f, imuiSurfaceGetSize( surface ) ), 1 );

	ImuiWidget* buttonsLayout = imuiWidgetBeginNamed( window, "buttons" );
	imuiWidgetSetLayoutHorizontalSpacing( buttonsLayout, 10.0f );

	if( imuiToolboxButtonLabel( window, "Button 1" ) )
	{
		// do stuff
	}

	if( imuiToolboxButtonLabel( window, "Button 2" ) )
	{
		// do stuff
	}

	if( imuiToolboxButtonLabel( window, "Button 3" ) )
	{
		// do stuff
	}

	imuiWidgetEnd( buttonsLayout );

	imuiWindowEnd( window );
	imuiSurfaceEnd( surface );
	imuiEnd( frame );
}
