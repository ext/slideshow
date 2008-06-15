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

class Install extends Module {
  function welcome(){
  	session_start();

	if ( !isset( $_SESSION['config'] ) ){
		$_SESSION['config'] = array(
			'BasePath' => '',

			'Path' => array(
				'Image' => 'image',
				'Video' => 'video',
				'Temp' => 'tmp'
			),

			'Files' => array(
				'BinaryPath' => '',
				'Log' => array (
					'Base' => 'slideshow.log',
					'Debug' => 'slideshow.debug.log',
					'Activity' => 'activity.log'
  				),
				'PID' => 'slideshow.pid',
				'convert' => ''
			),

			'Apparence' => array(
				'Background' => '',
				'Font' => '',
				'Resolution' => array(800, 600)
			),

			'Database' => array(
				'Username' => '',
				'Password' => '',
				'Hostname' => 'localhost',
				'Name' => ''
			)
		);
  	}

	Module::set_template('welcome.tmpl');

	return array();
  }

  function step( $n ){
  	Module::set_template('install.tmpl');

  	session_start();
  	if ( !isset( $_SESSION['config'] ) ){
  		$this->redirect('/index.php/install/welcome');
  		return;
  	}

	$ret = array(
		 'path' => realpath('..'),
		 'step' => $n
		 );

	 switch ( $n ){
	 	case 1:
	 		$ret['writable_config'] = is_writable( '..' );
	 		$ret['have_mysql'] = extension_loaded( 'mysql' );
	 		$ret['have_json'] = extension_loaded( 'json' );
	 		$ret['have_gd'] = extension_loaded( 'gd' );
	 		//$ret['have_mysql'] = extension_loaded( 'mysql' );

	 		// Try to find ImageMagick (test derived from phpBB)
	 		$exe = (DIRECTORY_SEPARATOR == '\\') ? '.exe' : '';
	 		$locations = array('C:/WINDOWS/', 'C:/WINNT/', 'C:/WINDOWS/SYSTEM/', 'C:/WINNT/SYSTEM/', 'C:/WINDOWS/SYSTEM3
2/', 'C:/WINNT/SYSTEM32/', '/usr/bin/', '/usr/sbin/', '/usr/local/bin/', '/usr/local/sbin/', '/opt/', '/usr/imagemagick/', '/usr/bin
/imagemagick/');
	 		$path_locations = str_replace('\\', '/', (explode(($exe) ? ';' : ':', getenv('PATH'))));
	 		$locations = array_merge($path_locations, $locations);

	 		foreach ($locations as $location){
	 			// The path might not end properly, fudge it
				if (substr($location, -1, 1) !== '/'){
					$location .= '/';
				}

				if (@is_readable($location . 'mogrify' . $exe) && @filesize($location . 'mogrify' . $exe) > 3000){
					$imagick_path = str_replace('\\', '/', $location);
					continue;
				}
	 		}

	 		$ret['have_imagick'] = false;
	 		$ret['imagick_path'] = '';
	 		if ( isset($imagick_path) ){
	 			$ret['have_imagick'] = true;
	 			$ret['imagick_path'] = $imagick_path;

	 			$_SESSION['config']['Files']['convert'] = $imagick_path . "convert$exe";
	 		}

	 		break;

	 	case 2:
	 		$ret['basepath'] = $_SESSION['config']['BasePath'];
	 		$ret['binpath'] = $_SESSION['config']['Files']['BinaryPath'];

	 		// This is a bit ugly but to suppres error messages it assumes paths are valid
	 		$ret['basepath_found'] = isset($_GET['basepath_found']) ? $_GET['basepath_found'] : true;
	 		$ret['basepath_writable'] = isset($_GET['basepath_writable']) ? $_GET['basepath_writable'] : true;
	 		$ret['binpath_found'] = isset($_GET['binpath_found']) ? $_GET['binpath_found'] : true;

	 		break;

	 	case 3:
	 		$ret['database_host'] = $_SESSION['config']['Database']['Hostname'];
	 		$ret['database_name'] = $_SESSION['config']['Database']['Name'];
	 		$ret['database_username'] = $_SESSION['config']['Database']['Username'];
	 		$ret['database_password'] = $_SESSION['config']['Database']['Password'];

	 		if ( isset( $_GET['connection_failed'] ) ){
	 			$ret['connection_failed'] = stripslashes( $_GET['connection_failed'] );
	 		}

	 		break;

	 	case 4:
	 		$ret['resolution'] = sprintf("%dx%d", $_SESSION['config']['Apparence']['Resolution'][0], $_SESSION['config']['Apparence']['Resolution'][1]);
	 		$ret['background'] = $_SESSION['config']['Apparence']['Background'];
	 		$ret['font'] = $_SESSION['config']['Apparence']['Font'];

			if ( isset( $_GET['valid_resolution'] ) ){
	 			$ret['valid_resolution'] =  $_GET['valid_resolution'];
	 		}
	 		if ( isset( $_GET['valid_background'] ) ){
	 			$ret['valid_background'] =  $_GET['valid_background'];
	 		}
	 		if ( isset( $_GET['valid_font'] ) ){
	 			$ret['valid_font'] =  $_GET['valid_font'];
	 		}

	 		break;

	 	case 5:
	 		$ret['json'] = $this->_configuration_as_json();

	 		break;

	 	case 6:
	 		if ( file_exists('../settings.json') ){
	 			$this->redirect("/index.php/install/complete");
	 		}
	 }


	return $ret;
  }

	function submit( $n ){
		session_start();

		switch ( $n ){
		case 2:
			$basepath = $_POST['basepath'];
			$_SESSION['config']['BasePath'] = $basepath;
			$basepath_found = file_exists($basepath) && is_dir($basepath);
			$basepath_writable = is_writable($basepath);

			$binpath = $_POST['binpath'];
			$_SESSION['config']['Files']['BinaryPath'] = $binpath;
			$binpath_found = file_exists($binpath) && is_executable($binpath);

			if ( $basepath_found && $basepath_writable && $binpath_found ){
				$this->redirect("/index.php/install/step/3");
			} else {
				$this->redirect("/index.php/install/step/2?basepath_found=$basepath_found&basepath_writable=$basepath_writable&binpath_found=$binpath_found");
			}

			break;

		case 3:
			$database_host = $_POST['database_host'];
			$database_name = $_POST['database_name'];
			$database_username = $_POST['database_username'];
			$database_password = $_POST['database_password'];

			$_SESSION['config']['Database']['Hostname'] = $database_host;
			$_SESSION['config']['Database']['Name'] = $database_name;
			$_SESSION['config']['Database']['Username'] = $database_username;
			$_SESSION['config']['Database']['Password'] = $database_password;

			$connection_ok = @mysql_connect($database_host, $database_username, $database_password) !== false;

			$select_ok = false;
			if ( $connection_ok ){
				$select_ok = mysql_select_db($database_name);
			}

			if ( $connection_ok && $select_ok ){
				$this->redirect("/index.php/install/step/4");
			} else {
				$this->redirect("/index.php/install/step/3?connection_failed=".urlencode( mysql_error() ));
			}

			break;

		case 4:
			$resolution = sscanf($_POST['resolution'], "%dx%d");
			$background = trim($_POST['background']);
			$font = trim($_POST['font']);

			$_SESSION['config']['Apparence']['Resolution'] = $resolution;
			$_SESSION['config']['Apparence']['Background'] = $background;
			$_SESSION['config']['Apparence']['Font'] = $font;

			$valid_resolution = count($resolution) == 2;

			$have_background = strlen($background) > 0;
			$valid_background = true;

			if ( $have_background ){
				$valid_background = file_exists($background) && is_readable($background);
			}

			$have_font = strlen($font) > 0;
			$valid_font = true;

			if ( $have_font ){
				$valid_font = file_exists($font) && is_readable($font);
			}

			if ( $valid_resolution && $valid_background && $valid_font ){
				$this->redirect("/index.php/install/step/5");
			} else {
				$this->redirect("/index.php/install/step/4?valid_resolution=$valid_resolution&valid_background=$valid_background&valid_font=$valid_font");
			}

			break;

		case 5:
			$database_host = $_SESSION['config']['Database']['Hostname'];
			$database_name = $_SESSION['config']['Database']['Name'];
			$database_username = $_SESSION['config']['Database']['Username'];
			$database_password = $_SESSION['config']['Database']['Password'];

			mysql_connect($database_host, $database_username, $database_password);

			mysql_query("SOURCE ../maintenance/install.sql");

			$basepath = $_SESSION['config']['BasePath'];
			$imagepath = $basepath . '/' . $_SESSION['config']['Path']['Image'];
			$videopath = $basepath . '/' . $_SESSION['config']['Path']['Video'];
			$temppath = $basepath . '/' . $_SESSION['config']['Path']['Temp'];

			if ( !file_exists($imagepath)){
				mkdir( $imagepath );
			}

			if ( !file_exists($videopath)){
				mkdir( $videopath );
			}

			if ( !file_exists($temppath)){
				mkdir( $temppath );
			}

			if ( is_writable( '..' ) ){
				$file = fopen('../settings.json');
				fwrite($file, $this->_configuration_as_json());
				fclose($file);
				$this->redirect("/index.php/install/complete");
			} else {
				$this->redirect("/index.php/install/step/6");
			}
		}

		return array();
	}

	function complete(){
		Module::set_template('install_complete.tmpl');
		return array();
	}

	function download(){
		session_start();
		Module::set_template('download_config.tmpl', true);
		return array('data' => $this->_configuration_as_json());
	}
};

?>