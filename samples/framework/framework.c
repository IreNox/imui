#include "framework.h"

#include "imui/imui.h"

#include <GL/glew.h>
#if defined( __EMSCRIPTEN__ )
#	include <SDL2/SDL.h>
#else
#	include <SDL.h>
#endif

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#	include <crtdbg.h>
#elif defined( __EMSCRIPTEN__ )
#	include <emscripten.h>
#endif

#include "../../src/imui_types.h"

struct ImUiFrameworkTexture
{
	GLuint						handle;
	bool						isFont;
};

typedef struct ImUiFrameworkContext ImUiFrameworkContext;
struct ImUiFrameworkContext
{
	SDL_Window*					window;
	int							windowWidth;
	int							windowHeight;
	SDL_GLContext				glContext;

	GLuint						vertexShader;
	GLuint						fragmentShader;
	GLuint						fragmentShaderFont;

	GLuint						program;
	GLuint						programFont;
	GLint						programUniformProjection;
	GLint						programUniformTexture;

	GLuint						vertexArray;
	GLuint						vertexBuffer;
	GLuint						elementBuffer;

	ImUiFrameworkTexture			whiteTexture;

	ImUiContext*				imui;
};

static ImUiFrameworkContext s_context;
static bool s_running;

static ImUiInputKey s_inputKeyMapping[ SDL_NUM_SCANCODES ];
static const ImUiInputMouseButton s_inputMouseButtonMapping[] =
{
	ImUiInputMouseButton_MAX,		// Invalid 0
	ImUiInputMouseButton_Left,		// SDL_BUTTON_LEFT     1
	ImUiInputMouseButton_Middle,	// SDL_BUTTON_MIDDLE   2
	ImUiInputMouseButton_Right,		// SDL_BUTTON_RIGHT    3
	ImUiInputMouseButton_X1,		// SDL_BUTTON_X1       4
	ImUiInputMouseButton_X2			// SDL_BUTTON_X2       5
};

static void ImFrameworkLoop();
static bool ImFrameworkRendererInitialize( ImUiFrameworkContext* context );
static bool ImFrameworkRendererCompileShader( GLuint shader, const char* pShaderCode );
static void ImFrameworkRendererShutdown( ImUiFrameworkContext* context );
static void ImFrameworkRendererDraw( ImUiFrameworkContext* context, const ImUiDrawData* drawData );

int main( int argc, char* argv[] )
{
#ifdef _WIN32
	int tmpFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );

	// Turn on leak-checking bit.
	tmpFlag |= _CRTDBG_LEAK_CHECK_DF;

	// Turn off CRT block checking bit.
	tmpFlag |= _CRTDBG_CHECK_ALWAYS_DF;

	// Set flag to the new value.
	_CrtSetDbgFlag( tmpFlag );
	//_CrtSetBreakAlloc( 100 );
#endif

	if (SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO /*| SDL_INIT_TIMER*/) < 0)
	{
		SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "I'm Ui", "Failed to initialize SDL.", NULL );
		return 1;
	}

	s_context.window = SDL_CreateWindow( "I'm Ui", SDL_WINDOWPOS_UNDEFINED_DISPLAY(2), SDL_WINDOWPOS_UNDEFINED_DISPLAY(2), 1280, 720, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	if( s_context.window == NULL)
	{
		SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "I'm Ui", "Failed to create Window.", NULL );
		return 1;
	}

	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 0 );
#ifdef __EMSCRIPTEN__
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES );
#else
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY );
#endif

	s_context.glContext = SDL_GL_CreateContext( s_context.window );
	if( s_context.glContext == NULL )
	{
		SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "I'm Ui", "Failed to initialize OpenGL.", NULL );
		return 1;
	}

	if( glewInit() != GLEW_OK )
	{
		SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "I'm Ui", "Failed to initialize GLEW.\n", NULL );
		return 1;
	}

	if( !ImFrameworkRendererInitialize( &s_context ) )
	{
		ImFrameworkRendererShutdown( &s_context );
		return 1;
	}

	ImUiParameters parameters = { 0 };
	s_context.imui = ImUiCreate( &parameters );

	const bool init = ImUiFrameworkInitialize( s_context.imui );

#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop( ImFrameworkLoop, 0, 1 );
#else
	s_running = init;
	while( s_running )
	{
		ImFrameworkLoop();
	}
#endif

	if( init )
	{
		ImUiFrameworkShutdown( s_context.imui );
	}

	ImUiDestroy( s_context.imui );
	s_context.imui = NULL;

	ImFrameworkRendererShutdown( &s_context );
	SDL_GL_DeleteContext( s_context.glContext );
	SDL_DestroyWindow( s_context.window );
	SDL_Quit();

	return 0;
}

static void ImFrameworkLoop()
{
	SDL_GL_SetSwapInterval( 1 );

	ImUiInput* input = ImUiInputBegin( s_context.imui );
	SDL_Event sdlEvent;
	while( SDL_PollEvent( &sdlEvent ) )
	{
		switch( sdlEvent.type )
		{
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			{
				const SDL_KeyboardEvent* pKeyEvent = &sdlEvent.key;

				const ImUiInputKey mappedKey = s_inputKeyMapping[ pKeyEvent->keysym.scancode ];
				if( mappedKey != ImUiInputKey_None )
				{
					(pKeyEvent->type == SDL_KEYDOWN ? ImUiInputPushKeyDown : ImUiInputPushKeyUp)(input, mappedKey);
				}
			}
			break;

		case SDL_MOUSEMOTION:
			{
				const SDL_MouseMotionEvent* pMouseEvent = &sdlEvent.motion;
				ImUiInputPushMouseMove( input, (float)pMouseEvent->x, (float)pMouseEvent->y );
			}
			break;

		case SDL_MOUSEBUTTONUP:
		case SDL_MOUSEBUTTONDOWN:
			{
				const SDL_MouseButtonEvent* pMouseEvent = &sdlEvent.button;
				(pMouseEvent->type == SDL_MOUSEBUTTONDOWN ? ImUiInputPushMouseDown : ImUiInputPushMouseUp)(input, s_inputMouseButtonMapping[ pMouseEvent->button ]);
			}
			break;

		case SDL_MOUSEWHEEL:
			{
				const SDL_MouseWheelEvent* pMouseEvent = &sdlEvent.wheel;
				ImUiInputPushMouseScroll( input, (float)pMouseEvent->x, (float)pMouseEvent->y );
			}
			break;

		case SDL_QUIT:
#ifdef __EMSCRIPTEN__
			emscripten_cancel_main_loop();
#else
			s_running = false;
#endif
			break;

		default:
			break;
		}
	}
	ImUiInputEnd( s_context.imui );

	SDL_GetWindowSize( s_context.window, &s_context.windowWidth, &s_context.windowHeight );

	ImUiFrame* frame = ImUiBegin( s_context.imui, SDL_GetTicks64() / 1000.0f );
	ImUiSurface* surface = ImUiSurfaceBegin( frame, ImUiStringViewCreate( "main" ), ImUiSizeCreate( (float)s_context.windowWidth, (float)s_context.windowHeight ), 1.0f );

	ImUiFrameworkTick( surface );

	const ImUiDrawData* drawData = ImUiSurfaceEnd( surface );

	ImFrameworkRendererDraw( &s_context, drawData );

	ImUiEnd( frame );

	SDL_GL_SwapWindow( s_context.window );
}

static const char s_vertexShader[] =
	"#version 100\n"
	"uniform mat4 ProjectionMatrix;\n"
	"attribute vec2 Position;\n"
	"attribute vec2 TexCoord;\n"
	"attribute vec4 Color;\n"
	"varying vec2 vtfUV;\n"
	"varying vec4 vtfColor;\n"
	"void main() {\n"
	"	vtfUV		= TexCoord;\n"
	"	vtfColor	= Color;\n"
	"	gl_Position	= ProjectionMatrix * vec4(Position.xy, 0, 1);\n"
	"}\n";

static const char s_fragmentShader[] =
	"#version 100\n"
	"precision mediump float;\n"
	"uniform sampler2D Texture;\n"
	"varying vec2 vtfUV;\n"
	"varying vec4 vtfColor;\n"
	//"out vec4 OutColor;\n"
	//"float rand(float n){ return fract(sin(n) * 43758.5453123); }\n"
	//"float noise( float p ){ float fl = floor( p ); float fc = fract( p ); return mix( rand( fl ), rand( fl + 1.0 ), fc ); }\n"
	"void main(){\n"
	"	vec4 texColor = texture2D(Texture, vtfUV.xy);\n"
	"	gl_FragColor = vtfColor * texColor;\n"
	//"	OutColor.xyz += vec3( noise( gl_FragCoord.x ) + noise( gl_FragCoord.y ) ) / 2;\n"
	"}\n";

static const char s_fragmentShaderFont[] =
	"#version 100\n"
	"precision mediump float;\n"
	"uniform sampler2D Texture;\n"
	"varying vec2 vtfUV;\n"
	"varying vec4 vtfColor;\n"
	"void main(){\n"
	"	vec4 fontChar = texture2D(Texture, vtfUV.xy);\n"
	"	gl_FragColor = vtfColor * fontChar;\n"
	"}\n";

static bool ImFrameworkRendererInitialize( ImUiFrameworkContext* context )
{
	// Shader
	context->vertexShader		= glCreateShader( GL_VERTEX_SHADER );
	context->fragmentShader		= glCreateShader( GL_FRAGMENT_SHADER );
	context->fragmentShaderFont	= glCreateShader( GL_FRAGMENT_SHADER );
	if( context->vertexShader == 0u ||
		context->fragmentShader == 0u ||
		context->fragmentShaderFont == 0u )
	{
		printf( "[renderer] Failed to create GL Shader.\n" );
		return false;
	}

	if( !ImFrameworkRendererCompileShader( context->vertexShader, s_vertexShader ) ||
		!ImFrameworkRendererCompileShader( context->fragmentShader, s_fragmentShader ) ||
		!ImFrameworkRendererCompileShader( context->fragmentShaderFont, s_fragmentShaderFont ) )
	{
		printf( "[renderer] Failed to compile GL Shader.\n" );
		return false;
	}

	context->program = glCreateProgram();
	context->programFont = glCreateProgram();
	if( context->program == 0u ||
		context->programFont == 0u )
	{
		printf( "[renderer] Failed to create GL Program.\n" );
		return false;
	}

	glAttachShader( context->program, context->vertexShader );
	glAttachShader( context->program, context->fragmentShader );
	glLinkProgram( context->program );

	glAttachShader( context->programFont, context->vertexShader );
	glAttachShader( context->programFont, context->fragmentShaderFont );
	glLinkProgram( context->programFont );

	bool ok = true;
	GLint programStatus;
	glGetProgramiv( context->program, GL_LINK_STATUS, &programStatus );
	ok &= (programStatus == GL_TRUE);
	glGetProgramiv( context->programFont, GL_LINK_STATUS, &programStatus );
	ok &= (programStatus == GL_TRUE);

	if( !ok )
	{
		printf( "[renderer] Failed to link GL Program.\n" );
		return false;
	}

	context->programUniformProjection	= glGetUniformLocation( context->program, "ProjectionMatrix" );
	context->programUniformTexture		= glGetUniformLocation( context->program, "Texture" );

	// Buffer
	const GLuint attributePosition	= (GLuint)glGetAttribLocation( context->program, "Position" );
	const GLuint attributeTexCoord	= (GLuint)glGetAttribLocation( context->program, "TexCoord" );
	const GLuint attributeColor		= (GLuint)glGetAttribLocation( context->program, "Color" );

	glGenBuffers( 1, &context->vertexBuffer );
	glGenBuffers( 1, &context->elementBuffer );
	glGenVertexArrays( 1, &context->vertexArray );

	glBindVertexArray( context->vertexArray );
	glBindBuffer( GL_ARRAY_BUFFER, context->vertexBuffer );
	//glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, context->elementBuffer );

	glEnableVertexAttribArray( attributePosition );
	glEnableVertexAttribArray( attributeTexCoord );
	glEnableVertexAttribArray( attributeColor );

	const GLsizei vertexSize	= sizeof( float ) * 8;
	size_t vertexPositionOffset	= 0u;
	size_t vertexUvOffset		= sizeof( float ) * 2u;
	size_t vertexColorOffset	= sizeof( float ) * 4u;
	glVertexAttribPointer( attributePosition, 2, GL_FLOAT, GL_FALSE, vertexSize, (void*)vertexPositionOffset );
	glVertexAttribPointer( attributeTexCoord, 2, GL_FLOAT, GL_FALSE, vertexSize, (void*)vertexUvOffset );
	glVertexAttribPointer( attributeColor, 4, GL_FLOAT, GL_FALSE, vertexSize, (void*)vertexColorOffset );

	context->whiteTexture.isFont = false;
	glGenTextures( 1, &context->whiteTexture.handle );
	glBindTexture( GL_TEXTURE_2D, context->whiteTexture.handle );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	const uint32 data = 0xffffffffu;
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &data );

	return true;
}

static bool ImFrameworkRendererCompileShader( GLuint shader, const char* pShaderCode )
{
	glShaderSource( shader, 1, &pShaderCode, 0 );
	glCompileShader( shader );

	GLint shaderStatus;
	glGetShaderiv( shader, GL_COMPILE_STATUS, &shaderStatus );

	if( shaderStatus != GL_TRUE )
	{
		char buffer[ 2048u ];
		GLsizei infoLength;
		glGetShaderInfoLog( shader, sizeof( buffer ), &infoLength, buffer);

		printf( "[renderer] Failed to compile Shader. Error: %s\n", buffer );
		return false;
	}

	return true;
}

static void ImFrameworkRendererShutdown( ImUiFrameworkContext* context )
{
	if( context->whiteTexture.handle != 0u )
	{
		glDeleteTextures( 1u, &context->whiteTexture.handle );
		context->whiteTexture.handle = 0u;
	}

	if( context->vertexArray != 0u )
	{
		glDeleteVertexArrays( 1, &context->vertexArray );
		context->vertexArray = 0u;
	}

	if( context->elementBuffer != 0u )
	{
		glDeleteBuffers( 1, &context->elementBuffer );
		context->elementBuffer = 0u;
	}

	if( context->vertexBuffer != 0u )
	{
		glDeleteBuffers( 1, &context->vertexBuffer );
		context->vertexBuffer = 0u;
	}

	if( context->program != 0u )
	{
		glDetachShader( context->program, context->fragmentShader );
		glDetachShader( context->program, context->vertexShader );
		glDeleteProgram( context->program );

		context->program = 0u;
	}

	if( context->programFont != 0u )
	{
		glDetachShader( context->programFont, context->fragmentShaderFont );
		glDetachShader( context->programFont, context->vertexShader );
		glDeleteProgram( context->programFont );

		context->programFont = 0u;
	}

	if( context->vertexShader != 0u )
	{
		glDeleteShader( context->vertexShader );
		context->vertexShader = 0u;
	}

	if( context->fragmentShader != 0u )
	{
		glDeleteShader( context->fragmentShader );
		context->fragmentShader = 0u;
	}

	if( context->fragmentShaderFont != 0u )
	{
		glDeleteShader( context->fragmentShaderFont );
		context->fragmentShaderFont = 0u;
	}
}

static void ImFrameworkRendererDraw( ImUiFrameworkContext* context, const ImUiDrawData* drawData )
{
	glViewport( 0, 0, context->windowWidth, context->windowHeight );

	const float color[ 4u ] = { 0.01f, 0.2f, 0.7f, 1.0f };
	glClearColor( color[ 0u ], color[ 1u ], color[ 2u ], color[ 3u ] );
	glClear( GL_COLOR_BUFFER_BIT );

	glEnable( GL_BLEND );
	glBlendEquation( GL_FUNC_ADD );

	glDisable( GL_CULL_FACE );
	glDisable( GL_DEPTH_TEST );
	glEnable( GL_SCISSOR_TEST );

	glUseProgram( context->program );
	glUniform1i( context->programUniformTexture, 0 );

	const GLfloat projectionMatrix[ 4 ][ 4 ] ={
		{  2.0f / context->windowWidth,	 0.0f,							 0.0f,	0.0f },
		{  0.0f,						-2.0f / context->windowHeight,	 0.0f,	0.0f },
		{  0.0f,						 0.0f,							-1.0f,	0.0f },
		{ -1.0f,						 1.0f,							 0.0f,	1.0f }
	};
	glUniformMatrix4fv( context->programUniformProjection, 1, GL_FALSE, &projectionMatrix[ 0u ][ 0u ] );

	// bind buffers
	glBindVertexArray( context->vertexArray );
	glBindBuffer( GL_ARRAY_BUFFER, context->vertexBuffer );
	//glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, context->elementBuffer );

	glBufferData( GL_ARRAY_BUFFER, drawData->vertexDataSize, NULL, GL_STREAM_DRAW );
	//glBufferData( GL_ELEMENT_ARRAY_BUFFER, drawData->indexCount * 4u, NULL, GL_STREAM_DRAW );

	// upload
	{
		void* vertexData = glMapBufferRange( GL_ARRAY_BUFFER, 0, drawData->vertexDataSize, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT );
		//void* pElementData = glMapBufferRange( GL_ELEMENT_ARRAY_BUFFER, 0, drawData->indexCount * 4u, GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT );

		memcpy( vertexData, drawData->vertexData, drawData->vertexDataSize );
		//memcpy( pElementData, drawData->indexData, drawData->indexCount * 4u );

		glUnmapBuffer( GL_ARRAY_BUFFER );
		//glUnmapBuffer( GL_ELEMENT_ARRAY_BUFFER );
	}

	GLint offset = 0;
	for( size_t i = 0u; i < drawData->commandCount; ++i )
	{
		const ImUiDrawCommand* command = &drawData->commands[ i ];
		IMUI_ASSERT( command->count >= 0u );

		ImUiFrameworkTexture* texture = (ImUiFrameworkTexture*)command->texture;
		if( !texture )
		{
			texture = &context->whiteTexture;
		}

		if( !texture->isFont )
		{
			glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

			glUseProgram( context->program );
		}
		else
		{
			glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA );

			glUseProgram( context->programFont );
			glUniform1i( context->programUniformTexture, 0 );
			glUniformMatrix4fv( context->programUniformProjection, 1, GL_FALSE, &projectionMatrix[ 0u ][ 0u ] );
		}

		glBindTexture( GL_TEXTURE_2D, texture->handle );

		glScissor(
			(GLint)(command->clipRect.pos.x),
			(GLint)((context->windowHeight - (int)(command->clipRect.pos.y + command->clipRect.size.height))),
			(GLint)(command->clipRect.size.width),
			(GLint)(command->clipRect.size.height)
		);

		glDrawArrays( GL_TRIANGLES, offset, (GLsizei)command->count );
		//glDrawElements( GL_TRIANGLES, (GLsizei)pCommand->count, GL_UNSIGNED_SHORT, &pCommand->offset );
		offset += (GLint)command->count;
	}

	// reset OpenGL state
	glUseProgram( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
	glBindVertexArray( 0 );
	glDisable( GL_BLEND );
	glDisable( GL_SCISSOR_TEST );
}

ImUiFrameworkTexture* ImUiFrameworkTextureCreate( void* textureData, uint32_t width, uint32_t height, bool isFont )
{
	ImUiFrameworkTexture* texture = (ImUiFrameworkTexture*)malloc( sizeof( ImUiFrameworkTexture ) );
	if( !texture )
	{
		return NULL;
	}

	texture->handle	= 0u;
	texture->isFont = isFont;

	glGenTextures( 1, &texture->handle );
	glBindTexture( GL_TEXTURE_2D, texture->handle );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexImage2D( GL_TEXTURE_2D, 0, isFont ? GL_INTENSITY8 : GL_RGBA8, width, height, 0, isFont ? GL_RED : GL_RGBA, GL_UNSIGNED_BYTE, textureData );

	return texture;
}

void ImUiFrameworkTextureDestroy( ImUiFrameworkTexture* texture )
{
	glDeleteTextures( 1u, &texture->handle );
	free( texture );
}

bool ImUiFrameworkFontCreate( ImUiFont** font, ImUiTexture* texture, const char* fontFilename, float fontSize )
{
	uint8_t* fileData;
	size_t fileSize;
	{
		FILE* file = fopen( fontFilename, "rb" );
		if( !file )
		{
			return false;
		}

		fseek( file, 0, SEEK_END );
		fileSize = ftell( file );
		fseek( file, 0, SEEK_SET );

		fileData = (uint8_t*)malloc( fileSize );
		if( !fileData )
		{
			fclose( file );
			return false;
		}

		fread( fileData, fileSize, 1, file );
		fclose( file );
	}

	ImUiFontTrueTypeData* ttf = ImUiFontTrueTypeDataCreate( s_context.imui, fileData, fileSize );
	if( !ttf )
	{
		free( fileData );
		return false;
	}

	ImUiFontTrueTypeDataAddCodepointRange( ttf, 0x20, 0x7e );
	ImUiFontTrueTypeDataAddCodepointRange( ttf, 0x370, 0x3ff );
	ImUiFontTrueTypeDataAddCodepointRange( ttf, 0xfffd, 0xfffd );

	uint32_t width;
	uint32_t height;
	ImUiFontTrueTypeDataCalculateMinTextureSize( ttf, fontSize, &width, &height );
	width = (width + 4u - 1u) & (0 - 4);
	height = (height + 4u - 1u) & (0 - 4);

	void* textureData = malloc( width * height );
	if( !textureData )
	{
		ImUiFontTrueTypeDataDestroy( ttf );
		free( fileData );
		return false;
	}

	ImUiFontTrueTypeImage* image = ImUiFontTrueTypeDataGenerateTextureData( ttf, fontSize, textureData, width * height, width, height );
	if( !image )
	{
		ImUiFontTrueTypeDataDestroy( ttf );
		free( fileData );
		return false;
	}

	texture->data = ImUiFrameworkTextureCreate( textureData, width, height, true );
	texture->size = ImUiSizeCreate( (float)width, (float)height );

	free( textureData );

	if( !texture->data )
	{
		ImUiFontTrueTypeDataDestroy( ttf );
		free( fileData );
		return false;
	}

	*font = ImUiFontCreateTrueType( s_context.imui, image, *texture );

	ImUiFontTrueTypeDataDestroy( ttf );
	free( fileData );

	return *font != NULL;
}

void ImUiFrameworkFontDestroy( ImUiFont** font, ImUiTexture* texture )
{
	ImUiFrameworkTextureDestroy( (ImUiFrameworkTexture*)texture->data );
	ImUiFontDestroy( s_context.imui, *font );

	texture->data = NULL;
	*font = NULL;
}

bool ImUiFrameworkSkinCreate( ImUiSkin* skin, ImUiTexture* texture, uint32_t size, float radius, float factor, bool horizontal )
{
	uint8_t* skinData = malloc( size * size * 4u );
	if( !skinData )
	{
		return false;
	}

	const float maxDistance = sqrtf( (radius * radius) + (radius * radius) );
	const float halfSize = size * 0.5f;
	const float halfRadius = radius * 0.5f;
	for( size_t y = 0u; y < size; ++y )
	{
		uint8_t* line = skinData + (y * size * 4u);
		for( size_t x = 0u; x < size; ++x )
		{
			uint8_t value;
			if( horizontal )
			{
				float xFactor = 1.0f;
				if( x < radius || x > size - radius )
				{
					const float xdiff = x - (x < radius ? radius : size - radius);
					const float disNorm	= fabsf( xdiff ) / halfSize;

					xFactor = 1.0f - (disNorm > 1.0f ? 1.0f : disNorm);
				}

				const float ydiff = y - halfSize;
				const float disNorm	= fabsf( ydiff ) / (halfSize - halfRadius);

				float dis = 1.0f - (disNorm > 1.0f ? 1.0f : disNorm);
				dis *= radius * factor * xFactor;
				value = dis > 255.0f ? 255u : (uint8_t)dis;
			}
			else
			{
				if( (x < radius || x > size - radius) &&
					(y < radius || y > size - radius) )
				{
					const float points[][ 2u ] =
					{
						{ radius, radius },
						{ size - radius - 0.5f, radius },
						{ radius, size - radius - 0.5f },
						{ size - radius - 0.5f, size - radius - 0.5f }
					};

					float minDis = FLT_MAX;
					for( uint8_t i = 0u; i < 4u; ++i )
					{
						const float udiff = x - points[ i ][ 0u ];
						const float vdiff = y - points[ i ][ 1u ];

						const float disBase	= sqrtf( (udiff * udiff) + (vdiff * vdiff) );
						const float disNorm	= disBase / maxDistance;
						if( disNorm > 1.0f )
						{
							continue;
						}

						const float dis = 1.0f - disNorm;
						minDis = minDis < dis ? minDis : dis;
					}

					minDis *= radius * factor;
					value = minDis > 255.0f ? 255u : (uint8_t)minDis;
				}
				else
				{
					value = 255u;
				}
			}

			uint8_t* pixel = line + (x * 4u);
			pixel[ 0u ]		= 0xffu;
			pixel[ 1u ]		= 0xffu;
			pixel[ 2u ]		= 0xffu;
			pixel[ 3u ]		= value;
		}
	}

	texture->data = ImUiFrameworkTextureCreate( skinData, size, size, false );
	texture->size = ImUiSizeCreate( (float)size, (float)size );

	free( skinData );

	skin->texture	= *texture;
	skin->uv.u0		= 0.0f;
	skin->uv.v0		= 0.0f;
	skin->uv.u1		= 1.0f;
	skin->uv.v1		= 1.0f;

	if( horizontal )
	{
		skin->border	= ImUiBorderCreateHorizontalVertical( radius, 0.0f );
	}
	else
	{
		skin->border	= ImUiBorderCreateAll( radius );
	}

	return texture->data != NULL;
}

void ImUiFrameworkSkinDestroy( ImUiSkin* skin, ImUiTexture* texture )
{
	ImUiFrameworkTextureDestroy( (ImUiFrameworkTexture*)texture->data );
	texture->data = NULL;
}
