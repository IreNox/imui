#pragma once

#include "imui/imui.h"

#ifdef __cplusplus
extern "C"
{
#endif

bool					ImUiHelloWorldSampleInitialize( ImUiContext* imui );
void					ImUiHelloWorldSampleShutdown( ImUiContext* imui );
void					ImUiHelloWorldSampleTick( ImUiSurface* surface );

bool					ImUiLayoutSampleInitialize( ImUiContext* imui );
void					ImUiLayoutSampleShutdown( ImUiContext* imui );
void					ImUiLayoutSampleTick( ImUiSurface* surface );

bool					ImUiInputSampleInitialize( ImUiContext* imui );
void					ImUiInputSampleShutdown( ImUiContext* imui );
void					ImUiInputSampleTick( ImUiSurface* surface );

bool					ImUiToolboxSampleInitialize( ImUiContext* imui );
void					ImUiToolboxSampleShutdown( ImUiContext* imui );
void					ImUiToolboxSampleTick( ImUiSurface* surface );

bool					ImUiToolboxCppSampleInitialize( ImUiContext* imui );
void					ImUiToolboxCppSampleShutdown( ImUiContext* imui );
void					ImUiToolboxCppSampleTick( ImUiSurface* surface );

#ifdef __cplusplus
}
#endif
