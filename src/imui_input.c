#include "imui_input.h"

#include "imui_internal.h"
#include "imui_memory.h"

#include <math.h>
#include <string.h>

static char*		imuiInputTextGet( ImuiInputText* text );
static const char*	imuiInputTextGetRead( const ImuiInputText* text );
static void			imuiInputTextFree( ImuiInput* input, ImuiInputText* text );
static bool			imuiInputTextCheckCapacity( ImuiInput* input, ImuiInputText* text, uintsize requiredCapacity );
static bool			imuiInputTextPush( ImuiInput* input, ImuiInputText* text, const char* string, uintsize length );

bool imuiInputConstruct( ImuiInput* input, ImuiAllocator* allocator, const ImuiInputShortcutConfig* shortcuts, size_t shortcutCount )
{
	input->allocator = allocator;

	ImuiInputShortcutConfig* newShortcuts = NULL;
	if( shortcutCount > 0u )
	{
		newShortcuts = IMUI_MEMORY_ARRAY_NEW( allocator, ImuiInputShortcutConfig, shortcutCount );
		if( !newShortcuts )
		{
			return false;
		}

		memcpy( newShortcuts, shortcuts, sizeof( ImuiInputShortcutConfig ) * shortcutCount );
	}

	input->shortcuts		= newShortcuts;
	input->shortcutCount	= shortcutCount;

	imuiInputTextFree( input, &input->copyText );
	imuiInputTextFree( input, &input->pasteText );

	return true;
}

void imuiInputDestruct( ImuiInput* input )
{
	// call end frame twice to release all states as states live for two ticks
	imuiInputEndFrame( input );
	imuiInputEndFrame( input );

	while( input->firstStateChunk )
	{
		ImuiInputStateChunk* chunk = input->firstStateChunk;
		input->firstStateChunk = chunk->nextChunk;

		imuiMemoryFree( input->allocator, chunk );
	}

	imuiInputTextFree( input, &input->copyText );
	imuiInputTextFree( input, &input->pasteText );

	imuiMemoryFree( input->allocator, input->shortcuts );

	input->allocator = NULL;
}

void imuiInputEndFrame( ImuiInput* input )
{
	while( input->usedStates )
	{
		ImuiInputState* state = input->usedStates;
		input->usedStates = state->nextState;

		imuiInputTextFree( input, &state->current.text );
		imuiInputTextFree( input, &state->last.text );

		state->nextState = input->freeStates;
		input->freeStates = state;
	}

	input->usedStates = input->newStates;
	input->newStates = NULL;

	input->mouseCursor = ImuiInputMouseCursor_Arrow;
}

bool imuiInputBeginState( ImuiInput* input, const ImuiInputState* previousState )
{
	assert( input->pushState == NULL );

	ImuiInputState* state;
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
			ImuiInputStateChunk* chunk = IMUI_MEMORY_NEW( input->allocator, ImuiInputStateChunk );
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

	imuiInputTextFree( input, &state->current.text );
	imuiInputTextFree( input, &state->last.text );

	state->current.focusExecute = false;

	for( uintsize i = 0u; i < ImuiInputMouseButton_MAX; ++i )
	{
		state->current.mouseButtonDoubleClick[ i ] = false;
	}

	state->current.mouseScroll = imuiPosCreateZero();

	return true;
}

const ImuiInputState* imuiInputEndState( ImuiInput* input )
{
	ImuiInputState* state = input->pushState;

	state->current.shortcut = ImuiInputShortcut_None;

	for( size_t i = 0u; i < input->shortcutCount; ++i )
	{
		const ImuiInputShortcutConfig* shortcut = &input->shortcuts[ i ];
		if( (state->current.keyModifiers & shortcut->modifiers) != shortcut->modifiers ||
			!imuiInputHasKeyPressed( state, shortcut->key ) )
		{
			continue;
		}

		state->current.shortcut = shortcut->type;
		break;
	}

	input->pushState = NULL;
	return state;
}

ImuiInputMouseCursor imuiInputGetMouseCursor( const ImuiContext* imui )
{
	return imui->input.mouseCursor;
}

void imuiInputSetMouseCursor( ImuiContext* imui, ImuiInputMouseCursor cursor )
{
	imui->input.mouseCursor = cursor;
}

const char* imuiInputGetCopyText( const ImuiContext* imui )
{
	return imuiInputTextGetRead( &imui->input.copyText );
}

void imuiInputSetCopyText( ImuiContext* imui, const char* text, size_t textLength )
{
	imuiInputTextFree( &imui->input, &imui->input.copyText );
	imuiInputTextPush( &imui->input, &imui->input.copyText, text, textLength );
}

const char* imuiInputGetPasteText( const ImuiContext* imui )
{
	return imuiInputTextGetRead( &imui->input.pasteText );
}

void imuiInputSetPasteText( ImuiContext* imui, const char* text )
{
	imuiInputTextFree( &imui->input, &imui->input.pasteText );
	imuiInputTextPush( &imui->input, &imui->input.pasteText, text, strlen( text ) );
}

char* imuiInputBeginWritePasteText( ImuiContext* imui, size_t maxLength )
{
	if( !imuiInputTextCheckCapacity( &imui->input, &imui->input.pasteText, maxLength +1u ) )
	{
		return NULL;
	}

	return imuiInputTextGet( &imui->input.pasteText );
}

void imuiInputEndWritePasteText( ImuiContext* imui, size_t finalLength )
{
	IMUI_ASSERT( finalLength < imui->input.pasteText.capacity );

	char* buffer = imuiInputTextGet( &imui->input.pasteText );
	buffer[ finalLength ] = '\0';

	imui->input.pasteText.length = finalLength;
}

void imuiInputPushKeyDown( ImuiInput* input, ImuiInputKey key )
{
	if( key == ImuiInputKey_LeftShift )
	{
		input->pushState->current.keyModifiers |= ImuiInputModifier_LeftShift;
	}
	else if( key == ImuiInputKey_RightShift )
	{
		input->pushState->current.keyModifiers |= ImuiInputModifier_RightShift;
	}
	else if( key == ImuiInputKey_LeftControl )
	{
		input->pushState->current.keyModifiers |= ImuiInputModifier_LeftCtrl;
	}
	else if( key == ImuiInputKey_RightControl )
	{
		input->pushState->current.keyModifiers |= ImuiInputModifier_RightCtrl;
	}
	else if( key == ImuiInputKey_LeftAlt )
	{
		input->pushState->current.keyModifiers |= ImuiInputModifier_LeftAlt;
	}
	else if( key == ImuiInputKey_RightAlt )
	{
		input->pushState->current.keyModifiers |= ImuiInputModifier_RightAlt;
	}

	input->pushState->current.keys[ key ] = true;
}

void imuiInputPushKeyUp( ImuiInput* input, ImuiInputKey key )
{
	if( key == ImuiInputKey_LeftShift )
	{
		input->pushState->current.keyModifiers &= ~ImuiInputModifier_LeftShift;
	}
	else if( key == ImuiInputKey_RightShift )
	{
		input->pushState->current.keyModifiers &= ~ImuiInputModifier_RightShift;
	}
	else if( key == ImuiInputKey_LeftControl )
	{
		input->pushState->current.keyModifiers &= ~ImuiInputModifier_LeftCtrl;
	}
	else if( key == ImuiInputKey_RightControl )
	{
		input->pushState->current.keyModifiers &= ~ImuiInputModifier_RightCtrl;
	}
	else if( key == ImuiInputKey_LeftAlt )
	{
		input->pushState->current.keyModifiers &= ~ImuiInputModifier_LeftAlt;
	}
	else if( key == ImuiInputKey_RightAlt )
	{
		input->pushState->current.keyModifiers &= ~ImuiInputModifier_RightAlt;
	}

	input->pushState->current.keys[ key ] = false;
}

void imuiInputPushKeyRepeat( ImuiInput* input, ImuiInputKey key )
{
	// fake key repeat by setting last state to released so 'was pressed' trigger again
	input->pushState->last.keys[ key ] = false;
}

void imuiInputPushText( ImuiInput* input, const char* text )
{
	imuiInputTextPush( input, &input->pushState->current.text, text, strlen( text ) );
}

void imuiInputPushTextChar( ImuiInput* input, uint32_t c )
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
		imuiInputTextPush( input, &input->pushState->current.text, bytes, sizeof( bytes ) );
	}
	else if( c >= 0x800u )
	{
		char bytes[ 3u ];
		bytes[ 0u ]	= 0xe0 | (char)(c >> 12);
		bytes[ 1u ] = 0x80 | (char)((c >> 6) & 0x3f);
		bytes[ 2u ] = 0x80 | (char)(c & 0x3f);
		imuiInputTextPush( input, &input->pushState->current.text, bytes, sizeof( bytes ) );
	}
	else if( c >= 0x80u )
	{
		char bytes[ 2u ];
		bytes[ 0u ]	= 0xc0 | (char)(c >> 6);
		bytes[ 1u ] = 0x80 | (char)(c & 0x3f);
		imuiInputTextPush( input, &input->pushState->current.text, bytes, sizeof( bytes ) );
	}
	else
	{
		char bytes[ 1u ];
		bytes[ 0u ] = (char)c;
		imuiInputTextPush( input, &input->pushState->current.text, bytes, sizeof( bytes ) );
	}
}

void imuiInputPushMouseDown( ImuiInput* input, ImuiInputMouseButton button )
{
	input->pushState->current.mouseButtons[ button ] = true;
}

void imuiInputPushMouseUp( ImuiInput* input, ImuiInputMouseButton button )
{
	input->pushState->current.mouseButtons[ button ] = false;
}

void imuiInputPushMouseDoubleClick( ImuiInput* input, ImuiInputMouseButton button )
{
	input->pushState->current.mouseButtonDoubleClick[ button ] = true;
}

void imuiInputPushMouseMove( ImuiInput* input, float x, float y )
{
	input->pushState->current.mousePos = imuiPosCreate( x, y );
}

void imuiInputPushMouseMoveDelta( ImuiInput* input, float deltaX, float deltaY )
{
	input->pushState->current.mousePos = imuiPosAdd( input->pushState->current.mousePos, deltaX, deltaY );
}

void imuiInputPushMouseScroll( ImuiInput* input, float horizontalOffset, float verticalOffset )
{
	input->pushState->current.mouseScroll = imuiPosCreate( horizontalOffset, verticalOffset );
}

void imuiInputPushMouseScrollDelta( ImuiInput* input, float horizontalDelta, float verticalDelta )
{
	input->pushState->current.mouseScroll = imuiPosAddPos( input->pushState->current.mouseScroll, imuiPosCreate( horizontalDelta, verticalDelta ) );
}

void imuiInputPushDirection( ImuiInput* input, float x, float y )
{
	const float lengthSquare = (x * x) + (y * y);
	if( lengthSquare == 0.0f )
	{
		input->pushState->current.focusDirection = imuiPosCreateZero();
		return;
	}

	const float length = sqrtf( lengthSquare );
	input->pushState->current.focusDirection.x = x / length;
	input->pushState->current.focusDirection.y = y / length;
}

void imuiInputPushFocusExecute( ImuiInput* input )
{
	input->pushState->current.focusExecute = true;
}

uint32_t imuiInputGetKeyModifiers( const ImuiInputState* input )
{
	return input->current.keyModifiers;
}

bool imuiInputIsKeyDown( const ImuiInputState* input, ImuiInputKey key )
{
	return input->current.keys[ key ];
}

bool imuiInputIsKeyUp( const ImuiInputState* input, ImuiInputKey key )
{
	return !input->current.keys[ key ];
}

bool imuiInputHasKeyPressed( const ImuiInputState* input, ImuiInputKey key )
{
	return input->current.keys[ key ] && !input->last.keys[ key ];
}

bool imuiInputHasKeyReleased( const ImuiInputState* input, ImuiInputKey key )
{
	return !input->current.keys[ key ] && input->last.keys[ key ];
}

ImuiInputShortcut imuiInputGetShortcut( const ImuiInputState* input )
{
	return input->current.shortcut;
}

const char* imuiInputGetText( const ImuiInputState* input )
{
	return imuiInputTextGetRead( &input->current.text );
}

ImuiPos imuiInputGetMousePos( const ImuiInputState* input )
{
	return input->current.mousePos;
}

bool imuiInputIsMouseInRect( const ImuiInputState* input, ImuiRect rectangle )
{
	return imuiRectIncludesPos( rectangle, input->current.mousePos );
}

bool imuiInputIsMouseButtonDown( const ImuiInputState* input, ImuiInputMouseButton button )
{
	return input->current.mouseButtons[ button ];
}

bool imuiInputIsMouseButtonUp( const ImuiInputState* input, ImuiInputMouseButton button )
{
	return !input->current.mouseButtons[ button ];
}

bool imuiInputHasMouseButtonPressed( const ImuiInputState* input, ImuiInputMouseButton button )
{
	return input->current.mouseButtons[ button ] && !input->last.mouseButtons[ button ];
}

bool imuiInputHasMouseButtonReleased( const ImuiInputState* input, ImuiInputMouseButton button )
{
	return !input->current.mouseButtons[ button ] && input->last.mouseButtons[ button ];
}

bool imuiInputHasMouseButtonDoubleClicked( const ImuiInputState* input, ImuiInputMouseButton button )
{
	return input->current.mouseButtonDoubleClick[ button ];
}

ImuiPos imuiInputGetMouseScrollDelta( const ImuiInputState* input )
{
	return input->current.mouseScroll;
}

ImuiPos imuiInputGetDirection( const ImuiInputState* input )
{
	return input->current.focusDirection;
}

bool imuiInputGetFocusExecute( const ImuiInputState* input )
{
	return input->current.focusExecute;
}

static char* imuiInputTextGet( ImuiInputText* text )
{
	return text->capacity > sizeof( text->data.buffer ) ? text->data.pointer : text->data.buffer;
}

static const char* imuiInputTextGetRead( const ImuiInputText* text )
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

static void imuiInputTextFree( ImuiInput* input, ImuiInputText* text )
{
	if( text->capacity > sizeof( text->data.buffer ) )
	{
		imuiMemoryFree( input->allocator, text->data.pointer );
	}

	text->data.buffer[ 0u ]	= '\0';
	text->capacity			= sizeof( text->data.buffer );
	text->length			= 0u;
}

static bool imuiInputTextCheckCapacity( ImuiInput* input, ImuiInputText* text, uintsize requiredCapacity )
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
		char* newText = (char*)imuiMemoryAlloc( input->allocator, nextCapacity );
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
		char* newText = (char*)imuiMemoryRealloc( input->allocator, text->data.pointer, text->capacity, nextCapacity );
		if( !newText )
		{
			return false;
		}

		text->data.pointer	= newText;
		text->capacity		= nextCapacity;
	}

	return true;
}

static bool imuiInputTextPush( ImuiInput* input, ImuiInputText* text, const char* string, uintsize length )
{
	const uintsize requiredLength = text->length + length;
	if( !imuiInputTextCheckCapacity( input, text, requiredLength + 1u ) )
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