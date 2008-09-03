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

require_once('../version.php');
require_once('../core/path.inc.php');
require_once('../db_functions.inc.php');
require_once('../thumb_functions.inc.php');
require_once('../core/module.inc.php');
require_once('../core/page_exception.php');
require_once('../models/settings.php');

$path = new Path();
$settings = NULL;

try {
	$settings = new Settings();
} catch ( CorruptSettings $e ){
	die("The settings file is corrupt");
} catch ( InvalidSettings $e ){
	if ( $path->module() != 'install'){
		$path = new Path( 'install', 'welcome' );
	}

	$settings = new Settings('../settings.json.default', true);
} catch ( Exception $e ){
	die($e->message());
}

try {
	$page = Module::factory( $path->module() );
	$page->execute( $path->section(), $path->argv() );
} catch ( Exception $e ){
	$page = Module::factory( 'error' );
	$page->execute( 'display', array($e) );
}

if ( $page->has_custom_view() ){
	$page->render();
	exit();
}

?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
	"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">

<head>
	<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
	<link rel="stylesheet" href="/css/common.css" type="text/css" media="screen" />
	<link rel="stylesheet" href="/css/main.css" type="text/css" media="screen" />
	<title>Slideshow</title>
<?
  //@note hack to get the maintenance page refresh
  if ( $path->module() == 'maintenance' ) { ?>
	<meta http-equiv="refresh" content="5" />
<? } ?>
</head>

<body>
	<div id="header">
		<h1>Slideshow</h1>
	</div>

	<div id="menu">
		<h2>Menu</h2>
		<ul>
			<li class="first<? if ( $path->module() == 'main' ){ ?> bajs<? } ?>"><a href="/index.php/">Main</a></li>
			<li<? if ( $path->module() == 'slides' ){ ?> class="bajs"<? } ?>><a href="/index.php/slides/upload">Slides</a></li>
			<li<? if ( $path->module() == 'video' ){ ?> class="bajs"<? } ?>><a href="/index.php/video">Video</a></li>
			<li<? if ( $path->module() == 'bins' ){ ?> class="bajs"<? } ?>><a href="/index.php/bins">Bins</a></li>
			<li class="last<? if ( $path->module() == 'maintenance' ){ ?> bajs<? } ?>"><a href="/index.php/maintenance">Maintenance</a></li>
		</ul>
	</div>
<?

$page->render();

// Usually the site is protected with http basic auth but sometimes it is not,
// during the installation for instance. Therefore we cannot rely on PHP_AUTH_USER
// being set.
if ( isset($_SERVER['PHP_AUTH_USER']) ){ ?>
	<hr>
	Inloggad som <?=$_SERVER['PHP_AUTH_USER'];?> (<a href="/logout.php">Logout</a>)
<? } ?>

	<div id="footer">
		Powered by <a href="http://sidvind.com:8000/slideshow">Slideshow <?=version_as_string()?></a>
	</div>

</body>

</html>
