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
#include "log.h"
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
static GLuint sp;
static GLuint vp;
static GLuint fp;
static GLuint s;

static const GLchar* vertex_program =
	"#version 120\n"
	"in vec2 in_pos;\n"
	"in vec2 in_uv;\n"
	"varying vec2 uv;\n"
	"\n"
	"void main(void){\n"
	"  gl_Position = gl_ModelViewProjectionMatrix * vec4(in_pos, 0.0, 1.0);\n"
	"  uv = in_uv;\n"
	"}\n";

static const GLchar* fragment_program =
	"#version 120\n"
	"uniform sampler2D texture_0;\n"
	"uniform sampler2D texture_1;\n"
	"uniform float s;\n"
	"varying vec2 uv;\n"
	"\n"
	"void main(void){\n"
	"  vec4 t0 = texture2D(texture_0, uv);\n"
	"  vec4 t1 = texture2D(texture_1, uv);\n"
	"  gl_FragColor = mix(t0,t1,s);\n"
	"}\n";

static void render(transition_context_t* context){
	/* set state */
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glUseProgram(sp);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)(0));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)(sizeof(float)*2));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);

	/* render */
	glUniform1f(s, context->state);
	glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, context->texture[0]);
	glActiveTexture(GL_TEXTURE0);	glBindTexture(GL_TEXTURE_2D, context->texture[1]);
	glDrawElements(GL_QUADS, 4, GL_UNSIGNED_BYTE, 0);

	/* restore state */
	glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);	glBindTexture(GL_TEXTURE_2D, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glUseProgram(0);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

void* module_alloc(){
	return malloc(sizeof(transition_module_t));
}

static void check_log(GLuint target){
	void (*query_func)(GLuint target, GLenum pname, GLint* param) = NULL;
	void (*get_func)(GLuint target, GLsizei maxLength, GLsizei* length, GLchar* infoLog) = NULL;

	/* setup function pointers depending on whenever we want the log for a
	 * shader object or shader program. */
	if ( glIsShader(target) ){
		query_func = glGetShaderiv;
		get_func = glGetShaderInfoLog;
	} else{
		query_func = glGetProgramiv;
		get_func = glGetProgramInfoLog;
	}

	GLint size;
	query_func(target, GL_INFO_LOG_LENGTH, &size);
	if ( size > 1 ){ /* ignore line with only a \n */
		char* log = malloc(size);
		get_func(target, size, &size, log);
		log_message(Log_Info, "Shader log (%d):\n%s\n", size, log);
		free(log);
	}
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

	GLint r;

	/* compile vertex shader */
	vp = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vp, 1, &vertex_program, 0);
	glCompileShader(vp);
	glGetShaderiv(vp, GL_COMPILE_STATUS, &r);
	if ( r == GL_FALSE ){
		check_log(vp);
		log_message(Log_Fatal, "Failed to compile vertex program\n");
		return 1;
	}

	/* compile fragment shader */
	fp = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fp, 1, &fragment_program, 0);
	glCompileShader(fp);
	glGetShaderiv(fp, GL_COMPILE_STATUS, &r);
	if ( r == GL_FALSE ){
		check_log(fp);
		log_message(Log_Fatal, "Failed to compile fragment program\n");
		return 1;
	}

	/* link shader */
	sp = glCreateProgram();
	glAttachShader(sp, vp);
	glAttachShader(sp, fp);
	glLinkProgram(sp);
	glGetProgramiv(sp, GL_LINK_STATUS, &r);
	if ( r == GL_FALSE ){
		check_log(sp);
		log_message(Log_Fatal, "Failed to link program\n");
		return 1;
	}

	/* check shader log (for warnings) */
	check_log(fp);
	check_log(sp);

	/* bind uniforms */
	GLint loc;
	glUseProgram(sp);
	loc = glGetUniformLocation(sp, "texture_0");
	glUniform1i(loc, 0);
	loc = glGetUniformLocation(sp, "texture_1");
	glUniform1i(loc, 1);
	s = glGetUniformLocation(sp, "s");
	glUseProgram(0);

	return 0;
}

