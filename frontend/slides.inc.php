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

function get_slides(){
  $slides = array();

  $set = array(
	       q("SELECT id, fullpath FROM files ORDER BY id")
	       );

  foreach( $set as $result ){
    while ( ( $row = mysql_fetch_assoc($result) ) ){
      $item = array(
		    'id' => $row['id'],
		    'fullpath' => $row['fullpath'],
		    'name' => basename($row['fullpath'])
		    );
      $slides[] = $item;
    }
  }

  return $slides;
}

?>