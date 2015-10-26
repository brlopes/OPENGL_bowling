//most code by Prof. Calvacanti
//Horse model by me
//=========================================================//
/*Task 3 B – Bowling Simulator / Oct. 21st

Implement a physically base billiard simulator:
1. Render the 3D Workspace using GLUquadricObj; (1 point)
2. Render the bowling runway, the pins, including the red ball and at least 6 pins; (2 points)
3. The area representing where the pins fall in; (1 point)
4. Implement collision detection to avoid object enter each other; (1 point)
5. Every time the red ball hits the target, the pins should split moving in expected/random directions; (2 points)
6. Implement sound effect: activate the sound every time the ball hits the targets; (1 point)
7. Use SDL/GLUT for interactive input, allowing the user to choose different speeds and direction for hitting the ball: slow, medium or fast. (2 point)
 *
 */
//=========================================================//
#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
//=========================================================//
//=========================================================//
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
//=========================================================//
//=========================================================//
#define window_width  640
#define window_height 480
//=========================================================//
//=========================================================//
// person position in the environment
void move_camera(void);
static void keyboard(void);
void update_camera();

GLdouble  g_playerPos[] = { 0.0, 3, 10.0 };//{ 0.0, 0.5, 10.0 };  //start at 5 height
GLdouble  g_lookAt[] = { 0.0, 0.0, 0.0 };
GLfloat   g_viewAngle = -90.0;
GLfloat   g_elevationAngle = 0.0;
GLfloat   change_collor = 1.0;
float rad =0;
const float DEFAULT_SPEED   = 0.4f;

int ball_speed_multi = 1.0;
bool isgalloping = false;
int speedfactor = 4;
GLfloat hanim = 0.0;
bool goingup = true;
//=========================================================//
//=========================================================//
// Collision detection
GLfloat ball_radius = 0.3f;
GLfloat p2_radius = 0.3f;
GLfloat p1_x = -2.0f;
GLfloat p2_x = 2.0f;
GLfloat p1_y = 2.0f;
GLfloat p2_y = 2.0f;
GLfloat p1_z = 5.0f;
GLfloat p2_z = 5.0f;
GLfloat change_direction = 1.0;
GLfloat ball_roll_speed = -8.0;
GLfloat ball_roll_angle = 1;

bool ballrolling = false;
bool angle_selected = false;
GLfloat ball_direction = 0.0;

//=========================================================//
const int   WORLD_SIZE = 100;
static void text_onScreen(int row, int col, const char *fmt, ...);
//=========================================================//
// quadric objects

GLUquadricObj *g_normalObject     = NULL;
void init_data(void);
GLvoid  DrawGround();
GLvoid  DrawNormalObjects(GLfloat gallop);

void setup_sceneEffects(void);
void makeSound(void);
void backgroundMusic(void);
void openingAudio(void);
void closingAudio(void);

//=========================================================//
//=========================================================//
// Keydown booleans
bool key[321];
// Process pending events
bool events()
{
	SDL_Event event;
	if( SDL_PollEvent(&event) )
	{
		switch( event.type )
		{
			case SDL_KEYDOWN : key[ event.key.keysym.sym ]=true ;   break;
			case SDL_KEYUP   : key[ event.key.keysym.sym ]=false;   break;
			case SDL_QUIT    : return false; break;
		}
	}
	return true;
}
//=========================================================//
//=========================================================//
static void text_onScreen (int row, int col, const char *fmt, ...)
{
    static char buf[256];
    int viewport[4];
    va_list args;

    va_start(args, fmt);
    (void) vsprintf (buf, fmt, args);
    va_end(args);

    glGetIntegerv(GL_VIEWPORT,viewport);

    glPushMatrix();
    glLoadIdentity();

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    glOrtho(0,viewport[2],0,viewport[3],-1,1);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}
//=========================================================//
//=========================================================//
void setup_sceneEffects(void)
{
  // enable lighting
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

  // enable using glColor to change material properties
  // we'll use the default glColorMaterial setting (ambient and diffuse)
  glEnable(GL_COLOR_MATERIAL);

  // set the default blending function
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // set up the fog parameters for reflections
  glFogi(GL_FOG_MODE, GL_LINEAR);
  glFogf(GL_FOG_START, -100.0);
  glFogf(GL_FOG_END, 100.0);

  // enable line anti-aliasing and make the lines slightly bigger than default
  glEnable(GL_LINE_SMOOTH);
  glLineWidth(1.5f);
}
//=========================================================//
//=========================================================//
void init_data(void)
{
  setup_sceneEffects();
  // create a normal quadric (uses default settings)
  g_normalObject = gluNewQuadric();

}
//=========================================================//
//=========================================================//
GLvoid DrawGround()
{ // enable blending for anti-aliased lines
  glEnable(GL_BLEND);

  // set the color to a bright blue
  glColor3f(0, 0, 0); // from glColor3f(0.5f, 0.7f, 1.0f);

  // draw the lines
  glBegin(GL_LINES);
    for (int x = -WORLD_SIZE; x < WORLD_SIZE; x += 6)
    {
      glVertex3i(x, 0, -WORLD_SIZE);
      glVertex3i(x, 0, WORLD_SIZE);
    }

    for (int z = -WORLD_SIZE; z < WORLD_SIZE; z += 6)
    {
      glVertex3i(-WORLD_SIZE, 0, z);
      glVertex3i(WORLD_SIZE, 0, z);
    }
  glEnd();

  // turn blending off
  glDisable(GL_BLEND);
} // end DrawGround()
//=========================================================//
//=========================================================//
GLvoid drawCollision()
{
	GLfloat alphaTransparency = 0.5;
	GLfloat distance;

	// move forward and backward
	p1_x+=0.05f*change_direction;
	p2_x-=0.05f*change_direction;

	if(p1_x > 2){
		change_direction=1.0f-2.0f;
	}
	if(p1_x < -2){
		change_direction=1.0f;
	}
	// check-collision
	distance = sqrt(( (p1_x - p2_x) * (p1_x - p2_x) )
			         + ((p1_y - p2_y) * (p1_y - p2_y))
			         + ((p1_z - p2_z) * (p1_z - p2_z)));
	//if (distance <= p2_radius + p1_radius){
	  // Red :: collision
	  change_collor = 0.0;
	  makeSound();
	//}else{
	  // Yellow :: no collision
	  //change_collor = 1.0;
	//}
    // enable blending for transparent sphere
    glEnable(GL_BLEND);     // Turn Blending On
    glDisable(GL_DEPTH_TEST);   // Turn Depth Testing Off
	   glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		  // first sphere collides against the other
		  glPushMatrix();
			glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, rand() % 128);
			glColor4f(1.0f,change_collor,0.0f,alphaTransparency);
			glTranslatef(p1_x, p1_y, p1_z);
		//	gluSphere(g_normalObject, p1_radius, 16, 10);
		  glPopMatrix();
		  // second sphere collides against the first
		  glPushMatrix();
			glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, rand() % 128);
			glColor4f(1.0f,change_collor,0.0f,alphaTransparency);
			glTranslatef(p2_x, p2_y, p2_z);
			gluSphere(g_normalObject, p2_radius, 16, 10);
		  glPopMatrix();

	glDisable(GL_BLEND);        // Turn Blending Off
	glEnable(GL_DEPTH_TEST);    // Turn Depth Testing On
}

void drawAlley()
{

	  //bowl alley:
	  glPushMatrix();
		glColor3ub(153,102,51);
		glTranslatef(0.0f, 0.0f, -10.0f);

	    //ground
		glPushMatrix();
			glRotatef(45, 0, 1, 0);
			glBegin(GL_POLYGON);
				glVertex3f(20.0f, 0.0f, 0.0f);
				glVertex3f(0.0f, 0.0f, 20.0f);
				glVertex3f(-20.0f, 0.0f, 0.0f);
				glVertex3f(0.0f, 0.0f, -20.0f);
			glEnd();
	    glPopMatrix();

		//left_lane
	    glPushMatrix();
			glColor3ub(255,0,0);
			glRotatef(90, 0, 1, 0);
			glTranslatef(-5.0f, 0.0f, 0.0f);

			glBegin(GL_POLYGON);  //left wall
				glVertex3f(2.0, 0.0, 0.0);
				glVertex3f(12.0, 0.0, 0.0);
				glVertex3f(12.0, 0.5, 0.0);
				glVertex3f(2.0, 0.5, 0.0);
			glEnd();

			glTranslatef(0.0f, 0.0f, 0.5f);
			glBegin(GL_POLYGON);  //right wall
				glVertex3f(2.0, 0.0, 0.0);
				glVertex3f(12.0, 0.0, 0.0);
				glVertex3f(12.0, 0.5, 0.0);
				glVertex3f(2.0, 0.5, 0.0);
			glEnd();

			glTranslatef(0.0f, 0.5f, -0.5f);
			glRotatef(90, 1, 0, 0);
			glBegin(GL_POLYGON);  //top wall
				glVertex3f(2.0, 0.0, 0.0);
				glVertex3f(12.0, 0.0, 0.0);
				glVertex3f(12.0, 0.5, 0.0);
				glVertex3f(2.0, 0.5, 0.0);
			glEnd();

			glRotatef(90, 0, 1, 0);
			glTranslatef(-3, 0.0, 2); // xy reversed
			glBegin(GL_POLYGON);  //front closing square
				glVertex3f(2.0, 0.0, 0.0);
				glVertex3f(3, 0.0, 0.0);
				glVertex3f(3, 0.5, 0.0);
				glVertex3f(2.0, 0.5, 0.0);
			glEnd();
		glPopMatrix();


		//right_lane
		glTranslatef(3, 0, 0);
	    glPushMatrix();
			glColor3ub(255,0,0);
			glRotatef(90, 0, 1, 0);
			glTranslatef(-5.0f, 0.0f, 0.0f);

			glBegin(GL_POLYGON);  //left wall
				glVertex3f(2.0, 0.0, 0.0);
				glVertex3f(12.0, 0.0, 0.0);
				glVertex3f(12.0, 0.5, 0.0);
				glVertex3f(2.0, 0.5, 0.0);
			glEnd();

			glTranslatef(0.0f, 0.0f, 0.5f);
			glBegin(GL_POLYGON);  //right wall
				glVertex3f(2.0, 0.0, 0.0);
				glVertex3f(12.0, 0.0, 0.0);
				glVertex3f(12.0, 0.5, 0.0);
				glVertex3f(2.0, 0.5, 0.0);
			glEnd();

			glTranslatef(0.0f, 0.5f, -0.5f);
			glRotatef(90, 1, 0, 0);
			glBegin(GL_POLYGON);  //top wall
				glVertex3f(2.0, 0.0, 0.0);
				glVertex3f(12.0, 0.0, 0.0);
				glVertex3f(12.0, 0.5, 0.0);
				glVertex3f(2.0, 0.5, 0.0);
			glEnd();

			glRotatef(90, 0, 1, 0);
			glTranslatef(-3, 0.0, 2); // xy reversed
			glBegin(GL_POLYGON);  //front closing square
				glVertex3f(2.0, 0.0, 0.0);
				glVertex3f(3, 0.0, 0.0);
				glVertex3f(3, 0.5, 0.0);
				glVertex3f(2.0, 0.5, 0.0);
			glEnd();
		glPopMatrix();

		//right_lane
		glTranslatef(3, 0, 0);
	    glPushMatrix();
			glColor3ub(255,0,0);
			glRotatef(90, 0, 1, 0);
			glTranslatef(-5.0f, 0.0f, 0.0f);

			glBegin(GL_POLYGON);  //left wall
				glVertex3f(2.0, 0.0, 0.0);
				glVertex3f(12.0, 0.0, 0.0);
				glVertex3f(12.0, 0.5, 0.0);
				glVertex3f(2.0, 0.5, 0.0);
			glEnd();

			glTranslatef(0.0f, 0.0f, 0.5f);
			glBegin(GL_POLYGON);  //right wall
				glVertex3f(2.0, 0.0, 0.0);
				glVertex3f(12.0, 0.0, 0.0);
				glVertex3f(12.0, 0.5, 0.0);
				glVertex3f(2.0, 0.5, 0.0);
			glEnd();

			glTranslatef(0.0f, 0.5f, -0.5f);
			glRotatef(90, 1, 0, 0);
			glBegin(GL_POLYGON);  //top wall
				glVertex3f(2.0, 0.0, 0.0);
				glVertex3f(12.0, 0.0, 0.0);
				glVertex3f(12.0, 0.5, 0.0);
				glVertex3f(2.0, 0.5, 0.0);
			glEnd();

			glRotatef(90, 0, 1, 0);
			glTranslatef(-3, 0.0, 2); // xy reversed
			glBegin(GL_POLYGON);  //front closing square
				glVertex3f(2.0, 0.0, 0.0);
				glVertex3f(3, 0.0, 0.0);
				glVertex3f(3, 0.5, 0.0);
				glVertex3f(2.0, 0.5, 0.0);
			glEnd();
		glPopMatrix();

		//lane
		glTranslatef(-9, 0, 0);
	    glPushMatrix();
			glColor3ub(255,0,0);
			glRotatef(90, 0, 1, 0);
			glTranslatef(-5.0f, 0.0f, 0.0f);

			glBegin(GL_POLYGON);  //left wall
				glVertex3f(2.0, 0.0, 0.0);
				glVertex3f(12.0, 0.0, 0.0);
				glVertex3f(12.0, 0.5, 0.0);
				glVertex3f(2.0, 0.5, 0.0);
			glEnd();

			glTranslatef(0.0f, 0.0f, 0.5f);
			glBegin(GL_POLYGON);  //right wall
				glVertex3f(2.0, 0.0, 0.0);
				glVertex3f(12.0, 0.0, 0.0);
				glVertex3f(12.0, 0.5, 0.0);
				glVertex3f(2.0, 0.5, 0.0);
			glEnd();

			glTranslatef(0.0f, 0.5f, -0.5f);
			glRotatef(90, 1, 0, 0);
			glBegin(GL_POLYGON);  //top wall
				glVertex3f(2.0, 0.0, 0.0);
				glVertex3f(12.0, 0.0, 0.0);
				glVertex3f(12.0, 0.5, 0.0);
				glVertex3f(2.0, 0.5, 0.0);
			glEnd();

			glRotatef(90, 0, 1, 0);
			glTranslatef(-3, 0.0, 2); // xy reversed
			glBegin(GL_POLYGON);  //front closing square
				glVertex3f(2.0, 0.0, 0.0);
				glVertex3f(3, 0.0, 0.0);
				glVertex3f(3, 0.5, 0.0);
				glVertex3f(2.0, 0.5, 0.0);
			glEnd();
		glPopMatrix();



		//back wall
		glPushMatrix();
			glTranslatef(-7, 0.0, -7);
			glRotatef(0, 0, 1, 0);

			//whole where the pins fall:
			glColor3ub(0,0,0);
			glBegin(GL_POLYGON);
				glVertex3f(2.0, 0.0, 0.0);
				glVertex3f(20.0, 0.0, 0.0);
				glVertex3f(20.0, 1, 0.0);
				glVertex3f(2.0, 1, 0.0);
			glEnd();

			//wood detail top
			glColor3ub(255,128,0);
			glTranslatef(0, 1, 0);
			glBegin(GL_POLYGON); //whole where the pins fall
				glVertex3f(2.0, 0.0, 0.0);
				glVertex3f(20.0, 0.0, 0.0);
				glVertex3f(20.0, 0.1, 0.0);
				glVertex3f(2.0, 0.1, 0.0);
			glEnd();

			glColor3ub(102,51,0);
			glTranslatef(0, 0.1, 0);
			glRotatef(-90, 1, 0, 0);
			glBegin(GL_POLYGON); //wood top-z
				glVertex3f(2.0, 0.0, 0.0);
				glVertex3f(20.0, 0.0, 0.0);
				glVertex3f(20.0, 4, 0.0);
				glVertex3f(2.0, 4, 0.0);
			glEnd();

		glPopMatrix(); //endof back wall

		//side walls (right)
	    glPushMatrix();
	    glTranslatef(13, 0, 4);
			glColor3ub(102,51,0);
			glRotatef(90, 0, 1, 0);
			glTranslatef(-5.0f, 0.0f, 0.0f);

			glBegin(GL_POLYGON);  //left wall
				glVertex3f(2.0, 0.0, 0.0);
				glVertex3f(20.0, 0.0, 0.0);
				glVertex3f(20.0, 2, 0.0);
				glVertex3f(2.0, 2, 0.0);
			glEnd();

			glTranslatef(0.0f, 0.0f, 0.5f);
			glBegin(GL_POLYGON);  //right wall
				glVertex3f(2.0, 0.0, 0.0);
				glVertex3f(20.0, 0.0, 0.0);
				glVertex3f(20.0, 2, 0.0);
				glVertex3f(2.0, 2, 0.0);
			glEnd();

			glTranslatef(0.0f, 2.0f, -0.5f);
			glRotatef(90, 1, 0, 0);
			glBegin(GL_POLYGON);  //top wall
				glVertex3f(2.0, 0.0, 0.0);
				glVertex3f(20.0, 0.0, 0.0);
				glVertex3f(20.0, 0.5, 0.0);
				glVertex3f(2.0, 0.5, 0.0);
			glEnd();

			glColor3ub(255,0,0);
			glRotatef(90, 0, 1, 0);
			glTranslatef(-4, 0.0, 2); // xy reversed
			glBegin(GL_POLYGON);  //front closing square
				glVertex3f(2.0, 0.0, 0.0);
				glVertex3f(4, 0.0, 0.0);
				glVertex3f(4, 0.5, 0.0);
				glVertex3f(2.0, 0.5, 0.0);
			glEnd();
		glPopMatrix();

		//side walls (left)
	    glPushMatrix();
	    glTranslatef(-5, 0, 4);
			glColor3ub(102,51,0);
			glRotatef(90, 0, 1, 0);
			glTranslatef(-5.0f, 0.0f, 0.0f);

			glBegin(GL_POLYGON);  //left wall
				glVertex3f(2.0, 0.0, 0.0);
				glVertex3f(20.0, 0.0, 0.0);
				glVertex3f(20.0, 2, 0.0);
				glVertex3f(2.0, 2, 0.0);
			glEnd();

			glTranslatef(0.0f, 0.0f, 0.5f);
			glBegin(GL_POLYGON);  //right wall
				glVertex3f(2.0, 0.0, 0.0);
				glVertex3f(20.0, 0.0, 0.0);
				glVertex3f(20.0, 2, 0.0);
				glVertex3f(2.0, 2, 0.0);
			glEnd();

			glTranslatef(0.0f, 2.0f, -0.5f);
			glRotatef(90, 1, 0, 0);
			glBegin(GL_POLYGON);  //top wall
				glVertex3f(2.0, 0.0, 0.0);
				glVertex3f(20.0, 0.0, 0.0);
				glVertex3f(20.0, 0.5, 0.0);
				glVertex3f(2.0, 0.5, 0.0);
			glEnd();

			glColor3ub(255,0,0);
			glRotatef(90, 0, 1, 0);
			glTranslatef(-4, 0.0, 2); // xy reversed
			glBegin(GL_POLYGON);  //front closing square
				glVertex3f(2.0, 0.0, 0.0);
				glVertex3f(4, 0.0, 0.0);
				glVertex3f(4, 0.5, 0.0);
				glVertex3f(2.0, 0.5, 0.0);
			glEnd();
		glPopMatrix();

	  glPopMatrix();//end of bowl alley
}

//=========================================================//
GLvoid DrawNormalObjects(GLfloat gallop)
{
  // make sure the random color values we get are the same every time
  srand(200);

  // save the existing color properties
  glPushAttrib(GL_CURRENT_BIT);

//  transparency stuff:
//	glEnable(GL_BLEND);     // Turn Blending On
//	glDisable(GL_DEPTH_TEST);   // Turn Depth Testing Off

    //NO LIGHT ANGLE / WEIRD SHADING
	GLfloat light_ambient[] = { 1.0, 1.0, 1.0, 0};
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);




	//if checkCollision()

	int pin1[] = {1.8, 0.0, -14.0};
	int pin2[] = {2.2, 0.0, -15.0};


	drawAlley();

  	glPushMatrix();
		glColor4f(0.0f,1.0f,0.0f,1.0f);
		glTranslatef(1.6, 0.1,-3);
		glRotatef(90, 0, 1, 0);

		if (angle_selected == true)
		{
			glRotatef(10 * ball_direction, 0, 1, 0);
		}

		//drawing of direction
		glRotatef(90, 1, 0, 0);
		glBegin(GL_POLYGON);
			glVertex3f(2.0, 0.0, 0.0);
			glVertex3f(6.0, 0.0, 0.0);
			glVertex3f(6.0, 0.2, 0.0);
			glVertex3f(2.0, 0.4, 0.0);
		glEnd();
	glPopMatrix();

  //bowling ball
  glPushMatrix();
  	  glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, rand() % 128);
  	  glColor4f(0.0f,0.0f,1.0f,1.0f);
  	  glTranslatef(1.8*(ball_roll_angle), 0.4, ball_roll_speed);  // "ball is life" -Ghandi
  	  gluSphere(g_normalObject, ball_radius, 16, 10);
  glPopMatrix();



  //pins
  glPushMatrix();
	  glTranslatef(1.8, 0.0,-14.0);
	  glRotatef(-90, 1.0, 0.0, 0.0);
	  glColor4f(1.0f,1.0f,1.0f,1.0f);
	  ///glrotatef
	  gluCylinder(g_normalObject, 0.13, 0.13, 0.5, 32, 4);
	  //top of pin
	  glPushMatrix();
			  glTranslatef(0.0, 0.0, 0.5);
			  gluCylinder(g_normalObject, 0.06, 0.06, 0.25, 32, 4);
	  glPopMatrix();
  glPopMatrix();

  glPushMatrix();
	  glTranslatef(2.2, 0.0,-15.0);
	  glRotatef(-90, 1.0, 0.0, 0.0);
	  glColor4f(1.0f,1.0f,1.0f,1.0f);
	  ///glrotatef
	  gluCylinder(g_normalObject, 0.13, 0.13, 0.5, 32, 4);
	  //top of pin
	  glPushMatrix();
			  glTranslatef(0.0, 0.0, 0.5);
			  gluCylinder(g_normalObject, 0.06, 0.06, 0.25, 32, 4);
	  glPopMatrix();
  glPopMatrix();

  glPushMatrix();
	  glTranslatef(1.4, 0.0,-15.0);
	  glRotatef(-90, 1.0, 0.0, 0.0);
	  glColor4f(1.0f,1.0f,1.0f,1.0f);
	  ///glrotatef
	  gluCylinder(g_normalObject, 0.13, 0.13, 0.5, 32, 4);
	  //top of pin
	  glPushMatrix();
			  glTranslatef(0.0, 0.0, 0.5);
			  gluCylinder(g_normalObject, 0.06, 0.06, 0.25, 32, 4);
	  glPopMatrix();
  glPopMatrix();

  glPushMatrix();
	  glTranslatef(2.4, 0.0,-16.0);
	  glRotatef(-90, 1.0, 0.0, 0.0);
	  glColor4f(1.0f,1.0f,1.0f,1.0f);
	  ///glrotatef
	  gluCylinder(g_normalObject, 0.13, 0.13, 0.5, 32, 4);
	  //top of pin
	  glPushMatrix();
			  glTranslatef(0.0, 0.0, 0.5);
			  gluCylinder(g_normalObject, 0.06, 0.06, 0.25, 32, 4);
	  glPopMatrix();
  glPopMatrix();

  glPushMatrix();
	  glTranslatef(1.8, 0.0,-16.0);
	  glRotatef(-90, 1.0, 0.0, 0.0);
	  glColor4f(1.0f,1.0f,1.0f,1.0f);
	  ///glrotatef
	  gluCylinder(g_normalObject, 0.13, 0.13, 0.5, 32, 4);
	  //top of pin
	  glPushMatrix();
			  glTranslatef(0.0, 0.0, 0.5);
			  gluCylinder(g_normalObject, 0.06, 0.06, 0.25, 32, 4);
	  glPopMatrix();
  glPopMatrix();

  glPushMatrix();
	  glTranslatef(1.2, 0.0,-16.0);
	  glRotatef(-90, 1.0, 0.0, 0.0);
	  glColor4f(1.0f,1.0f,1.0f,1.0f);
	  ///glrotatef
	  gluCylinder(g_normalObject, 0.13, 0.13, 0.5, 32, 4);
	  //top of pin
	  glPushMatrix();
			  glTranslatef(0.0, 0.0, 0.5);
			  gluCylinder(g_normalObject, 0.06, 0.06, 0.25, 32, 4);
	  glPopMatrix();
  glPopMatrix();



  // restore the previous color values
  glPopAttrib();
} // end DrawNormalObjects()
//=========================================================//
//=========================================================//
void update_camera()
{
// don't allow the player to wander past the "edge of the world"
    if (g_playerPos[0] < -(WORLD_SIZE-50))
    g_playerPos[0] = -(WORLD_SIZE-50);
    if (g_playerPos[0] > (WORLD_SIZE-50))
    g_playerPos[0] = (WORLD_SIZE-50);
    if (g_playerPos[2] < -(WORLD_SIZE-50))
    g_playerPos[2] = -(WORLD_SIZE-50);
    if (g_playerPos[2] > (WORLD_SIZE-50))
    g_playerPos[2] = (WORLD_SIZE-50);

  // calculate the player's angle of rotation in radians
    float rad =  float(3.14159 * g_viewAngle / 180.0f);
    // use the players view angle to correctly set up the view matrix
    g_lookAt[0] = float(g_playerPos[0] + 100*cos(rad));
    g_lookAt[2] = float(g_playerPos[2] + 100*sin(rad));

    rad = float (3.13149 * g_elevationAngle / 180.0f);

    g_lookAt[1] = float (g_playerPos[1] + 100 * sin(rad));

    // clear the modelview matrix
    glLoadIdentity();

    // setup the view matrix
    gluLookAt(g_playerPos[0], g_playerPos[1], g_playerPos[2],
              g_lookAt[0],    g_lookAt[1],    g_lookAt[2],
              0.0,            1.0,            0.0);
}
//=========================================================//
//=========================================================//
//void move_camera(int specialKEY,char normalKEY)
void move_camera(void)
{
    // looking up
	if( key[SDLK_a] ){
		g_elevationAngle += 2.0;
	}
    // looking down
	if( key[SDLK_z] ){
		g_elevationAngle -= 2.0;
	}

	if( key[SDLK_RIGHT] ){
        g_viewAngle += 2.0;
       // calculate camera rotation angle radians
       rad =  float(3.14159 * g_viewAngle / 180.0f);
	}
	if( key[SDLK_LEFT] ){
        g_viewAngle -= 2.0;
        // calculate camera rotation angle radians
        rad =  float(3.14159 * g_viewAngle / 180.0f);
	}
	if( key[SDLK_UP] ){
        g_playerPos[2] += sin(rad) * DEFAULT_SPEED;
        g_playerPos[0] += cos(rad) * DEFAULT_SPEED;
	}
	if( key[SDLK_DOWN] ){
        g_playerPos[2] -= sin(rad) * DEFAULT_SPEED;
        g_playerPos[0] -= cos(rad) * DEFAULT_SPEED;
	}
	if ( key[SDLK_1] ) {
		ball_speed_multi = 1;
	}
	if ( key[SDLK_2] ) {
		ball_speed_multi = 2;
	}
	if ( key[SDLK_3] ) {
		ball_speed_multi = 3;
	}
	if ( key[SDLK_r] ) {
		ball_direction = -1;
		angle_selected = true;
	}
	if ( key[SDLK_l] ) {
		ball_direction = 1;
		angle_selected = true;
	}
	if ( key[SDLK_m] ) {
		ball_direction = 0;
		angle_selected = true;
	}
}
//=========================================================//
//=========================================================//
//static void keyboard(unsigned char key, int x, int y)
static void keyboard(void)
{   move_camera();

	if ( key[SDLK_s] )
	{
		if (ballrolling == false) {
			ballrolling = true;
		}
	}
}
//=========================================================//
//=========================================================//
static void display(void)
{
    keyboard();
    update_camera();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_LIGHTING);

    // position the light
    GLfloat pos[4] = { 5.0, 5.0, 5.0, 0.0 };
    glLightfv(GL_LIGHT0, GL_POSITION, pos);

    if (ballrolling == true) {
    	if (ball_speed_multi == 1) {
				ball_roll_speed -= 0.1;

				if(ball_direction > 0)
				{
					ball_roll_angle -= 0.003;  // haters gonna hate
				}
				if(ball_direction < 0)
				{
					ball_roll_angle += 0.003;  // haters gonna hate
				}

    	}
    	if (ball_speed_multi == 2) {
    			ball_roll_speed -= 0.2;
				if(ball_direction > 0)
				{
					ball_roll_angle -= 0.006;  // haters gonna hate
				}
				if(ball_direction < 0)
				{
					ball_roll_angle += 0.006;  // haters gonna hate
				}
    	}
    	if (ball_speed_multi == 3) {
    			ball_roll_speed -= 0.3;
				if(ball_direction > 0)
				{
					ball_roll_angle -= 0.009;  // haters gonna hate
				}
				if(ball_direction < 0)
				{
					ball_roll_angle += 0.009;  // haters gonna hate
				}
    	}
    }else{
    	ball_roll_speed = -4.0;
    }

    if (ball_roll_speed < -18.0){
    	ball_roll_speed = -4.0;
    	ballrolling = false;
    }





    // gallop is used for animation
    static GLfloat gallop = 100.0;
    // it's increased by one every frame
  	gallop -= 1.0;

		if(goingup == true)
		{
			hanim += 1.0;
		}
		if(goingup == false)
		{
			hanim -= 1.0;
		}
  	// and ranges between 0 and 360
  	if (gallop < 0)
		gallop = 100.0;

		if(hanim > 50)
		{
			goingup = false;
		}
		if(hanim < -50)
		{
			goingup = true;
		}
    // draw all of our objects in their normal position
    DrawNormalObjects(gallop);

    glColor3d(0.1,0.1,0.4);


    DrawGround();

	SDL_GL_SwapBuffers();
}
//=========================================================//
//=========================================================//
void main_loop_function()
{
	float angle;
	while( events() )
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glLoadIdentity();
		glTranslatef(0,0, -10);
		glRotatef(angle, 0, 0, 1);
		glBegin(GL_QUADS);
			glColor3ub(255, 000, 000); glVertex2f(-1,  1);
			glColor3ub(000, 255, 000); glVertex2f( 1,  1);
			glColor3ub(000, 000, 255); glVertex2f( 1, -1);
			glColor3ub(255, 255, 000); glVertex2f(-1, -1);
		glEnd();

		display();
	}
}
//=========================================================//
//=========================================================//
// Initialze OpenGL perspective matrix
void GL_Setup(int width, int height)
{
	glViewport( 0, 0, width, height );
	glMatrixMode( GL_PROJECTION );
	glEnable( GL_DEPTH_TEST );
	gluPerspective( 45, (float)width/height, 0.1, 100 );
	glMatrixMode( GL_MODELVIEW );
}
//=========================================================//
//=========================================================//
// Sound variables
int channel;				//Channel on which our sound is played
int audio_rate = 44100;			//Frequency of audio playback
Uint16 audio_format = AUDIO_S16SYS; 	//Format of the audio we're playing
int audio_channels = 2;			//2 channels = stereo
int audio_buffers = 640;		//Size of the audio buffers in memory
Mix_Music *mus , *mus2 ;  // Background Music
Mix_Chunk *wav , *wav2 ;  // For Sounds
//=========================================================//
//=========================================================//
void openingAudio(void){
	//Initialize SDL_mixer with our chosen audio settings
	if(Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers) != 0) {
		printf("Unable to initialize audio: %s\n", Mix_GetError());
		exit(1);
	}
	mus = Mix_LoadMUS("/home/green/Desktop/sandbox/CGI/sound002/mixer/ff2prlde.mid");
	mus2 = Mix_LoadMUS("/home/green/Desktop/sandbox/CGI/sound002/mixer/aria.mp3");
	wav = Mix_LoadWAV("/home/gabe/downloads/GL/gallop-sound03.wav");
	wav2 = Mix_LoadWAV("/home/green/Desktop/sandbox/CGI/sound002/mixer/start.wav");
}
//=========================================================//
//=========================================================//
void closingAudio(void){
	//Release the memory allocated to our sound
	Mix_FreeChunk(wav);
	Mix_FreeChunk(wav2);
	Mix_FreeMusic(mus);
	Mix_FreeMusic(mus2);
	//Need to make sure that SDL_mixer and SDL have a chance to clean up
	Mix_CloseAudio();
}
//=========================================================//
//=========================================================//
void backgroundMusic(void){
	Mix_PlayMusic(mus,1); //Music loop=1
}
//=========================================================//
//=========================================================//
void makeSound(void){
	backgroundMusic();
	Mix_PlayChannel(-1,wav,0);
}
//=========================================================//
//=========================================================//
int main()
{
	// Initialize SDL with best video mode
	SDL_Init(SDL_INIT_VIDEO);
    init_data();
    openingAudio();

	const SDL_VideoInfo* info = SDL_GetVideoInfo();
	int vidFlags = SDL_OPENGL | SDL_GL_DOUBLEBUFFER;
	if (info->hw_available) {vidFlags |= SDL_HWSURFACE;}
	else {vidFlags |= SDL_SWSURFACE;}
	int bpp = info->vfmt->BitsPerPixel;
	SDL_SetVideoMode(window_width, window_height, bpp, vidFlags);
	GL_Setup(window_width, window_height);

    // environment background color
    glClearColor(0.5, 0.7, 1.0, 1.0);//(1,1,1,1); // from glColor3f(0.5f, 0.7f, 1.0f);
    // deepth efect to objects
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    // light and material in the environment
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);

	main_loop_function();
	closingAudio();
}
//=========================================================//
//=========================================================//
