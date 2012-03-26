/**
 * This file is part of Slideshow.
 * Copyright (C) 2008 David Sveningsson <ext@sidvind.com>
 *
 * Slideshow is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Slideshow is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with Slideshow.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "module.h"
#include "Transition.h"
#include "gl.h"

MODULE_INFO("Fade", TRANSITION_MODULE, "David Sveningsson");

typedef struct {
	float x;
	float y;
	float u;
	float v;
} vertex_t;

static vertex_t vertices[4] = {
	{0.0f, 0.0f, 0.0f, 0.0f},
	{0.0f, 1.0f, 0.0f, 1.0f},
	{1.0f, 1.0f, 1.0f, 1.0f},
	{1.0f, 0.0f, 1.0f, 0.0f},
};
static unsigned char indices[4] = {
	0, 1, 2, 3
};

static GLuint vbo[2];

static void render(transition_context_t* context){
	glPushClientAttrib(GL_VERTEX_ARRAY|GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexPointer(2,   GL_FLOAT, sizeof(vertex_t), (void*)0);
	glTexCoordPointer(2, GL_FLOAT, sizeof(vertex_t), (void*)(sizeof(float)*2));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);

	glColor4f(1,1,1,1);
	glBindTexture(GL_TEXTURE_2D, context->texture[1]);
	glDrawElements(GL_QUADS, 4, GL_UNSIGNED_BYTE, 0);

	glColor4f(1,1,1,context->state);
	glBindTexture(GL_TEXTURE_2D, context->texture[0]);
	glDrawElements(GL_QUADS, 4, GL_UNSIGNED_BYTE, 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glPopClientAttrib();
}

void* module_alloc(){
	return malloc(sizeof(transition_module_t));
}

int EXPORT module_init(transition_module_t* module){
	module->render = render;

	/* generate VBO with vertices and indices */
	glGenBuffers(2, vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_t)*4, vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned char)*4, indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return 0;
}

