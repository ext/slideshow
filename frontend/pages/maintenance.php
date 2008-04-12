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

require_once('../settings.inc.php');
require_once('../core/module.inc.php');
require_once('../log_functions.inc.php');
require_once ("../dbus/dbus_session.php");

class Maintenance extends Module {
  function index(){
    Module::set_template('maintenance.tmpl');

    $show_debug = false;
    if ( isset($_GET['show_debug']) ){
      $show_debug = true;
    }

    global $BasePath, $Files;

    $activity_log = $BasePath . "/" . $Files['Log']['Activity'];
    $activity = explode("\n", `tail $activity_log`);

    return array(
		 'log' => get_log(25, $show_debug),
		 'activity' => $activity,
		 'status' => $this->_get_daemon_status(),
		 'show_debug' => $show_debug
		 );
  }

  function forcestart(){
    global $Files;

    $pid_file = $Files['PID'];
	
    $pid = (int)`cat $pid_file`;
    posix_kill($pid, 9);

    unlink($pid_file);
    unlink($Files['Log']['Base']);
    $this->start();
  }

  function start(){
    if ( $this->_get_daemon_status() != 0 ){
      throw new exception("Deamon is not stopped, cannot start.");
    }

    global $BasePath, $Path, $Files, $Database, $Settings;
    
    $binary = "$Path[Build]/$Files[Binary]";
    $resolution_x = $Settings['Resolution'][0];
    $resolution_y = $Settings['Resolution'][1];

    if ( !file_exists($binary) ){
      throw new exception("Could not find binary \"$binary\" or did not have permission to execute it");
    }
    
    $cmd = "ulimit -c unlimited; DISPLAY=\":0\" $binary --daemon --fullscreen --db_user {$Database['Username']} --db_pass {$Database['Password']} --db_name {$Database['Name']} --resolution {$resolution_x}x{$resolution_y} >> {$Files['Log']['Base']} 2>&1";
    chdir($BasePath);

    $stdout = array();
    $ret = 0;
    exec($cmd, $stdout, $ret);

    if ( $ret != 0 ){
      $lines = implode('\n', $stdout);
      throw new exception( $lines );
    }
    
    usleep(1000*200);

    Module::redirect('/index.php/maintenance', array("show_debug"));
  }

  function stop(){
    if ( $this->_get_daemon_status() != 1 ){
      throw new exception("Deamon is not started, cannot stop.");
    }

    $this->send_signal("Quit");
    usleep(1000*200);

    Module::redirect('/index.php/maintenance', array("show_debug"));
  }

  // Returns:
  //   0: Daemon stopped
  //   1: Daemon started
  //   2: Daemon crashed
  function _get_daemon_status(){
    global $Files, $BasePath;

    $pid_file = $BasePath."/".$Files['PID'];

    if ( !file_exists( $pid_file) ){
      return 0;
    }

    $pid = (int)`cat $pid_file`;
    if ( posix_kill($pid, 0) ){
      return 1;
    }

    return 2;
  }

  function coredump(){
    global $BasePath;

    $filename = "$BasePath/core";
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