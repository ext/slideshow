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

require_once ('file_not_found.inc.php');

class Module {
	private $_view = '';
	private $_template = 'main';
	private $_data = array();
	private $_name = 'undefined';

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

	private function set_view( $name, $section ){
		$this->_view = '../pages/' . $name . '/' . $section . '.tmpl';
	}

	private function view(){
		return $this->_view;
	}

	public function set_template($template){
		$this->_template = $template;
	}

	private function template(){
		return "../templates/$this->_template.php";
	}

	function execute( $section, array $argv ){
		$this->set_view($this->_name, $section);

		$functor = array( $this, $section );

		if ( !is_callable($functor) ){
			throw new FileNotFound();
		}

		$this->_data = call_user_func_array( $functor, $argv );
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

		$user = $_SERVER['REMOTE_ADDR'];
		if ( isset($_SERVER['PHP_AUTH_USER']) ){
			$user = $_SERVER['PHP_AUTH_USER'];
		}

		$settings->activity_log()->write("$user $msg\n");
	}

	function render(){
		if ( !file_exists($this->template()) ){
			throw new Exception("Fatal: template \"{$this->template()}\" not found.");
		}

		extract(array(
			"page" => $this,
			"path" => $GLOBALS['path'],
			"daemon" => $GLOBALS['daemon'],
			"settings" => $GLOBALS['settings']
		));

		require($this->template());
	}

	function render_content(){
		if ( !file_exists($this->view()) ){
			return;
		}

		if ( is_array($this->_data) ){
			extract($this->_data);
		}

		require("../pages/$this->_view");
	}
};

?>