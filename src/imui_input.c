#include "imui_input.h"

#include "imui_internal.h"
#include "imui_memory.h"

#include <math.h>
#include <string.h>

static char*		ImUiInputTextGet( ImUiInputText* text );
static const char*	ImUiInputTextGetRead( const ImUiInputText* text );
static void			ImUiInputTextFree( ImUiInput* input, ImUiInputText* text );
static bool			ImUiInputTextCheckCapacity( ImUiInput* input, ImUiInputText* text, uintsize requiredCapacity );
static bool			ImUiInputTextPush( ImUiInput* input, ImUiInputText* text, const char* string, uintsize length );

bool ImUiInputConstruct( ImUiInput* input, ImUiAllocator* allocator, const ImUiInputShortcutConfig* shortcuts, size_t shortcutCount )
{
	input->allocator = allocator;

	ImUiInputShortcutConfig* newShortcuts = NULL;
	if( shortcutCount > 0u )
	{
		newShortcuts = IMUI_MEMORY_ARRAY_NEW( allocator, ImUiInputShortcutConfig, shortcutCount );
		if( !newShortcuts )
		{
			return false;
		}

		memcpy( newShortcuts, shortcuts, sizeof( ImUiInputShortcutConfig ) * shortcutCount );
	}

	input->shortcuts		= newShortcuts;
	input->shortcutCount	= shortcutCount;

	ImUiInputTextFree( input, &input->copyText );
	ImUiInputTextFree( input, &input->pasteText );

	return true;
}

void ImUiInputDestruct( ImUiInput* input )
{
	// call end frame twice to release all states as states live for two ticks
	ImUiInputEndFrame( input );
	ImUiInputEndFrame( input );

	while( input->firstStateChunk )
	{
		ImUiInputStateChunk* chunk = input->firstStateChunk;
		input->firstStateChunk = chunk->nextChunk;

		ImUiMemoryFree( input->allocator, chunk );
	}

	ImUiInputTextFree( input, &input->copyText );
	ImUiInputTextFree( input, &input->pasteText );

	ImUiMemoryFree( input->allocator, input->shortcuts );

	input->allocator = NULL;
}

void ImUiInputEndFrame( ImUiInput* input )
{
	while( input->usedStates )
	{
		ImUiInputState* state = input->usedStates;
		input->usedStates = state->nextState;

		ImUiInputTextFree( input, &state->current.text );
		ImUiInputTextFree( input, &state->last.text );

		state->nextState = input->freeStates;
		input->freeStates = state;
	}

	input->usedStates = input->newStates;
	input->newStates = NULL;

	input->mouseCursor = ImUiInputMouseCursor_Arrow;
}

bool ImUiInputBeginState( ImUiInput* input, const ImUiInputState* previousState )
{
	assert( input->pushState == NULL );

	ImUiInputState* state;
	if( input->freeStates )
	{
		state = input->freeStates;
		input->freeStates = input->freeStates->nextState;
	}
	else
	{
		if( !input->firstStateChunk ||
			input->firstStateChunk->usedCount == IMUI_ARRAY_COUNT( input->firstStateChunk->states ) )
		{
			ImUiInputStateChunk* chunk = IMUI_MEMORY_NEW( input->allocator, ImUiInputStateChunk );
			if( !chunk )
			{
				return false;
			}

			chunk->nextChunk = input->firstStateChunk;
			chunk->usedCount = 0u;
			input->firstStateChunk = chunk;
		}

		state = &input->firstStateChunk->states[ input->firstStateChunk->usedCount ];
		input->firstStateChunk->usedCount++;
	}

	state->nextState = input->newStates;
	input->newStates = state;
	input->pushState = state;

	if( previousState )
	{
		state->current	= previousState->current;
		state->last		= previousState->current;

		memset( &state->current.text, 0, sizeof( state->current.text ) );
		memset( &state->last.text, 0, sizeof( state->last.text ) );
	}
	else
	{
		memset( &state->current, 0, sizeof( state->current ) );
		memset( &state->last, 0, sizeof( state->last ) );
	}

	ImUiInputTextFree( input, &state->current.text );
	ImUiInputTextFree( input, &state->last.text );

	state->current.focusExecute = false;

	for( uintsize i = 0u; i < ImUiInputMouseButton_MAX; ++i )
	{
		state->current.mouseButtonDoubleClick[ i ] = false;
	}

	state->current.mouseScroll = ImUiPosCreateZero();

	return true;
}

const ImUiInputState* ImUiInputEndState( ImUiInput* input )
{
	ImUiInputState* state = input->pushState;

	state->current.shortcut = ImUiInputShortcut_None;

	for( size_t i = 0u; i < input->shortcutCount; ++i )
	{
		const ImUiInputShortcutConfig* shortcut = &input->shortcuts[ i ];
		if( (state->current.keyModifiers & shortcut->modifiers) != shortcut->modifiers ||
			!ImUiInputHasKeyPressed( state, shortcut->key ) )
		{
			continue;
		}

		state->current.shortcut = shortcut->type;
		break;
	}

	input->pushState = NULL;
	return state;
}

ImUiInputMouseCursor ImUiInputGetMouseCursor( const ImUiContext* imui )
{
	return imui->input.mouseCursor;
}

void ImUiInputSetMouseCursor( ImUiContext* imui, ImUiInputMouseCursor cursor )
{
	imui->input.mouseCursor = cursor;
}

const char* ImUiInputGetCopyText( const ImUiContext* imui )
{
	return ImUiInputTextGetRead( &imui->input.copyText );
}

void ImUiInputSetCopyText( ImUiContext* imui, const char* text, size_t textLength )
{
	ImUiInputTextFree( &imui->input, &imui->input.copyText );
	ImUiInputTextPush( &imui->input, &imui->input.copyText, text, textLength );
}

const char* ImUiInputGetPasteText( const ImUiContext* imui )
{
	return ImUiInputTextGetRead( &imui->input.pasteText );
}

void ImUiInputSetPasteText( ImUiContext* imui, const char* text )
{
	ImUiInputTextFree( &imui->input, &imui->input.pasteText );
	ImUiInputTextPush( &imui->input, &imui->input.pasteText, text, strlen( text ) );
}

char* ImUiInputBeginWritePasteText( ImUiContext* imui, size_t maxLength )
{
	if( !ImUiInputTextCheckCapacity( &imui->input, &imui->input.pasteText, maxLength +1u ) )
	{
		return NULL;
	}

	return ImUiInputTextGet( &imui->input.pasteText );
}

void ImUiInputEndWritePasteText( ImUiContext* imui, size_t finalLength )
{
	IMUI_ASSERT( finalLength < imui->input.pasteText.capacity );

	char* buffer = ImUiInputTextGet( &imui->input.pasteText );
	buffer[ finalLength ] = '\0';

	imui->input.pasteText.length = finalLength;
}

void ImUiInputPushKeyDown( ImUiInput* input, ImUiInputKey key )
{
	if( key == ImUiInputKey_LeftShift )
	{
		input->pushState->current.keyModifiers |= ImUiInputModifier_LeftShift;
	}
	else if( key == ImUiInputKey_RightShift )
	{
		input->pushState->current.keyModifiers |= ImUiInputModifier_RightShift;
	}
	else if( key == ImUiInputKey_LeftControl )
	{
		input->pushState->current.keyModifiers |= ImUiInputModifier_LeftCtrl;
	}
	else if( key == ImUiInputKey_RightControl )
	{
		input->pushState->current.keyModifiers |= ImUiInputModifier_RightCtrl;
	}
	else if( key == ImUiInputKey_LeftAlt )
	{
		input->pushState->current.keyModifiers |= ImUiInputModifier_LeftAlt;
	}
	else if( key == ImUiInputKey_RightAlt )
	{
		input->pushState->current.keyModifiers |= ImUiInputModifier_RightAlt;
	}

	input->pushState->current.keys[ key ] = true;
}

void ImUiInputPushKeyUp( ImUiInput* input, ImUiInputKey key )
{
	if( key == ImUiInputKey_LeftShift )
	{
		input->pushState->current.keyModifiers &= ~ImUiInputModifier_LeftShift;
	}
	else if( key == ImUiInputKey_RightShift )
	{
		input->pushState->current.keyModifiers &= ~ImUiInputModifier_RightShift;
	}
	else if( key == ImUiInputKey_LeftControl )
	{
		input->pushState->current.keyModifiers &= ~ImUiInputModifier_LeftCtrl;
	}
	else if( key == ImUiInputKey_RightControl )
	{
		input->pushState->current.keyModifiers &= ~ImUiInputModifier_RightCtrl;
	}
	else if( key == ImUiInputKey_LeftAlt )
	{
		input->pushState->current.keyModifiers &= ~ImUiInputModifier_LeftAlt;
	}
	else if( key == ImUiInputKey_RightAlt )
	{
		input->pushState->current.keyModifiers &= ~ImUiInputModifier_RightAlt;
	}

	input->pushState->current.keys[ key ] = false;
}

void ImUiInputPushKeyRepeat( ImUiInput* input, ImUiInputKey key )
{
	// fake key repeat by setting last state to released so 'was pressed' trigger again
	input->pushState->last.keys[ key ] = false;
}

void ImUiInputPushText( ImUiInput* input, const char* text )
{
	ImUiInputTextPush( input, &input->pushState->current.text, text, strlen( text ) );
}

void ImUiInputPushTextChar( ImUiInput* input, uint32_t c )
{
	if( c > 0x1fffff )
	{
		// invalid character
		return;
	}
	else if( c >= 0x10000u )
	{
		char bytes[ 4u ];
		bytes[ 0u ]	= 0xf0 | (char)(c >> 18);
		bytes[ 1u ] = 0x80 | (char)((c >> 12) & 0x3f);
		bytes[ 2u ] = 0x80 | (char)((c >> 6) & 0x3f);
		bytes[ 3u ] = 0x80 | (char)(c & 0x3f);
		ImUiInputTextPush( input, &input->pushState->current.text, bytes, sizeof( bytes ) );
	}
	else if( c >= 0x800u )
	{
		char bytes[ 3u ];
		bytes[ 0u ]	= 0xe0 | (char)(c >> 12);
		bytes[ 1u ] = 0x80 | (char)((c >> 6) & 0x3f);
		bytes[ 2u ] = 0x80 | (char)(c & 0x3f);
		ImUiInputTextPush( input, &input->pushState->current.text, bytes, sizeof( bytes ) );
	}
	else if( c >= 0x80u )
	{
		char bytes[ 2u ];
		bytes[ 0u ]	= 0xc0 | (char)(c >> 6);
		bytes[ 1u ] = 0x80 | (char)(c & 0x3f);
		ImUiInputTextPush( input, &input->pushState->current.text, bytes, sizeof( bytes ) );
	}
	else
	{
		char bytes[ 1u ];
		bytes[ 0u ] = (char)c;
		ImUiInputTextPush( input, &input->pushState->current.text, bytes, sizeof( bytes ) );
	}
}

void ImUiInputPushMouseDown( ImUiInput* input, ImUiInputMouseButton button )
{
	input->pushState->current.mouseButtons[ button ] = true;
}

void ImUiInputPushMouseUp( ImUiInput* input, ImUiInputMouseButton button )
{
	input->pushState->current.mouseButtons[ button ] = false;
}

void ImUiInputPushMouseDoubleClick( ImUiInput* input, ImUiInputMouseButton button )
{
	input->pushState->current.mouseButtonDoubleClick[ button ] = true;
}

void ImUiInputPushMouseMove( ImUiInput* input, float x, float y )
{
	input->pushState->current.mousePos = ImUiPosCreate( x, y );
}

void ImUiInputPushMouseMoveDelta( ImUiInput* input, float deltaX, float deltaY )
{
	input->pushState->current.mousePos = ImUiPosAdd( input->pushState->current.mousePos, deltaX, deltaY );
}

void ImUiInputPushMouseScroll( ImUiInput* input, float horizontalOffset, float verticalOffset )
{
	input->pushState->current.mouseScroll = ImUiPosCreate( horizontalOffset, verticalOffset );
}

void ImUiInputPushMouseScrollDelta( ImUiInput* input, float horizontalDelta, float verticalDelta )
{
	input->pushState->current.mouseScroll = ImUiPosAddPos( input->pushState->current.mouseScroll, ImUiPosCreate( horizontalDelta, verticalDelta ) );
}

void ImUiInputPushDirection( ImUiInput* input, float x, float y )
{
	const float lengthSquare = (x * x) + (y * y);
	if( lengthSquare == 0.0f )
	{
		input->pushState->current.focusDirection = ImUiPosCreateZero();
		return;
	}

	const float length = sqrtf( lengthSquare );
	input->pushState->current.focusDirection.x = x / length;
	input->pushState->current.focusDirection.y = y / length;
}

void ImUiInputPushFocusExecute( ImUiInput* input )
{
	input->pushState->current.focusExecute = true;
}

uint32_t ImUiInputGetKeyModifiers( const ImUiInputState* input )
{
	return input->current.keyModifiers;
}

bool ImUiInputIsKeyDown( const ImUiInputState* input, ImUiInputKey key )
{
	return input->current.keys[ key ];
}

bool ImUiInputIsKeyUp( const ImUiInputState* input, ImUiInputKey key )
{
	return !input->current.keys[ key ];
}

bool ImUiInputHasKeyPressed( const ImUiInputState* input, ImUiInputKey key )
{
	return input->current.keys[ key ] && !input->last.keys[ key ];
}

bool ImUiInputHasKeyReleased( const ImUiInputState* input, ImUiInputKey key )
{
	return !input->current.keys[ key ] && input->last.keys[ key ];
}

ImUiInputShortcut ImUiInputGetShortcut( const ImUiInputState* input )
{
	return input->current.shortcut;
}

const char* ImUiInputGetText( const ImUiInputState* input )
{
	return ImUiInputTextGetRead( &input->current.text );
}

ImUiPos ImUiInputGetMousePos( const ImUiInputState* input )
{
	return input->current.mousePos;
}

bool ImUiInputIsMouseInRect( const ImUiInputState* input, ImUiRect rectangle )
{
	return ImUiRectIncludesPos( rectangle, input->current.mousePos );
}

bool ImUiInputIsMouseButtonDown( const ImUiInputState* input, ImUiInputMouseButton button )
{
	return input->current.mouseButtons[ button ];
}

bool ImUiInputIsMouseButtonUp( const ImUiInputState* input, ImUiInputMouseButton button )
{
	return !input->current.mouseButtons[ button ];
}

bool ImUiInputHasMouseButtonPressed( const ImUiInputState* input, ImUiInputMouseButton button )
{
	return input->current.mouseButtons[ button ] && !input->last.mouseButtons[ button ];
}

bool ImUiInputHasMouseButtonReleased( const ImUiInputState* input, ImUiInputMouseButton button )
{
	return !input->current.mouseButtons[ button ] && input->last.mouseButtons[ button ];
}

bool ImUiInputHasMouseButtonDoubleClicked( const ImUiInputState* input, ImUiInputMouseButton button )
{
	return input->current.mouseButtonDoubleClick[ button ];
}

ImUiPos ImUiInputGetMouseScrollDelta( const ImUiInputState* input )
{
	return input->current.mouseScroll;
}

ImUiPos ImUiInputGetDirection( const ImUiInputState* input )
{
	return input->current.focusDirection;
}

bool ImUiInputGetFocusExecute( const ImUiInputState* input )
{
	return input->current.focusExecute;
}

static char* ImUiInputTextGet( ImUiInputText* text )
{
	return text->capacity > sizeof( text->data.buffer ) ? text->data.pointer : text->data.buffer;
}

static const char* ImUiInputTextGetRead( const ImUiInputText* text )
{
	if( text->length == 0u )
	{
		return NULL;
	}
	else if( text->capacity > sizeof( text->data.buffer ) )
	{
		return text->data.pointer;
	}

	return text->data.buffer;
}

static void ImUiInputTextFree( ImUiInput* input, ImUiInputText* text )
{
	if( text->capacity > sizeof( text->data.buffer ) )
	{
		ImUiMemoryFree( input->allocator, text->data.pointer );
	}

	text->data.buffer[ 0u ]	= '\0';
	text->capacity			= sizeof( text->data.buffer );
	text->length			= 0u;
}

static bool ImUiInputTextCheckCapacity( ImUiInput* input, ImUiInputText* text, uintsize requiredCapacity )
{
	if( text->capacity >= requiredCapacity )
	{
		return true;
	}

	uintsize nextCapacity = text->capacity << 1u;
	while( nextCapacity < requiredCapacity )
	{
		nextCapacity <<= 1u;
	}

	if( text->capacity <= sizeof( text->data.buffer ) )
	{
		char* newText = (char*)ImUiMemoryAlloc( input->allocator, nextCapacity );
		if( !newText )
		{
			return false;
		}

		memcpy( newText, text->data.buffer, text->length );
		newText[ text->length ] = '\0';

		text->data.pointer	= newText;
		text->capacity		= nextCapacity;
	}
	else
	{
		char* newText = (char*)ImUiMemoryRealloc( input->allocator, text->data.pointer, text->capacity, nextCapacity );
		if( !newText )
		{
			return false;
		}

		text->data.pointer	= newText;
		text->capacity		= nextCapacity;
	}

	return true;
}

static bool ImUiInputTextPush( ImUiInput* input, ImUiInputText* text, const char* string, uintsize length )
{
	const uintsize requiredLength = text->length + length;
	if( !ImUiInputTextCheckCapacity( input, text, requiredLength + 1u ) )
	{
		return false;
	}

	if( text->capacity <= sizeof( text->data.buffer ) )
	{
		memcpy( text->data.buffer + text->length, string, length );
		text->length += length;
		text->data.buffer[ text->length ] = '\0';
	}
	else
	{
		memcpy( text->data.pointer + text->length, string, length );
		text->length += length;
		text->data.pointer[ text->length ] = '\0';
	}

	return true;
}