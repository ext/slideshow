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

function video_thumbnail($filename){
  global $video_dir, $tmp_dir;

  $needs_escape = array(" ", "&", "-");
  $escaped = array("\\ ", "\\&", "\\-");

  $escaped_filename = str_replace($needs_escape, $escaped, $filename);

  if ( !file_exists( "video_thumbs/$filename.gif" ) ){
    $tmp = array();

    for ( $i = 0; $i < 5; $i++ ){
      $name = tempnam($tmp_dir,"thumb");
      $tmp[] = $name;
      shell_exec("ffmpeg -i $video_dir/$escaped_filename -vframes 1 -ss ".($i*5)." -f image2 $name");
    }

    echo shell_exec("convert -delay 50 $tmp[0] $tmp[1] $tmp[2] $tmp[3] $tmp[4] -loop 0 video_thumbs/$escaped_filename.gif");

    for ( $i = 0; $i < 5; $i++ ){
      unlink($tmp[$i]);
    }
  }

  return "video_thumbs/$filename.gif";	
}