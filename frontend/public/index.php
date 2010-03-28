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

require_once('../version.php');
require_once('../constants.php');
require_once('../core/path.inc.php');
require_once('../db_functions.inc.php');
require_once('../common.inc.php');
require_once('../thumb_functions.inc.php');
require_once('../core/module.inc.php');
require_once('../core/exception.php');
require_once('../models/settings.php');
require_once('../daemonlib/daemonlib.php');

$path = new Path();
$settings = NULL;
$daemon = NULL;

try {
	try {
		$settings = new Settings();
		$daemon = new SlideshowInst($settings->slideshow_executable(), $settings->pid_file());
	} catch ( InvalidSettings $e ){
		if ( $path->module() != 'install'){
			$path = new Path( 'install', 'welcome' );
		}

		$settings = new Settings('../settings.json.default', true);
	}

	$page = Module::factory( $path->module() );
	$page->execute( $path->section(), $path->argv() );
} catch ( Exception $e ){
	$page = Module::factory( 'error' );
	$page->execute( 'display', array($e) );
}

$page->render();

?>