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

require_once('../core/module.inc.php');
require_once ("../dbus/dbus_session.php");

class Maintenance extends Module {
	function __construct(){
		connect();
	}

	function __descturuct(){
		disconnect();
	}

	function index(){
		global $settings;

		Module::set_template('maintenance.tmpl');

		$show_debug = isset($_GET['show_debug']);

		$ret =  array(
			'log' => array(),
			'activity' => array(),
			'status' => $this->_get_daemon_status(),
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

  function forcestart(){
	global $settings;

	$pid = $settings->pid();

	if ( $pid > 0 ){
		posix_kill($pid, 9);
		unlink($settings->pid_file());
	}

	$this->start();
  }

	function start(){
		global $settings;

		if ( $this->_get_daemon_status() != 0 ){
			throw new exception("Deamon is not stopped, cannot start.");
		}

		$binary = $settings->binary();

		if ( !(file_exists($binary) && is_executable($binary)) ){
			throw new exception("Could not find binary `$binary' or did not have permission to execute it");
		}

		$resolution = $settings->resolution_as_string();
		$db_hostname = $settings->database_hostname();
		$db_username = $settings->database_username();
		$db_password = $settings->database_password();
		$db_name = $settings->database_name();
		$logfile = $settings->log_file();
		$bin_id = $settings->current_bin();

		$cmd = "ulimit -c unlimited; echo '$db_password' | DISPLAY=\":0\" $binary --daemon --fullscreen --db_user $db_username --db_name $db_name --resolution $resolution --bin-id $bin_id >> $logfile 2>&1";

		$old_wd = getcwd();
		chdir( $settings->base_path() );

		$stdout = array();
		$ret = 0;
		exec($cmd, $stdout, $ret);

		chdir( $old_wd );

		if ( $ret != 0 ){
			switch ( $ret ) {
			case 1:
				throw new exception( "A connection to the X server could not be made, check permissions." );
				break;

			default:
				$lines = implode('\n', $stdout);
				throw new exception( $lines );
			}
		}

		usleep(1000*200);

		Module::redirect('/index.php/maintenance', array("show_debug"));
	}

  function stop(){
	if ( $this->_get_daemon_status() != 1 ){
	  throw new exception("Deamon is not started, cannot stop.");
	}

	$this->send_signal("Quit");
	usleep(1000*400);

	Module::redirect('/index.php/maintenance', array("show_debug"));
  }

  // Returns:
  //   0: Daemon stopped
  //   1: Daemon started
  //   2: Daemon crashed
  function _get_daemon_status(){
	global $settings;

	$pid = $settings->pid();

	// If $pid is zero the process isn't available.
	if ( $pid == 0 ){
	  return 0;
	}

	// Check if the process accepts signals
	///@todo Not portable
	if ( posix_kill($pid, 0) ){
	  return 1;
	}

	// The process does not exist or is in a unresponsive state,
	// either way, it is malfunctioning.
	return 2;
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
	$this->send_signal("Ping");
	usleep(1000*200);
	Module::redirect('/index.php/maintenance', array("show_debug"));
  }

  function debug_dumpqueue(){
	$this->send_signal("Debug_DumpQueue");
	usleep(1000*200);
	Module::redirect('/index.php/maintenance', array("show_debug"));
  }

};

?>