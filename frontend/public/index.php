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
Inloggad som <?=$_SERVER['PHP_AUTH_USER'];?>
</body>

</html>
