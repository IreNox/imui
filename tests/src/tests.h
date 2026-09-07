#pragma once

#include "imui/imui.h"

#include <stdio.h>

#define IMUI_CHECK( expr )															\
	do {																			\
		if (!(expr))																\
		{																			\
			fprintf( stderr, "%s:%d: failed: %s\n", __FILE__, __LINE__, #expr );	\
			return 1;																\
		}																			\
	} while (0)

typedef void(*ImuiTestUiCallback)( ImuiFrame* frame );
typedef void(*ImuiTestCheckCallback)(ImuiFrame* frame);

extern ImuiContext* g_imuiContext;

int					runBaseTests( ImuiFrame* frame );
int					runDataTypeTests( ImuiFrame* frame );
int					runInputTests( ImuiFrame* frame );
int					runSurfaceTests( ImuiFrame* frame );
int					runWindowTests( ImuiFrame* frame );
int					runWidgetTests( ImuiFrame* frame );
int					runLayoutTests( ImuiFrame* frame );
