/**
 * This file is part of Slideshow.
 * Copyright (C) 2008-2010 David Sveningsson <ext@sidvind.com>
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

#ifndef SLIDELIB_H
#define SLIDELIB_H

typedef struct {
	const char* name;
	char* fullpath;
	char* data_path;
	char* sample_path;
} slide_path_t;

typedef struct {
	int width;
	int height;
} resolution_t;

typedef struct {
	char* type;
	char** datafiles;
	slide_path_t path;
} slide_t;

enum {
	USAGE_ERROR = 1,
	ACCESS_ERROR = 2
};

#ifdef __cplusplus
extern "C" {
#endif

int slide_create(const char* name);
int slide_resample(slide_t* target, resolution_t* resolution, resolution_t* virtual_resolution);

/**
 * Get an assembled sample. Will assemble if it does not exist.
 * @param target slide
 * @param resolution sample resolution
 * @return
 */
char* slide_sample(slide_t* target, resolution_t* resolution);

/**
 * Get path only to a sample.
 * @param target slide
 * @param resolution sample resolution
 * @return
 */
char* slide_sample_path(const slide_t* target, const resolution_t* resolution);

slide_t* slide_from_name(const char* name);
void slide_free(slide_t* slide);

resolution_t resolution_from_string(const char* str);
char* resolution_as_string(const resolution_t* resolution);

/**
 * Appends datapath
 */
char* slide_datapath(const slide_t* slide, const char* path);

#ifdef __cplusplus
}
#endif

#endif /* SLIDELIB_H */
