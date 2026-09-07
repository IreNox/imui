#include "tests.h"

// DISCLAIMER: This tests are written by AI.

static int runPositionTest( void )
{
	const ImuiPos pos = imuiPosCreate( -4.0f, 8.0f );
	ImuiPos result = imuiPosAdd( pos, 6.0f, -2.0f );
	IMUI_CHECK( result.x == 2.0f && result.y == 6.0f );
	result = imuiPosSubPos( result, imuiPosCreate( 6.0f, -2.0f ) );
	IMUI_CHECK( result.x == pos.x && result.y == pos.y );
	result = imuiPosScale( pos, -0.5f );
	IMUI_CHECK( result.x == 2.0f && result.y == -4.0f );
	result = imuiPosMin( pos, imuiPosCreate( 2.0f, 3.0f ) );
	IMUI_CHECK( result.x == -4.0f && result.y == 3.0f );
	result = imuiPosMax( pos, imuiPosCreate( 2.0f, 3.0f ) );
	IMUI_CHECK( result.x == 2.0f && result.y == 8.0f );
	return 0;
}

static int runSizeBorderTest( void )
{
	const ImuiBorder border = imuiBorderCreate( 2.0f, 3.0f, 5.0f, 7.0f );
	const ImuiSize size = imuiSizeCreate( 40.0f, 30.0f );
	ImuiSize result = imuiSizeShrinkBorder( size, border );
	IMUI_CHECK( result.width == 30.0f && result.height == 23.0f );
	result = imuiSizeExpandBorder( result, border );
	IMUI_CHECK( result.width == 40.0f && result.height == 30.0f );
	result = imuiBorderGetMinSize( border );
	IMUI_CHECK( result.width == 10.0f && result.height == 7.0f );
	const ImuiBorder scaled = imuiBorderScale( border, 2.0f );
	IMUI_CHECK( scaled.top == 4.0f && scaled.left == 6.0f );
	IMUI_CHECK( scaled.bottom == 10.0f && scaled.right == 14.0f );
	result = imuiSizeLerp2( imuiSizeCreateZero(), size, 0.25f, 0.5f );
	IMUI_CHECK( result.width == 10.0f && result.height == 15.0f );
	result = imuiSizeFloor( imuiSizeCreate( 2.75f, -1.25f ) );
	IMUI_CHECK( result.width == 2.0f && result.height == -2.0f );
	result = imuiSizeCeil( imuiSizeCreate( 2.75f, -1.25f ) );
	IMUI_CHECK( result.width == 3.0f && result.height == -1.0f );
	return 0;
}

static int runRectangleTest( void )
{
	const ImuiRect rect = imuiRectCreateCenter( 30.0f, 40.0f, 40.0f, 20.0f );
	IMUI_CHECK( rect.pos.x == 10.0f && rect.pos.y == 30.0f );
	IMUI_CHECK( imuiRectGetRight( rect ) == 50.0f );
	IMUI_CHECK( imuiRectGetBottom( rect ) == 50.0f );
	IMUI_CHECK( imuiRectIncludesPos( rect, imuiPosCreate( 10.0f, 30.0f ) ) );
	IMUI_CHECK( imuiRectIncludesPos( rect, imuiPosCreate( 50.0f, 50.0f ) ) );
	IMUI_CHECK( !imuiRectIncludesPos( rect, imuiPosCreate( 51.0f, 40.0f ) ) );
	const ImuiRect overlap = imuiRectCreate( 40.0f, 35.0f, 20.0f, 30.0f );
	IMUI_CHECK( imuiRectIntersectsRect( rect, overlap ) );
	ImuiRect result = imuiRectIntersection( rect, overlap );
	IMUI_CHECK( result.pos.x == 40.0f && result.pos.y == 35.0f );
	IMUI_CHECK( result.size.width == 10.0f && result.size.height == 15.0f );
	const ImuiRect separate = imuiRectCreate( 60.0f, 60.0f, 10.0f, 10.0f );
	IMUI_CHECK( !imuiRectIntersectsRect( rect, separate ) );
	result = imuiRectIntersection( rect, separate );
	IMUI_CHECK( result.size.width == 0.0f && result.size.height == 0.0f );
	result = imuiRectShrinkBorder( rect, imuiBorderCreateAll( 100.0f ) );
	IMUI_CHECK( result.size.width == 0.0f && result.size.height == 0.0f );
	return 0;
}

static int runColorTest( void )
{
	ImuiColor color = imuiColorCreate( 1u, 2u, 3u, 4u );
	IMUI_CHECK( color.red == 1u && color.green == 2u && color.blue == 3u && color.alpha == 4u );
	color = imuiColorCreateFloat( 0.0f, 0.5f, 1.0f, 0.25f );
	IMUI_CHECK( color.red == 0u && color.green == 128u && color.blue == 255u && color.alpha == 64u );
	color = imuiColorCreateWhiteA( 42u );
	IMUI_CHECK( color.red == 255u && color.green == 255u && color.blue == 255u && color.alpha == 42u );
	color = imuiColorCreateGrayA( 17u, 91u );
	IMUI_CHECK( color.red == 17u && color.green == 17u && color.blue == 17u && color.alpha == 91u );
	color = imuiColorCreateTransparentBlack();
	IMUI_CHECK( color.red == 0u && color.green == 0u && color.blue == 0u && color.alpha == 0u );
	return 0;
}

int runDataTypeTests( ImuiFrame* frame )
{
	(void)frame;
	int failed = 0;
	failed += runPositionTest();
	failed += runSizeBorderTest();
	failed += runRectangleTest();
	failed += runColorTest();
	return failed;
}
