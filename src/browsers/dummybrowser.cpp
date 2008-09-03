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

#include "dummybrowser.h"

DummyBrowser::DummyBrowser():
	n(0){

	_img[0] = "IMG_0298.JPG";
	_img[1] = "IMG_0299.JPG";
	_img[2] = "IMG_0300.JPG";
	_img[3] = "IMG_0301.JPG";
}

DummyBrowser::~DummyBrowser(){

}

const char* DummyBrowser::get_next_file(){
	return _img[++n%4];
}
