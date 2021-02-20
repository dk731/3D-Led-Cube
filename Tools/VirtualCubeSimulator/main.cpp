#pragma once
#include <GL/glut.h>
#include <iostream>
#include <stdlib.h>
#include <time.h>

#include "SharedColosBuffer.h"
#include "Point.h"

const int WIDTH = 16, HEIGHT = 16, DEPH = 16;

Point cube_size = Point(WIDTH, HEIGHT, DEPH);
float spacing = 1.5f;
float pixel_size = 0.3f;

SharedColosBuffer *colorBuf = new SharedColosBuffer(cube_size);

Point start_pos = cube_size * spacing / 2.0f + spacing / -2.0f;

int FPS = 60;

float zoom = -50.0f;
Point cube_rotation = Point(0.0f, 0.0f, 0.0f);

bool pressed = false;

Point prev_mouse_pos;

float zoom_sens = 0.1f;
float rotate_sens = 0.5f;

float found_size = WIDTH * spacing / 2.0f - 2.0f;


void drawBox(Point p, float a, float size) {
	glColor4f(p.x, p.y, p.z, a);
	glBegin(GL_TRIANGLES);

	float ssize = 0.5 * size;

	glVertex3f(0.0f, ssize, 0.0f);
	glVertex3f(ssize, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, ssize);

	glVertex3f(0.0f, ssize, 0.0f);
	glVertex3f(-ssize, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, ssize);

	glVertex3f(0.0f, ssize, 0.0f);
	glVertex3f(ssize, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, -ssize);

	glVertex3f(0.0f, ssize, 0.0f);
	glVertex3f(-ssize, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, -ssize);



	glVertex3f(0.0f, -ssize, 0.0f);
	glVertex3f(ssize, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, ssize);

	glVertex3f(0.0f, -ssize, 0.0f);
	glVertex3f(-ssize, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, ssize);

	glVertex3f(0.0f, -ssize, 0.0f);
	glVertex3f(ssize, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, -ssize);

	glVertex3f(0.0f, -ssize, 0.0f);
	glVertex3f(-ssize, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, -ssize);

	glEnd();
}

void drawFound(int height, float spacing, float found_size) {
	glColor4f(0.545f, 0.270f, 0.074, 1.0f);
	glDisable(GL_CULL_FACE);
	glBegin(GL_TRIANGLES);
	float y_h = -height * spacing / 2.0f - 3.0f;
	glVertex3d(-found_size, y_h, found_size);
	glVertex3d(-found_size, y_h, -found_size);
	glVertex3d(found_size, y_h, -found_size);

	glVertex3d(-found_size, y_h, found_size);
	glVertex3d(found_size, y_h, found_size);
	glVertex3d(found_size, y_h, -found_size);

	glEnd();
}


void displayFunc(void)
{	
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	gluPerspective(50.0, 1.0, 1.0, 100.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glTranslatef(0.0f, 0.0f, zoom);

	glRotatef(cube_rotation.x, 1.0f, 0.0f, 0.0f);
	glRotatef(cube_rotation.y, 0.0f, 1.0f, 0.0f);
	
	for (int z = 0; z < DEPH; z++) {
		for (int y = 0; y < HEIGHT; y++) {
			for (int x = 0; x < WIDTH; x++) {
				glPushMatrix();
				glColor4f(1.0f, 1.0f, 1.0f, 0.8f);
				Point cur_pos = Point(start_pos.x + x * spacing, start_pos.y - y * spacing, start_pos.z - z * spacing);
				glTranslatef(cur_pos.x, cur_pos.y, cur_pos.z);

				colorBuf->readMemmory();

				drawBox(colorBuf->getAt(x, y, z), 0.8f, pixel_size);

				glPopMatrix();
			}
		}
	}

	drawFound(HEIGHT, spacing, found_size);
	glutSwapBuffers();
}

void mousePressed(int button, int state, int x, int y)
{

	if (button == 3) {
		zoom += zoom_sens;
	}
	else if (button == 4) {
		zoom -= zoom_sens;
	}
	if (button == 0) {
		pressed = !state;
		prev_mouse_pos.x = x;
		prev_mouse_pos.y = y;
	}
}

void mouseMoved(int x, int y)
{
	//std::cout << "mouse moved" << std::endl;
	if (pressed) {
		float diff_x = x - prev_mouse_pos.x;
		float diff_y = y - prev_mouse_pos.y;

		cube_rotation.y += diff_x * rotate_sens;
		cube_rotation.x += diff_y * rotate_sens;

		prev_mouse_pos.x = x;
		prev_mouse_pos.y = y;
	}


}

void removeResizing(std::string szWindowTitle)
{
	long dwStyle;
	HWND hwndGlut;
	std::wstring stemp = std::wstring(szWindowTitle.begin(), szWindowTitle.end());
	hwndGlut = FindWindow(NULL, stemp.c_str());
	dwStyle = GetWindowLong(hwndGlut, GWL_STYLE);
	dwStyle ^= WS_MAXIMIZEBOX | WS_SIZEBOX | WS_THICKFRAME;
	SetWindowLong(hwndGlut, GWL_STYLE, dwStyle);
}

void initGlut() {
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glutInitWindowSize(600, 600);
	glutInitWindowPosition(0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	gluPerspective(50.0, 1.0, 1.0, 100.0);
	glMatrixMode(GL_MODELVIEW);
	gluLookAt(0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f);
	
}

void timer(int value) {
	glutPostRedisplay();      // Post re-paint request to activate display()
	glutTimerFunc(0, timer, 0); // next timer call milliseconds later
}

int main(int argc, char** argv)
{
	srand(time(NULL));
	start_pos.x *= -1.0f;

	initGlut();
	glutInit(&argc, argv);
	glutCreateWindow("Virtual Cube");
	removeResizing("Virtual Cube");


	glutMotionFunc(mouseMoved);
	glutMouseFunc(mousePressed);
	glutDisplayFunc(displayFunc);
	glutTimerFunc(0, timer, 0);

	glutMainLoop();
	return 0;
}

