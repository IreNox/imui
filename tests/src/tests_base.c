#include "tests.h"

typedef void(*ImuiTestSetLayoutFunc)( ImuiWidget* widget );
typedef void(*ImuiTestSetSizeFunc)( ImuiWidget* widget, float size );
typedef float(*ImuiTestGetSizeFunc)( const ImuiWidget* widget );

static int	runMarginTest( ImuiFrame* frame );
//static int	runLayoutTest( ImuiFrame* frame );
static int runLayoutTest( ImuiFrame* frame, ImuiTestSetLayoutFunc setLayoutCallback, ImuiTestSetSizeFunc setSizeCallback, ImuiTestGetSizeFunc getSizeFunc );

static void doUiLayoutChilds( ImuiWindow* window, ImuiTestSetSizeFunc setSizeCallback, float childSize1, float childSize2, float childSize3, float childSize4 );

int	runBaseTests( ImuiFrame* frame )
{
	int failed = 0;
	failed += runMarginTest( frame );
	failed += runLayoutTest( frame, imuiWidgetSetLayoutHorizontal, imuiWidgetSetMinWidth, imuiWidgetGetSizeWidth );
	return failed;
}

static int runMarginTest( ImuiFrame* frame )
{
	ImuiSurface* surface = imuiSurfaceBegin( frame, "base", imuiSizeCreate( 500.0f, 500.0f ), NULL, 1.0f );
	ImuiWindow* window = imuiWindowBegin( surface, "base", imuiSurfaceGetRect( surface ), 1 );

	ImuiWidget* widget1 = imuiWidgetBegin( window );
	imuiWidgetSetStretchOne( widget1 );
	imuiWidgetSetMargin( widget1, imuiBorderCreateAll( 50.0f ) );

	ImuiWidget* widget2 = imuiWidgetBegin( window );
	imuiWidgetSetMargin( widget2, imuiBorderCreateAll( 50.0f ) );

	imuiWidgetEnd( widget2 );

	imuiWidgetEnd( widget1 );

	imuiWindowEnd( window );
	imuiSurfaceEnd( surface );

	const ImuiRect widget1Rect = imuiWidgetGetRect( widget1 );
	IMUI_CHECK( widget1Rect.pos.x == 50.0f );
	IMUI_CHECK( widget1Rect.pos.y == 50.0f );
	IMUI_CHECK( widget1Rect.size.width == 400.0f );
	IMUI_CHECK( widget1Rect.size.height == 400.0f );

	return 0;
}

static int runLayoutTest( ImuiFrame* frame, ImuiTestSetLayoutFunc setLayoutCallback, ImuiTestSetSizeFunc setSizeCallback, ImuiTestGetSizeFunc getSizeFunc )
{
	ImuiSurface* surface = imuiSurfaceBegin( frame, "layout", imuiSizeCreate( 400.0f, 400.0f ), NULL, 1.0f );
	ImuiWindow* window = imuiWindowBegin( surface, "layout", imuiSurfaceGetRect( surface ), 1 );

	ImuiWidget* root = imuiWidgetBegin( window );
	imuiWidgetSetStretchOne( root );
	setLayoutCallback( root );

	ImuiWidget* layout1 = imuiWidgetBegin( window );
	imuiWidgetSetStretchOne( layout1 );
	setLayoutCallback( layout1 );
	setSizeCallback( layout1, 100.0f );
	doUiLayoutChilds( window, setSizeCallback, 0.0f, 10.0f, 20.0f, 5.0f );
	imuiWidgetEnd( layout1 );

	ImuiWidget* layout2 = imuiWidgetBegin( window );
	imuiWidgetSetStretchOne( layout2 );
	setLayoutCallback( layout2 );
	setSizeCallback( layout1, 200.0f );
	doUiLayoutChilds( window, setSizeCallback, 5.0f, 10.0f, 20.0f, 0.0f );
	imuiWidgetEnd( layout2 );

	imuiWidgetEnd( root );

	imuiWindowEnd( window );
	imuiSurfaceEnd( surface );

	//const ImuiRect layout1Rect = imuiWidgetGetRect( layout1 );
	//const ImuiRect layout2Rect = imuiWidgetGetRect( layout2 );
	IMUI_CHECK( getSizeFunc( layout1 ) == 200.0f );
	IMUI_CHECK( getSizeFunc( layout2 ) == 200.0f );

	ImuiWidget* child1 = imuiWidgetGetFirstChild( layout1 );
	ImuiWidget* child2 = imuiWidgetGetNextSibling( child1 );
	ImuiWidget* child3 = imuiWidgetGetNextSibling( child2 );
	ImuiWidget* child4 = imuiWidgetGetNextSibling( child3 );
	IMUI_CHECK( getSizeFunc( child1 ) == 50.0f );
	IMUI_CHECK( getSizeFunc( child2 ) == 50.0f );
	IMUI_CHECK( getSizeFunc( child3 ) == 50.0f );
	IMUI_CHECK( getSizeFunc( child4 ) == 50.0f );

	return 0;
}

static void doUiLayoutChilds( ImuiWindow* window, ImuiTestSetSizeFunc setSizeCallback, float childSize1, float childSize2, float childSize3, float childSize4 )
{
	ImuiWidget* child1 = imuiWidgetBegin( window );
	imuiWidgetSetStretchOne( child1 );
	setSizeCallback( child1, childSize1 );
	imuiWidgetEnd( child1 );

	ImuiWidget* child2 = imuiWidgetBegin( window );
	imuiWidgetSetStretchOne( child2 );
	setSizeCallback( child2, childSize2 );
	imuiWidgetEnd( child2 );

	ImuiWidget* child3 = imuiWidgetBegin( window );
	imuiWidgetSetStretchOne( child3 );
	setSizeCallback( child3, childSize3 );
	imuiWidgetEnd( child3 );

	ImuiWidget* child4 = imuiWidgetBegin( window );
	imuiWidgetSetStretchOne( child4 );
	setSizeCallback( child4, childSize4 );
	imuiWidgetEnd( child4 );
}
