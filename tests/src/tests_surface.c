#include "tests.h"

// DISCLAIMER: This tests are written by AI.

static int runSurfacePropertiesTest( ImuiFrame* frame )
{
	ImuiContext* context = imuiFrameGetContext( frame );
	imuiInputBegin( context, NULL );
	const ImuiInputState* input = imuiInputEnd( context );
	ImuiSurface* surface = imuiSurfaceBeginId( frame, "surface_properties", 101u, imuiSizeCreate( 640.0f, 480.0f ), input, 2.0f );
	imuiSurfaceEnd( surface );

	IMUI_CHECK( imuiSurfaceGetContext( surface ) == context );
	IMUI_CHECK( imuiSurfaceGetInput( surface ) == input );
	IMUI_CHECK( imuiSurfaceGetDpiScale( surface ) == 2.0f );
	IMUI_CHECK( imuiSurfaceGetSize( surface ).width == 640.0f );
	IMUI_CHECK( imuiSurfaceGetSize( surface ).height == 480.0f );
	const ImuiRect rect = imuiSurfaceGetRect( surface );
	IMUI_CHECK( rect.pos.x == 0.0f && rect.pos.y == 0.0f );
	IMUI_CHECK( rect.size.width == 640.0f && rect.size.height == 480.0f );
	return 0;
}

static int runSurfaceIsolationTest( ImuiFrame* frame )
{
	ImuiSurface* first = imuiSurfaceBeginId( frame, "surface_isolation_first", 102u, imuiSizeCreate( 100.0f, 200.0f ), NULL, 1.0f );
	imuiSurfaceEnd( first );
	ImuiSurface* second = imuiSurfaceBeginId( frame, "surface_isolation_second", 103u, imuiSizeCreate( 300.0f, 400.0f ), NULL, 1.5f );
	imuiSurfaceEnd( second );

	IMUI_CHECK( first != second );
	IMUI_CHECK( imuiSurfaceGetSize( first ).width == 100.0f );
	IMUI_CHECK( imuiSurfaceGetSize( second ).width == 300.0f );
	IMUI_CHECK( imuiSurfaceGetDpiScale( first ) == 1.0f );
	IMUI_CHECK( imuiSurfaceGetDpiScale( second ) == 1.5f );
	return 0;
}

int runSurfaceTests( ImuiFrame* frame )
{
	int failed = 0;
	failed += runSurfacePropertiesTest( frame );
	failed += runSurfaceIsolationTest( frame );
	return failed;
}
