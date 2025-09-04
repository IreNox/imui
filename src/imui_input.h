#pragma once

#include "imui/imui.h"

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

typedef struct ImUiInputState ImUiInputState;
struct ImUiInputState
{
	ImUiPos							focusDirection;
	bool							focusExecute;

	bool							mouseButtons[ ImUiInputMouseButton_MAX ];
	bool							mouseButtonDoubleClick[ ImUiInputMouseButton_MAX ];
	ImUiPos							mousePos;
	ImUiPos							mouseScroll;
	ImUiInputMouseCursor			mouseCursor;

	bool							keys[ ImUiInputKey_MAX ];
	uint32_t						keyModifiers;

	ImUiInputText					text;

	ImUiInputShortcut				shortcut;
};

struct ImUiInput
{
	ImUiAllocator*					allocator;

	const ImUiInputShortcutConfig*	shortcuts;
	size_t							shortcutCount;

	ImUiInputState					currentState;
	ImUiInputState					lastState;

	ImUiInputText					copyText;
	ImUiInputText					pasteText;
};

bool								ImUiInputConstruct( ImUiInput* input, ImUiAllocator* allocator, const ImUiInputShortcutConfig* shortcuts, size_t shortcutCount );
void								ImUiInputDestruct( ImUiInput* input );

void								ImUiInputNextTick( ImUiInput* input );
