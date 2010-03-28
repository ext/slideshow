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

require_once('../core/exception.php');

class SlidetoolException extends PageException {
	private $rc;
	private $stdout;

	public function __construct($message, $rc, $stdout){

		$this->rc = $rc;
		if ( is_array($stdout) ){
			$stdout = implode($stdout, "\n");
		}
		$this->stdout = $stdout;
		parent::__construct($message, SLIDETOOL_ERROR);
	}

	public function get_rc(){
		return $this->rc;
	}

	public function get_stdout(){
		return $this->stdout;
	}
}

class Slide {
	private $fullpath;

	static public function create_image(){
		global $settings;

		$id = Slide::unique_id();
		$path = $settings->image_path() . '/' . $id;
		$fullpath = $path . '.slide';
		$slidetool = $settings->slidetool_executable();

		Slide::slidetool("create $path");

		$datafiles = func_get_args();
		foreach ( $datafiles as $file ){
			$basename = basename($file);
			copy($file, "$fullpath/data/$basename");
		}
		$datafiles = array_map('basename', $datafiles);

		$meta = array(
			'type' => 'image',
			'data' => $datafiles
		);

		$meta = json_encode($meta) . "\n";
		$file = fopen("$fullpath/meta", 'w');
		fwrite($file, $meta);
		fclose($file);

		return new Slide($fullpath);
	}

	public function fullpath(){
		return $this->fullpath;
	}

	public function thumbnail_url(){
		return 'image/' . basename($this->fullpath()) . '/samples/200x200.png';
	}

	public function resample($resolution, $virtual_resolution = NULL){
		Slide::slidetool("resample {$this->fullpath} $resolution");
	}

	private function __construct($fullpath){
		$this->fullpath = $fullpath;
	}

	static private function unique_id(){
		return md5(uniqid());
	}

	static private function slidetool($command){
		global $settings;

		$slidetool = $settings->slidetool_executable();
		$env = '';

		foreach ( $settings->environment() as $key => $value ){
			$env .= "SLIDESHOW_$key=\"$value\" ";
		}

		$command = $env . ' ' . $slidetool . ' ' . $command . ' 2>&1';

		$stdout = '';
		$rc = 0;
		exec($command, $stdout, $rc);

		if ( $rc != 0 ){
			throw new SlidetoolException("Failed to execute \"$command\"" , $rc, $stdout);
		}
	}
}

?>
