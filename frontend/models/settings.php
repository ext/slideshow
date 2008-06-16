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
	private $filename;
	private $readonly;
	private $data;

	function __construct($filename = '../settings.json', $readonly = false){
		if ( !file_exists($filename) ){
			throw new Exception($filename . " not found!");
		}

		$this->filename = realpath($filename);
		$this->readonly = $readonly;
		$json_string = file_get_contents($this->filename);
		$this->data = json_decode( $json_string, true );
	}

	function store(){
		$file = fopen('../settings.json');
		fwrite($file, $this->as_json());
		fclose($file);
	}

	function base_path(){
		return $this->data['BasePath'];
	}

	function set_base_path($new_path){
		$this->data['BasePath'] = $new_path;
	}

	function image_path(){
		return $this->base_path() . '/' . $this->data['Path']['Image'];
	}

	function set_image_path($new_path){
		$this->data['Path']['Image'] = $new_path;
	}

	function video_path(){
		return $this->base_path() . '/' . $this->data['Path']['Video'];
	}

	function set_video_path($new_path){
		$this->data['Path']['Video'] = $new_path;
	}

	function temp_path(){
		return $this->base_path() . '/' . $this->data['Path']['Temp'];
	}

	function set_temp_path($new_path){
		$this->data['Path']['Temp'] = $new_path;
	}

	function binary(){
		return $this->data['Files']['BinaryPath'];
	}

	function set_binary($new_path){
		$this->data['Files']['Binary'] = $new_path;
	}

	function log(){
		return new Log( $this->log_file() );
	}

	function log_file(){
		return $this->base_path() . '/' . $this->data['Files']['Log']['Base'];
	}

	function set_log($new_path){
		$this->data['Files']['Log']['Base'] = $new_path;
	}

	function debug_log(){
		return new Log( $this->debug_log_file() );
	}

	function debug_log_file(){
		return $this->base_path() . '/' . $this->data['Files']['Log']['Debug'];
	}

	function set_debug_log($new_path){
		$this->data['Files']['Log']['Debug'] = $new_path;
	}

	function activity_log(){
		return new Log( $this->activity_log_file() );
	}

	function activity_log_file(){
		return $this->base_path() . '/' .  $this->data['Files']['Log']['Activity'];
	}

	function set_activity_log($new_path){
		$this->data['Files']['Log']['Activity'] = $new_path;
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
		return $this->base_path() . '/' . $this->data['Files']['PID'];
	}

	function set_pid_file($new_path){
		$this->data['Files']['PID'] = $new_path;
	}

	function motd(){
		if ( !( is_readable($this->motd_file()) && filesize($this->motd_file()) > 0 ) ){
			return "";
		}

		return file_get_contents( $this->motd_file() );
	}

	function motd_file(){
		return $this->base_path() . '/' . $this->data['Files']['MOTD'];
	}

	function set_motd($text){
		$file = fopen( $this->motd_file() );
		fwrite($text);
		fclose($file);
	}

	function set_motd_file($new_path){
		$this->data['Files']['MOTD'] = $new_path;
	}

	function convert_binary(){
		return $this->data['Files']['convert'];
	}

	function set_convert_binary($new_path){
		$this->data['Files']['convert'] = $new_path;
	}

	function background(){
		return $this->data['Apparence']['Background'];
	}

	///@todo The new background should be copied into basepath
	function set_background($new_path){
		$this->data['Apparence']['Background'] = trim($new_path);
	}

	function font(){
		return $this->data['Apparence']['Font'];
	}

	///@todo The new font should be copied into basepath
	function set_font($new_path){
		$this->data['Apparence']['Font'] = trim($new_path);
	}

	function resolution(){
		return $this->data['Apparence']['Resolution'];
	}

	function resolution_as_string(){
		return sprintf("%dx%d", $this->data['Apparence']['Resolution'][0], $this->data['Apparence']['Resolution'][1]);
	}

	function set_resolution(){
		$nr_of_arguments = func_num_args();
		if ( $nr_of_arguments == 0 || $nr_of_arguments > 2 ){
			throw new Exception('Wrong number of arguments');
		}

		$args = func_get_args();

		if ( $nr_of_arguments == 1 ){
			if ( is_string($args[0]) ){
				$this->data['Apparence']['Resolution'] = sscanf("%dx%d", $args[0]);
				return;
			}
			if ( is_array($args[0]) ){
				if ( count($args[0]) != 2 ){
					throw new Exception('Array count is not two!');
				}
				$this->data['Apparence']['Resolution'] = $args[0];
				return;
			}
			throw new Exception('Wrong argument type, must be string or array, got ' . gettype($args[0]));
		}

		if ( !( is_integer($args[0]) && is_integer($args[1]) )){
			throw new Exception('Wrong argument type, must be integers.');
		}

		$this->data['Apparence']['Resolution'] = array($args[1], $args[2]);
	}

	function database_hostname(){
		return $this->data['Database']['Hostname'];
	}

	function set_database_hostname($new_value){
		$this->data['Database']['Hostname'] = $new_value;
	}

	function database_password(){
		return $this->data['Database']['Password'];
	}

	function set_database_password($new_value){
		$this->data['Database']['Password'] = $new_value;
	}

	function database_username(){
		return $this->data['Database']['Username'];
	}

	function set_database_username($new_value){
		$this->data['Database']['Username'] = $new_value;
	}

	function database_name(){
		return $this->data['Database']['Name'];
	}

	function set_database_name($new_value){
		$this->data['Database']['Name'] = $new_value;
	}

	function current_bin(){
		return isset( $this->data['Runtime']['Bin'] ) ? $this->data['Runtime']['Bin'] : 1;
	}

	function set_current_bin($n){
		return $this->data['Runtime']['Bin'] = $n;
	}

	function as_json(){
		return $this->_prettify( json_encode( $this->data) );
	}

	function persist(){
		if ( $this->readonly ){
			return;
		}

		$file = fopen($this->filename, 'w');
		fwrite($file, $this->as_json());
		fclose($file);
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
