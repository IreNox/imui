#include "tests.h"

#include <string.h>

static int runKeyboardTest( ImuiContext* context )
{
	ImuiInput* input = imuiInputBegin( context, NULL );
	imuiInputPushKeyDown( input, ImuiInputKey_A );
	imuiInputPushKeyDown( input, ImuiInputKey_LeftControl );
	const ImuiInputState* pressed = imuiInputEnd( context );
	IMUI_CHECK( imuiInputIsKeyDown( pressed, ImuiInputKey_A ) );
	IMUI_CHECK( !imuiInputIsKeyUp( pressed, ImuiInputKey_A ) );
	IMUI_CHECK( imuiInputHasKeyPressed( pressed, ImuiInputKey_A ) );
	IMUI_CHECK( !imuiInputHasKeyReleased( pressed, ImuiInputKey_A ) );
	IMUI_CHECK( imuiInputGetKeyModifiers( pressed ) == ImuiInputModifier_LeftCtrl );

	imuiInputBegin( context, pressed );
	const ImuiInputState* held = imuiInputEnd( context );
	IMUI_CHECK( imuiInputIsKeyDown( held, ImuiInputKey_A ) );
	IMUI_CHECK( !imuiInputHasKeyPressed( held, ImuiInputKey_A ) );

	input = imuiInputBegin( context, held );
	imuiInputPushKeyRepeat( input, ImuiInputKey_A );
	const ImuiInputState* repeated = imuiInputEnd( context );
	IMUI_CHECK( imuiInputHasKeyPressed( repeated, ImuiInputKey_A ) );

	input = imuiInputBegin( context, repeated );
	imuiInputPushKeyUp( input, ImuiInputKey_A );
	imuiInputPushKeyUp( input, ImuiInputKey_LeftControl );
	const ImuiInputState* released = imuiInputEnd( context );
	IMUI_CHECK( imuiInputIsKeyUp( released, ImuiInputKey_A ) );
	IMUI_CHECK( imuiInputHasKeyReleased( released, ImuiInputKey_A ) );
	IMUI_CHECK( !imuiInputHasKeyPressed( released, ImuiInputKey_A ) );
	IMUI_CHECK( imuiInputGetKeyModifiers( released ) == ImuiInputModifier_None );
	return 0;
}

static int runMouseTest( ImuiContext* context )
{
	ImuiInput* input = imuiInputBegin( context, NULL );
	imuiInputPushMouseMove( input, 10.0f, 20.0f );
	imuiInputPushMouseMoveDelta( input, -3.0f, 4.0f );
	imuiInputPushMouseDown( input, ImuiInputMouseButton_Left );
	imuiInputPushMouseDoubleClick( input, ImuiInputMouseButton_Left );
	imuiInputPushMouseScroll( input, 1.0f, -2.0f );
	imuiInputPushMouseScrollDelta( input, 2.0f, 1.0f );
	imuiInputPushFocusExecute( input );
	imuiInputPushDirection( input, 0.0f, -2.0f );
	const ImuiInputState* pressed = imuiInputEnd( context );
	const ImuiPos pos = imuiInputGetMousePos( pressed );
	IMUI_CHECK( pos.x == 7.0f && pos.y == 24.0f );
	IMUI_CHECK( imuiInputIsMouseInRect( pressed, imuiRectCreate( 0.0f, 0.0f, 10.0f, 30.0f ) ) );
	IMUI_CHECK( !imuiInputIsMouseInRect( pressed, imuiRectCreate( 30.0f, 30.0f, 10.0f, 10.0f ) ) );
	IMUI_CHECK( imuiInputIsMouseButtonDown( pressed, ImuiInputMouseButton_Left ) );
	IMUI_CHECK( imuiInputHasMouseButtonPressed( pressed, ImuiInputMouseButton_Left ) );
	IMUI_CHECK( imuiInputHasMouseButtonDoubleClicked( pressed, ImuiInputMouseButton_Left ) );
	IMUI_CHECK( imuiInputGetMouseScrollDelta( pressed ).x == 3.0f );
	IMUI_CHECK( imuiInputGetMouseScrollDelta( pressed ).y == -1.0f );
	IMUI_CHECK( imuiInputGetFocusExecute( pressed ) );
	IMUI_CHECK( imuiInputGetDirection( pressed ).x == 0.0f );
	IMUI_CHECK( imuiInputGetDirection( pressed ).y == -1.0f );

	imuiInputBegin( context, pressed );
	const ImuiInputState* held = imuiInputEnd( context );
	IMUI_CHECK( imuiInputIsMouseButtonDown( held, ImuiInputMouseButton_Left ) );
	IMUI_CHECK( !imuiInputHasMouseButtonPressed( held, ImuiInputMouseButton_Left ) );
	IMUI_CHECK( !imuiInputHasMouseButtonDoubleClicked( held, ImuiInputMouseButton_Left ) );
	IMUI_CHECK( !imuiInputGetFocusExecute( held ) );
	IMUI_CHECK( imuiInputGetMouseScrollDelta( held ).x == 0.0f );
	IMUI_CHECK( imuiInputGetMouseScrollDelta( held ).y == 0.0f );
	IMUI_CHECK( imuiInputGetMousePos( held ).x == 7.0f );

	input = imuiInputBegin( context, held );
	imuiInputPushMouseUp( input, ImuiInputMouseButton_Left );
	const ImuiInputState* released = imuiInputEnd( context );
	IMUI_CHECK( imuiInputIsMouseButtonUp( released, ImuiInputMouseButton_Left ) );
	IMUI_CHECK( imuiInputHasMouseButtonReleased( released, ImuiInputMouseButton_Left ) );
	return 0;
}

static int runTextInputTest( ImuiContext* context )
{
	ImuiInput* input = imuiInputBegin( context, NULL );
	imuiInputPushText( input, "Hello " );
	imuiInputPushTextChar( input, 0x00e4u );
	imuiInputPushTextChar( input, 0x20acu );
	imuiInputPushTextChar( input, 0x1f600u );
	const ImuiInputState* state = imuiInputEnd( context );
	IMUI_CHECK( imuiInputGetText( state ) != NULL );
	IMUI_CHECK( strcmp( imuiInputGetText( state ), "Hello \xc3\xa4\xe2\x82\xac\xf0\x9f\x98\x80" ) == 0 );
	imuiInputBegin( context, state );
	state = imuiInputEnd( context );
	IMUI_CHECK( imuiInputGetText( state ) == NULL );
	return 0;
}

static int runClipboardTest( ImuiContext* context )
{
	char text[ 257u ];
	memset( text, 'x', sizeof( text ) - 1u );
	text[ sizeof( text ) - 1u ] = '\0';
	imuiInputSetCopyText( context, text, sizeof( text ) - 1u );
	IMUI_CHECK( imuiInputGetCopyText( context ) != NULL );
	IMUI_CHECK( strcmp( imuiInputGetCopyText( context ), text ) == 0 );
	imuiInputSetCopyText( context, "shorter", 5u );
	IMUI_CHECK( strcmp( imuiInputGetCopyText( context ), "short" ) == 0 );
	imuiInputSetPasteText( context, text );
	IMUI_CHECK( imuiInputGetPasteText( context ) != NULL );
	IMUI_CHECK( strcmp( imuiInputGetPasteText( context ), text ) == 0 );
	char* buffer = imuiInputBeginWritePasteText( context, 8u );
	IMUI_CHECK( buffer != NULL );
	memcpy( buffer, "paste", 5u );
	imuiInputEndWritePasteText( context, 5u );
	IMUI_CHECK( strcmp( imuiInputGetPasteText( context ), "paste" ) == 0 );
	imuiInputSetCopyText( context, "", 0u );
	imuiInputSetPasteText( context, "" );
	IMUI_CHECK( imuiInputGetCopyText( context ) == NULL );
	IMUI_CHECK( imuiInputGetPasteText( context ) == NULL );
	return 0;
}

int runInputTests( ImuiFrame* frame )
{
	ImuiContext* context = imuiFrameGetContext( frame );
	int failed = 0;
	failed += runKeyboardTest( context );
	failed += runMouseTest( context );
	failed += runTextInputTest( context );
	failed += runClipboardTest( context );
	return failed;
}
