#include "tests.h"

#include <math.h>

// Window coordinates are physical pixels; widget sizes, borders and spacing
// are logical units. Keep the entire surface and window rectangle fixed
// across DPI values so increased UI scaling consumes more of the same space.
static ImuiSurface* beginLayoutSurface( ImuiFrame* frame, const char* name, float dpiScale, float width, float height )
{
	char surfaceName[ 128u ];
	snprintf( surfaceName, sizeof( surfaceName ), "%s_%.2f", name, dpiScale );
	return imuiSurfaceBegin( frame, surfaceName, imuiSizeCreate( width + 26.0f, height + 34.0f ), NULL, dpiScale );
}

static ImuiWindow* beginLayoutWindow( ImuiSurface* surface, float width, float height )
{
	return imuiWindowBegin( surface, "window", imuiRectCreate( 13.0f, 17.0f, width, height ), 1u );
}

static int checkRect( const ImuiWidget* widget, const char* scenario, const char* name, float dpiScale, ImuiRect expected )
{
	const ImuiRect actual = imuiWidgetGetRect( widget );
	if( actual.pos.x != expected.pos.x || actual.pos.y != expected.pos.y ||
		actual.size.width != expected.size.width || actual.size.height != expected.size.height )
	{
		fprintf( stderr, "%s / %s (DPI %.2f): expected rect (%g, %g, %g, %g), got (%g, %g, %g, %g)\n",
			scenario, name, dpiScale,
			expected.pos.x, expected.pos.y, expected.size.width, expected.size.height,
			actual.pos.x, actual.pos.y, actual.size.width, actual.size.height );
	}
	IMUI_CHECK( actual.pos.x == expected.pos.x );
	IMUI_CHECK( actual.pos.y == expected.pos.y );
	IMUI_CHECK( actual.size.width == expected.size.width );
	IMUI_CHECK( actual.size.height == expected.size.height );
	return 0;
}

// Expected physical offsets within the fixed window.
static int checkPixelLayoutRect( const ImuiWidget* widget, const char* scenario, const char* name, float dpiScale, float x, float y, float width, float height )
{
	return checkRect( widget, scenario, name, dpiScale, imuiRectCreate(
		floorf( 13.0f + x ), floorf( 17.0f + y ), floorf( width ), floorf( height ) ) );
}

// For geometry determined entirely by logical widget sizes and borders.
static int checkLayoutRect( const ImuiWidget* widget, const char* scenario, const char* name, float dpiScale, float x, float y, float width, float height )
{
	return checkRect( widget, scenario, name, dpiScale, imuiRectCreate(
		floorf( 13.0f + x * dpiScale ), floorf( 17.0f + y * dpiScale ),
		floorf( width * dpiScale ), floorf( height * dpiScale ) ) );
}

static int runSpacingTest( ImuiFrame* frame, float dpiScale, bool vertical )
{
	const char* scenario = vertical ? "layout_vertical" : "layout_horizontal";
	ImuiSurface* surface = beginLayoutSurface( frame, scenario, dpiScale, 300.0f, 300.0f );
	ImuiWindow* window = beginLayoutWindow( surface, 300.0f, 300.0f );
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
	int failed = checkLayoutRect( first, scenario, "first", dpiScale, 0.0f, 0.0f, 30.0f, 20.0f );
	// Siblings start after the first child's already pixel-rounded size.
	const ImuiRect secondRect = imuiRectCreate(
		vertical ? 13.0f : floorf( 13.0f + floorf( 30.0f * dpiScale ) + 7.0f * dpiScale ),
		vertical ? floorf( 17.0f + floorf( 20.0f * dpiScale ) + 7.0f * dpiScale ) : 17.0f,
		floorf( 40.0f * dpiScale ), floorf( 50.0f * dpiScale ) );
	failed += checkRect( second, scenario, "second", dpiScale, secondRect );
	return failed;
}

static int runAlignmentTest( ImuiFrame* frame, float dpiScale )
{
	const char* scenario = "layout_alignment";
	ImuiSurface* surface = beginLayoutSurface( frame, scenario, dpiScale, 200.0f, 100.0f );
	ImuiWindow* window = beginLayoutWindow( surface, 200.0f, 100.0f );
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
	return checkPixelLayoutRect( child, scenario, "child", dpiScale, 100.0f - 20.0f * dpiScale, 100.0f - 30.0f * dpiScale, 40.0f * dpiScale, 20.0f * dpiScale );
}

static int runStretchLimitTest( ImuiFrame* frame, float dpiScale )
{
	const char* scenario = "layout_stretch_limit";
	ImuiSurface* surface = beginLayoutSurface( frame, scenario, dpiScale, 200.0f, 200.0f );
	ImuiWindow* window = beginLayoutWindow( surface, 200.0f, 200.0f );
	ImuiWidget* widget = imuiWidgetBegin( window );
	imuiWidgetSetStretchOne( widget );
	imuiWidgetSetMinSizeFloat( widget, 30.0f, 40.0f );
	imuiWidgetSetMaxSizeFloat( widget, 80.0f, 90.0f );
	imuiWidgetEnd( widget );
	imuiWindowEnd( window );
	imuiSurfaceEnd( surface );

	IMUI_CHECK( imuiWidgetGetHStretch( widget ) == 1.0f );
	IMUI_CHECK( imuiWidgetGetVStretch( widget ) == 1.0f );
	return checkLayoutRect( widget, scenario, "widget", dpiScale, 0.0f, 0.0f, 80.0f, 90.0f );
}

static int runDeepStackTest( ImuiFrame* frame, float dpiScale )
{
	const char* scenario = "layout_deep_stack";
	ImuiSurface* surface = beginLayoutSurface( frame, scenario, dpiScale, 640.0f, 480.0f );
	ImuiWindow* window = beginLayoutWindow( surface, 640.0f, 480.0f );
	ImuiWidget* levels[ 5u ];
	for( size_t i = 0u; i < 5u; ++i )
	{
		levels[ i ] = imuiWidgetBegin( window );
		imuiWidgetSetStretchOne( levels[ i ] );
		imuiWidgetSetMargin( levels[ i ], imuiBorderCreate( 4.0f, 8.0f, 12.0f, 16.0f ) );
		imuiWidgetSetPadding( levels[ i ], imuiBorderCreate( 4.0f, 12.0f, 8.0f, 20.0f ) );
	}
	ImuiWidget* leaf = imuiWidgetBegin( window );
	imuiWidgetSetFixedSizeFloat( leaf, 24.0f, 16.0f );
	imuiWidgetSetAlign( leaf, 0.5f, 1.0f );
	imuiWidgetEnd( leaf );
	for( size_t i = 5u; i > 0u; --i )
	{
		imuiWidgetEnd( levels[ i - 1u ] );
	}
	imuiWindowEnd( window );
	imuiSurfaceEnd( surface );

	int failed = 0;
	// Every level consumes 24 + 32 horizontal and 16 + 12 vertical units.
	for( size_t i = 0u; i < 5u; ++i )
	{
		char name[ 32u ];
		snprintf( name, sizeof( name ), "level_%u", (unsigned)i );
		failed += checkPixelLayoutRect( levels[ i ], scenario, name, dpiScale,
			(8.0f + 20.0f * (float)i) * dpiScale, (4.0f + 8.0f * (float)i) * dpiScale,
			640.0f - (24.0f + 56.0f * (float)i) * dpiScale, 480.0f - (16.0f + 28.0f * (float)i) * dpiScale );
	}
	failed += checkPixelLayoutRect( leaf, scenario, "leaf", dpiScale, 320.0f - 52.0f * dpiScale, 480.0f - 116.0f * dpiScale, 24.0f * dpiScale, 16.0f * dpiScale );
	return failed;
}

static int runMixedHierarchyTest( ImuiFrame* frame, float dpiScale )
{
	const char* scenario = "layout_mixed_hierarchy";
	ImuiSurface* surface = beginLayoutSurface( frame, scenario, dpiScale, 320.0f, 240.0f );
	ImuiWindow* window = beginLayoutWindow( surface, 320.0f, 240.0f );
	ImuiWidget* root = imuiWidgetBegin( window );
	imuiWidgetSetStretchOne( root );
	imuiWidgetSetPadding( root, imuiBorderCreateAll( 8.0f ) );
	imuiWidgetSetLayoutVerticalSpacing( root, 8.0f );
	ImuiWidget* header = imuiWidgetBegin( window );
	imuiWidgetSetHStretch( header, 1.0f );
	imuiWidgetSetFixedHeight( header, 20.0f );
	imuiWidgetEnd( header );

	ImuiWidget* row = imuiWidgetBegin( window );
	imuiWidgetSetStretchOne( row );
	imuiWidgetSetLayoutHorizontalSpacing( row, 8.0f );
	ImuiWidget* sidebar = imuiWidgetBegin( window );
	imuiWidgetSetFixedWidth( sidebar, 64.0f );
	imuiWidgetSetVStretch( sidebar, 1.0f );
	imuiWidgetEnd( sidebar );
	ImuiWidget* panel = imuiWidgetBegin( window );
	imuiWidgetSetStretchOne( panel );
	imuiWidgetSetPadding( panel, imuiBorderCreateAll( 4.0f ) );
	ImuiWidget* column = imuiWidgetBegin( window );
	imuiWidgetSetStretchOne( column );
	imuiWidgetSetLayoutVerticalSpacing( column, 4.0f );
	ImuiWidget* toolbar = imuiWidgetBegin( window );
	imuiWidgetSetHStretch( toolbar, 1.0f );
	imuiWidgetSetFixedHeight( toolbar, 24.0f );
	imuiWidgetEnd( toolbar );
	ImuiWidget* content = imuiWidgetBegin( window );
	imuiWidgetSetStretchOne( content );
	ImuiWidget* leaf = imuiWidgetBegin( window );
	imuiWidgetSetFixedSizeFloat( leaf, 20.0f, 12.0f );
	imuiWidgetSetAlign( leaf, 1.0f, 1.0f );
	imuiWidgetEnd( leaf );
	imuiWidgetEnd( content );
	imuiWidgetEnd( column );
	imuiWidgetEnd( panel );
	imuiWidgetEnd( row );

	ImuiWidget* footer = imuiWidgetBegin( window );
	imuiWidgetSetHStretch( footer, 1.0f );
	imuiWidgetSetFixedHeight( footer, 12.0f );
	imuiWidgetEnd( footer );
	imuiWidgetEnd( root );
	imuiWindowEnd( window );
	imuiSurfaceEnd( surface );

	int failed = 0;
	failed += checkPixelLayoutRect( root, scenario, "root", dpiScale, 0.0f, 0.0f, 320.0f, 240.0f );
	failed += checkPixelLayoutRect( header, scenario, "header", dpiScale, 8.0f * dpiScale, 8.0f * dpiScale, 320.0f - 16.0f * dpiScale, 20.0f * dpiScale );
	failed += checkPixelLayoutRect( row, scenario, "row", dpiScale, 8.0f * dpiScale, 36.0f * dpiScale, 320.0f - 16.0f * dpiScale, 240.0f - 64.0f * dpiScale );
	failed += checkPixelLayoutRect( sidebar, scenario, "sidebar", dpiScale, 8.0f * dpiScale, 36.0f * dpiScale, 64.0f * dpiScale, 240.0f - 64.0f * dpiScale );
	failed += checkPixelLayoutRect( panel, scenario, "panel", dpiScale, 80.0f * dpiScale, 36.0f * dpiScale, 320.0f - 88.0f * dpiScale, 240.0f - 64.0f * dpiScale );
	failed += checkPixelLayoutRect( column, scenario, "column", dpiScale, 84.0f * dpiScale, 40.0f * dpiScale, 320.0f - 96.0f * dpiScale, 240.0f - 72.0f * dpiScale );
	failed += checkPixelLayoutRect( toolbar, scenario, "toolbar", dpiScale, 84.0f * dpiScale, 40.0f * dpiScale, 320.0f - 96.0f * dpiScale, 24.0f * dpiScale );
	failed += checkPixelLayoutRect( content, scenario, "content", dpiScale, 84.0f * dpiScale, 68.0f * dpiScale, 320.0f - 96.0f * dpiScale, 240.0f - 100.0f * dpiScale );
	failed += checkPixelLayoutRect( leaf, scenario, "leaf", dpiScale, 320.0f - 32.0f * dpiScale, 240.0f - 44.0f * dpiScale, 20.0f * dpiScale, 12.0f * dpiScale );
	failed += checkPixelLayoutRect( footer, scenario, "footer", dpiScale, 8.0f * dpiScale, 240.0f - 20.0f * dpiScale, 320.0f - 16.0f * dpiScale, 12.0f * dpiScale );
	return failed;
}

static int runWeightedHierarchyTest( ImuiFrame* frame, float dpiScale, bool vertical )
{
	const char* scenario = vertical ? "layout_weighted_vertical" : "layout_weighted_horizontal";
	ImuiSurface* surface = beginLayoutSurface( frame, scenario, dpiScale, 320.0f, 256.0f );
	ImuiWindow* window = beginLayoutWindow( surface, 320.0f, 256.0f );
	ImuiWidget* root = imuiWidgetBegin( window );
	imuiWidgetSetStretchOne( root );
	imuiWidgetSetPadding( root, imuiBorderCreateAll( 8.0f ) );
	ImuiWidget* layout = imuiWidgetBegin( window );
	imuiWidgetSetStretchOne( layout );
	if( vertical )
	{
		imuiWidgetSetLayoutVerticalSpacing( layout, 8.0f );
	}
	else
	{
		imuiWidgetSetLayoutHorizontalSpacing( layout, 8.0f );
	}
	ImuiWidget* fixed = imuiWidgetBegin( window );
	if( vertical )
	{
		imuiWidgetSetFixedHeight( fixed, 48.0f );
		imuiWidgetSetHStretch( fixed, 1.0f );
	}
	else
	{
		imuiWidgetSetFixedWidth( fixed, 48.0f );
		imuiWidgetSetVStretch( fixed, 1.0f );
	}
	imuiWidgetEnd( fixed );
	ImuiWidget* first = imuiWidgetBegin( window );
	imuiWidgetSetStretchOne( first );
	imuiWidgetEnd( first );
	ImuiWidget* last = imuiWidgetBegin( window );
	imuiWidgetSetStretch( last, vertical ? 1.0f : 3.0f, vertical ? 3.0f : 1.0f );
	imuiWidgetSetPadding( last, imuiBorderCreateAll( 4.0f ) );
	ImuiWidget* inner = imuiWidgetBegin( window );
	imuiWidgetSetStretchOne( inner );
	ImuiWidget* leaf = imuiWidgetBegin( window );
	imuiWidgetSetFixedSizeFloat( leaf, 16.0f, 12.0f );
	imuiWidgetSetAlign( leaf, 1.0f, 1.0f );
	imuiWidgetEnd( leaf );
	imuiWidgetEnd( inner );
	imuiWidgetEnd( last );
	imuiWidgetEnd( layout );
	imuiWidgetEnd( root );
	imuiWindowEnd( window );
	imuiSurfaceEnd( surface );

	// Subtract scaled padding, the fixed sibling and two gaps from the
	// unchanged window size, then divide the remaining pixels in a 1:3 ratio.
	const float innerWidth = 320.0f - 16.0f * dpiScale;
	const float innerHeight = 256.0f - 16.0f * dpiScale;
	const float firstSize = ((vertical ? 256.0f : 320.0f) - 80.0f * dpiScale) / 4.0f;
	const float lastPos = 72.0f * dpiScale + firstSize;
	const float lastSize = firstSize * 3.0f;
	int failed = 0;
	if( vertical )
	{
		failed += checkPixelLayoutRect( fixed, scenario, "fixed", dpiScale, 8.0f * dpiScale, 8.0f * dpiScale, innerWidth, 48.0f * dpiScale );
		failed += checkPixelLayoutRect( first, scenario, "first", dpiScale, 8.0f * dpiScale, 64.0f * dpiScale, innerWidth, firstSize );
		failed += checkPixelLayoutRect( last, scenario, "last", dpiScale, 8.0f * dpiScale, lastPos, innerWidth, lastSize );
		failed += checkPixelLayoutRect( inner, scenario, "inner", dpiScale, 12.0f * dpiScale, lastPos + 4.0f * dpiScale, innerWidth - 8.0f * dpiScale, lastSize - 8.0f * dpiScale );
	}
	else
	{
		failed += checkPixelLayoutRect( fixed, scenario, "fixed", dpiScale, 8.0f * dpiScale, 8.0f * dpiScale, 48.0f * dpiScale, innerHeight );
		failed += checkPixelLayoutRect( first, scenario, "first", dpiScale, 64.0f * dpiScale, 8.0f * dpiScale, firstSize, innerHeight );
		failed += checkPixelLayoutRect( last, scenario, "last", dpiScale, lastPos, 8.0f * dpiScale, lastSize, innerHeight );
		failed += checkPixelLayoutRect( inner, scenario, "inner", dpiScale, lastPos + 4.0f * dpiScale, 12.0f * dpiScale, lastSize - 8.0f * dpiScale, innerHeight - 8.0f * dpiScale );
	}
	failed += checkPixelLayoutRect( leaf, scenario, "leaf", dpiScale, 320.0f - 28.0f * dpiScale, 256.0f - 24.0f * dpiScale, 16.0f * dpiScale, 12.0f * dpiScale );
	return failed;
}

static int runGridHierarchyTest( ImuiFrame* frame, float dpiScale )
{
	const char* scenario = "layout_grid_hierarchy";
	ImuiSurface* surface = beginLayoutSurface( frame, scenario, dpiScale, 320.0f, 272.0f );
	ImuiWindow* window = beginLayoutWindow( surface, 320.0f, 272.0f );
	ImuiWidget* root = imuiWidgetBegin( window );
	imuiWidgetSetStretchOne( root );
	imuiWidgetSetPadding( root, imuiBorderCreateAll( 8.0f ) );
	ImuiWidget* grid = imuiWidgetBegin( window );
	imuiWidgetSetStretchOne( grid );
	imuiWidgetSetPadding( grid, imuiBorderCreateAll( 4.0f ) );
	imuiWidgetSetLayoutGrid( grid, 3u, 4.0f, 4.0f );
	ImuiWidget* cells[ 7u ];
	ImuiWidget* leaves[ 7u ];
	for( size_t i = 0u; i < 7u; ++i )
	{
		cells[ i ] = imuiWidgetBegin( window );
		imuiWidgetSetStretchOne( cells[ i ] );
		imuiWidgetSetPadding( cells[ i ], imuiBorderCreateAll( 4.0f ) );
		leaves[ i ] = imuiWidgetBegin( window );
		imuiWidgetSetFixedSizeFloat( leaves[ i ], 16.0f, 12.0f );
		imuiWidgetSetAlign( leaves[ i ], 1.0f, 1.0f );
		imuiWidgetEnd( leaves[ i ] );
		imuiWidgetEnd( cells[ i ] );
	}
	imuiWidgetEnd( grid );
	imuiWidgetEnd( root );
	imuiWindowEnd( window );
	imuiSurfaceEnd( surface );

	// Three equal columns and rows, including an incomplete final row.
	// Remove 24 logical units of padding and two 4-unit gaps from the
	// fixed window size before distributing the remaining physical pixels.
	const float cellWidth = (320.0f - 32.0f * dpiScale) / 3.0f;
	const float cellHeight = (272.0f - 32.0f * dpiScale) / 3.0f;
	int failed = checkPixelLayoutRect( grid, scenario, "grid", dpiScale, 8.0f * dpiScale, 8.0f * dpiScale, 320.0f - 16.0f * dpiScale, 272.0f - 16.0f * dpiScale );
	for( size_t i = 0u; i < 7u; ++i )
	{
		const float x = 12.0f * dpiScale + (cellWidth + 4.0f * dpiScale) * (float)(i % 3u);
		const float y = 12.0f * dpiScale + (cellHeight + 4.0f * dpiScale) * (float)(i / 3u);
		char name[ 32u ];
		snprintf( name, sizeof( name ), "cell_%u", (unsigned)i );
		failed += checkPixelLayoutRect( cells[ i ], scenario, name, dpiScale, x, y, cellWidth, cellHeight );
		snprintf( name, sizeof( name ), "leaf_%u", (unsigned)i );
		failed += checkPixelLayoutRect( leaves[ i ], scenario, name, dpiScale,
			floorf( x ) + floorf( cellWidth ) - 20.0f * dpiScale,
			floorf( y ) + floorf( cellHeight ) - 16.0f * dpiScale, 16.0f * dpiScale, 12.0f * dpiScale );
	}
	return failed;
}

static int runMinimumHierarchyTest( ImuiFrame* frame, float dpiScale )
{
	const char* scenario = "layout_minimum_hierarchy";
	ImuiSurface* surface = beginLayoutSurface( frame, scenario, dpiScale, 320.0f, 240.0f );
	ImuiWindow* window = beginLayoutWindow( surface, 320.0f, 240.0f );
	ImuiWidget* root = imuiWidgetBegin( window );
	imuiWidgetSetPadding( root, imuiBorderCreateAll( 4.0f ) );
	ImuiWidget* row = imuiWidgetBegin( window );
	imuiWidgetSetPadding( row, imuiBorderCreateAll( 4.0f ) );
	imuiWidgetSetLayoutHorizontalSpacing( row, 8.0f );
	ImuiWidget* column = imuiWidgetBegin( window );
	imuiWidgetSetPadding( column, imuiBorderCreateAll( 4.0f ) );
	imuiWidgetSetLayoutVerticalSpacing( column, 4.0f );
	ImuiWidget* branch = imuiWidgetBegin( window );
	imuiWidgetSetPadding( branch, imuiBorderCreateAll( 4.0f ) );
	ImuiWidget* leaf = imuiWidgetBegin( window );
	imuiWidgetSetFixedSizeFloat( leaf, 32.0f, 20.0f );
	imuiWidgetSetMargin( leaf, imuiBorderCreateAll( 4.0f ) );
	imuiWidgetEnd( leaf );
	imuiWidgetEnd( branch );
	ImuiWidget* columnSibling = imuiWidgetBegin( window );
	imuiWidgetSetFixedSizeFloat( columnSibling, 16.0f, 12.0f );
	imuiWidgetEnd( columnSibling );
	imuiWidgetEnd( column );
	ImuiWidget* rowSibling = imuiWidgetBegin( window );
	imuiWidgetSetFixedSizeFloat( rowSibling, 24.0f, 16.0f );
	imuiWidgetEnd( rowSibling );
	imuiWidgetEnd( row );
	imuiWidgetEnd( root );
	imuiWindowEnd( window );
	imuiSurfaceEnd( surface );

	// No ancestor has an explicit size or stretch: descendants determine
	// the size all the way back up to the root.
	int failed = 0;
	failed += checkLayoutRect( root, scenario, "root", dpiScale, 0.0f, 0.0f, 104.0f, 76.0f );
	failed += checkLayoutRect( row, scenario, "row", dpiScale, 4.0f, 4.0f, 96.0f, 68.0f );
	failed += checkLayoutRect( column, scenario, "column", dpiScale, 8.0f, 8.0f, 56.0f, 60.0f );
	failed += checkLayoutRect( branch, scenario, "branch", dpiScale, 12.0f, 12.0f, 48.0f, 36.0f );
	failed += checkLayoutRect( leaf, scenario, "leaf", dpiScale, 20.0f, 20.0f, 32.0f, 20.0f );
	failed += checkLayoutRect( columnSibling, scenario, "column_sibling", dpiScale, 12.0f, 52.0f, 16.0f, 12.0f );
	failed += checkLayoutRect( rowSibling, scenario, "row_sibling", dpiScale, 72.0f, 8.0f, 24.0f, 16.0f );
	return failed;
}

static int runScrollHierarchyTest( ImuiFrame* frame, float dpiScale )
{
	const char* scenario = "layout_scroll_hierarchy";
	ImuiSurface* surface = beginLayoutSurface( frame, scenario, dpiScale, 320.0f, 240.0f );
	ImuiWindow* window = beginLayoutWindow( surface, 320.0f, 240.0f );
	ImuiWidget* root = imuiWidgetBegin( window );
	imuiWidgetSetStretchOne( root );
	imuiWidgetSetPadding( root, imuiBorderCreateAll( 8.0f ) );
	imuiWidgetSetLayoutVerticalSpacing( root, 8.0f );
	ImuiWidget* viewport = imuiWidgetBegin( window );
	imuiWidgetSetFixedSizeFloat( viewport, 160.0f, 120.0f );
	imuiWidgetSetPadding( viewport, imuiBorderCreateAll( 4.0f ) );
	// Scroll offsets are supplied in physical pixels.
	imuiWidgetSetLayoutScroll( viewport, 8.0f * dpiScale, 12.0f * dpiScale );
	ImuiWidget* content = imuiWidgetBegin( window );
	imuiWidgetSetFixedSizeFloat( content, 240.0f, 180.0f );
	imuiWidgetSetPadding( content, imuiBorderCreateAll( 4.0f ) );
	imuiWidgetSetLayoutVerticalSpacing( content, 4.0f );
	ImuiWidget* first = imuiWidgetBegin( window );
	imuiWidgetSetFixedSizeFloat( first, 32.0f, 16.0f );
	ImuiWidget* leaf = imuiWidgetBegin( window );
	imuiWidgetSetFixedSizeFloat( leaf, 12.0f, 8.0f );
	imuiWidgetEnd( leaf );
	imuiWidgetEnd( first );
	ImuiWidget* second = imuiWidgetBegin( window );
	imuiWidgetSetFixedSizeFloat( second, 32.0f, 20.0f );
	imuiWidgetEnd( second );
	imuiWidgetEnd( content );
	imuiWidgetEnd( viewport );
	ImuiWidget* footer = imuiWidgetBegin( window );
	imuiWidgetSetFixedSizeFloat( footer, 20.0f, 12.0f );
	imuiWidgetEnd( footer );
	imuiWidgetEnd( root );
	imuiWindowEnd( window );
	imuiSurfaceEnd( surface );

	int failed = 0;
	failed += checkLayoutRect( viewport, scenario, "viewport", dpiScale, 8.0f, 8.0f, 160.0f, 120.0f );
	failed += checkLayoutRect( content, scenario, "content", dpiScale, 4.0f, 0.0f, 240.0f, 180.0f );
	failed += checkLayoutRect( first, scenario, "first", dpiScale, 8.0f, 4.0f, 32.0f, 16.0f );
	failed += checkLayoutRect( leaf, scenario, "leaf", dpiScale, 8.0f, 4.0f, 12.0f, 8.0f );
	failed += checkLayoutRect( second, scenario, "second", dpiScale, 8.0f, 24.0f, 32.0f, 20.0f );
	// Overflowing content must not enlarge the viewport or move its sibling.
	failed += checkLayoutRect( footer, scenario, "footer", dpiScale, 8.0f, 136.0f, 20.0f, 12.0f );
	return failed;
}

static int runNestedRoundingTest( ImuiFrame* frame )
{
	const char* scenario = "layout_nested_rounding";
	const float dpiScale = 1.25f;
	ImuiSurface* surface = beginLayoutSurface( frame, scenario, dpiScale, 100.0f, 80.0f );
	ImuiWindow* window = beginLayoutWindow( surface, 100.0f, 80.0f );
	ImuiWidget* root = imuiWidgetBegin( window );
	imuiWidgetSetStretchOne( root );
	imuiWidgetSetMargin( root, imuiBorderCreateAll( 1.0f ) );
	ImuiWidget* child = imuiWidgetBegin( window );
	imuiWidgetSetStretchOne( child );
	imuiWidgetSetMargin( child, imuiBorderCreateAll( 1.0f ) );
	ImuiWidget* leaf = imuiWidgetBegin( window );
	imuiWidgetSetFixedSizeFloat( leaf, 10.0f, 6.0f );
	imuiWidgetEnd( leaf );
	imuiWidgetEnd( child );
	imuiWidgetEnd( root );
	imuiWindowEnd( window );
	imuiSurfaceEnd( surface );

	// Hand-calculated physical rectangles: each level floors its geometry
	// before its descendants are positioned, rather than rounding only once.
	int failed = 0;
	failed += checkRect( root, scenario, "root", dpiScale, imuiRectCreate( 14.0f, 18.0f, 97.0f, 77.0f ) );
	failed += checkRect( child, scenario, "child", dpiScale, imuiRectCreate( 15.0f, 19.0f, 94.0f, 74.0f ) );
	failed += checkRect( leaf, scenario, "leaf", dpiScale, imuiRectCreate( 15.0f, 19.0f, 12.0f, 7.0f ) );
	return failed;
}

int runLayoutTests( ImuiFrame* frame )
{
	const float dpiScales[] = { 0.5f, 0.75f, 1.0f, 1.25f, 1.5f, 2.0f };
	int failed = 0;
	for( size_t i = 0u; i < sizeof( dpiScales ) / sizeof( dpiScales[ 0u ] ); ++i )
	{
		const float dpiScale = dpiScales[ i ];
		failed += runSpacingTest( frame, dpiScale, false );
		failed += runSpacingTest( frame, dpiScale, true );
		failed += runAlignmentTest( frame, dpiScale );
		failed += runStretchLimitTest( frame, dpiScale );
		failed += runDeepStackTest( frame, dpiScale );
		failed += runMixedHierarchyTest( frame, dpiScale );
		failed += runWeightedHierarchyTest( frame, dpiScale, false );
		failed += runWeightedHierarchyTest( frame, dpiScale, true );
		failed += runGridHierarchyTest( frame, dpiScale );
		failed += runMinimumHierarchyTest( frame, dpiScale );
		failed += runScrollHierarchyTest( frame, dpiScale );
	}
	failed += runNestedRoundingTest( frame );
	return failed;
}
