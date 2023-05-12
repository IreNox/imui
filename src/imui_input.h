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
	ImUiPosition	mousePosition;
	bool			mouseButtons[ ImUiInputMouseButton_MAX ];
	ImUiPosition	mouseScroll;

	bool			keys[ ImUiInputKey_MAX ];
	uint32_t		keyModifiers;

	ImUiInputText	text;
	size_t			textCapacity;
	size_t			textSize;
};

struct ImUiInput
{
	ImUiAllocator*	allocator;

	ImUiInputState	currentState;
	ImUiInputState	lastState;
};

void				ImUiInputConstruct( ImUiInput* input, ImUiAllocator* allocator );
void				ImUiInputDestruct( ImUiInput* input );

void				ImUiInputNextTick( ImUiInput* input );