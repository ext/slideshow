#include "dummy.h"
#include <GL/gl.h>

DummyTransition::DummyTransition(){}
DummyTransition::~DummyTransition(){}

void DummyTransition::render(unsigned int texture_0, unsigned int texture_1, float state){
	glBindTexture(GL_TEXTURE_2D, texture_0);
	glBegin( GL_QUADS );
		glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f, 0.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f, 1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, 1.0f);
	glEnd();
}
