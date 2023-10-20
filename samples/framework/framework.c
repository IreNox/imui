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

	ImUiFrameworkTexture		whiteTexture;

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
static void ImFrameworkRendererDraw( ImUiFrameworkContext* context, ImUiSurface* surface );

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


	for( size_t i = 0u; i < ImUiInputKey_MAX; ++i )
	{
		const ImUiInputKey keyValue = (ImUiInputKey)i;

		SDL_Scancode scanCode = SDL_SCANCODE_UNKNOWN;
		switch( keyValue )
		{
		case ImUiInputKey_None:				scanCode = SDL_SCANCODE_UNKNOWN; break;
		case ImUiInputKey_A:				scanCode = SDL_SCANCODE_A; break;
		case ImUiInputKey_B:				scanCode = SDL_SCANCODE_B; break;
		case ImUiInputKey_C:				scanCode = SDL_SCANCODE_C; break;
		case ImUiInputKey_D:				scanCode = SDL_SCANCODE_D; break;
		case ImUiInputKey_E:				scanCode = SDL_SCANCODE_E; break;
		case ImUiInputKey_F:				scanCode = SDL_SCANCODE_F; break;
		case ImUiInputKey_G:				scanCode = SDL_SCANCODE_G; break;
		case ImUiInputKey_H:				scanCode = SDL_SCANCODE_H; break;
		case ImUiInputKey_I:				scanCode = SDL_SCANCODE_I; break;
		case ImUiInputKey_J:				scanCode = SDL_SCANCODE_J; break;
		case ImUiInputKey_K:				scanCode = SDL_SCANCODE_K; break;
		case ImUiInputKey_L:				scanCode = SDL_SCANCODE_L; break;
		case ImUiInputKey_M:				scanCode = SDL_SCANCODE_M; break;
		case ImUiInputKey_N:				scanCode = SDL_SCANCODE_N; break;
		case ImUiInputKey_O:				scanCode = SDL_SCANCODE_O; break;
		case ImUiInputKey_P:				scanCode = SDL_SCANCODE_P; break;
		case ImUiInputKey_Q:				scanCode = SDL_SCANCODE_Q; break;
		case ImUiInputKey_R:				scanCode = SDL_SCANCODE_R; break;
		case ImUiInputKey_S:				scanCode = SDL_SCANCODE_S; break;
		case ImUiInputKey_T:				scanCode = SDL_SCANCODE_T; break;
		case ImUiInputKey_U:				scanCode = SDL_SCANCODE_U; break;
		case ImUiInputKey_V:				scanCode = SDL_SCANCODE_V; break;
		case ImUiInputKey_W:				scanCode = SDL_SCANCODE_W; break;
		case ImUiInputKey_X:				scanCode = SDL_SCANCODE_X; break;
		case ImUiInputKey_Y:				scanCode = SDL_SCANCODE_Y; break;
		case ImUiInputKey_Z:				scanCode = SDL_SCANCODE_Z; break;
		case ImUiInputKey_1:				scanCode = SDL_SCANCODE_1; break;
		case ImUiInputKey_2:				scanCode = SDL_SCANCODE_2; break;
		case ImUiInputKey_3:				scanCode = SDL_SCANCODE_3; break;
		case ImUiInputKey_4:				scanCode = SDL_SCANCODE_4; break;
		case ImUiInputKey_5:				scanCode = SDL_SCANCODE_5; break;
		case ImUiInputKey_6:				scanCode = SDL_SCANCODE_6; break;
		case ImUiInputKey_7:				scanCode = SDL_SCANCODE_7; break;
		case ImUiInputKey_8:				scanCode = SDL_SCANCODE_8; break;
		case ImUiInputKey_9:				scanCode = SDL_SCANCODE_9; break;
		case ImUiInputKey_0:				scanCode = SDL_SCANCODE_0; break;
		case ImUiInputKey_Enter:			scanCode = SDL_SCANCODE_RETURN; break;
		case ImUiInputKey_Escape:			scanCode = SDL_SCANCODE_ESCAPE; break;
		case ImUiInputKey_Backspace:		scanCode = SDL_SCANCODE_BACKSPACE; break;
		case ImUiInputKey_Tab:				scanCode = SDL_SCANCODE_TAB; break;
		case ImUiInputKey_Space:			scanCode = SDL_SCANCODE_SPACE; break;
		case ImUiInputKey_LeftShift:		scanCode = SDL_SCANCODE_LSHIFT; break;
		case ImUiInputKey_RightShift:		scanCode = SDL_SCANCODE_RSHIFT; break;
		case ImUiInputKey_LeftControl:		scanCode = SDL_SCANCODE_LCTRL; break;
		case ImUiInputKey_RightControl:		scanCode = SDL_SCANCODE_RCTRL; break;
		case ImUiInputKey_LeftAlt:			scanCode = SDL_SCANCODE_LALT; break;
		case ImUiInputKey_RightAlt:			scanCode = SDL_SCANCODE_RALT; break;
		case ImUiInputKey_Minus:			scanCode = SDL_SCANCODE_MINUS; break;
		case ImUiInputKey_Equals:			scanCode = SDL_SCANCODE_EQUALS; break;
		case ImUiInputKey_LeftBracket:		scanCode = SDL_SCANCODE_LEFTBRACKET; break;
		case ImUiInputKey_RightBracket:		scanCode = SDL_SCANCODE_RIGHTBRACKET; break;
		case ImUiInputKey_Backslash:		scanCode = SDL_SCANCODE_BACKSLASH; break;
		case ImUiInputKey_Semicolon:		scanCode = SDL_SCANCODE_SEMICOLON; break;
		case ImUiInputKey_Apostrophe:		scanCode = SDL_SCANCODE_APOSTROPHE; break;
		case ImUiInputKey_Grave:			scanCode = SDL_SCANCODE_GRAVE; break;
		case ImUiInputKey_Comma:			scanCode = SDL_SCANCODE_COMMA; break;
		case ImUiInputKey_Period:			scanCode = SDL_SCANCODE_PERIOD; break;
		case ImUiInputKey_Slash:			scanCode = SDL_SCANCODE_SLASH; break;
		case ImUiInputKey_F1:				scanCode = SDL_SCANCODE_F1; break;
		case ImUiInputKey_F2:				scanCode = SDL_SCANCODE_F2; break;
		case ImUiInputKey_F3:				scanCode = SDL_SCANCODE_F3; break;
		case ImUiInputKey_F4:				scanCode = SDL_SCANCODE_F4; break;
		case ImUiInputKey_F5:				scanCode = SDL_SCANCODE_F5; break;
		case ImUiInputKey_F6:				scanCode = SDL_SCANCODE_F6; break;
		case ImUiInputKey_F7:				scanCode = SDL_SCANCODE_F7; break;
		case ImUiInputKey_F8:				scanCode = SDL_SCANCODE_F8; break;
		case ImUiInputKey_F9:				scanCode = SDL_SCANCODE_F9; break;
		case ImUiInputKey_F10:				scanCode = SDL_SCANCODE_F10; break;
		case ImUiInputKey_F11:				scanCode = SDL_SCANCODE_F11; break;
		case ImUiInputKey_F12:				scanCode = SDL_SCANCODE_F12; break;
		case ImUiInputKey_Print:			scanCode = SDL_SCANCODE_PRINTSCREEN; break;
		case ImUiInputKey_Pause:			scanCode = SDL_SCANCODE_PAUSE; break;
		case ImUiInputKey_Insert:			scanCode = SDL_SCANCODE_INSERT; break;
		case ImUiInputKey_Delete:			scanCode = SDL_SCANCODE_DELETE; break;
		case ImUiInputKey_Home:				scanCode = SDL_SCANCODE_HOME; break;
		case ImUiInputKey_End:				scanCode = SDL_SCANCODE_END; break;
		case ImUiInputKey_PageUp:			scanCode = SDL_SCANCODE_PAGEUP; break;
		case ImUiInputKey_PageDown:			scanCode = SDL_SCANCODE_PAGEDOWN; break;
		case ImUiInputKey_Up:				scanCode = SDL_SCANCODE_UP; break;
		case ImUiInputKey_Left:				scanCode = SDL_SCANCODE_LEFT; break;
		case ImUiInputKey_Down:				scanCode = SDL_SCANCODE_DOWN; break;
		case ImUiInputKey_Right:			scanCode = SDL_SCANCODE_RIGHT; break;
		case ImUiInputKey_Numpad_Divide:	scanCode = SDL_SCANCODE_KP_DIVIDE; break;
		case ImUiInputKey_Numpad_Multiply:	scanCode = SDL_SCANCODE_KP_MULTIPLY; break;
		case ImUiInputKey_Numpad_Minus:		scanCode = SDL_SCANCODE_KP_MINUS; break;
		case ImUiInputKey_Numpad_Plus:		scanCode = SDL_SCANCODE_KP_PLUS; break;
		case ImUiInputKey_Numpad_Enter:		scanCode = SDL_SCANCODE_KP_ENTER; break;
		case ImUiInputKey_Numpad_1:			scanCode = SDL_SCANCODE_KP_1; break;
		case ImUiInputKey_Numpad_2:			scanCode = SDL_SCANCODE_KP_2; break;
		case ImUiInputKey_Numpad_3:			scanCode = SDL_SCANCODE_KP_3; break;
		case ImUiInputKey_Numpad_4:			scanCode = SDL_SCANCODE_KP_4; break;
		case ImUiInputKey_Numpad_5:			scanCode = SDL_SCANCODE_KP_5; break;
		case ImUiInputKey_Numpad_6:			scanCode = SDL_SCANCODE_KP_6; break;
		case ImUiInputKey_Numpad_7:			scanCode = SDL_SCANCODE_KP_7; break;
		case ImUiInputKey_Numpad_8:			scanCode = SDL_SCANCODE_KP_8; break;
		case ImUiInputKey_Numpad_9:			scanCode = SDL_SCANCODE_KP_9; break;
		case ImUiInputKey_Numpad_0:			scanCode = SDL_SCANCODE_KP_0; break;
		case ImUiInputKey_Numpad_Period:	scanCode = SDL_SCANCODE_KP_PERIOD; break;
		case ImUiInputKey_MAX:				break;
		}

		s_inputKeyMapping[ scanCode ] = keyValue;
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
				const SDL_KeyboardEvent* keyEvent = &sdlEvent.key;

				const ImUiInputKey mappedKey = s_inputKeyMapping[ keyEvent->keysym.scancode ];
				if( mappedKey != ImUiInputKey_None )
				{
					(keyEvent->type == SDL_KEYDOWN ? ImUiInputPushKeyDown : ImUiInputPushKeyUp)(input, mappedKey);
				}

				if( keyEvent->repeat )
				{
					ImUiInputPushKeyRepeat( input, mappedKey );
				}
			}
			break;

		case SDL_TEXTINPUT:
			{
				const SDL_TextInputEvent* textEvent = &sdlEvent.text;
				ImUiInputPushText( input, textEvent->text );
			}
			break;

		case SDL_MOUSEMOTION:
			{
				const SDL_MouseMotionEvent* mouseEvent = &sdlEvent.motion;
				ImUiInputPushMouseMove( input, (float)mouseEvent->x, (float)mouseEvent->y );
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

	ImUiSurfaceEnd( surface );

	ImFrameworkRendererDraw( &s_context, surface );

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

static void ImFrameworkRendererDraw( ImUiFrameworkContext* context, ImUiSurface* surface )
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

	uintsize vertexDataSize = 0u;
	ImUiSurfaceGetMaxBufferSizes( surface, &vertexDataSize, NULL );

	glBufferData( GL_ARRAY_BUFFER, vertexDataSize, NULL, GL_STREAM_DRAW );

	// upload
	const ImUiDrawData* drawData;
	{
		void* vertexData = glMapBufferRange( GL_ARRAY_BUFFER, 0, vertexDataSize, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT );

		drawData = ImUiSurfaceGenerateDrawData( surface, vertexData, &vertexDataSize, NULL, NULL );

		glUnmapBuffer( GL_ARRAY_BUFFER );
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

		const GLenum topology = (command->topology == ImUiDrawTopology_LineList ? GL_LINES : GL_TRIANGLES);
		glDrawArrays( topology, offset, (GLsizei)command->count );
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

bool ImUiFrameworkFontCreate( ImUiFont** font, ImUiImage* image, const char* fontFilename, float fontSize )
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

	ImUiFontTrueTypeImage* ttfImage = ImUiFontTrueTypeDataGenerateTextureData( ttf, fontSize, textureData, width * height, width, height );
	if( !ttfImage )
	{
		ImUiFontTrueTypeDataDestroy( ttf );
		free( fileData );
		return false;
	}

	image->textureData	= ImUiFrameworkTextureCreate( textureData, width, height, true );
	image->width		= width;
	image->height		= height;
	image->uv.u0		= 0.0f;
	image->uv.v0		= 0.0f;
	image->uv.u1		= 1.0f;
	image->uv.v1		= 1.0f;

	free( textureData );

	if( !image->textureData )
	{
		ImUiFontTrueTypeDataDestroy( ttf );
		free( fileData );
		return false;
	}

	*font = ImUiFontCreateTrueType( s_context.imui, ttfImage, *image );

	ImUiFontTrueTypeDataDestroy( ttf );
	free( fileData );

	return *font != NULL;
}

void ImUiFrameworkFontDestroy( ImUiFont** font, ImUiImage* image )
{
	ImUiFrameworkTextureDestroy( (ImUiFrameworkTexture*)image->textureData );
	ImUiFontDestroy( s_context.imui, *font );

	image->textureData = NULL;
	*font = NULL;
}

bool ImUiFrameworkSkinCreate( ImUiSkin* skin, ImUiImage* image, uint32_t size, float radius, float factor, bool horizontal )
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

	image->textureData	= ImUiFrameworkTextureCreate( skinData, size, size, false );
	image->width	= size;
	image->height	= size;
	image->uv.u0	= 0.0f;
	image->uv.v0	= 0.0f;
	image->uv.u1	= 1.0f;
	image->uv.v1	= 1.0f;

	free( skinData );

	skin->textureData	= image->textureData;
	skin->width			= image->width;
	skin->height		= image->height;
	skin->uv			= image->uv;

	if( horizontal )
	{
		skin->border	= ImUiBorderCreateHorizontalVertical( radius, 0.0f );
	}
	else
	{
		skin->border	= ImUiBorderCreateAll( radius );
	}

	return image->textureData != NULL;
}

void ImUiFrameworkSkinDestroy( ImUiSkin* skin, ImUiImage* image )
{
	ImUiFrameworkTextureDestroy( (ImUiFrameworkTexture*)image->textureData );
	image->textureData = NULL;
}
