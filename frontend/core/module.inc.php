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

require_once ("../dbus/dbus_session.php");

class Module {
  private $_template;
  private $_data;

  function Module(){
    $this->_template = '';
  }

  function factory( $module ){
    require_once("../pages/$module.php");
    return new $module;
  }

  function set_template( $template ){
    $this->_template = $template;
  }

  function index(){
    echo "Module::default";
  }

  function execute( $section ){
    try {
      $this->_data = $this->$section();
    } catch ( Exception $e ){
      $this->set_template( "exception.tmpl" );
      $this->_data = array(
			   "Message" => $e->getMessage(),
			   "Stack" => $e->getTrace()
			   );
    }
  }

  function redirect($location, array $keep = NULL){
    if ( $keep != NULL){
      $post = array();

      foreach ( $keep as $var ){
	if ( array_key_exists($var, $_GET) ){
	  $post[$var] = $_GET[$var];
	}
      }

      if ( count($post) > 0 ){
	$n = 0;
	foreach ( $post as $key => $value ){
	  $location .= $n == 0 ? '?' : '&';
	  $location .= "$key=".urlencode($value);
	  $n++;
	}
      }
    }

    header("Location: $location");
    exit();
  }

  function log($msg){
    global $BasePath, $Files;

    $user = $_SERVER['PHP_AUTH_USER'];
    $file = fopen($BasePath . "/" . $Files['Log']['Activity'], 'a');
    
    fwrite($file, "$user $msg\n");
    fclose($file);
  }

  function render(){
    if ( $this->_template == '' ){
      throw new Exception('No template specified for module \''.get_class($this).'\'');
    }

    extract($this->_data);
    require("../pages/$this->_template");
  }

  function send_signal($name, $signature = NULL, $payload = NULL){
    $dbus = new direct_dbus_session("unix:///var/run/dbus/system_bus_socket", "../dbus/", false);
    $dbus->dclass_connect();

    $dbus->dclass_send_signal("/com/slideshow/dbus/ping", "com.slideshow.dbus.Signal", $name, NULL, $signature, $payload);

    $dbus->dclass_disconnect();
  }
};

?>