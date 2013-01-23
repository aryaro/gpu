/*
 * pyramid.cpp
 *
 *  Created on: 08-11-2012
 *      Author: s
 */


#include<GL/gl.h>
#include<GL/glu.h>
#include<GL/glut.h>
#include<stdio.h>
#include <stdlib.h>
#include<unistd.h>


int window;

float rtri = 0.0f;
float rquad = 0.0f;

#define ESCAPE 27
unsigned int texture[1];

int light;

/* L pressed (1 = yes, 0 = no) */
int lp;

/* F pressed (1 = yes, 0 = no) */
int fp;

/* white ambient light at half intensity (rgba) */
GLfloat LightAmbient[] = { 0.5f, 0.5f, 0.5f, 1.0f };

/* super bright, full intensity diffuse light. */
GLfloat LightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };

/* position of light (x, y, z, (position of light)) */
GLfloat LightPosition[] = { 0.0f, 0.0f, 2.0f, 1.0f };

/* Image type - contains height, width, and data */
struct Image {
    unsigned long sizeX;
    unsigned long sizeY;
    char *data;
};
typedef struct Image Image;

// quick and dirty bitmap loader...for 24 bit bitmaps with 1 plane only.
// See http://www.dcs.ed.ac.uk/~mxr/gfx/2d/BMP.txt for more info.
int ImageLoad(char *filename, Image *image) {
    FILE *file;
    unsigned long size;                 // size of the image in bytes.
    unsigned long i;                    // standard counter.
    unsigned short int planes;          // number of planes in image (must be 1)
    unsigned short int bpp;             // number of bits per pixel (must be 24)
    char temp;                          // temporary color storage for bgr-rgb conversion.

    // make sure the file is there.
    if ((file = fopen(filename, "rb"))==NULL)
    {
	printf("File Not Found : %s\n",filename);
	return 0;
    }

    // seek through the bmp header, up to the width/height:
    fseek(file, 18, SEEK_CUR);

    // read the width
    if ((i = fread(&image->sizeX, 4, 1, file)) != 1) {
	printf("Error reading width from %s.\n", filename);
	return 0;
    }
    printf("Width of %s: %lu\n", filename, image->sizeX);

    // read the height
    if ((i = fread(&image->sizeY, 4, 1, file)) != 1) {
	printf("Error reading height from %s.\n", filename);
	return 0;
    }
    printf("Height of %s: %lu\n", filename, image->sizeY);

    // calculate the size (assuming 24 bits or 3 bytes per pixel).
    size = image->sizeX * image->sizeY * 3;

    // read the planes
    if ((fread(&planes, 2, 1, file)) != 1) {
	printf("Error reading planes from %s.\n", filename);
	return 0;
    }
    if (planes != 1) {
	printf("Planes from %s is not 1: %u\n", filename, planes);
	return 0;
    }

    // read the bpp
    if ((i = fread(&bpp, 2, 1, file)) != 1) {
	printf("Error reading bpp from %s.\n", filename);
	return 0;
    }
    if (bpp != 24) {
	printf("Bpp from %s is not 24: %u\n", filename, bpp);
	return 0;
    }

    // seek past the rest of the bitmap header.
    fseek(file, 24, SEEK_CUR);

    // read the data.
    image->data = (char *) malloc(size);
    if (image->data == NULL) {
	printf("Error allocating memory for color-corrected image data");
	return 0;
    }

    if ((i = fread(image->data, size, 1, file)) != 1) {
	printf("Error reading image data from %s.\n", filename);
	return 0;
    }

    for (i=0;i<size;i+=3) { // reverse all of the colors. (bgr -> rgb)
	temp = image->data[i];
	image->data[i] = image->data[i+2];
	image->data[i+2] = temp;
    }

    // we're done.
    return 1;
}

// Load Bitmaps And Convert To Textures
void LoadGLTextures() {
    // Load Texture
    Image *image1;

    // allocate space for texture
    image1 = (Image *) malloc(sizeof(Image));
    if (image1 == NULL) {
	printf("Error allocating space for image");
	exit(0);
    }

    if (!ImageLoad("Data/crate.bmp", image1)) {
	exit(1);
    }

    // Create Texture
    glGenTextures(1, &texture[0]);
    glBindTexture(GL_TEXTURE_2D, texture[0]);   // 2d texture (x and y size)

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // scale linearly when image bigger than texture
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // scale linearly when image smalled than texture

    // 2d texture, level of detail 0 (normal), 3 components (red, green, blue), x size from image, y size from image,
    // border 0 (normal), rgb color data, unsigned byte data, and finally the data itself.
    glTexImage2D(GL_TEXTURE_2D, 0, 3, image1->sizeX, image1->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, image1->data);
};

void InitGL(int width, int height )
{

    LoadGLTextures();
    glEnable(GL_TEXTURE_2D);


	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glClearDepth( 1.0 );
	glDepthFunc( GL_LESS );
	glEnable( GL_DEPTH_TEST );
	glShadeModel( GL_SMOOTH );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 45.0f, ( (GLfloat)width / (GLfloat)height ), 0.1f, 100.0f);

	glMatrixMode( GL_MODELVIEW );

    glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient);  // add lighting. (ambient)
    glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);  // add lighting. (diffuse).
    glLightfv(GL_LIGHT1, GL_POSITION,LightPosition); // set light position.
    glEnable(GL_LIGHT1);                             // turn light 1 on.


}

void onResize( int width, int height )
{

	if( height == 0)
	{
		height = 1;
	}

	glViewport( 0, 0, width, height ); // reset aktualnego widoku
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

	gluPerspective( 45.0f, ( (GLfloat)width / (GLfloat)height ), 0.1f, 100.0f);

	glMatrixMode( GL_MODELVIEW );
}

void leftPirmyd()
{

	glBindTexture(GL_TEXTURE_2D, texture[0]);

	glTranslatef( -1.0f, 0.0f, -16.0f ); //przesuwamy widok
	glRotatef( rtri, 0.0f, 0.1f, 0.0f );

	//

	glBegin( GL_POLYGON );
		glColor3f( 1.0f, 0.0f, 0.0f );
		glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.0f, 1.0f, 0.0f );
		glTexCoord2f(1.0f, 0.0f); glVertex3f( -1.0f, -1.0f, 1.0f );
		glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f, -1.0f, 1.0f );

		 glColor3f( 0.0f, 1.0f, 0.0f );
		 glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.0f, 1.0f, 0.0f );
		 glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f, 1.0f );
		 glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f );

		glColor3f( 0.0f, 0.0f, 1.0f );
		glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.0f, 1.0f, 0.0f );
		glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f );
		glTexCoord2f(1.0f, 1.0f); glVertex3f( -1.0f, -1.0f, -1.0f );

		glColor3f( 0.0f, 1.0f, 1.0f );
		glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.0f, 1.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f( -1.0f,-1.0f,-1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f,-1.0f, 1.0f);

	glEnd();


   glBegin(GL_QUADS);
   	   glColor3f( 1.0f, 1.0f, 1.0f );

   	    glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
   	 	glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f, 1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 1.0f);
	glEnd();

	glTranslatef( 0.0f, 0.0f, 0.0f ); //przesuwamy widok

}

void rightPirmyd()
{

	glTranslatef( 8.0f, 0.0f, 0.0f ); //przesuwamy widok
	glRotatef( rtri, 0.0f, 0.1f, 0.0f );

	glBegin( GL_POLYGON );
		glColor3f( 1.0f, 0.0f, 0.0f );
		glVertex3f( 0.0f, 1.0f, 0.0f );
		glVertex3f( -1.0f, -1.0f, 1.0f );
		glVertex3f( 1.0f, -1.0f, 1.0f );

		glColor3f( 0.0f, 1.0f, 0.0f );
		glVertex3f( 0.0f, 1.0f, 0.0f );
		glVertex3f( 1.0f, -1.0f, 1.0f );
		glVertex3f( 1.0f, -1.0f, -1.0f );

		glColor3f( 0.0f, 0.0f, 1.0f );
		glVertex3f( 0.0f, 1.0f, 0.0f );
		glVertex3f( 1.0f, -1.0f, -1.0f );
		glVertex3f( -1.0f, -1.0f, -1.0f );

		glColor3f( 0.0f, 1.0f, 1.0f );
	    glVertex3f( 0.0f, 1.0f, 0.0f);
	    glVertex3f( -1.0f,-1.0f,-1.0f);
	    glVertex3f(-1.0f,-1.0f, 1.0f);

	glEnd();

   glBegin(GL_QUADS);
   	    glColor3f( 1.0f, 1.0f, 1.0f );

   	    glVertex3f(-1.0f, -1.0f, -1.0f);
		glVertex3f( 1.0f, -1.0f, -1.0f);
		glVertex3f( 1.0f, -1.0f, 1.0f);
		glVertex3f(-1.0f, -1.0f, 1.0f);
	glEnd();

	glTranslatef( 0.0f, 0.0f, -16.0f ); //przesuwamy widok

}


void DrawGlScene()
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); // czysci ekran i bufor
	glLoadIdentity(); // resetuj wyglad

	gluLookAt(0, 2, 3, 0, 2, 0, 0, 1, 0);

	leftPirmyd();
	rightPirmyd();


	glLoadIdentity();

	rtri += 1.0f;
	rquad -= 15.0f;

	glutSwapBuffers(); //mamy podwojny bufoe w glClear dlatego musimy je polaczyc

}


void onKeyPress( unsigned char key, int x, int y)
{
	usleep( 1000 );

//	if( key == ESCAPE )
//	{
//		glutDestroyWindow( window );
//		exit( 0 );
//
//	}

    switch (key) {
		case ESCAPE: // kill everything.
		/* shut down our window */
		glutDestroyWindow(window);

		/* exit the program...normal termination. */
		exit(1);
		break; // redundant.

		case 76:
		case 108: // switch the lighting.
		printf("L/l pressed; light is: %d\n", light);
		light = light ? 0 : 1;              // switch the current value of light, between 0 and 1.
		printf("Light is now: %d\n", light);
		if (!light) {
			glDisable(GL_LIGHTING);
		} else {
			glEnable(GL_LIGHTING);
		}
		break;

		default:
		break;
    }

}

int main(int argc, char **argv)
{
	glutInit( &argc, argv );

	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH );

	glutInitWindowSize( 640, 480);
	glutInitWindowPosition( 0, 0 );

	window = glutCreateWindow( "Pierwsze okno" );

	//rejstruje funkcje ktora bedzie wszystko rysowala
	glutDisplayFunc( &DrawGlScene );

	//glutFullScreen();

	//to jest od eventw, ale podobno dobrze jest zrestartowac obraz nawet jak ich nie ma
	glutIdleFunc( &DrawGlScene );

	glutKeyboardFunc( &onKeyPress );

	InitGL( 640, 480 );

	glutMainLoop();

	return 1;
}
