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

require_once('log.php');
require_once('../core/json.php');

class InvalidSettings extends Exception {}
class CorruptSettings extends Exception {}

class Settings {
	private $filename;
	private $readonly;
	private $data;

	function __construct($source = '../settings.json', $readonly = false){
		$this->readonly = $readonly;

		if ( is_array($source) ){
			$this->from_array($source);
		} elseif ( is_string($source) ){
			$this->from_file($source);
		}
	}

	private function from_array($array){
		$this->data = $array;
		if ( is_string($this->data['Apparence']['Resolution']) ){
			$this->set_resolution($this->data['Apparence']['Resolution']);
		}
		if ( is_string($this->data['Apparence']['VirtualResolution']) ){
			$this->set_virtual_resolution($this->data['Apparence']['VirtualResolution']);
		}
	}

	private function from_file($filename){
		if ( !file_exists($filename) ){
			throw new InvalidSettings($filename . " not found!");
		}

		$this->filename = realpath($filename);
		$json_string = file_get_contents($this->filename);
		$this->data = json_decode( $json_string, true );

		if ( ! is_array($this->data) ){
			$test = trim($this->data);
			if ( empty($test) ){
				throw new InvalidSettings("Empty settings");
			}

			throw new CorruptSettings();
		}
	}

	function base_path(){
		return $this->data['Path']['BasePath'];
	}

	function set_base_path($new_path){
		$this->data['Path']['BasePath'] = $new_path;
	}

	function environment(){
		return isset($this->data['Env']) ? $this->data['Env'] : array();;
	}

	function add_environment($line){
		$part = explode('=', $line, 2);
		$this->data['Env'][$part[0]] = $part[1];
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
		$this->data['Files']['BinaryPath'] = $new_path;
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
		$this->data['Apparence']['Resolution'] = $this->transform_resolution(func_get_args());
	}

	function virtual_resolution(){
		if ( isset($this->data['Apparence']['VirtualResolution']) && !empty($this->data['Apparence']['VirtualResolution']) ){
			return $this->data['Apparence']['VirtualResolution'];
		} else {
			return $this->resolution();
		}
	}

	function virtual_resolution_as_string(){
		if ( isset($this->data['Apparence']['VirtualResolution']) && !empty($this->data['Apparence']['VirtualResolution']) ){
			return sprintf("%dx%d", $this->data['Apparence']['VirtualResolution'][0], $this->data['Apparence']['VirtualResolution'][1]);
		} else {
			return $this->resolution_as_string();
		}
	}

	function set_virtual_resolution(){
		$this->data['Apparence']['VirtualResolution'] = $this->transform_resolution(func_get_args());
	}

	private function transform_resolution($args){
		$nr_of_arguments = count($args);
		if ( $nr_of_arguments == 0 || $nr_of_arguments > 2 ){
			throw new Exception('Wrong number of arguments');
		}

		if ( $nr_of_arguments == 1 ){
			if ( is_string($args[0]) ){
				return sscanf($args[0], "%dx%d");
			}
			if ( is_array($args[0]) ){
				if ( count($args[0]) != 2 ){
					throw new Exception('Array count is not two!');
				}
				return $args[0];
			}
			throw new Exception('Wrong argument type, must be string or array, got ' . gettype($args[0]));
		}

		if ( !( is_integer($args[0]) && is_integer($args[1]) )){
			throw new Exception('Wrong argument type, must be integers.');
		}

		return array($args[0], $args[1]);
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

	function title(){
		return $this->data['Slideshow']['Title'];
	}

	function as_json(){
		return pretty_json( json_encode( $this->data) );
	}

	function as_array(){
		return $this->data;
	}

	function persist( $filename = NULL ){
		if ( $filename == NULL && $this->readonly ){
			return;
		}

		if ( $filename == NULL ){
		  $filename = $this->filename;
		}

		$file = fopen($filename, 'w');
		fwrite($file, $this->as_json());
		fclose($file);
	}
};

?>
