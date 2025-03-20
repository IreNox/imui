#pragma once

#include "imui/imui.h"

#ifdef __cplusplus
extern "C"
{
#endif

bool					ImUiHelloWorldSampleInitialize( ImUiContext* imui );
void					ImUiHelloWorldSampleShutdown( ImUiContext* imui );
void					ImUiHelloWorldSampleTick( ImUiWindow* window );

bool					ImUiLayoutSampleInitialize( ImUiContext* imui );
void					ImUiLayoutSampleShutdown( ImUiContext* imui );
void					ImUiLayoutSampleTick( ImUiWindow* window );

bool					ImUiInputSampleInitialize( ImUiContext* imui );
void					ImUiInputSampleShutdown( ImUiContext* imui );
void					ImUiInputSampleTick( ImUiWindow* window );

bool					ImUiToolboxSampleInitialize( ImUiContext* imui );
void					ImUiToolboxSampleShutdown( ImUiContext* imui );
void					ImUiToolboxSampleTick( ImUiWindow* window );

bool					ImUiToolboxCppSampleInitialize( ImUiContext* imui );
void					ImUiToolboxCppSampleShutdown( ImUiContext* imui );
void					ImUiToolboxCppSampleTick( ImUiWindow* window );

#ifdef __cplusplus
}
#endif
