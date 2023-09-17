#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct ImUiContext ImUiContext;
typedef struct ImUiSurface ImUiSurface;

typedef struct ImUiFrameworkTexture ImUiFrameworkTexture;

bool					ImUiFrameworkInitialize( ImUiContext* imui );
void					ImUiFrameworkShutdown( ImUiContext* imui );
void					ImUiFrameworkTick( ImUiSurface* surface );

ImUiFrameworkTexture*	ImUiFrameworkTextureCreate( void* textureData, uint32_t width, uint32_t height, bool isFont );
void					ImUiFrameworkTextureDestroy( ImUiFrameworkTexture* texture );
