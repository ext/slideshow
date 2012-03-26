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

static void render(transition_context_t* context){
	glPushClientAttrib(GL_VERTEX_ARRAY|GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glVertexPointer(2, GL_FLOAT, sizeof(vertex_t), &vertices[0].x);
	glTexCoordPointer(2, GL_FLOAT, sizeof(vertex_t), &vertices[0].u);

	glColor4f(1,1,1,1);
	glBindTexture(GL_TEXTURE_2D, context->texture[1]);
	glDrawElements(GL_QUADS, 4, GL_UNSIGNED_BYTE, indices);

	glColor4f(1,1,1,context->state);
	glBindTexture(GL_TEXTURE_2D, context->texture[0]);
	glDrawElements(GL_QUADS, 4, GL_UNSIGNED_BYTE, indices);

	glPopClientAttrib();
}

void* module_alloc(){
	return malloc(sizeof(transition_module_t));
}

int EXPORT module_init(transition_module_t* module){
	module->render = render;
	return 0;
}

