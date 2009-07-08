<?
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

class PageException extends Exception {
	function get_rc(){
		return 0;
	}

	function get_stdout(){
		return "";
	}
}

define('UPLOAD_ERROR', 1);
define('EXECUTABLE_ERROR', 2);
define('XQUERY_ERROR', 3);
define('SLIDETOOL_ERROR', 4);
define('FILE_NOT_FOUND', 404);

?>