#pragma once

#include "imui/imui.h"

#include "imui_types.h"

typedef struct ImuiInputText
{
	union imuiInputTextData
	{
		char*						pointer;
		char						buffer[ sizeof( char* ) ];
	}								data;
	size_t							capacity;
	size_t							length;
} ImuiInputText;

typedef struct ImuiInputData
{
	ImuiPos							focusDirection;
	bool							focusExecute;

	bool							mouseButtons[ ImuiInputMouseButton_MAX ];
	bool							mouseButtonDoubleClick[ ImuiInputMouseButton_MAX ];
	ImuiPos							mousePos;
	ImuiPos							mouseScroll;

	bool							keys[ ImuiInputKey_MAX ];
	uint32_t						keyModifiers;

	ImuiInputText					text;

	ImuiInputShortcut				shortcut;
} ImuiInputData;

typedef struct ImuiInputState
{
	ImuiInputState*					nextState;

	ImuiInputData					current;
	ImuiInputData					last;
} ImuiInputState;

typedef struct ImuiInputStateChunk
{
	struct ImuiInputStateChunk*		nextChunk;
	ImuiInputState					states[ IMUI_DEFAULT_INPUT_STATE_CHUNK_SIZE ];
	uintsize						usedCount;
} ImuiInputStateChunk;

struct ImuiInput
{
	ImuiAllocator*					allocator;

	const ImuiInputShortcutConfig*	shortcuts;
	size_t							shortcutCount;

	ImuiInputStateChunk*			firstStateChunk;

	ImuiInputState*					newStates;
	ImuiInputState*					usedStates;
	ImuiInputState*					freeStates;

	ImuiInputState*					pushState;

	ImuiInputText					copyText;
	ImuiInputText					pasteText;

	ImuiInputMouseCursor			mouseCursor;
};

bool								imuiInputConstruct( ImuiInput* input, ImuiAllocator* allocator, const ImuiInputShortcutConfig* shortcuts, size_t shortcutCount );
void								imuiInputDestruct( ImuiInput* input );

void								imuiInputEndFrame( ImuiInput* input );

bool								imuiInputBeginState( ImuiInput* input, const ImuiInputState* previousState );
const ImuiInputState*				imuiInputEndState( ImuiInput* input );
