#include "imui_input.h"

#include "imui_internal.h"
#include "imui_memory.h"

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

	ImUiMemoryFree( input->allocator, input->shortcuts );

	input->allocator = NULL;
}

void ImUiInputNextTick( ImUiInput* input )
{
	ImUiInputTextFree( input, &input->lastState.text );

	input->lastState = input->currentState;

	input->currentState.focusExecute = false;

	for( uintsize i = 0u; i < ImUiInputMouseButton_MAX; ++i )
	{
		input->currentState.mouseButtonDoubleClick[ i ] = false;
	}

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
		const ImUiInputShortcutConfig* shortcut = &imui->input.shortcuts[ i ];
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
		ImUiInputTextPush( input, &input->currentState.text, bytes, sizeof( bytes ) );
	}
	else if( c >= 0x800u )
	{
		char bytes[ 3u ];
		bytes[ 0u ]	= 0xe0 | (char)(c >> 12);
		bytes[ 1u ] = 0x80 | (char)((c >> 6) & 0x3f);
		bytes[ 2u ] = 0x80 | (char)(c & 0x3f);
		ImUiInputTextPush( input, &input->currentState.text, bytes, sizeof( bytes ) );
	}
	else if( c >= 0x80u )
	{
		char bytes[ 2u ];
		bytes[ 0u ]	= 0xc0 | (char)(c >> 6);
		bytes[ 1u ] = 0x80 | (char)(c & 0x3f);
		ImUiInputTextPush( input, &input->currentState.text, bytes, sizeof( bytes ) );
	}
	else
	{
		char bytes[ 1u ];
		bytes[ 0u ] = (char)c;
		ImUiInputTextPush( input, &input->currentState.text, bytes, sizeof( bytes ) );
	}
}

void ImUiInputPushMouseDown( ImUiInput* input, ImUiInputMouseButton button )
{
	input->currentState.mouseButtons[ button ] = true;
}

void ImUiInputPushMouseUp( ImUiInput* input, ImUiInputMouseButton button )
{
	input->currentState.mouseButtons[ button ] = false;
}

void ImUiInputPushMouseDoubleClick( ImUiInput* input, ImUiInputMouseButton button )
{
	input->currentState.mouseButtonDoubleClick[ button ] = true;
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

void ImUiInputPushFocusDirection( ImUiInput* input, float x, float y )
{
	const float length = sqrtf( (x * x) + (y * y) );
	input->currentState.focusDirection.x = x / length;
	input->currentState.focusDirection.y = y / length;
}

void ImUiInputPushFocusExecute( ImUiInput* input )
{
	input->currentState.focusExecute = true;
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

ImUiInputShortcut ImUiInputGetShortcut( const ImUiContext* imui )
{
	return imui->input.currentState.shortcut;
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

bool ImUiInputHasMouseButtonDoubleClicked( const ImUiContext* imui, ImUiInputMouseButton button )
{
	return imui->input.currentState.mouseButtonDoubleClick[ button ];
}

ImUiPos ImUiInputGetMouseScrollDelta( const ImUiContext* imui )
{
	return imui->input.currentState.mouseScroll;
}

ImUiPos ImUiInputGetFocusDirection( const ImUiContext* imui )
{
	return imui->input.currentState.focusDirection;
}

bool ImUiInputGetFocusExecute( const ImUiContext* imui )
{
	return imui->input.currentState.focusExecute;
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