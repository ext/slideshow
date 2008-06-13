<?
/**
 * This file is part of Slideshow.
 * Copyright (C) 2008 David Sveningsson <ext@sidvind.com>
 *
 * Slideshow is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Slideshow is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Slideshow.  If not, see <http://www.gnu.org/licenses/>.
 */
?>
<?

require_once('../core/module.inc.php');
require_once('../slides.inc.php');

class Main extends Module {
	function __construct(){
		connect();
	}

	function __descturuct(){
		disconnect();
	}

  function index(){
	Module::set_template('main.tmpl');

	$ret = array();

	$ret['slides'] = get_slides();

	global $BasePath;
	$motd_file = "$BasePath/motd";

	if ( file_exists($motd_file) ){
	  $file = fopen("$BasePath/motd", "r");
	  $content = str_replace("\n", "<br/>\n", fread($file, filesize($motd_file)));

	  if ( strlen($content) > 0 ){
	$ret['motd'] = $content;
	  }
	}

	return $ret;
  }
};

?>