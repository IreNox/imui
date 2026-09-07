#include "tests.h"

static int runSpacingTest( ImuiFrame* frame, bool vertical )
{
	ImuiSurface* surface = imuiSurfaceBegin( frame, vertical ? "layout_vertical" : "layout_horizontal", imuiSizeCreateAll( 300.0f ), NULL, 1.0f );
	ImuiWindow* window = imuiWindowBegin( surface, "window", imuiSurfaceGetRect( surface ), 1u );
	ImuiWidget* parent = imuiWidgetBegin( window );
	imuiWidgetSetStretchOne( parent );
	if( vertical )
	{
		imuiWidgetSetLayoutVerticalSpacing( parent, 7.0f );
	}
	else
	{
		imuiWidgetSetLayoutHorizontalSpacing( parent, 7.0f );
	}
	ImuiWidget* first = imuiWidgetBegin( window );
	imuiWidgetSetFixedSizeFloat( first, 30.0f, 20.0f );
	imuiWidgetEnd( first );
	ImuiWidget* second = imuiWidgetBegin( window );
	imuiWidgetSetFixedSizeFloat( second, 40.0f, 50.0f );
	imuiWidgetEnd( second );
	imuiWidgetEnd( parent );
	imuiWindowEnd( window );
	imuiSurfaceEnd( surface );

	IMUI_CHECK( imuiWidgetGetLayout( parent ) == (vertical ? ImuiLayout_Vertical : ImuiLayout_Horizontal) );
	IMUI_CHECK( imuiWidgetGetPosX( first ) == 0.0f );
	IMUI_CHECK( imuiWidgetGetPosY( first ) == 0.0f );
	IMUI_CHECK( imuiWidgetGetPosX( second ) == (vertical ? 0.0f : 37.0f) );
	IMUI_CHECK( imuiWidgetGetPosY( second ) == (vertical ? 27.0f : 0.0f) );
	IMUI_CHECK( imuiWidgetGetSizeWidth( second ) == 40.0f );
	IMUI_CHECK( imuiWidgetGetSizeHeight( second ) == 50.0f );
	return 0;
}

static int runAlignmentTest( ImuiFrame* frame )
{
	ImuiSurface* surface = imuiSurfaceBegin( frame, "layout_alignment", imuiSizeCreate( 200.0f, 100.0f ), NULL, 1.0f );
	ImuiWindow* window = imuiWindowBegin( surface, "window", imuiSurfaceGetRect( surface ), 1u );
	ImuiWidget* parent = imuiWidgetBegin( window );
	imuiWidgetSetStretchOne( parent );
	imuiWidgetSetPadding( parent, imuiBorderCreateAll( 10.0f ) );
	ImuiWidget* child = imuiWidgetBegin( window );
	imuiWidgetSetFixedSizeFloat( child, 40.0f, 20.0f );
	imuiWidgetSetAlign( child, 0.5f, 1.0f );
	imuiWidgetEnd( child );
	imuiWidgetEnd( parent );
	imuiWindowEnd( window );
	imuiSurfaceEnd( surface );

	IMUI_CHECK( imuiWidgetGetLayout( parent ) == ImuiLayout_Stack );
	IMUI_CHECK( imuiWidgetGetHAlign( child ) == 0.5f );
	IMUI_CHECK( imuiWidgetGetVAlign( child ) == 1.0f );
	IMUI_CHECK( imuiWidgetGetPosX( child ) == 80.0f );
	IMUI_CHECK( imuiWidgetGetPosY( child ) == 70.0f );
	return 0;
}

static int runStretchLimitTest( ImuiFrame* frame )
{
	ImuiSurface* surface = imuiSurfaceBegin( frame, "layout_stretch_limit", imuiSizeCreateAll( 200.0f ), NULL, 1.0f );
	ImuiWindow* window = imuiWindowBegin( surface, "window", imuiSurfaceGetRect( surface ), 1u );
	ImuiWidget* widget = imuiWidgetBegin( window );
	imuiWidgetSetStretchOne( widget );
	imuiWidgetSetMinSizeFloat( widget, 30.0f, 40.0f );
	imuiWidgetSetMaxSizeFloat( widget, 80.0f, 90.0f );
	imuiWidgetEnd( widget );
	imuiWindowEnd( window );
	imuiSurfaceEnd( surface );

	IMUI_CHECK( imuiWidgetGetHStretch( widget ) == 1.0f );
	IMUI_CHECK( imuiWidgetGetVStretch( widget ) == 1.0f );
	IMUI_CHECK( imuiWidgetGetSizeWidth( widget ) == 80.0f );
	IMUI_CHECK( imuiWidgetGetSizeHeight( widget ) == 90.0f );
	return 0;
}

int runLayoutTests( ImuiFrame* frame )
{
	int failed = 0;
	failed += runSpacingTest( frame, false );
	failed += runSpacingTest( frame, true );
	failed += runAlignmentTest( frame );
	failed += runStretchLimitTest( frame );
	return failed;
}
