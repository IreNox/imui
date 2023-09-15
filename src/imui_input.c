#include "imui_input.h"

#include "imui_internal.h"
#include "imui_memory.h"

#include <string.h>

static void ImUiInputFreeText( ImUiInput* input, ImUiInputState* state );
static bool ImUiInputPushTextSize( ImUiInput* input, ImUiInputState* state, const char* text, uintsize size );

void ImUiInputConstruct( ImUiInput* input, ImUiAllocator* allocator )
{
	input->allocator = allocator;
}

void ImUiInputDestruct( ImUiInput* input )
{
	ImUiInputFreeText( input, &input->currentState );
	ImUiInputFreeText( input, &input->lastState );
}

void ImUiInputNextTick( ImUiInput* input )
{
	ImUiInputFreeText( input, &input->lastState );

	input->lastState = input->currentState;
}

ImUiInput* ImUiInputBegin( ImUiContext* imui )
{
	ImUiInputNextTick( &imui->input );
	return &imui->input;
}

void ImUiInputEnd( ImUiContext* imui )
{
}

void ImUiInputPushKeyDown( ImUiInput* input, ImUiInputKey key )
{
	input->currentState.keys[ key ] = true;
}

void ImUiInputPushKeyUp( ImUiInput* input, ImUiInputKey key )
{
	input->currentState.keys[ key ] = false;
}

void ImUiInputPushKeyRepeate( ImUiInput* input, ImUiInputKey key )
{
	// ???
}

void ImUiInputPushText( ImUiInput* input, const char* text )
{
	ImUiInputPushTextSize( input, &input->currentState, text, strlen( text ) );
}

void ImUiInputPushTextChar( ImUiInput* input, char c )
{
	ImUiInputPushTextSize( input, &input->currentState, &c, 1u );
}

void ImUiInputPushMouseDown( ImUiInput* input, ImUiInputMouseButton button )
{
	input->currentState.mouseButtons[ button ] = true;
}

void ImUiInputPushMouseUp( ImUiInput* input, ImUiInputMouseButton button )
{
	input->currentState.mouseButtons[ button ] = false;
}

void ImUiInputPushMouseMove( ImUiInput* input, float x, float y )
{
	input->currentState.mousePosition = ImUiPositionCreate( x, y );
}

void ImUiInputPushMouseMoveDelta( ImUiInput* input, float deltaX, float deltaY )
{
	input->currentState.mousePosition = ImUiPositionAdd( input->currentState.mousePosition, deltaX, deltaY );
}

void ImUiInputPushMouseScroll( ImUiInput* input, float horizontalOffset, float verticalOffset )
{
	input->currentState.mouseScroll = ImUiPositionCreate( horizontalOffset, verticalOffset );
}

void ImUiInputPushMouseScrollDelta( ImUiInput* input, float horizontalDelta, float verticalDelta )
{
	input->currentState.mouseScroll = ImUiPositionAddPos( input->currentState.mouseScroll, ImUiPositionCreate( horizontalDelta, verticalDelta ) );
}

uint32_t ImUiInputGetKeyModifiers( ImUiContext* imui )
{
	return imui->input.currentState.keyModifiers;
}

bool ImUiInputIsKeyDown( ImUiContext* imui, ImUiInputKey key )
{
	return imui->input.currentState.keys[ key ];
}

bool ImUiInputIsKeyUp( ImUiContext* imui, ImUiInputKey key )
{
	return !imui->input.currentState.keys[ key ];
}

bool ImUiInputHasKeyPressed( ImUiContext* imui, ImUiInputKey key )
{
	return imui->input.currentState.keys[ key ] && !imui->input.lastState.keys[ key ];
}

bool ImUiInputHasKeyReleased( ImUiContext* imui, ImUiInputKey key )
{
	return !imui->input.currentState.keys[ key ] && imui->input.lastState.keys[ key ];
}

bool ImUiInputIsMouseInRectangle( ImUiContext* imui, ImUiRectangle rectangle )
{
	return ImUiRectangleIncludesPosition( rectangle, imui->input.currentState.mousePosition );
}

bool ImUiInputIsMouseButtonDown( ImUiContext* imui, ImUiInputMouseButton button )
{
	return imui->input.currentState.mouseButtons[ button ];
}

bool ImUiInputIsMouseButtonUp( ImUiContext* imui, ImUiInputMouseButton button )
{
	return !imui->input.currentState.mouseButtons[ button ];
}

bool ImUiInputHasMouseButtonPressed( ImUiContext* imui, ImUiInputMouseButton button )
{
	return imui->input.currentState.mouseButtons[ button ] && !imui->input.lastState.mouseButtons[ button ];
}

bool ImUiInputHasMouseButtonReleased( ImUiContext* imui, ImUiInputMouseButton button )
{
	return !imui->input.currentState.mouseButtons[ button ] && imui->input.lastState.mouseButtons[ button ];
}

void ImUiInputGetWidgetState( ImUiWidget* widget, ImUiInputWidgetState* target )
{
	ImUiInput* input = &widget->window->imui->input;

	target->isMouseOver			= ImUiRectangleIncludesPosition( widget->rectangle, input->currentState.mousePosition );
	target->isMouseDown			= target->isMouseOver && input->currentState.mouseButtons[ ImUiInputMouseButton_Left ];
	target->hasMouseReleased	= target->isMouseOver && !input->currentState.mouseButtons[ ImUiInputMouseButton_Left ] && input->lastState.mouseButtons[ ImUiInputMouseButton_Left ];
}

static void ImUiInputFreeText( ImUiInput* input, ImUiInputState* state )
{
	if( state->textCapacity <= sizeof( state->text.buffer ) )
	{
		return;
	}

	ImUiMemoryFree( input->allocator, state->text.pointer );
	state->text.buffer[ 0u ]	= '\0';
	state->textCapacity			= sizeof( state->text.buffer );
	state->textSize				= 0u;
}

static bool ImUiInputPushTextSize( ImUiInput* input, ImUiInputState* state, const char* text, uintsize size )
{
	const uintsize requiredSize = state->textSize + size;
	if( state->textCapacity < requiredSize + 1u )
	{
		const uintsize requiredCapacity = state->textCapacity << 1u;

		if( state->textCapacity <= sizeof( state->text.buffer ) )
		{
			char* newText = (char*)ImUiMemoryAlloc( input->allocator, requiredCapacity );
			if( !newText )
			{
				return false;
			}

			memcpy( newText, state->text.buffer, state->textSize );
			newText[ state->textSize ] = '\0';

			state->text.pointer	= newText;
			state->textCapacity = requiredCapacity;
		}
		else
		{
			char* newText = (char*)ImUiMemoryRealloc( input->allocator, state->text.pointer, state->textCapacity, requiredCapacity );
			if( !newText )
			{
				return false;
			}

			state->text.pointer	= newText;
			state->textCapacity	= requiredCapacity;
		}
	}

	if( state->textCapacity <= sizeof( state->text.buffer ) )
	{
		memcpy( state->text.buffer + state->textSize, text, size );
		state->textSize += size;
		state->text.buffer[ state->textSize ] = '\0';
	}
	else
	{
		memcpy( state->text.pointer + state->textSize, text, size );
		state->textSize += size;
		state->text.pointer[ state->textSize ] = '\0';
	}

	return true;
}