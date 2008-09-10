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

require_once ('daemonargs.php');
require_once ('../dbus/dbus_session.php');

class SlideshowInst {
	private $pid;
	private $pidfile;
	private $binary;

	const StatusStopped = 0;
	const StatusRunning = 1;
	const StatusCrashed = 2;

	public function __construct($binaryfile, $pidfile){
		$this->pid = 0;
		$this->pidfile = $pidfile;
		$this->binary = $binaryfile;

		// This is required if we are going to start the executable.
		if ( !(file_exists($binaryfile) && is_executable($binaryfile)) ){
			throw new ExecutableException("Could not find binary `$binaryfile' or did not have permission to execute it");
		}

		if ( file_exists($pidfile) ){
			///@todo Not portable
			$this->pid = (int)`cat $pidfile`;
		}
	}

	public function get_status(){
		// If $pid is zero the process isn't available.
		if ( $this->pid == 0 ){
			return SlideshowInst::StatusStopped;
		}

		// Check if the process accepts signals
		///@todo Not portable
		if ( posix_kill($this->pid, 0) ){
			return SlideshowInst::StatusRunning;
		}

		// The process does not exist or is in a unresponsive state,
		// either way, it is malfunctioning.
		return SlideshowInst::StatusCrashed;
	}

	public function get_status_string(){
		$names = array(
			'Stopped',
			'Running',
			'Crashed'
		);

		return $names[$this->get_status()];
	}

	function start($arguments, $force = false){
		if ( !($arguments instanceof DaemonArguments) ){
			die("Argument 1 is not an instance of DaemonArguments");
		}

		if ( !$force && $this->get_status() == SlideshowInst::StatusRunning ){
			throw new ExecutableException("Deamon is not stopped, cannot start.");
		}

		if ( $force && $this->pid > 0 ){
			posix_kill($this->pid, 9);
			unlink($this->pidfile);
		}

		$cmd = 	"ulimit -c unlimited; echo '{$arguments->password()}' | DISPLAY=\":0\" $this->binary {$arguments->as_string()} >> {$arguments->logfile()} 2>&1";

		$old_wd = getcwd();
		chdir( $arguments->basepath() );

		$stdout = array();
		$ret = 0;
		exec($cmd, $stdout, $ret);

		chdir( $old_wd );

		if ( $ret != 0 ){
			switch ( $ret ) {
			case 1:
				throw new ExecutableException( "A connection to the X server could not be made, check permissions." );
				break;

			case 4:
				throw new ExecutableException( "The arguments could not be handled, see log for details." );
				break;

			default:
				throw new ExecutableException( "Unknown error" );
			}
		}

		usleep(1000*200);
	}

	function stop(){
		if ( $this->get_status() != SlideshowInst::StatusRunning ){
		  throw new exception("Deamon is not started, cannot stop.");
		}

		$this->send_signal("Quit");
		usleep(1000*400);
	}

	public function ping(){
		$this->send_signal("Ping");
		usleep(1000*200);
	}

	function debug_dumpqueue(){
		$this->send_signal("Debug_DumpQueue");
		usleep(1000*200);
	}

	private function send_signal($name, $signature = NULL, $payload = NULL){
		$dbus = new direct_dbus_session("unix:///var/run/dbus/system_bus_socket", "../dbus/", false);
		$dbus->dclass_connect();

		$dbus->dclass_send_signal("/com/slideshow/dbus/ping", "com.slideshow.dbus.Signal", $name, NULL, $signature, $payload);

		$dbus->dclass_disconnect();
	}
}