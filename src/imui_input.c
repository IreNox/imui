#include "imui_input.h"

#include "imui_internal.h"
#include "imui_memory.h"

#include <string.h>

static char*		ImUiInputTextGet( ImUiInputText* text );
static const char*	ImUiInputTextGetRead( const ImUiInputText* text );
static void			ImUiInputTextFree( ImUiInput* input, ImUiInputText* text );
static bool			ImUiInputTextCheckCapacity( ImUiInput* input, ImUiInputText* text, uintsize requiredCapacity );
static bool			ImUiInputTextPush( ImUiInput* input, ImUiInputText* text, const char* string, uintsize length );

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

	ImUiInputTextFree( input, &input->currentState.text );
	ImUiInputTextFree( input, &input->lastState.text );
	ImUiInputTextFree( input, &input->copyText );
	ImUiInputTextFree( input, &input->pasteText );

	return true;
}

void ImUiInputDestruct( ImUiInput* input )
{
	ImUiInputTextFree( input, &input->currentState.text );
	ImUiInputTextFree( input, &input->lastState.text );
}

void ImUiInputNextTick( ImUiInput* input )
{
	ImUiInputTextFree( input, &input->lastState.text );

	input->lastState = input->currentState;
	input->currentState.mouseScroll	= ImUiPosCreateZero();
	input->currentState.mouseCursor	= ImUiInputMouseCursor_Arrow;

	ImUiInputTextFree( input, &input->currentState.text );
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
	ImUiInputTextPush( input, &input->currentState.text, text, strlen( text ) );
}

void ImUiInputPushTextChar( ImUiInput* input, char c )
{
	ImUiInputTextPush( input, &input->currentState.text, &c, 1u );
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
	return ImUiInputTextGetRead( &imui->input.currentState.text );
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

static char* ImUiInputTextGet( ImUiInputText* text )
{
	return text->capacity > sizeof( text->buffer ) ? text->pointer : text->buffer;
}

static const char* ImUiInputTextGetRead( const ImUiInputText* text )
{
	if( text->length == 0u )
	{
		return NULL;
	}
	else if( text->capacity > sizeof( text->buffer ) )
	{
		return text->pointer;
	}

	return text->buffer;
}

static void ImUiInputTextFree( ImUiInput* input, ImUiInputText* text )
{
	if( text->capacity > sizeof( text->buffer ) )
	{
		ImUiMemoryFree( input->allocator, text->pointer );
	}

	text->buffer[ 0u ]	= '\0';
	text->capacity		= sizeof( text->buffer );
	text->length		= 0u;
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

	if( text->capacity <= sizeof( text->buffer ) )
	{
		char* newText = (char*)ImUiMemoryAlloc( input->allocator, nextCapacity );
		if( !newText )
		{
			return false;
		}

		memcpy( newText, text->buffer, text->length );
		newText[ text->length ] = '\0';

		text->pointer	= newText;
		text->capacity	= nextCapacity;
	}
	else
	{
		char* newText = (char*)ImUiMemoryRealloc( input->allocator, text->pointer, text->capacity, nextCapacity );
		if( !newText )
		{
			return false;
		}

		text->pointer	= newText;
		text->capacity	= nextCapacity;
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

	if( text->capacity <= sizeof( text->buffer ) )
	{
		memcpy( text->buffer + text->length, string, length );
		text->length += length;
		text->buffer[ text->length ] = '\0';
	}
	else
	{
		memcpy( text->pointer + text->length, string, length );
		text->length += length;
		text->pointer[ text->length ] = '\0';
	}

	return true;
}