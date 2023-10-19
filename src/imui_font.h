#pragma once

#include "imui/imui.h"

#include "imui_types.h"
#include "imui_helpers.h"

struct ImUiFont
{
	ImUiImage			image;
	ImUiFontCodepoint*	codepoints;
	uintsize			codepointCount;

	float				fontSize;
	float				lineGap;

	ImUiHashMap			codepointMap;
};
