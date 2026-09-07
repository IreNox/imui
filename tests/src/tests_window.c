#include "tests.h"

static int runWindowPropertiesTest( ImuiFrame* frame )
{
	ImuiContext* context = imuiFrameGetContext( frame );
	imuiInputBegin( context, NULL );
	const ImuiInputState* input = imuiInputEnd( context );
	ImuiSurface* surface = imuiSurfaceBegin( frame, "window_properties", imuiSizeCreate( 500.0f, 400.0f ), input, 1.5f );
	ImuiWindow* window = imuiWindowBeginId( surface, "window", 201u, imuiRectCreate( 10.0f, 20.0f, 300.0f, 200.0f ), 1u );
	imuiWindowEnd( window );
	imuiSurfaceEnd( surface );

	IMUI_CHECK( imuiWindowGetContext( window ) == context );
	IMUI_CHECK( imuiWindowGetSurface( window ) == surface );
	IMUI_CHECK( imuiWindowGetInput( window ) == input );
	IMUI_CHECK( imuiWindowGetTime( window ) == imuiSurfaceGetTime( surface ) );
	IMUI_CHECK( imuiWindowGetDpiScale( window ) == 1.5f );
	const ImuiRect rect = imuiWindowGetRect( window );
	IMUI_CHECK( rect.pos.x == 10.0f && rect.pos.y == 20.0f );
	IMUI_CHECK( rect.size.width == 300.0f && rect.size.height == 200.0f );
	IMUI_CHECK( imuiWindowGetFirstChild( window ) == NULL );
	IMUI_CHECK( imuiWindowGetLastChild( window ) == NULL );
	return 0;
}

static int runWindowChildrenTest( ImuiFrame* frame )
{
	ImuiSurface* surface = imuiSurfaceBegin( frame, "window_children", imuiSizeCreateAll( 200.0f ), NULL, 1.0f );
	ImuiWindow* window = imuiWindowBegin( surface, "window", imuiSurfaceGetRect( surface ), 1u );
	ImuiWidget* first = imuiWidgetBegin( window );
	imuiWidgetEnd( first );
	ImuiWidget* last = imuiWidgetBegin( window );
	imuiWidgetEnd( last );
	imuiWindowEnd( window );
	imuiSurfaceEnd( surface );

	IMUI_CHECK( imuiWindowGetFirstChild( window ) == first );
	IMUI_CHECK( imuiWindowGetLastChild( window ) == last );
	IMUI_CHECK( imuiWidgetGetPrevSibling( first ) == NULL );
	IMUI_CHECK( imuiWidgetGetNextSibling( first ) == last );
	IMUI_CHECK( imuiWidgetGetPrevSibling( last ) == first );
	IMUI_CHECK( imuiWidgetGetNextSibling( last ) == NULL );
	return 0;
}

int runWindowTests( ImuiFrame* frame )
{
	int failed = 0;
	failed += runWindowPropertiesTest( frame );
	failed += runWindowChildrenTest( frame );
	return failed;
}
