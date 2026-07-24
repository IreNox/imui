#pragma once

#include "imui/imui.h"

#ifdef __cplusplus
extern "C"
{
#endif

bool					imuiHelloWorldSampleInitialize( ImuiContext* imui );
void					imuiHelloWorldSampleShutdown( ImuiContext* imui );
void					imuiHelloWorldSampleTick( ImuiWindow* window );

bool					imuiLayoutSampleInitialize( ImuiContext* imui );
void					imuiLayoutSampleShutdown( ImuiContext* imui );
void					imuiLayoutSampleTick( ImuiWindow* window );

bool					imuiInputSampleInitialize( ImuiContext* imui );
void					imuiInputSampleShutdown( ImuiContext* imui );
void					imuiInputSampleTick( ImuiWindow* window );

bool					imuiToolboxSampleInitialize( ImuiContext* imui );
void					imuiToolboxSampleShutdown( ImuiContext* imui );
void					imuiToolboxSampleTick( ImuiWindow* window );

bool					imuiToolboxCppSampleInitialize( ImuiContext* imui );
void					imuiToolboxCppSampleShutdown( ImuiContext* imui );
void					imuiToolboxCppSampleTick( ImuiWindow* window );

#ifdef __cplusplus
}
#endif
