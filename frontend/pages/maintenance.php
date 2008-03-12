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

    return array(
		 'log' => get_log(25, $show_debug),
		 'status' => $this->_get_daemon_status(),
		 'show_debug' => $show_debug
		 );
  }

  function forcestart(){
    global $pid_file, $log_file;

    $pid = (int)`cat $pid_file`;
    posix_kill($pid, 9);

    unlink($pid_file);
    unlink($log_file);
    $this->start();
  }

  function start(){
    if ( $this->_get_daemon_status() != 0 ){
      throw new exception("Deamon is not stopped, cannot start.");
    }

    global $app_dir, $binary;
    $cmd = "ulimit -c unlimited; DISPLAY=\":0\" $binary --daemon --fullscreen >> /dev/null 2>&1";
    chdir($app_dir);
    
    echo "$cmd<br/>\n";

    $stdout = array();
    $ret = 0;
    exec($cmd, $stdout, $ret);

    if ( $ret != 0 ){
      $lines = implode('\n', $stdout);
      
      foreach ( $lines as $line ){
        echo "$line<br/>\n";
      }
       
      throw new exception( $lines );
    }

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
    global $pid_file;
    if ( !file_exists( $pid_file ) ){
      return 0;
    }

    $pid = (int)`cat $pid_file`;
    if ( posix_kill($pid, 0) ){
      return 1;
    }

    return 2;
  }

  function coredump(){
    global $app_dir;

    $filename = "$app_dir/core";
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