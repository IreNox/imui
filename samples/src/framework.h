#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include "imui/imui.h"

#include <stdbool.h>
#include <stdint.h>

typedef struct ImuiContext ImuiContext;
typedef struct ImuiFont ImuiFont;
typedef struct ImuiSkin ImuiSkin;
typedef struct ImuiSurface ImuiSurface;
typedef struct ImuiImage ImuiImage;

typedef struct ImuiFrameworkTexture ImuiFrameworkTexture;

typedef struct ImuiFrameworkToolboxConfigData
{
	ImuiFont*	font;
	ImuiImage	fontTexture;

	ImuiSkin	skinRect;
	ImuiImage	skinRectTexture;
	ImuiSkin	skinLine;
	ImuiImage	skinLineTexture;
} ImuiFrameworkToolboxConfigData;

bool					imuiFrameworkInitialize( ImuiContext* imui );
void					imuiFrameworkShutdown( ImuiContext* imui );
void					imuiFrameworkTick( ImuiSurface* surface );

void					imuiFrameworkSetDpiScale( float dpiScale );

ImuiFrameworkTexture*	imuiFrameworkTextureCreate( void* textureData, uint32_t width, uint32_t height, bool isFont );
void					imuiFrameworkTextureDestroy( ImuiFrameworkTexture* texture );

bool					imuiFrameworkFontCreate( ImuiFont** font, ImuiImage* image, const char* fontFilename, float fontSize );
void					imuiFrameworkFontDestroy( ImuiFont** font, ImuiImage* image );

bool					imuiFrameworkSkinCreate( ImuiSkin* skin, ImuiImage* image, uint32_t size, float radius, float factor, bool horizontal );
void					imuiFrameworkSkinDestroy( ImuiSkin* skin, ImuiImage* image );

bool					imuiFrameworkToolboxConfigDataInitialize( ImuiFrameworkToolboxConfigData* data, ImuiContext* imui );
void					imuiFrameworkToolboxConfigDataShutdown( ImuiFrameworkToolboxConfigData* data, ImuiContext* imui );
void					imuiFrameworkToolboxConfigDataApply( const ImuiFrameworkToolboxConfigData* data );

#ifdef __cplusplus
}
#endif
