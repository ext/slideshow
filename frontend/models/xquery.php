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
}

?>
