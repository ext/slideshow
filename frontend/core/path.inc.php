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
?>
<?

class Path {
  private $path;

  function Path( $module = NULL, $section = NULL, $argv = NULL ){
	if ( isset( $module ) && isset( $section ) ){
	  $this->path = array( $module, $section );
	  $this->argv = isset($argv) ? $argv : array();
	  return;
	}

	if ( !isset($_SERVER['PATH_INFO']) || $_SERVER['PATH_INFO'] == '/' ){
	  $this->path = array( 'main', 'index' );
	  $this->argv = array();
	  return;
	}

	$path = $_SERVER['PATH_INFO'];

	$path_array = explode("/",$_SERVER['PATH_INFO']);
	$this->path = array_slice($path_array, 1, 2);
	$this->argv = array_slice($path_array, 3);

	$section_missing = count($this->path) == 1;
	if ( $section_missing ){
	  $this->path[1] = 'index';
	}

	$section_invalid = $this->path[1] == '';
	if ( $section_invalid ){
	  $this->path[1] = 'index';
	}
  }

  function module(){
	///@todo Check for valid modules
	return $this->path[0];
  }

  function section(){
	return $this->path[1];
  }

  function argv(){
  	return $this->argv;
  }
};

?>
