// (Ethan)
// Chess animation starter kit.

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include <ctime>
using namespace std;
#include "glut.h"
#include "graphics.h"
void InitializeMyStuff();
enum PIECES {PAWN = 10, KNIGHT, BISHOP, ROOK, KING, QUEEN};
// Global Variables
// Some colors you can use, or make your own and add them
// here and in graphics.h
GLdouble redMaterial[] = {0.7, 0.1, 0.2, 1.0};
GLdouble greenMaterial[] = {0.1, 0.7, 0.4, 1.0};
GLdouble brightGreenMaterial[] = {0.1, 0.9, 0.1, 1.0};
GLdouble blueMaterial[] = {0.1, 0.2, 0.7, 1.0};
GLdouble whiteMaterial[] = {1.0, 1.0, 1.0, 1.0};
bool bishkill = false;
double screen_x = 1000;
double screen_y = 600;
double gX = 11000;
double gY = 4000;
double gZ = -4000;
double eye[3] = { gX, gY, gZ }; // pick a nice vantage point.
double at[3] = { 4500, 0,     4000 };

void DrawSphere(float x, float y, float z, float radius) {
	//draw sphere with glut solid shpere
}
void DrawCircle(double x1, double y1, double z1, double radius)
{
	glBegin(GL_POLYGON);
	for (int i = 0; i<32; i++)
	{
		double theta = (double)i / 32.0 * 2.0 * 3.1415926;
		double x = x1 + radius * cos(theta);
		double y = y1 + radius * sin(theta);
		glVertex3d(x, y, z1);
	}
	glEnd();
}
// Outputs a string of text at the specified location.
void text_output(double x, double y, char *string)
{
	void *font = GLUT_BITMAP_9_BY_15;

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
	
	int len, i;
	glRasterPos2d(x, y);
	len = (int) strlen(string);
	for (i = 0; i < len; i++) 
	{
		glutBitmapCharacter(font, string[i]);
	}

    glDisable(GL_BLEND);
}

// Given the three triangle points x[0],y[0],z[0],
//		x[1],y[1],z[1], and x[2],y[2],z[2],
//		Finds the normal vector n[0], n[1], n[2].
void FindTriangleNormal(double x[], double y[], double z[], double n[])
{
	// Convert the 3 input points to 2 vectors, v1 and v2.
	double v1[3], v2[3];
	v1[0] = x[1] - x[0];
	v1[1] = y[1] - y[0];
	v1[2] = z[1] - z[0];
	v2[0] = x[2] - x[0];
	v2[1] = y[2] - y[0];
	v2[2] = z[2] - z[0];
	
	// Take the cross product of v1 and v2, to find the vector perpendicular to both.
	n[0] = v1[1]*v2[2] - v1[2]*v2[1];
	n[1] = -(v1[0]*v2[2] - v1[2]*v2[0]);
	n[2] = v1[0]*v2[1] - v1[1]*v2[0];

	double size = sqrt(n[0]*n[0] + n[1]*n[1] + n[2]*n[2]);
	n[0] /= -size;
	n[1] /= -size;
	n[2] /= -size;
}

// Loads the given data file and draws it at its default position.
// Call glTranslate before calling this to get it in the right place.
void DrawPiece(char filename[])
{
	// Try to open the given file.
	char buffer[200];
	ifstream in(filename);
	if(!in)
	{
		cerr << "Error. Could not open " << filename << endl;
		exit(1);
	}

	double x[100], y[100], z[100]; // stores a single polygon up to 100 vertices.
	int done = false;
	int verts = 0; // vertices in the current polygon
	int polygons = 0; // total polygons in this file.
	do
	{
		in.getline(buffer, 200); // get one line (point) from the file.
		int count = sscanf_s(buffer, "%lf, %lf, %lf", &(x[verts]), &(y[verts]), &(z[verts]));
		done = in.eof();
		if(!done)
		{
			if(count == 3) // if this line had an x,y,z point.
			{
				verts++;
			}
			else // the line was empty. Finish current polygon and start a new one.
			{
				if(verts>=3)
				{
					glBegin(GL_POLYGON);
					double n[3];
					FindTriangleNormal(x, y, z, n);
					glNormal3dv(n);
					for(int i=0; i<verts; i++)
					{
						glVertex3d(x[i], y[i], z[i]);
					}
					glEnd(); // end previous polygon
					polygons++;
					verts = 0;
				}
			}
		}
	}
	while(!done);

	if(verts>0)
	{
		cerr << "Error. Extra vertices in file " << filename << endl;
		exit(1);
	}

}

int Interpolate(double t, double t1, double t2, double &v, double v1, double v2) {
	if (t < t1) {
		t = t1;
	}
	if (t > t2) {
		t = t2;
	}
	double ratio = (t - t1) / (t2 - t1);
	v = v1 + ratio * (v2 - v1);
	if (t == t2) { return 1; }
	return 0;
}

// NOTE: Y is the UP direction for the chess pieces.


//
// GLUT callback functions
//

// This callback function gets called by the Glut
// system whenever it decides things need to be redrawn.
void drawBoard(double y) {
	glBegin(GL_QUADS);
	GLfloat board1[] = { 0.0, 4.0, 0.0, 1.0 };
	GLfloat board2[] = { 0.2, 0.9, 0.1, 1.0 };
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j <= 8; j++) {
			//counterclockwise order for opengl when using quads
			if ((i + j) % 2 == 0) {
				glMaterialfv(GL_FRONT, GL_DIFFUSE, board1);
			}
			else {
				glMaterialfv(GL_FRONT, GL_DIFFUSE, board2);
			}
			//beveling of the edges. based on the edge, need to change the vector. so draw all edges for the test
			//anything with lighting on needs to have the glnormal
			glNormal3d(0, 1, 0);
			double offset_x = 500;
			double offset_z = 500;
			glVertex3d(i * 1000 + offset_x, y, j * 1000 + offset_z);
			glVertex3d(i * 1000 + offset_x, y, (j + 1) * 1000 + offset_z);
			glVertex3d((i + 1) * 1000 + offset_x, y, (j + 1) * 1000 + offset_z);
			glVertex3d((i + 1) * 1000 + offset_x, y, j * 1000 + offset_z);
		}
	}
	glEnd();
}
void drawTable() {
	glBegin(GL_QUADS);
	GLfloat board1[] = { 0.1, 0.1, 1, 1.0 };
			glMaterialfv(GL_FRONT, GL_DIFFUSE, board1);

			glNormal3d(0, 1, 0);
			glVertex3d(10000, -500, -1000);
			glVertex3d(-1000, -500, -1000);
			glVertex3d(-1000,-500,11000);
			glVertex3d(10000,-500,11000);
			
	glEnd();
}
void drawBox(double x1, double y1, double z1, double x2, double y2, double z2) {
	//reset the material before the call of this function
	GLfloat box[] = { 1.0, 1.0, 1.0, 0.3 };
	glBegin(GL_QUADS);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, box);
	//front face
	glNormal3d(0, 0, -1); // neg z dir
	glVertex3d(x1, y1, z1);
	glVertex3d(x2, y1, z1);
	glVertex3d(x2, y2, z1);
	glVertex3d(x1, y2, z1);
	//right face
	glNormal3d(-1, 0, 0); //pos x dir
	glVertex3d(x2, y1, z1);
	glVertex3d(x2, y1, z2);
	glVertex3d(x2, y2, z2);
	glVertex3d(x2, y2, z1);
	//left face
	glNormal3d(1, 0, 0); //neg x dir
	glVertex3d(x1, y1, z2);
	glVertex3d(x1, y1, z1);
	glVertex3d(x1, y2, z1);
	glVertex3d(x1, y2, z2);
	//rear face
	glNormal3d(0, 0, 1); //pos z dir
	glVertex3d(x1, y1, z2);
	glVertex3d(x2, y1, z2);
	glVertex3d(x2, y2, z2);
	glVertex3d(x1, y2, z2);
	//draw the rest of the faces
	glEnd();
}
void display(void)
{
	int interpolate_done = 0;
	static clock_t start = clock();
	clock_t finish = clock();
	double t = double(finish - start) / CLOCKS_PER_SEC;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();
	gluLookAt(eye[0], eye[1], eye[2],  at[0], at[1], at[2],  0,1,0); // Y is up!

	// Set the color for one side (white), and draw its 16 pieces.
	GLfloat mat_amb_diff1[] = {0.4, 0.05, 0.05, 1.0};
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat_amb_diff1);

	
	double king_y = 0;
	double move_z = 2000;
	double king_x = 4000;
	double king_z = 1000;
	//Interpolate(t, 0, 1, king_y, 0, 1000);
	//Interpolate(t, 1, 1.5, king_z, 1000, move_z + 1000);
	//Interpolate(t, 1.5, 2, king_y, 1000, 0);
	glPushMatrix();
	glTranslatef(king_x, king_y, king_z);
	//DrawPiece("KING.POL");
	glCallList(KING);
	glPopMatrix();

	double queen_z = 1000;
	double queen_x2 = 5000;
	Interpolate(t, 3, 4, queen_z, 1000, 4000);
	Interpolate(t, 3, 4, queen_x2, 5000, 2000);
	glPushMatrix();
	glTranslatef(queen_x2, 0, queen_z);
	//DrawPiece("QUEEN.POL");
	glCallList(QUEEN);
	glPopMatrix();

	double bish_rot = 0;
	Interpolate(t, 0, 1000, bish_rot, 0, 360 * 1000);
	glPushMatrix();
	glTranslatef(3000, 0, 1000);
	//make bishop wobble. rotate the top only
	glRotated(bish_rot, 0, 1, .1);
	glCallList(BISHOP);
	glPopMatrix();

	double bish_rot4 = 0;
	Interpolate(t, 0, 1000, bish_rot4, 0, 360 * 1000);
	glPushMatrix();
	glTranslatef(6000, 0, 1000);
	//make bishop wobble. rotate the top only
	glRotated(bish_rot4, 0, 1, .1);
	glCallList(BISHOP);
	glPopMatrix();

	double knighty1 = 0;
	double knighty2 = 1000;
	double knightz2 = 1000;
	double knightx2 = 2000;
	int interpolate_done3;
	int interpolate_done2;
	interpolate_done3 = Interpolate(t, 5, 5.25, knighty1, 0, 1000);
	if (interpolate_done3 == 1) {
		interpolate_done2 = Interpolate(t, 5.25, 5.75, knightz2, 1000, 3000);
		Interpolate(t, 5.25, 5.75, knightx2, 2000, 1000);
	}
	if (interpolate_done2 == 1) {
		Interpolate(t, 5.75, 6, knighty2, 1000, 0);
	}
	glPushMatrix();
	if (interpolate_done3 == 0) {
		glTranslatef(2000, knighty1, 1000);
	}
	if (interpolate_done3 == 1 && interpolate_done2 == 0) {
		glTranslatef(knightx2, 1000, knightz2);
	}
	if (interpolate_done2 == 1) {
		glTranslatef(1000, knighty2, 3000);
	}
	glCallList(KNIGHT);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(7000, 0, 1000);
	glCallList(KNIGHT);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(1000, 0, 1000);
	glCallList(ROOK);
	glPopMatrix();
	glPushMatrix();
	glTranslatef(8000, 0, 1000);
	glCallList(ROOK);
	glPopMatrix();

	/*make the bishop go off the screen while rotating
	double bishop_x = 6000;
	double bishop_z = 1000;
	double bishop_rot = 0;
	Interpolate(t, 2, 4, bishop_x, 6000, -4000);
	Interpolate(t, 2, 4, bishop_z, 1000, 4000);
	Interpolate(t, 2, 4, bishop_rot, 0, 4000);

	glPushMatrix();
	glTranslatef(bishop_x, 0, bishop_z);
	//glTranslatef(0, 0, 700);
	glRotated(bishop_rot, 0, 0, 1);
	//glTranslatef(0, -700, 0);
	glCallList(BISHOP);
	glPopMatrix();
	*/
	for (int x = 1000; x <= 8000; x += 1000)
	{
		if (x == 4000) {
			continue;
		}
		if (bishkill == 1 && x == 5000) {
			continue;
		}
		glPushMatrix();
		glTranslatef(x, 0, 2000);
		//DrawPiece("PAWN.POL");
		glCallList(PAWN);
		glPopMatrix();
	}
	double pawnz1 = 2000;
	Interpolate(t, 1, 2, pawnz1, 2000, 4000);
	glPushMatrix();
	glTranslatef(4000, 0, pawnz1);
	//DrawPiece("QUEEN.POL");
	glCallList(PAWN);
	glPopMatrix();
	if (bishkill) {
		double pawnkillx = 5000;
		double pawnkillz = 2000;
		double pawnkilly = 0;
		double pawnkilly2 = 2000;
		double pawnkillrot = 0;
		double deg2 = -20;
		double deg3 = -90;
		int done = 0;
		int done2 = 0;
		int done3 = 0;
		Interpolate(t, 7, 8, pawnkillx, 5000, 9000);
		Interpolate(t, 7, 8, pawnkillz, 2000, 5000);
		done = Interpolate(t, 7, 7.5, pawnkilly, 0, 3000);
		Interpolate(t, 7, 8, pawnkillrot, 0, -540);
		if (done == 1) {
			done2 = Interpolate(t, 7.5, 8, pawnkilly, 3000, -200);
		}
		glPushMatrix();
		if (done3 != 1 && done2 == 1) {
			glTranslatef(9000, -200, 5000);
			done3 = Interpolate(t, 8, 8.15, deg2, -40, -90); // rotate based on z axis, so we can change the a different axis to have the piece rotate another way
			glRotated(deg2, 0, 0, 1);
		}
		if (done2 != 1) {
			glTranslatef(pawnkillx, pawnkilly, pawnkillz);
			glTranslatef(0, 0, 700);
			glRotated(pawnkillrot, 0, 0, 1);
			glTranslatef(0, -700, 0);
		}
		glCallList(PAWN);
		glPopMatrix();

	}

	// Set the color for one side (black), and draw its 16 pieces.
	GLfloat mat_amb_diff2[] = {0.05, 0.05, 0.2, 1.0};
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat_amb_diff2);
	

	glPushMatrix();
	glTranslatef(5000, 0, 9000);
	double deg = 0;
	//Interpolate(t, 0, 2, deg, 0, -90); // rotate based on z axis, so we can change the a different axis to have the piece rotate another way
	//glRotated(deg, 1, 0, 0);
	glCallList(KING);
	glPopMatrix();

	double queen_z2 = 9000;
	double queen_x1 = 4000;
	Interpolate(t, 2, 3, queen_z2, 9000, 5000);
	Interpolate(t, 2, 3, queen_x1, 4000, 8000);
	glPushMatrix();
	glTranslatef(queen_x1, 0, queen_z2);
	//DrawPiece("QUEEN.POL");
	glCallList(QUEEN);
	glPopMatrix();

	double bish_rot2 = 0;
	Interpolate(t, 0, 1000, bish_rot2, 0, 360 * 1000);
	glPushMatrix();
	glTranslatef(3000, 0, 9000);
	//make bishop wobble. rotate the top only
	glRotated(bish_rot2, 0, 1, .1);
	glCallList(BISHOP);
	glPopMatrix();

	double bish_rot3 = 0;
	double bishx2 = 6000;
	double bishz2 = 9000;
	double bishx3 = 2000;
	double bishz3 = 5000;
	int interpolate_done11;
	Interpolate(t, 0, 1000, bish_rot3, 0, 360 * 1000);
	Interpolate(t, 4, 5, bishx2, 6000, 2000);
	interpolate_done = Interpolate(t, 4, 5, bishz2, 9000, 5000);
	if (interpolate_done == 1) {
		Interpolate(t, 6, 7, bishx3, 2000, 5000);
		interpolate_done11 = Interpolate(t, 6, 7, bishz3, 5000, 2000);
	}
	glPushMatrix();
	if (interpolate_done == 0) {
		glTranslatef(bishx2, 0, bishz2);
	}
	if (interpolate_done == 1) {
		glTranslatef(bishx3, 0, bishz3);
		if (interpolate_done11 == 1) {
			bishkill = true;
		}
	}
	//make bishop wobble. rotate the top only
	//glRotated(10, 0, 0, 1);
	glRotated(bish_rot3, 0, 1, .1);
	glCallList(BISHOP);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(2000, 0, 9000);
	glCallList(KNIGHT);
	glPopMatrix();
	glPushMatrix();
	glTranslatef(7000, 0, 9000);
	glCallList(KNIGHT);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(1000, 0, 9000);
	glCallList(ROOK);
	glPopMatrix();
	glPushMatrix();
	glTranslatef(8000, 0, 9000);
	glCallList(ROOK);
	glPopMatrix();

	for(int x=1000; x<=8000; x+=1000)
	{
		if (x == 5000) {
			continue;
		}
		glPushMatrix();
		glTranslatef(x, 0, 8000);
		//DrawPiece("PAWN.POL");
		glCallList(PAWN);
		glPopMatrix();
	}
	double pawnz = 8000;
	Interpolate(t, 0, 1, pawnz, 8000, 6000);
	glPushMatrix();
	glTranslatef(5000, 0, pawnz);
	//DrawPiece("QUEEN.POL");
	glCallList(PAWN);
	glPopMatrix();
	/* 
	glPushMatrix();
	glTranslatef(3000, 800, 300);
	glutSolidSphere(800, 50, 50);
	glPopMatrix();
	*/

	//bevel on the board
	drawBox(8500,-500,500,500,0,9500);
	//table
	drawTable();
	drawBox(10000, -1000, -1000, -1000, -500, 11000);
	//table legs
	drawBox(10000, -10000, -1000, 9500, -1000, -500);
	drawBox(10000, -10000, 10500, 9500, -1000, 11000);
	drawBox(-500, -10000, -1000, -1000, -1000, -500);
	drawBox(-500, -10000, 10500, -1000, -1000, 11000);
	drawBoard(0);
	glutSwapBuffers();
	glutPostRedisplay();
}


// This callback function gets called by the Glut
// system whenever a key is pressed.
void keyboard(unsigned char c, int x, int y)
{
	switch (c) 
	{
		case 27: // escape character means to quit the program
			exit(0);
			break;
		case GLUT_KEY_F1:
			gZ += 1000;
			break;
		case 'l':
			eye[0] += 1000;
			break;
		case 'r':
			eye[0] -= 1000;
			break;
		case 'i':
			eye[2] += 1000;
			break;
		case 'o':
			eye[2] -= 1000;
			break;
		case 'u':
			eye[1] += 1000;
			break;
		case 'd':
			eye[1] -= 1000;
			break;
		default:
			return; // if we don't care, return without glutPostRedisplay()
	}

	glutPostRedisplay();
}



void SetPerspectiveView(int w, int h)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	double aspectRatio = (GLdouble) w/(GLdouble) h;
	gluPerspective( 
	/* field of view in degree */ 45.0,
	/* aspect ratio */ aspectRatio,
	/* Z near */ 30, /* Z far */ 30000.0);
	glMatrixMode(GL_MODELVIEW);
}

// This callback function gets called by the Glut
// system whenever the window is resized by the user.
void reshape(int w, int h)
{
	screen_x = w;
	screen_y = h;

	// Set the pixel resolution of the final picture (Screen coordinates).
	glViewport(0, 0, w, h);

	SetPerspectiveView(w,h);

}

// This callback function gets called by the Glut
// system whenever any mouse button goes up or down.
void mouse(int mouse_button, int state, int x, int y)
{
	if (mouse_button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) 
	{
	}
	if (mouse_button == GLUT_LEFT_BUTTON && state == GLUT_UP) 
	{
	}
	if (mouse_button == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN) 
	{
	}
	if (mouse_button == GLUT_MIDDLE_BUTTON && state == GLUT_UP) 
	{
	}
	glutPostRedisplay();
}

// Your initialization code goes here.
void InitializeMyStuff()
{
	// set material's specular properties
	GLfloat mat_specular[] = {1.0, 1.0, 1.0, 1.0};
	GLfloat mat_shininess[] = {50.0};
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

	// set light properties
	GLfloat light_position[] = { 1,2,-.1,0 }; //if fourth paramter is a one, then it a positional light. the light goes in every direction
											  // if its a zero then the light comes from the vector, or one source
	GLfloat white_light[] = {1,1,1,1};
	GLfloat low_light[] = {.3,.3,.3,1};
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	//glLightfv(GL_LIGHT0, GL_POSITION, light_position); // position first light
	glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light); // specify first light's color
	glLightfv(GL_LIGHT0, GL_SPECULAR, low_light);

	glEnable(GL_DEPTH_TEST); // turn on depth buffering
	glEnable(GL_LIGHTING);	// enable general lighting
	glEnable(GL_LIGHT0);	// enable the first light.
	glNewList(PAWN, GL_COMPILE);
		DrawPiece("PAWN.POL");
	glEndList();
	glNewList(KING, GL_COMPILE);
		DrawPiece("KING.POL");
	glEndList();
	glNewList(BISHOP, GL_COMPILE);
		DrawPiece("BISHOP.POL");
	glEndList();
	glNewList(ROOK, GL_COMPILE);
		DrawPiece("ROOK.POL");
	glEndList();
	glNewList(QUEEN, GL_COMPILE);
		DrawPiece("QUEEN.POL");
	glEndList();
	glNewList(KNIGHT, GL_COMPILE);
		DrawPiece("KNIGHT.POL");
	glEndList();
}


int main(int argc, char **argv)
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(screen_x, screen_y);
	glutInitWindowPosition(10, 10);

	int fullscreen = 0;
	if (fullscreen) 
	{
		glutGameModeString("800x600:32");
		glutEnterGameMode();
	} 
	else 
	{
		glutCreateWindow("Chess");
	}

	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouse);

	glClearColor(1,1,1,1);	
	InitializeMyStuff();

	glutMainLoop();

	return 0;
}
