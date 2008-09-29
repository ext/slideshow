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

function post($name, $default = ""){
  return array_get($_POST, $name, $default);
}

function get($name, $default = ""){
  return array_get($_GET, $name, $default);
}

function array_get(array $array, $name, $default){
  $data = $default;
  if ( isset ( $array[$name] ) ){
    $data = $array[$name];
  }

  if ( get_magic_quotes_gpc() ){
    $data = stripslashes($data);
  }

  return $data;
}

/**
 * @brief Returns a href built from the arguments
 * The href will have the format '/index.php/$main/$section/$argv?$get' where
 * $argv and $get will be flattened.
 *
 * Example of generated href:
 * href('foo', 'bar', array(5, 7), array('spam' => 'egg', 'tux' => 'quux'))
 * /index.php/foo/bar/5/7?spam=egg&amp;tux=quux
 *
 * @param $main Main part
 * @param $section Section part
 * @param $argv Array with arguments or a single string (or string convertable) argument
 * @param $get Array with GET arguments or a string containing the GET string (must include '?' character)
 */
function href($main = NULL, $section = NULL, $argv = NULL, $get = NULL){
	$href = '/index.php';

	$parts = array(
		$main,
		$section,
		$argv,
		$get
	);

	foreach ( $parts as $index => $part ){
		if ( empty($part) ){
			break;
		}

		switch ( $index ){
			case 0:
			case 1:
				$href .= "/$part";
				break;
			case 2:
				if ( is_array($part) ){
					$href .= '/' . implode('/', $part);
				} else {
					$href .= '/' . $part;
				}
				break;
			case 3:
				if ( is_array($part) ){
					$href .= '?' . http_build_query($part, '', '&amp;');
				} else {
					$href .= $part;
				}
				break;
		}
	}

	return $href;
}
