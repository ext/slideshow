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

require_once('log.php');

class Settings {
	function __construct($filename = '../settings.json'){
		if ( !file_exists($filename) ){
			throw new Exception($filename . " not found!");
		}

		$json_string = file_get_contents($filename);
		$this->_data = json_decode( $json_string, true );

		echo $this->base_path();
	}

	function store(){
		$file = fopen('../settings.json');
		fwrite($file, $this->as_json());
		fclose($file);
	}

	function base_path(){
		return $this->_data['BasePath'];
	}

	function set_base_path($new_path){
		$this->_data['BasePath'] = $new_path;
	}

	function image_path(){
		return $this->base_path() . '/' . $this->_data['Path']['Image'];
	}

	function set_image_path($new_path){
		$this->_data['Path']['Image'] = $new_path;
	}

	function video_path(){
		return $this->base_path() . '/' . $this->_data['Path']['Video'];
	}

	function set_video_path($new_path){
		$this->_data['Path']['Video'] = $new_path;
	}

	function temp_path(){
		return $this->base_path() . '/' . $this->data['Path']['Temp'];
	}

	function set_temp_path($new_path){
		$this->_data['Path']['Temp'] = $new_path;
	}

	function binary(){
		return $this->_data['Files']['BinaryPath'];
	}

	function set_binary($new_path){
		$this->_data['Files']['Binary'] = $new_path;
	}

	function log(){
		return new Log( $this->base_path() . '/' . $this->_data['Files']['Log']['Base'] );
	}

	function set_log($new_path){
		$this->_data['Files']['Log']['Base'] = $new_path;
	}

	function debug_log(){
		return new Log( $this->base_path() . '/' . $this->_data['Files']['Log']['Debug'] );
	}

	function set_debug_log($new_path){
		$this->_data['Files']['Log']['Debug'] = $new_path;
	}

	function activity_log(){
		return new Log( $this->base_path() . '/' .  $this->_data['Files']['Log']['Activity'] );
	}

	function set_activity_log($new_path){
		$this->_data['Files']['Log']['Activity'] = $new_path;
	}

	function pid(){
		$pid_file = $this->pid_file();

		if ( !file_exists($pid_file) ){
			return 0;
		}

		///@todo Not portable
		return (int)`cat $pid_file`;
	}

	function pid_file(){
		return $this->_data['Files']['PID'];
	}

	function set_pid_file($new_path){
		$this->_data['Files']['PID'] = $new_path;
	}

	function as_json(){
		return $this->_prettify( json_encode( $this->_data) );
	}

	function _prettify($json){
		$indent = 0;
		$no_escape = false;
		$in_string = false;

		//$json = trim($json);
		$len = strlen($json);

		$pretty = "";

		for ( $i = 0; $i < $len; $i++ ){

			// Remove any escaped forward slashes since JSON handles them anyway.
			// It is not correct to escape them anyway and makes reading it harder.
			if ( $in_string && $json[$i] == '\\' && $json[$i+1] == '/' ){
				continue;
			}

			$pretty .=  $json[$i];

			if ( $json[$i] == '"' && $json[$i-1] != '\\' ){
				$in_string = !$in_string;
				continue;
			}

			if ( $in_string ){
				continue;
			}

			switch ( $json[$i] ){
			case '"':
			case '[':
			case ']':
				$no_escape = !$no_escape;
			}

			if ( $no_escape ){
				continue;
			}

			switch ( $json[$i] ){
			case '{':
				$indent++;
				$pretty .= "\n" . str_repeat( "\t", $indent );
				break;

			case '}':
				$indent--;

				$pretty[strlen($pretty)-1] = "\n";
				$pretty .= str_repeat( "\t", $indent ) . '}';
				break;

			case ',':
				if ( $indent == 1 ){
					$pretty .= "\n";
				}

				$pretty .= "\n" . str_repeat( "\t", $indent );



				break;

			case ':':
				$pretty .= ' ';
				break;

			}
		}

		$pretty .= "\n";

		return $pretty;
	}
};





?>
