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

require_once('../settings.inc.php');
require_once('../common.inc.php');
require_once('../db_functions.inc.php');
require_once('../core/module.inc.php');

class Slides extends Module {
	function __construct(){
		connect();
	}

	function __descturuct(){
		disconnect();
	}

  function index(){
	Module::set_template('slides.tmpl');

	return array(
		 );
  }

  function upload(){
	Module::set_template('slides_upload.tmpl');

	$image = NULL;

	$title_orig = post("title");
	$content_orig = post("content");

	$title = htmlentities($title_orig, ENT_NOQUOTES, "utf-8" );
	$content = htmlentities($content_orig, ENT_NOQUOTES, "utf-8" );
	$align = post("align", 1);

	if ( isset($_POST['submit']) ){
	  if ( $_POST['submit'] == 'Preview' ){
	$image = '/index.php/slides/preview?title='.urlencode($title).'&content='.urlencode($content)."&align=$align";
	  }
	  if ( $_POST['submit'] == 'Upload' ){
	global $Path;

	$title = $this->_utf_hack($title_orig);
	$content = explode("\n", $this->_utf_hack($content_orig));

	$filename = md5(uniqid());
	$fullpath = "$Path[Image]/$filename.png";

	$this->_create_image($title, $content, $align, $fullpath);
	q("INSERT INTO files (fullpath) VALUES ('$fullpath')");
	$this->send_signal("Reload");

	Module::log("Added slide titled '$title'");
	Module::redirect("/index.php");
	exit();
	  }
	}

	return array(
		 'image' => $image,
		 'title' => $title,
		 'content' => $content,
		 'align' => $align
		 );
  }

  function submit_image(){
	global $Path, $Settings, $Files;

	$convert = $Files['convert'];

	if ( !file_exists($convert) ){
	  throw new Exception("Could not find ImageMagick 'convert' executable.");
	}

	$name = $_FILES['filename']['name'];
	$hash = md5(uniqid());
	$fullpath = "{$Path['Image']}/{$hash}_$name";

	$uploaded = $_FILES['filename']['tmp_name'];

	if ( !is_uploaded_file( $uploaded ) ){
	  die("Handling file that was not uploaded");
	}

	$resolution = $Settings['Resolution'];
	$resolution_x = $resolution[0];
	$resolution_y = $resolution[1];

	move_uploaded_file($uploaded, $fullpath);

	$cmd = "$convert ".escapeshellarg($fullpath)." -resize {$resolution_x}x{$resolution_y} -background black -gravity center -extent {$resolution_x}x{$resolution_y} ".escapeshellarg($fullpath)." 2>&1 ";

	passthru($cmd, $rc);

	if ( $rc != 0 ){
	  die("\n<br/>$cmd\n");
	}

	q("INSERT INTO files (fullpath) VALUES ('$fullpath')");
	$this->send_signal("Reload");

	Module::log("Uploaded $name");
	Module::redirect('/index.php');
  }

  function _utf_hack($str){
	return strtr($str,
		 array(
			   "Å" => "&#0197;",
			   "Ä" => "&#0196;",
			   "Ö" => "&#0214;",
			   "å" => "&#0229;",
			   "ä" => "&#0228;",
			   "ö" => "&#0246;"
			   ));
  }

  function preview(){
	$title = $this->_utf_hack(html_entity_decode(get('title'), ENT_NOQUOTES, 'UTF-8'));
	$content = explode("\n", $this->_utf_hack(html_entity_decode(get('content'), ENT_NOQUOTES, 'UTF-8')) );

	$align = $_GET['align'];

	$this->_create_image($title, $content, $align);

	exit();
  }

  function _create_image($title, $content, $alignment, $filename = NULL){


	global $Settings, $Files;

	$resolution_x = $Settings['Resolution'][0];
	$resolution_y = $Settings['Resolution'][1];

	$bgfile = $Files['background'];
	//die("$bgfile\n");
	$im = @imagecreatefrompng($bgfile);
	if ( !$im ){
	  $im  = imagecreatetruecolor($resolution_x, $resolution_y);
	  $black  = imagecolorallocate($im, 0, 0, 0);
	  $white  = imagecolorallocate($im, 255, 255, 255);
	  imagefilledrectangle($im, 0, 0, $resolution_x, $resolution_y, $black);
	} else {
	  $black  = imagecolorallocate($im, 0, 0, 0);
	  $white  = imagecolorallocate($im, 255, 255, 255);
	}

	//$font = "/usr/share/fonts/ttf-bitstream-vera/Vera.ttf";
	$font = $Files['font'];

	///@note Magic numbers
	$title_size = 82;
	$content_size = 42;

	// 1 is alignment (center)
	// 100 is y-coordinate
	$this->_render_string_aligned($im, 1, 100, $title_size, $font, $white, $title);

	$y = 180;
	foreach ( $content as $paragraph ){
	  $y = $this->_render_string_aligned($im, $alignment, $y, $content_size, $font, $white, $paragraph);
	}

	if ( $filename == NULL ){
	  header("Content-Type: image/png");
	}

	imagepng($im, $filename);
  }

  function _render_string_aligned($im, $alignment, $y, $size, $font, $color, $string){
	global $Settings;
	$width = $Settings['Resolution'][0];
	$margin = 60;

	$line_spacing = (int)($size * 1.5);

	$words = explode(" ", $string);
	$part = array_shift($words);

	$data = imagettfbbox( $size, 0, $font, $part );
	$old_part_width = $data[2] - $data[0];

	foreach ( $words as $word ){
	  $data = imagettfbbox( $size, 0, $font, "$part $word" );
	  $part_width = $data[2] - $data[0];

	  if ( $part_width > $width - $margin * 2 ){
	switch ( $alignment ){
	case 0:	$this->_render_string_left( $im, $old_part_width, $y, $margin, $width, $size, $font, $color, $part ); break;
	case 1:	$this->_render_string_center( $im, $old_part_width, $y, $margin, $width, $size, $font, $color, $part ); break;
	case 2:	$this->_render_string_right( $im, $old_part_width, $y, $margin, $width, $size, $font, $color, $part ); break;
	default: die("This alignemnt ($alignment) is not implemented yet");
	}

	$part = $word;
	$data = imagettfbbox( $size, 0, $font, $word );
	$old_part_width = $data[2] - $data[0];
	$y += $line_spacing;
	continue;
	  }

	  $part .= " $word";
	  $old_part_width = $part_width;
	}

	switch ( $alignment ){
	case 0:	$this->_render_string_left( $im, $old_part_width, $y, $margin, $width, $size, $font, $color, $part ); break;
	case 1:	$this->_render_string_center( $im, $old_part_width, $y, $margin, $width, $size, $font, $color, $part ); break;
	case 2:	$this->_render_string_right( $im, $old_part_width, $y, $margin, $width, $size, $font, $color, $part ); break;
	default: die("This alignemnt ($alignment) is not implemented yet");
	}

	return $y + $line_spacing;
  }


  function _render_string_left($im, $x, $y, $margin, $width, $size, $font, $color, $string){
	imagefttext( $im, $size, 0, $margin, $y, $color, $font, $string );
  }

  function _render_string_center($im, $x, $y, $margin, $width, $size, $font, $color, $string){
	imagefttext( $im, $size, 0, $width/2-$x/2, $y, $color, $font, $string );
  }

  function _render_string_right($im, $x, $y, $margin, $width, $size, $font, $color, $string){
	imagefttext( $im, $size, 0, $width - $margin - $x, $y, $color, $font, $string );
  }

  function delete(){
	$id = (int)$_GET['id'];
	if ( array_key_exists('confirm', $_GET) === true ){
	  q("DELETE FROM files WHERE id = $id");
	  $this->send_signal("Reload");
	  Module::log("Removed slide with id $id");
	  Module::redirect('/index.php');
	  return;
	}

	$row = mysql_fetch_assoc(q("SELECT id, fullpath FROM files WHERE id = $id"));

	Module::set_template('slides_delete.tmpl');

	return array(
		 'id' => $id,
		 'fullpath' => $row['fullpath'],
		 'filename' => basename($row['fullpath'])
		 );
  }
};

  ?>
