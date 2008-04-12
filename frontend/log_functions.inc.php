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

function get_log($lines, $show_debug){
  global $Files, $BasePath;

  //  $grep_expr = $show_debug ? "\"\"" : "-v \"(DD)\"";

  //  $lines = explode("\n", `tail -n $lines $log_file $grep_cmd`);
  $filename = $show_debug ? $Files['Log']['Debug'] :  $Files['Log']['Base'];
  $lines = explode("\n", `tail -n $lines $BasePath/$filename`);

  $log = array();

  foreach ( $lines as $line ){
    $severity = 'normal';
    if ( strncmp( $line, "(WW)", 4) == 0 ){
      $severity = 'warning';
    } else if ( strncmp( $line, "(!!)", 4) == 0 ){
      $severity = 'fatal';
    }

    $log[] = array(
		   'severity' => $severity,
		   'content' => str_replace(" ", "&nbsp;", $line)
		   );
  }   

  return $log;
}

?>