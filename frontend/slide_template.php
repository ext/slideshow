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

class Field {
	private $type;
	private $font;
	private $position;

	const type_text = 0;
	const type_textarea = 1;
	const type_static = 2;

	const align_right = 0;
	const align_left = 1;
	const align_center = 2;

	public function __construct($type, $font, $position){
		$this->type = $type;
		$this->font = $font;
		$this->set_position($position);
	}

	public function type(){ return $this->type; }
	public function font(){ return $this->font; }
	public function position(){ return $this->position; }

	private function set_position($string){
		$parts = explode(' ', $string);

		if ( count($parts) != 2 ){
			throw new Exception("Invalid position: " . $string);
		}

		$this->position = $parts;
	}
}

class TextField extends Field {
	private $name;
	private $align;

	public function __construct($name, $align, $font, $position){
		parent::__construct(Field::type_text, $font, $position);
		$this->name = $name;
		$this->align = $align;
	}

	public function name(){ return $this->name; }
	public function align(){ return $this->align; }
}

class TextAreaField extends Field {
	private $name;
	private $align;

	public function __construct($name, $align, $font, $position){
		parent::__construct(Field::type_textarea, $font, $position);
		$this->name = $name;
		$this->align = $align;
	}

	public function name(){ return $this->name; }
	public function align(){ return $this->align; }
}

class StaticField extends Field {
	private $value;
	private $size;

	public function __construct($value, $size, $font, $position){
		parent::__construct(Field::type_static, $font, $position);
		$this->value = $value;
		$this->size = $size;
	}

	public function value(){ return $this->value; }
	public function size(){ return $this->size; }
}

class Node {
	public $tag;
	public $prev;
	public $attrs;

	public function __construct($tag, $attrs, $prev){
		$this->tag = $tag;
		$this->attrs = $attrs;
		$this->prev = $prev;
	}
}

interface ISlideTemplate {
}

class SlideTemplate {
	private $node = NULL;
	private $fonts = array();
	private $default_font = '';
	private $background = '';
	private $fields = array();

	public function __construct($xmlfile){
		libxml_use_internal_errors(true);

		$doc = new DOMDocument;
		$doc->Load($xmlfile);

		if (!$doc->validate()) {
			echo "$xmlfile is not a valid slide template!\n";
		    $errors = libxml_get_errors();
		    foreach ($errors as $error) {
		    	echo basename($error->file), ':', $error->line, ' ', $error->message;
		    }

		    libxml_clear_errors();
		    die("");
		}

		$xml_parser = xml_parser_create();

		xml_set_element_handler($xml_parser, array($this, 'startElement'), array($this, 'endElement'));
		xml_set_character_data_handler($xml_parser, array($this, 'characterData'));

		$fp = fopen($xmlfile,'r');

		while ($data = fread($fp, 4096)){
		   xml_parse($xml_parser, $data, feof($fp));
		}

		fclose($fp);

		xml_parser_free($xml_parser);
	}

	private function add_field($field){
		$this->fields[] = $field;
	}

	private function parse_position(&$x, &$y, $src, $width, $height){
		$parts = array(
			&$x,
			&$y,
		);

		foreach ( $src as $index => $pos ){
			$suffix = substr($pos, -1);

			if ( $suffix == '%' ){
				$parts[$index] = ((int)(substr($pos, 0, -1))) * 0.01 * ( $index % 2 == 0 ? $width : $height );
			} else {
				$parts[$index] = (int)($pos);
			}

			if ( $parts[$index] < 0 ){
				$parts[$index] = ( $index % 2 == 0 ? $width : $height ) + $parts[$index];
			}
		}
	}

	private function startElement($parser, $tagname, $attrs) {
		$this->node = new Node($tagname, $attrs, $this->node);

		$name = isset($attrs['NAME']) ? $attrs['NAME'] : NULL;
		$align = isset($attrs['ALIGN']) ? $attrs['ALIGN'] : 'center';
		$position = isset($attrs['POSITION']) ? $attrs['POSITION'] : '50% 50%';
		$value = isset($attrs['VALUE']) ? $attrs['VALUE'] : NULL;
		$size = isset($attrs['SIZE']) ? $attrs['SIZE'] : 12;
		$font = isset($attrs['FONT']) ? $attrs['FONT'] : NULL;

		switch ( $tagname ){
			case 'TEXT':
				$this->add_field(new TextField($name, $align, $font, $position));
				break;
			case 'TEXTAREA':
				$this->add_field(new TextAreaField($name, $align, $font, $position));
				break;
			case 'STATIC':
				$this->add_field(new StaticField($value, $size, $font, $position));
				break;
		}
	}

	private function endElement($parser, $tagname) {
		$this->node = $this->node->prev;
	}

	private function characterData($parser, $data) {
		if ( empty($data) || strlen(trim($data)) == 0 ){
			return;
		}

		switch ( $this->node->tag ){
			case 'BACKGROUND':
				$this->background = $data;
				break;
			case 'FONT':
				$this->fonts[$this->node->attrs['NAME']] = $data;
				if ( isset($this->node->attrs['DEFAULT']) && $this->node->attrs['DEFAULT'] == 'true' ){
					$this->default_font = $data;
				}
				break;
			default:
				die("Unexpected data in tag {$this->node->tag}: $data");
		}

		$this->node->default = false;
	}

	public function render($dst, $width, $height, $data){
		$im  = imagecreatetruecolor(800, 600);
		$black  = imagecolorallocate($im, 0, 0, 0);
		$white  = imagecolorallocate($im, 255, 255, 255);
		imagefilledrectangle($im, 0, 0, 800, 600, $black);

		$x = $y = 0;

		foreach ( $this->fields as $field ){
			$this->parse_position($x, $y, $field->position(), $width, $height);

			$font = $this->default_font;
			if ( $field->font() ){
				$font = $this->fonts[$field->font()];
			}

			switch ( $field->type() ){
				case Field::type_text:
					imagefttext( $im, 24, 0, $x, $y, $white, $font, $data[$field->name()] );
					break;
				case Field::type_textarea:
					imagefttext( $im, 24, 0, $x, $y, $white, $font, $data[$field->name()] );
					break;
				case Field::type_static:
					imagefttext( $im, 12, 0, $x, $y, $white, $font, $field->value() );
					break;
			}
		}

		imagepng($im, $dst);
	}
}

class DefaultSlideTemplate extends SlideTemplate implements ISlideTemplate {
	public function __construct(){
		parent::__construct('slide_default_template.xml');
	}
}

header("Content-Type: image/png");

$template = new DefaultSlideTemplate;
$template->render(NULL, 800, 600, array('title' => 'bajs', 'content' => "foo bar baz\nLorem ipsum"));

?>
