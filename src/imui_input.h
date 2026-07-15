#pragma once

#include "imui/imui.h"

#include "imui_types.h"

typedef struct ImUiInputText
{
	union ImUiInputTextData
	{
		char*						pointer;
		char						buffer[ sizeof( char* ) ];
	}								data;
	size_t							capacity;
	size_t							length;
} ImUiInputText;

typedef struct ImUiInputData
{
	ImUiPos							focusDirection;
	bool							focusExecute;

	bool							mouseButtons[ ImUiInputMouseButton_MAX ];
	bool							mouseButtonDoubleClick[ ImUiInputMouseButton_MAX ];
	ImUiPos							mousePos;
	ImUiPos							mouseScroll;

	bool							keys[ ImUiInputKey_MAX ];
	uint32_t						keyModifiers;

	ImUiInputText					text;

	ImUiInputShortcut				shortcut;
} ImUiInputData;

typedef struct ImUiInputState
{
	ImUiInputState*					nextState;

	ImUiInputData					current;
	ImUiInputData					last;
} ImUiInputState;

typedef struct ImUiInputStateChunk
{
	struct ImUiInputStateChunk*		nextChunk;
	ImUiInputState					states[ IMUI_DEFAULT_INPUT_STATE_CHUNK_SIZE ];
	uintsize						usedCount;
} ImUiInputStateChunk;

struct ImUiInput
{
	ImUiAllocator*					allocator;

	const ImUiInputShortcutConfig*	shortcuts;
	size_t							shortcutCount;

	ImUiInputStateChunk*			firstStateChunk;

	ImUiInputState*					newStates;
	ImUiInputState*					usedStates;
	ImUiInputState*					freeStates;

	ImUiInputState*					pushState;

	ImUiInputText					copyText;
	ImUiInputText					pasteText;

	ImUiInputMouseCursor			mouseCursor;
};

bool								ImUiInputConstruct( ImUiInput* input, ImUiAllocator* allocator, const ImUiInputShortcutConfig* shortcuts, size_t shortcutCount );
void								ImUiInputDestruct( ImUiInput* input );

void								ImUiInputEndFrame( ImUiInput* input );

bool								ImUiInputBeginState( ImUiInput* input, const ImUiInputState* previousState );
const ImUiInputState*				ImUiInputEndState( ImUiInput* input );
