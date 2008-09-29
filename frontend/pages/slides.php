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

require_once('../common.inc.php');
require_once('../db_functions.inc.php');
require_once('../core/module.inc.php');
require_once('../core/page_exception.php');

class UploadException extends PageException {
	public function __construct($message){
		parent::__construct($message, UPLOAD_ERROR);
	}
}

class Slides extends Module {
	public function __construct(){
		connect();
	}

	public function __descturuct(){
		disconnect();
	}

	public function upload(){
		global $settings;

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
				$title = $this->utf_hack($title_orig);
				$content = explode("\n", $this->utf_hack($content_orig));

				$filename = md5(uniqid());
				$fullpath = $settings->image_path() . "/$filename.png";

				$this->create_image($title, $content, $align, $fullpath);
				$this->convert($fullpath, $fullpath . '.thumb.jpg', "200x200");

				q("INSERT INTO files (fullpath, type, title, content) VALUES ('$fullpath', 'text', '" . mysql_real_escape_string($title_orig) . "', '" . mysql_real_escape_string($content_orig) . "')");

				global $daemon;
				$daemon->reload_queue();

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

	public function submit_image(){
		$upload_ok = $_FILES['filename']['error'] === UPLOAD_ERR_OK;

		if ( !$upload_ok ){

			$error_code = $_FILES['filename']['error'];

			$error_string = array(
			    UPLOAD_ERR_INI_SIZE => 'The uploaded file exceeds the upload_max_filesize directive in php.ini.',
			    UPLOAD_ERR_FORM_SIZE => 'The uploaded file exceeds the MAX_FILE_SIZE directive that was specified in the HTML form.',
			    UPLOAD_ERR_PARTIAL => 'The uploaded file was only partially uploaded.',
			    UPLOAD_ERR_NO_FILE => 'No file was uploaded.',
			    UPLOAD_ERR_NO_TMP_DIR => 'Missing a temporary folder.',
			    UPLOAD_ERR_CANT_WRITE => 'Failed to write file to disk.',
			    UPLOAD_ERR_EXTENSION => 'File upload stopped by extension.',
			);

			if ( isset($error_string[$error_code]) ){
		        throw new UploadException($error_string[$error_code]);
			} else {
		        throw new UploadException("Unknown error uploading file.");
			}
		}

		global $settings;

		$convert = $settings->convert_binary();

		if ( !file_exists($convert) ){
			throw new Exception("Could not find ImageMagick 'convert' executable.");
		}

		$name = $_FILES['filename']['name'];
		$hash = md5(uniqid());
		$fullpath = "{$settings->image_path()}/{$hash}_$name";

		$uploaded = $_FILES['filename']['tmp_name'];

		if ( !is_uploaded_file( $uploaded ) ){
			die("Handling file that was not uploaded");
		}

		$resolution = $settings->resolution_as_string();
		$virtual_resolution = $settings->virtual_resolution_as_string();

		move_uploaded_file($uploaded, $fullpath);

		$this->convert($fullpath, $fullpath . '.thumb.jpg', "200x200");
		$this->convert($fullpath, $fullpath, $resolution, $virtual_resolution);

		q("INSERT INTO files (fullpath, type, title) VALUES ('$fullpath', 'image', '" . mysql_real_escape_string($name) . "')");

		global $daemon;
		$daemon->reload_queue();

		Module::log("Uploaded $name");
		Module::redirect('/index.php');
	}

	private function utf_hack($str){
		return strtr($str, array(
			"Å" => "&#0197;",
			"Ä" => "&#0196;",
			"Ö" => "&#0214;",
			"å" => "&#0229;",
			"ä" => "&#0228;",
			"ö" => "&#0246;"
		));
	}

	public function preview(){
		$title = $this->utf_hack(html_entity_decode(get('title'), ENT_NOQUOTES, 'UTF-8'));
		$content = explode("\n", $this->utf_hack(html_entity_decode(get('content'), ENT_NOQUOTES, 'UTF-8')) );

		$align = $_GET['align'];

		$this->create_image($title, $content, $align);

		exit();
	}

	private function create_image($title, $content, $alignment, $filename = NULL){
		global $settings;

		$resolution = $settings->resolution();

		$bgfile = $settings->background();

		$im = false;
		if ( !empty($bgfile) && is_readable($bgfile)  ){
			///@todo This only accepts PNG's
			$im = @imagecreatefrompng($bgfile);
		}

		if ( !$im ){
			$im  = imagecreatetruecolor($resolution[0], $resolution[1]);
			$black  = imagecolorallocate($im, 0, 0, 0);
			$white  = imagecolorallocate($im, 255, 255, 255);
			imagefilledrectangle($im, 0, 0, $resolution[0], $resolution[1], $black);
		} else {
			$black  = imagecolorallocate($im, 0, 0, 0);
			$white  = imagecolorallocate($im, 255, 255, 255);
		}

		$font = $settings->font();

		if ( !(empty($font) && is_readable($font) ) ){
			$font = "../Vera.ttf";
		}

		///@note Magic numbers
		$title_size = 28;
		$content_size = 12;

		// 1 is alignment (center)
		// 100 is y-coordinate
		$this->render_string_aligned($im, 1, 100, $title_size, $font, $white, $title);

		$y = 180;
		foreach ( $content as $paragraph ){
			$y = $this->render_string_aligned($im, $alignment, $y, $content_size, $font, $white, $paragraph);
		}

		if ( $filename == NULL ){
			header("Content-Type: image/png");
		}

		imagepng($im, $filename);
	}

	private function convert($src, $dst, $resolution, $virtual_resolution = NULL){
		global $settings;

		$convert = $settings->convert_binary();

		if ( empty($virtual_resolution) ){
			$this->convert_exec("" .
					" $convert " .
					escapeshellarg($src) .
					" -resize $resolution" .
					" -background black " .
					" -gravity center " .
					" -extent $resolution ".
					escapeshellarg($dst) .
					" 2>&1");
		} else {
			$this->convert_exec("" .
					" $convert " .
					escapeshellarg($src) .
					" -resize $virtual_resolution" .
					" -background black " .
					" -gravity center " .
					" -extent $virtual_resolution ".
					" -resize $resolution\! " .
					escapeshellarg($dst) .
					" 2>&1");
		}
	}

	private function convert_exec($command){
		$rc = 0;
		passthru($command, $rc);

		if ( $rc != 0 ){
			die("\n<br/>$command\n");
		}
	}

	function render_string_aligned($im, $alignment, $y, $size, $font, $color, $string){
		global $settings;

		$resolution = $settings->resolution();
		$width = $resolution[0];
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
					case 0:	$this->render_string_left( $im, $old_part_width, $y, $margin, $width, $size, $font, $color, $part ); break;
					case 1:	$this->render_string_center( $im, $old_part_width, $y, $margin, $width, $size, $font, $color, $part ); break;
					case 2:	$this->render_string_right( $im, $old_part_width, $y, $margin, $width, $size, $font, $color, $part ); break;
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
			case 0:	$this->render_string_left( $im, $old_part_width, $y, $margin, $width, $size, $font, $color, $part ); break;
			case 1:	$this->render_string_center( $im, $old_part_width, $y, $margin, $width, $size, $font, $color, $part ); break;
			case 2:	$this->render_string_right( $im, $old_part_width, $y, $margin, $width, $size, $font, $color, $part ); break;
			default: die("This alignemnt ($alignment) is not implemented yet");
		}

		return $y + $line_spacing;
	}

	private function render_string_left($im, $x, $y, $margin, $width, $size, $font, $color, $string){
		imagefttext( $im, $size, 0, $margin, $y, $color, $font, $string );
	}

	private function render_string_center($im, $x, $y, $margin, $width, $size, $font, $color, $string){
		imagefttext( $im, $size, 0, $width/2-$x/2, $y, $color, $font, $string );
	}

	private function render_string_right($im, $x, $y, $margin, $width, $size, $font, $color, $string){
		imagefttext( $im, $size, 0, $width - $margin - $x, $y, $color, $font, $string );
	}

	public function delete($id){
		global $daemon;

		if ( array_key_exists('confirm', $_GET) === true ){
			q("DELETE FROM files WHERE id = $id");
			$daemon->reload_queue();
			Module::log("Removed slide with id $id");
			Module::redirect('/index.php');
			return;
		}

		$row = mysql_fetch_assoc(q("SELECT id, fullpath FROM files WHERE id = $id"));

		return array(
			'id' => $id,
			'fullpath' => $row['fullpath'],
			'filename' => basename($row['fullpath'])
		);
	}

	public function deactivate( $id ){
		global $daemon;
		q('UPDATE files SET active = false WHERE id = ' . (int)$id );
		$daemon->reload_queue();
		Module::log("Deactivated slide with id $id");
		Module::redirect('/index.php');
	}

	public function activate( $id ){
		global $daemon;
		q('UPDATE files SET active = true WHERE id = ' . (int)$id );
		$daemon->reload_queue();
		Module::log("Activated slide with id $id");
		Module::redirect('/index.php');
	}

	public function activate_bin( $id ){
		global $settings, $daemon;

		$settings->set_current_bin($id);
		$settings->persist();
		$daemon->change_bin($id);

		Module::log("Changing active bin to id $id");
		Module::redirect('/index.php');
	}

	public function move( $id ){
		global $daemon;
		$bin = (int)$_POST['to_bin'];
		q("UPDATE files SET bin_id = $bin WHERE id = " . (int)$id);
		$daemon->reload_queue();
		Module::log("Moving slide $id to bin $bin");
		Module::redirect('/index.php');
	}

	public function moveajax(){
		global $daemon;

		$bin = (int)substr($_POST['bin'], 4);

		$slides = array();
		$data = explode(',', $_POST['slides']);
		foreach ($data as $str){
			$slides[] = (int)substr($str, 6);
		}

		q("START TRANSACTION");
		foreach ($slides as $i => $slide){
			q("UPDATE files SET bin_id = $bin, sortorder = $i WHERE id = $slide");
		}
		q("COMMIT");

		$daemon->reload_queue();
		Module::log("Reordering bin $bin: $_POST[slides]");

		$this->set_template("empty");
	}
};

  ?>
