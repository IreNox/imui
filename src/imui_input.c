#include "imui_input.h"

#include "imui_internal.h"
#include "imui_memory.h"

#include <string.h>

static void ImUiInputFreeText( ImUiInput* input, ImUiInputState* state );
static bool ImUiInputPushTextSize( ImUiInput* input, ImUiInputState* state, const char* text, uintsize size );

bool ImUiInputConstruct( ImUiInput* input, ImUiAllocator* allocator, const ImUiShortcut* shortcuts, size_t shortcutCount )
{
	input->allocator = allocator;

	ImUiShortcut* newShortcuts = NULL;
	if( shortcutCount > 0u )
	{
		newShortcuts = IMUI_MEMORY_ARRAY_NEW( allocator, ImUiShortcut, shortcutCount );
		if( !newShortcuts )
		{
			return false;
		}

		memcpy( newShortcuts, shortcuts, sizeof( ImUiShortcut ) * shortcutCount );
	}

	input->shortcuts		= newShortcuts;
	input->shortcutCount	= shortcutCount;

	ImUiInputFreeText( input, &input->currentState );
	ImUiInputFreeText( input, &input->lastState );

	return true;
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
	input->currentState.mouseScroll	= ImUiPosCreateZero();
	input->currentState.mouseCursor	= ImUiInputMouseCursor_Arrow;

	ImUiInputFreeText( input, &input->currentState );
}

ImUiInput* ImUiInputBegin( ImUiContext* imui )
{
	ImUiInputNextTick( &imui->input );
	return &imui->input;
}

void ImUiInputEnd( ImUiContext* imui )
{
	ImUiInputState* currentState = &imui->input.currentState;

	currentState->shortcut = ImUiInputShortcut_None;

	for( size_t i = 0u; i < imui->input.shortcutCount; ++i )
	{
		const ImUiShortcut* shortcut = &imui->input.shortcuts[ i ];
		if( (currentState->keyModifiers & shortcut->modifiers) != shortcut->modifiers ||
			!ImUiInputHasKeyPressed( imui, shortcut->key ) )
		{
			continue;
		}

		currentState->shortcut = shortcut->type;
		break;
	}
}

void ImUiInputSetMouseCursor( ImUiContext* imui, ImUiInputMouseCursor cursor )
{
	imui->input.currentState.mouseCursor = cursor;
}

void ImUiInputPushKeyDown( ImUiInput* input, ImUiInputKey key )
{
	if( key == ImUiInputKey_LeftShift )
	{
		input->currentState.keyModifiers |= ImUiInputModifier_LeftShift;
	}
	else if( key == ImUiInputKey_RightShift )
	{
		input->currentState.keyModifiers |= ImUiInputModifier_RightShift;
	}
	else if( key == ImUiInputKey_LeftControl )
	{
		input->currentState.keyModifiers |= ImUiInputModifier_LeftCtrl;
	}
	else if( key == ImUiInputKey_RightControl )
	{
		input->currentState.keyModifiers |= ImUiInputModifier_RightCtrl;
	}
	else if( key == ImUiInputKey_LeftAlt )
	{
		input->currentState.keyModifiers |= ImUiInputModifier_LeftAlt;
	}
	else if( key == ImUiInputKey_RightAlt )
	{
		input->currentState.keyModifiers |= ImUiInputModifier_RightAlt;
	}

	input->currentState.keys[ key ] = true;
}

void ImUiInputPushKeyUp( ImUiInput* input, ImUiInputKey key )
{
	if( key == ImUiInputKey_LeftShift )
	{
		input->currentState.keyModifiers &= ~ImUiInputModifier_LeftShift;
	}
	else if( key == ImUiInputKey_RightShift )
	{
		input->currentState.keyModifiers &= ~ImUiInputModifier_RightShift;
	}
	else if( key == ImUiInputKey_LeftControl )
	{
		input->currentState.keyModifiers &= ~ImUiInputModifier_LeftCtrl;
	}
	else if( key == ImUiInputKey_RightControl )
	{
		input->currentState.keyModifiers &= ~ImUiInputModifier_RightCtrl;
	}
	else if( key == ImUiInputKey_LeftAlt )
	{
		input->currentState.keyModifiers &= ~ImUiInputModifier_LeftAlt;
	}
	else if( key == ImUiInputKey_RightAlt )
	{
		input->currentState.keyModifiers &= ~ImUiInputModifier_RightAlt;
	}

	input->currentState.keys[ key ] = false;
}

void ImUiInputPushKeyRepeat( ImUiInput* input, ImUiInputKey key )
{
	// fake key repeat by setting last state to released so 'was pressed' trigger again
	input->lastState.keys[ key ] = false;
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
	input->currentState.mousePos = ImUiPosCreate( x, y );
}

void ImUiInputPushMouseMoveDelta( ImUiInput* input, float deltaX, float deltaY )
{
	input->currentState.mousePos = ImUiPosAdd( input->currentState.mousePos, deltaX, deltaY );
}

void ImUiInputPushMouseScroll( ImUiInput* input, float horizontalOffset, float verticalOffset )
{
	input->currentState.mouseScroll = ImUiPosCreate( horizontalOffset, verticalOffset );
}

void ImUiInputPushMouseScrollDelta( ImUiInput* input, float horizontalDelta, float verticalDelta )
{
	input->currentState.mouseScroll = ImUiPosAddPos( input->currentState.mouseScroll, ImUiPosCreate( horizontalDelta, verticalDelta ) );
}

uint32_t ImUiInputGetKeyModifiers( const ImUiContext* imui )
{
	return imui->input.currentState.keyModifiers;
}

bool ImUiInputIsKeyDown( const ImUiContext* imui, ImUiInputKey key )
{
	return imui->input.currentState.keys[ key ];
}

bool ImUiInputIsKeyUp( const ImUiContext* imui, ImUiInputKey key )
{
	return !imui->input.currentState.keys[ key ];
}

bool ImUiInputHasKeyPressed( const ImUiContext* imui, ImUiInputKey key )
{
	return imui->input.currentState.keys[ key ] && !imui->input.lastState.keys[ key ];
}

bool ImUiInputHasKeyReleased( const ImUiContext* imui, ImUiInputKey key )
{
	return !imui->input.currentState.keys[ key ] && imui->input.lastState.keys[ key ];
}

const char* ImUiInputGetText( const ImUiContext* imui )
{
	if( imui->input.currentState.textCapacity > sizeof( imui->input.currentState.text.buffer ) )
	{
		return imui->input.currentState.text.pointer;
	}

	return imui->input.currentState.text.buffer;
}

ImUiPos ImUiInputGetMousePos( const ImUiContext* imui )
{
	return imui->input.currentState.mousePos;
}

ImUiInputMouseCursor ImUiInputGetMouseCursor( ImUiContext* imui )
{
	return imui->input.currentState.mouseCursor;
}

bool ImUiInputIsMouseInRect( const ImUiContext* imui, ImUiRect rectangle )
{
	return ImUiRectIncludesPos( rectangle, imui->input.currentState.mousePos );
}

bool ImUiInputIsMouseButtonDown( const ImUiContext* imui, ImUiInputMouseButton button )
{
	return imui->input.currentState.mouseButtons[ button ];
}

bool ImUiInputIsMouseButtonUp( const ImUiContext* imui, ImUiInputMouseButton button )
{
	return !imui->input.currentState.mouseButtons[ button ];
}

bool ImUiInputHasMouseButtonPressed( const ImUiContext* imui, ImUiInputMouseButton button )
{
	return imui->input.currentState.mouseButtons[ button ] && !imui->input.lastState.mouseButtons[ button ];
}

bool ImUiInputHasMouseButtonReleased( const ImUiContext* imui, ImUiInputMouseButton button )
{
	return !imui->input.currentState.mouseButtons[ button ] && imui->input.lastState.mouseButtons[ button ];
}

ImUiPos ImUiInputGetMouseScrollDelta( const ImUiContext* imui )
{
	return imui->input.currentState.mouseScroll;
}

static void ImUiInputFreeText( ImUiInput* input, ImUiInputState* state )
{
	if( state->textCapacity > sizeof( state->text.buffer ) )
	{
		ImUiMemoryFree( input->allocator, state->text.pointer );
	}

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