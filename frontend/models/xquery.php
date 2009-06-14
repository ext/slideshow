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

class xqueryException extends PageException {
	private $rc;
	private $stdout;

	public function __construct($message, $rc, $stdout){
		parent::__construct($message, XQUERY_ERROR);
		$this->rc = $rc;
		if ( is_array($stdout) ){
			$stdout = implode($stdout, "\n");
		}
		$this->stdout = $stdout;
	}

	public function get_rc(){
		return $this->rc;
	}

	public function get_stdout(){
		return $this->stdout;
	}
}

class xquery {
	private $executable = null;
	private static $aspect_ratios = array(
		array(4, 3), // Regular 4:3 (VGA, PAL, SVGA, etc)
		array(3, 2), // NTSC
		array(5, 3),
		array(5, 4), // SXGA, QSXGA
		array(16, 10), // 16:10 "Widescreen"
		array(16, 9), // 16:9 Widescreen
		array(17, 9)
	);

	function __construct( $filename ){
		if ( !is_executable($filename) ){
			throw new Exception('Could not open xquery binary ' . $filename);
		}
		$this->executable = $filename;
	}

	function get_displays(){
		$stdout = NULL;
		$rc = 0;

		// Still only gets for :0, but better than nothing =)
		exec("DISPLAY=\":0\" {$this->executable} --get-screens", $stdout, $rc);

		if ( $rc != 0 ){
			throw new xqueryException("Failed to run xquery", $rc, $stdout);
		} else {
			return $stdout;
		}
	}

	function resolutions_for_display($display){
		$stdout = NULL;
		$rc = 0;

		// Still only gets for :0, but better than nothing =)
		exec("DISPLAY=\":0\" {$this->executable} --screen $display", $stdout, $rc);

		if ( $rc != 0 ){
			throw new xqueryException("Failed to run xquery", $rc, $stdout);
		} else {
			$keys = array_map('xquery::resolution_name_from_string', $stdout);
			$values = array_map('xquery::resolution_value_from_string', $stdout);
			return array_combine($keys, $values);
		}
	}

	static private function resolution_name_from_string($str){
		sscanf($str, "\t%d %dx%dx@%f", $id, $width, $height, $refresh);
		return "{$width}x{$height}";
	}
	static private function resolution_value_from_string($str){
		sscanf($str, "\t%d %dx%dx@%f", $id, $width, $height, $refresh);
		$value = "{$width}x{$height}";
		$aspect_ratio = xquery::resolution_aspect_ratio($width, $height);
		if ( $aspect_ratio ){
			$value .= " ($aspect_ratio[0]:$aspect_ratio[1])";
		}
		return $value;
	}

	static private function resolution_aspect_ratio($width, $height){
		foreach (xquery::$aspect_ratios as $ratio){
			if ( ($width/$ratio[0]) / ($height/$ratio[1]) == 1 ){
				return $ratio;
			}
		}
		return null;
	}
}

?>
