#include "tests.h"

static int runWidgetHierarchyTest( ImuiFrame* frame )
{
	ImuiContext* context = imuiFrameGetContext( frame );
	imuiInputBegin( context, NULL );
	const ImuiInputState* input = imuiInputEnd( context );
	ImuiSurface* surface = imuiSurfaceBegin( frame, "widget_hierarchy", imuiSizeCreateAll( 300.0f ), input, 1.0f );
	ImuiWindow* window = imuiWindowBegin( surface, "window", imuiSurfaceGetRect( surface ), 1u );
	ImuiWidget* parent = imuiWidgetBeginNamed( window, "parent" );
	ImuiWidget* first = imuiWidgetBeginId( window, 1u );
	imuiWidgetEnd( first );
	ImuiWidget* last = imuiWidgetBeginId( window, 2u );
	imuiWidgetEnd( last );
	imuiWidgetEnd( parent );
	imuiWindowEnd( window );
	imuiSurfaceEnd( surface );

	IMUI_CHECK( imuiWidgetGetContext( first ) == context );
	IMUI_CHECK( imuiWidgetGetSurface( first ) == surface );
	IMUI_CHECK( imuiWidgetGetWindow( first ) == window );
	IMUI_CHECK( imuiWidgetGetInput( first ) == input );
	IMUI_CHECK( imuiWidgetGetTime( first ) == imuiWindowGetTime( window ) );
	IMUI_CHECK( imuiWidgetGetDpiScale( first ) == 1.0f );
	IMUI_CHECK( imuiWidgetGetParent( first ) == parent );
	IMUI_CHECK( imuiWidgetGetParent( last ) == parent );
	IMUI_CHECK( imuiWidgetGetFirstChild( parent ) == first );
	IMUI_CHECK( imuiWidgetGetLastChild( parent ) == last );
	IMUI_CHECK( imuiWidgetGetNextSibling( first ) == last );
	IMUI_CHECK( imuiWidgetGetPrevSibling( last ) == first );
	IMUI_CHECK( imuiWidgetGetFirstChild( first ) == NULL );
	IMUI_CHECK( imuiWidgetGetLastChild( first ) == NULL );
	return 0;
}

static int runWidgetGeometryTest( ImuiFrame* frame )
{
	ImuiSurface* surface = imuiSurfaceBegin( frame, "widget_geometry", imuiSizeCreateAll( 300.0f ), NULL, 1.0f );
	ImuiWindow* window = imuiWindowBegin( surface, "window", imuiRectCreate( 10.0f, 20.0f, 200.0f, 200.0f ), 1u );
	ImuiWidget* widget = imuiWidgetBegin( window );
	imuiWidgetSetFixedSizeFloat( widget, 80.0f, 60.0f );
	imuiWidgetSetMargin( widget, imuiBorderCreate( 3.0f, 5.0f, 7.0f, 9.0f ) );
	imuiWidgetSetPadding( widget, imuiBorderCreate( 2.0f, 4.0f, 6.0f, 8.0f ) );
	imuiWidgetEnd( widget );
	imuiWindowEnd( window );
	imuiSurfaceEnd( surface );

	IMUI_CHECK( imuiWidgetGetMinSize( widget ).width == 80.0f );
	IMUI_CHECK( imuiWidgetGetMaxSize( widget ).height == 60.0f );
	IMUI_CHECK( imuiWidgetGetSizeWidth( widget ) == 80.0f );
	IMUI_CHECK( imuiWidgetGetSizeHeight( widget ) == 60.0f );
	IMUI_CHECK( imuiWidgetGetPosX( widget ) == 15.0f );
	IMUI_CHECK( imuiWidgetGetPosY( widget ) == 23.0f );
	const ImuiRect inner = imuiWidgetGetInnerRect( widget );
	IMUI_CHECK( inner.pos.x == 19.0f && inner.pos.y == 25.0f );
	IMUI_CHECK( inner.size.width == 68.0f && inner.size.height == 52.0f );
	IMUI_CHECK( imuiWidgetGetInnerSize( widget ).width == 68.0f );
	IMUI_CHECK( imuiWidgetGetInnerSize( widget ).height == 52.0f );
	IMUI_CHECK( imuiWidgetGetMargin( widget ).right == 9.0f );
	IMUI_CHECK( imuiWidgetGetPadding( widget ).bottom == 6.0f );
	return 0;
}

int runWidgetTests( ImuiFrame* frame )
{
	int failed = 0;
	failed += runWidgetHierarchyTest( frame );
	failed += runWidgetGeometryTest( frame );
	return failed;
}
