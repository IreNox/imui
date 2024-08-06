#pragma once

#include "imui/imui.h"

typedef union ImUiInputText ImUiInputText;
union ImUiInputText
{
	char*			pointer;
	char			buffer[ sizeof( char* ) ];
};

typedef struct ImUiInputState ImUiInputState;
struct ImUiInputState
{
	ImUiPos					mousePos;
	bool					mouseButtons[ ImUiInputMouseButton_MAX ];
	ImUiPos					mouseScroll;
	ImUiInputMouseCursor	mouseCursor;

	bool					keys[ ImUiInputKey_MAX ];
	uint32_t				keyModifiers;

	ImUiInputText			text;
	size_t					textCapacity;
	size_t					textSize;

	ImUiInputShortcut		shortcut;
};

struct ImUiInput
{
	ImUiAllocator*			allocator;

	const ImUiShortcut*		shortcuts;
	size_t					shortcutCount;

	ImUiInputState			currentState;
	ImUiInputState			lastState;
};

bool				ImUiInputConstruct( ImUiInput* input, ImUiAllocator* allocator, const ImUiShortcut* shortcuts, size_t shortcutCount );
void				ImUiInputDestruct( ImUiInput* input );

void				ImUiInputNextTick( ImUiInput* input );