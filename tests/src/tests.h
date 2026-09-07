#pragma once

#include "imui/imui.h"

#include <stdio.h>

#if !defined( NDEBUG ) && defined( WIN32 )
void imuiTestDebugBreak();
#define IMUI_BREAK imuiTestDebugBreak()
#define IMUI_BREAK_ENABLED
#else
#define IMUI_BREAK
#endif

#define IMUI_CHECK( expr )															\
	do {																			\
		if (!(expr))																\
		{																			\
			fprintf( stderr, "%s:%d: failed: %s\n", __FILE__, __LINE__, #expr );	\
			IMUI_BREAK;																\
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
