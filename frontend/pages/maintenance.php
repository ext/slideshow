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

require_once('../core/module.inc.php');
require_once ("../dbus/dbus_session.php");
require_once ("../daemonlib/daemonlib.php");

class ExecutableException extends PageException {
	public function __construct($message){
		parent::__construct($message, EXECUTABLE_ERROR);
	}
}

class Maintenance extends Module {
	private $deamon;

	function __construct(){
		global $settings;

		connect();
		$this->daemon = new SlideshowInst( $settings->binary(), $settings->pid_file() );
	}

	function __descturuct(){
		disconnect();
	}

	function index(){
		global $settings;

		$show_debug = isset($_GET['show_debug']);

		$ret =  array(
			'log' => array(),
			'activity' => array(),
			'status' => $this->daemon->get_status(),
			'show_debug' => $show_debug
		);

		try {
			$activity_log = $settings->activity_log();
			$ret['activity'] = $activity_log->read_lines(25);
		} catch ( Exception $e ){}

		try {
			$log = $settings->log();
			if ( $show_debug ){
				$log = $settings->debug_log();
			}

			$ret['log'] = $log->read_lines(25);
		} catch ( Exception $e ){}

		return $ret;
	}

	private function _get_arguments_from_settings(){
		global $settings;

		$arguments = new DaemonArguments;

		$arguments->set_database_settings($settings->database_hostname(), $settings->database_name(), $settings->database_username(), $settings->database_password());
		$arguments->set_resolution($settings->resolution_as_string());
		$arguments->set_logfile($settings->log_file());
		$arguments->set_bin_id($settings->current_bin());
		$arguments->set_basepath($settings->base_path());

		return $arguments;
	}

	function forcestart(){
  		$arguments = $this->_get_arguments_from_settings();
		$this->daemon->start($arguments, true);
		Module::redirect('/index.php/maintenance', array("show_debug"));
	}

	function start(){
		$arguments = $this->_get_arguments_from_settings();
		$this->daemon->start($arguments);
		Module::redirect('/index.php/maintenance', array("show_debug"));
	}

	function stop(){
		$this->daemon->stop();
		Module::redirect('/index.php/maintenance', array("show_debug"));
	}

	function coredump(){
		global $settings;

		$filename = $settings->base_path() . '/core';
		$filesize = filesize($filename);

		header("Pragma: public");
		header("Expires: 0");
		header("Cache-Control: must-revalidate, post-check=0, pre-check=0");
		header("Cache-Control: private",false);
		header("Content-Transfer-Encoding: binary");
		header("Content-Type: application/octet-stream");
		header("Content-Length: " . $filesize);
		header("Content-Disposition: attachment; filename=\"core\";" );

		$file = fopen($filename, "rb");

		echo fread($file, $filesize);

		fclose($file);
	}

	function kill_mplayer(){
		`killall mplayer`;
		Module::redirect("index.php/maintenance");
	}

	function ping(){
		$this->daemon->ping();
		Module::redirect('/index.php/maintenance', array("show_debug"));
	}

	function debug_dumpqueue(){
		$this->daemon->ping();
		Module::redirect('/index.php/maintenance', array("show_debug"));
	}
};

?>