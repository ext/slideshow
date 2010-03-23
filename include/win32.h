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

#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define NOCOMM
#define NOSOUND
#include <windows.h>

// undef shit defined by windows.h, seriously, whats wrong with this header?
#undef near
#undef far

// POSIX name "compability"-fixes
#define strdup _strdup
#define fileno _fileno

// Simple security enhancements fixes
#define scanf scanf_s
#define sscanf sscanf_s
#define strtok_r strtok_s
#define strncpy(dst, src, n) strncpy_s(dst, n, src, _TRUNCATE)
