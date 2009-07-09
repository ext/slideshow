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
#include "slidelib.h"
#include <wand/MagickWand.h>
#include <sys/stat.h>

MODULE_INFO("image", ASSEMBLER_MODULE, "David Sveningsson");

int module_init(){
}

int module_cleanup(){
}

int assemble(const slide_t* slide, const resolution_t* resolution){
	char* resolution_str = resolution_as_string(resolution);
	char* datafile = slide_datapath(slide, slide->datafiles[0]);
	char* samplefile = slide_sample_path(slide, resolution);

	printf("datafile: %s\n", datafile);
	printf("samplefile: %s\n", samplefile);

	char* commands[] = {
		"slidelib", // dummy program name
		datafile,
		"-resize", resolution_str,
		"-background", "black",
		"-gravity", "center",
		"-extent", resolution_str,
		samplefile
	};
	int nr_commands = sizeof(commands) / sizeof(char*);

	ImageInfo* image_info = AcquireImageInfo();
	ExceptionInfo* exception=AcquireExceptionInfo();
	MagickBooleanType status = ConvertImageCommand(image_info, nr_commands, commands,(char **) NULL, exception);
	if (exception->severity != UndefinedException){
		CatchException(exception);
		status = MagickTrue;
	}
	image_info=DestroyImageInfo(image_info);
	exception=DestroyExceptionInfo(exception);

	/* Must change permissions because sometimes the file is created with no
	 * permissions at all. Perhaps its just a weird umask or something but
	 * this ensures correct persmissions.
	 */
	chmod(samplefile, S_IRUSR | S_IWUSR | S_IRGRP);

	free(resolution_str);
	free(datafile);
	free(samplefile);

	return status == MagickFalse ? 0 : 3;
}
