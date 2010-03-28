<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
	"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en"
	xmlns:py="http://genshi.edgewall.org/" py:strip="">

<head>
	<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
	<link rel="stylesheet" href="/css/common.css" type="text/css" media="screen" />
	<link rel="stylesheet" href="/css/main.css" type="text/css" media="screen" />
<? if ( $page->have_extra_css() ){ ?>
	<style type="text/css">
<?= $page->extra_css(2); ?>
	</style>
<? } ?>
	<script src="/js/jquery-1.2.6.js" type="text/javascript"></script>
	<script src="/js/ui.core.js" type="text/javascript"></script>
	<script src="/js/ui.sortable.js" type="text/javascript"></script>
	<title><?=$settings->title();?></title>
<?
  //@note hack to get the maintenance page refresh
  if ( $path->module() == 'maintenance' ) { ?>
	<meta http-equiv="refresh" content="5" />
<? } ?>
</head>

<body>
	<div id="header">
		<h1><?=$settings->title();?></h1>
	</div>

	<div id="bar">
		<span class="daemonstatus">Status: <?= $daemon ? $daemon->get_status_string() : 'Unknown' ?></span>

		<div id="menu">
			<h2>Menu</h2>
			<ul>
				<li class="first<? if ( $path->module() == 'main' ){ ?> bajs<? } ?>"><a class="menu_main" href="<?=href()?>">Main</a></li>
				<li<? if ( $path->module() == 'slides' ){ ?> class="bajs"<? } ?> ><a class="menu_slide" href="<?=href('slides', 'upload')?>">Slides</a></li>
				<li<? if ( $path->module() == 'video' ){ ?> class="bajs"<? } ?>><a class="menu_video" href="<?=href('video')?>">Video</a></li>
				<li<? if ( $path->module() == 'bins' ){ ?> class="bajs"<? } ?>><a class="menu_bins" href="<?=href('bins')?>">Queues</a></li>
				<li class="last<? if ( $path->module() == 'maintenance' ){ ?> bajs<? } ?>"><a class="menu_maintenance" href="<?=href('maintenance')?>">Maintenance</a></li>
			</ul>
		</div>
	</div>
<?

$page->render_content();

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