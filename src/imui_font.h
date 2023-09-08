#include "imui/imui.h"

#include "imui_types.h"
#include "imui_helpers.h"

struct ImUiFont
{
	ImUiTexture			texture;
	ImUiFontCodepoint*	codepoints;
	uintsize			codepointCount;

	ImUiHashTable		codepointMap;
};
