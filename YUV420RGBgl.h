#ifndef YUV420RGBgl
#define YUV420RGBgl

#include <stdio.h>
#include <assert.h>

#define EGL_EGLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <bcm_host.h>

typedef struct
{
    // Handle to a program object
   GLuint programObject;

   // Attribute locations
   GLint  positionLoc;
   GLint  texCoordLoc;

   // Sampler location
   GLint samplerLoc;
   
   // Image size vector location vec2(WIDTH, HEIGHT)
   GLint sizeLoc;

   // Input texture
   GLuint tex;

   // YUV->RGB conversion matrix
   GLuint cmatrixLoc;

	// Frame & Render buffers
	GLuint canvasFrameBuffer;
	GLuint canvasRenderBuffer;

	char *outrgb;

	// Output texture
	GLuint outtex;
} UserData;

typedef struct
{
    uint32_t screen_width;
    uint32_t screen_height;

    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;

	UserData *user_data;

} CUBE_STATE_T;

void checkNoGLES2Error();
void init_ogl(CUBE_STATE_T *state, int codecWidth, int codecHeight);
void init_ogl2(CUBE_STATE_T *state, int pWidth, int pHeight);
void close_ogl2(CUBE_STATE_T *state);
GLuint LoadShader(GLenum type, const char *shaderSrc);
GLuint LoadProgram(const char *vertShaderSrc, const char *fragShaderSrc);
int Init(CUBE_STATE_T *state);
void setSize(CUBE_STATE_T *state, int width, int height);
void texImage2D(CUBE_STATE_T *state, char* buf, int width, int height);
void redraw_scene(CUBE_STATE_T *state);
void list_uniforms_of_program(CUBE_STATE_T *state);
#endif
