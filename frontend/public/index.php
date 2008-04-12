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
require_once("../settings.inc.php");
require_once("../db_functions.inc.php");
require_once("../thumb_functions.inc.php");

require_once("../core/path.inc.php");
require_once("../core/module.inc.php");

connect();

$path = new Path();

$module = $path->module();
$page = Module::factory( $module );

$section = $path->section();
$page->execute( $section );

?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
    "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">

<head>
  <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
  <link rel="stylesheet" href="/css/style.css" type="text/css" media="screen" />
  <title>Slideshow</title>
<?
  //@note hack to get the maintenance page refresh
  if ( $module == 'maintenance' ) { ?>
  <meta http-equiv="refresh" content="5" />
<? } ?>
</head>

<body>

<?

$page->render();

disconnect();

?>

<hr>
Inloggad som <?=$_SERVER['PHP_AUTH_USER'];?> (<a href="/logout.php">Logout</a>)
</body>

</html>
