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