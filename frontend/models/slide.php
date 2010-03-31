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
	private $id;
	private $active;
	private $meta;

	static public function create_image(){
		global $settings;

		$id = Slide::unique_id();
		$path = $settings->image_path() . '/' . $id;
		$fullpath = $path . '.slide';
		
		mkdir($fullpath);
		mkdir($fullpath . '/data');
		mkdir($fullpath . '/samples');

		$datafiles = func_get_args();
		foreach ( $datafiles as $file ){
			$basename = basename($file);
			copy($file, "$fullpath/data/$basename");
		}
		$datafiles = array_map('basename', $datafiles);

		$meta = array(
			'type' => 'image',
			'data' => $datafiles,
			'title' => $datafiles[0]
		);

		$metastr = json_encode($meta) . "\n";
		$file = fopen("$fullpath/meta", 'w');
		fwrite($file, $metastr);
		fclose($file);
		
		return new SlideImage($fullpath, $meta);
	}

	public function fullpath(){
		return $this->fullpath;
	}
	
	public function id(){
		return $this->id;
	}
	
	public function active(){
		return $this->active;
	}
	
	public function title(){
		return $this->meta['title'];
	}
	
	public function type(){
		return $this->meta['type'];
	}

	public function thumbnail_url(){
		return $this->sample_url('200x200');
	}
	
	public function image_url(){
		global $settings;
		return $this->sample_url($settings->resolution_as_string());
	}
	
	public function sample_url($resolution){
		return 'image/' . basename($this->fullpath()) . "/samples/$resolution.png";
	}

	public function resample($resolution, $virtual_resolution = NULL){
		global $settings;
		
		$src = $this->fullpath . '/data/' . $this->meta['data'][0];
		$dst = $this->fullpath() . "/samples/$resolution.png";
		
		$convert = $settings->convert_binary();
		if ( !is_executable($convert) ){
			throw new Exception("Could not find ImageMagick 'convert' executable.");
		}

		if ( empty($virtual_resolution) ){
			$this->convert_exec("" .
					" $convert " .
					escapeshellarg($src) .
					" -resize $resolution" .
					" -background black " .
					" -gravity center " .
					" -extent $resolution ".
					escapeshellarg($dst) .
					" 2>&1");
		} else {
			$this->convert_exec("" .
					" $convert " .
					escapeshellarg($src) .
					" -resize $virtual_resolution" .
					" -background black " .
					" -gravity center " .
					" -extent $virtual_resolution ".
					" -resize $resolution\! " .
					escapeshellarg($dst) .
					" 2>&1");
		}
		//Slide::slidetool("resample {$this->fullpath} $resolution");
	}
	
	private function convert_exec($command){
		$rc = 0;
		passthru($command, $rc);

		if ( $rc != 0 ){
			die("\n<br/>$command\n");
		}
	}

	private function __construct($fullpath, $meta, $id=-1, $active=false){
		$this->fullpath = $fullpath;
		$this->id = $id;
		$this->active = $active;
		$this->meta = $meta;
	}
	
	static public function from_row($row){
		$id = $row['id'];
		$name = basename($row['fullpath']);
		$fullpath = $row['fullpath'];
		$active = $row['active'];
		$type = $row['type'];
		$title = $row['title'];
		
		$meta = Slide::get_meta($fullpath);
		$type = $meta['type'];
		
		switch ( $type ){
			case 'image': return new SlideImage($fullpath, $meta, $id, $active);
			default: throw new PageException("could not open slide at $fullpath: unknown type $type");
		}
	}
	
	static public function from_path($fullpath){
		$meta = Slide::get_meta($fullpath);
		$type = $meta['type'];
		
		switch ( $type ){
			case 'image': return new SlideImage($fullpath, $meta);
			default: throw new PageException("could not open slide at $fullpath: unknown type $type");
		}
	}
	
	static private function get_meta($fullpath){
		$fp = fopen("$fullpath/meta", 'r');
		if ( !$fp ){
			throw PageException("could not open slide at $fullpath: not a valid slide");
		}
		
		$lines = stream_get_contents($fp);
		fclose($fp);

		return json_decode($lines, true);
	}

	static private function unique_id(){
		return md5(uniqid());
	}
}

class SlideImage extends Slide {

}

?>
