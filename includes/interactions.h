#pragma once

#include <stdio.h>
#define W 1080
#define H 1080

#define TITLE_STRING "Mandelbrot"
int2 loc, loc2;
float2 bound1;
float2 bound2;
int MAX = 500;
bool dragMode = false;


float Shiftrange(float c, float d, float t)
{
   return c + (float)(d - c) * t;
}


void keyboard(unsigned char key, int x, int y)
{
    if (key == 'w')
        MAX += 10;

    if (key == 'q')
        MAX -= 10;
    
    if (key == 27)
        exit(0);
    printf("MAX : %d\n", MAX);
    glutPostRedisplay();
}

void mouseMove(int x, int y)
{
    if (dragMode)
        return;
    //printf("Mouse pos %d, %d\n", x, y);
    glutPostRedisplay();
}

void mouseDrag(int x, int y)
{
    if (!dragMode)
        return;
    glutPostRedisplay();
}

void printInstructions()
{
    printf("w : Increase the number of iterations.\n");
    printf("q : Decrease the number of iterations.\n");
    printf("esc: close graphics window\n");
}