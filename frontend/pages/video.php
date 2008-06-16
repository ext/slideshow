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

require_once('../db_functions.inc.php');
require_once('../core/module.inc.php');

class Video extends Module {
	function __construct(){
		connect();
	}

	function __descturuct(){
		disconnect();
	}

  function index(){
	Module::set_template('video.tmpl');
	global $Path;

	$files = array();

	if ($dh = opendir($Path['Video'])) {
	  while (($file = readdir($dh)) !== false) {
	if ( $file[0] == "." ){
	  continue;
	}
	$files[] = $file;
	  }
	  closedir($dh);
	}

	return array(
		 'files' => $files
		 );
  }

  function play(){
	global $Path;
	$fullpath = "$Path[Video]/$_GET[name]";

	$this->send_signal("PlayVideo", "s", array($fullpath));
	Module::redirect('/index.php');
  }
};

?>