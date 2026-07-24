#pragma once

#include "imui/imui.h"

#include "imui_types.h"
#include "imui_helpers.h"

struct ImuiFont
{
	ImuiImage			image;

	ImuiFontCodepoint*	codepoints;
	uintsize			codepointCount;

	float				fontSize;
	float				lineGap;
	bool				isScalable;

	ImuiHashMap			codepointMap;
};
