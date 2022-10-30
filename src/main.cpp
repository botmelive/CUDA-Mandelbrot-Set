#include "kernel.h"
#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#define WINDOWS_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glew.h>
#include <GL/freeglut.h>
#endif
#include <cuda_runtime.h>
#include <cuda_gl_interop.h>
#include <curand_kernel.h>
#include "interactions.h"

// texture and pixel objects
GLuint pbo = 0; // OpenGL pixel buffer object
GLuint tex = 0; // OpenGL texture object
curandState *devStates;
struct cudaGraphicsResource *cuda_pbo_resource;
Bounds bounds;

void render()
{
	uchar4 *d_out = 0;
	cudaGraphicsMapResources(1, &cuda_pbo_resource, 0);
	size_t bufferSize = 0;
	cudaGraphicsResourceGetMappedPointer((void **)&d_out, &bufferSize, cuda_pbo_resource);
	kernelLauncher(d_out, W, H, MAX, bounds);
	cudaGraphicsUnmapResources(1, &cuda_pbo_resource, 0);
	// printf("Rendered!\n");
}

void drawTexture()
{
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, W, H, 0, GL_RGBA,
				 GL_UNSIGNED_BYTE, NULL);
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);
	glVertex2f(0, 0);
	glTexCoord2f(0.0f, 1.0f);
	glVertex2f(0, H);
	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(W, H);
	glTexCoord2f(1.0f, 0.0f);
	glVertex2f(W, 0);
	glEnd();
	glDisable(GL_TEXTURE_2D);
}

void display()
{
	render();
	drawTexture();
	glutSwapBuffers();
}

void initGLUT(int *argc, char **argv)
{
	glutInit(argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(W, H);
	glutCreateWindow(TITLE_STRING);
#ifndef __APPLE__
	glewInit();
#endif
}

void initPixelBuffer()
{
	glGenBuffers(1, &pbo);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
	glBufferData(GL_PIXEL_UNPACK_BUFFER, 4 * W * H * sizeof(GLubyte), 0,
				 GL_STREAM_DRAW);
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	cudaGraphicsGLRegisterBuffer(&cuda_pbo_resource, pbo,
								 cudaGraphicsMapFlagsWriteDiscard);
}

void exitfunc()
{
	if (pbo)
	{
		cudaGraphicsUnregisterResource(cuda_pbo_resource);
		glDeleteBuffers(1, &pbo);
		glDeleteTextures(1, &tex);
	}
}

void idle()
{
	glutPostRedisplay();
}

void mouse(int button, int state, int x, int y)
{
    // Save the left button state
    if (button == GLUT_LEFT_BUTTON)
    {
        if(state == GLUT_DOWN){
            if(!dragMode){
                loc.x = x;
                loc.y = y;
                dragMode = true;
            }
        }

        if(state == GLUT_UP){
            loc2.x = x;
            loc2.y = y;
            bound1 = {(float)loc.x / W, (float)loc.y / H};
            bound2 = {(float)loc2.x / W, (float)loc2.y / H};

            float x1 = Shiftrange(bounds.B1.x, bounds.B2.x, bound1.x);
            float y1 = Shiftrange(bounds.B1.y, bounds.B2.y, bound1.y);
            float x2 = Shiftrange(bounds.B1.x, bounds.B2.x, bound2.x);
            float y2 = Shiftrange(bounds.B1.y, bounds.B2.y, bound2.y);

            bounds.B1.x = x1;
            bounds.B1.y = y1;
            bounds.B2.x = x2;
            bounds.B2.y = y2;
            
            dragMode = false;
        }
    }

    else if (button == GLUT_RIGHT_BUTTON)
    {
        bool rightMouseButtonDown = (state == GLUT_DOWN);
        
    }
}

int main(int argc, char **argv)
{
	printInstructions();

	initGLUT(&argc, argv);
	gluOrtho2D(0, W, H, 0);
	glutKeyboardFunc(keyboard);
	glutIdleFunc(idle);
	glutPassiveMotionFunc(mouseMove);
	glutMotionFunc(mouseDrag);
	glutMouseFunc(mouse);
	glutDisplayFunc(display);

	initPixelBuffer();

	glutMainLoop();
	atexit(exitfunc);
	return 0;
}
