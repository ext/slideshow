#include "fade.h"
#include <GL/gl.h>

FadeTransition::FadeTransition(){}
FadeTransition::~FadeTransition(){}

void FadeTransition::render(unsigned int texture_0, unsigned int texture_1, float state){
	glColor4f(1,1,1,1);
	glBindTexture(GL_TEXTURE_2D, texture_1);
	glBegin( GL_QUADS );
		glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f, 0.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f, 1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, 1.0f);
	glEnd();
	
	glColor4f(1,1,1,state);
	glBindTexture(GL_TEXTURE_2D, texture_0);
	glBegin( GL_QUADS );
		glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f, 0.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f, 1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, 1.0f);
	glEnd();
}
