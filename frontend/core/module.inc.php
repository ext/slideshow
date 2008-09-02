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
require_once ('file_not_found.inc.php');

class Module {
	private $_template = '';
	private $_data = array();
	private $_name = 'undefined';
	private $_custom_view = false;

	function factory( $module ){
		$fullpath = "../pages/$module.php";

		if ( !file_exists($fullpath) ){
			throw new FileNotFound();
		}

		require_once($fullpath);

		$m = new $module;
		$m->_name = $module;

		return $m;
	}

  function set_template( $template, $custom_view = false ){
	$this->_template = $template;
	$this->_custom_view = $custom_view;
  }

  function has_custom_view(){
  	return $this->_custom_view;
  }

  function index(){
	echo "Module::default";
  }

	function execute( $section, array $argv ){

		// We set the template to correspond to the section being executed.
		$this->set_template($this->_name . '/' . $section . '.tmpl');

		try {
			$functor = array( $this, $section );

			if ( !is_callable($functor) ){
				throw new FileNotFound();
			}

			$this->_data = call_user_func_array( $functor, $argv );

			if ( !is_array($this->_data) ){
				throw new Exception("Call did not return an array: " . print_r($this->_data, true));
			}
		} catch ( FileNotFound $e ){
			throw $e;
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

	global $settings;
	$settings->persist();

	header("Location: $location");
	exit();
  }

	function log($msg){
		global $settings;

		$user = 'anon';
		if ( isset($_SERVER['PHP_AUTH_USER']) ){
			$user = $_SERVER['PHP_AUTH_USER'];
		}

		$settings->activity_log()->write("$user $msg\n");
	}

	function render(){
		if ( !file_exists("../pages/$this->_template") ){
			throw new Exception("The template '$this->_template' could not be found!");
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