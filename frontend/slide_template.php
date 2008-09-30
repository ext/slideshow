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

	protected function parse_alignment($align){
		switch ( $align ){
			case 'left': return Field::align_left;
			case 'right':return Field::align_right;
			case 'center': return Field::align_center;
		}
	}

	/**
	 * @param $components The number of components to fill
	 */
	static protected function extract_units($dst, $src, $components, $width, $height){
		$parts = explode(' ', $src);
		$n = count($parts);

		if ( $n < $components ){
			throw new Exception("To few components, got '$src', expected $components components");
		}

		for ( $i = 0; $i < $components; $i++ ){
			$part = $parts[$i % $n];
			$suffix = substr($part, -1);

			if ( $suffix == '%' ){
				$value = (int)(substr($part, 0, -1));
				$dst[$i] = $value * 0.01 * ( $i % 2 == 0 ? $width : $height );
			} else {
				$dst[$i] = (int)($part);
			}

			if ( $dst[$i] < 0 ){
				$dst[$i] = ( $i % 2 == 0 ? $width : $height ) + $dst[$i];
			}
		}
	}
}

class TextField extends Field {
	private $name;
	private $align;

	public function __construct($name, $align, $font, $position){
		parent::__construct(Field::type_text, $font, $position);
		$this->name = $name;
		$this->align = $this->parse_alignment($align);
	}

	public function name(){ return $this->name; }
	public function alignment(){ return $this->align; }
}

class TextAreaField extends Field {
	private $name;
	private $align;
	private $width;
	private $height;

	public function __construct($name, $align, $boxsize, $font, $position, $width, $height){
		parent::__construct(Field::type_textarea, $font, $position);
		$this->name = $name;
		$this->align = $this->parse_alignment($align);
		$this->set_boxsize($boxsize, $width, $height);
	}

	public function name(){ return $this->name; }
	public function alignment(){ return $this->align; }
	public function width(){ return $this->width; }
	public function height(){ return $this->height; }

	private function set_boxsize($boxsize, $width, $height){
		Field::extract_units(array(&$this->width, &$this->height), $boxsize, 2, $width, $height);
	}
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

class RenderContext {
	public $width;
	public $height;
	public $font;
	public $color;
	public $size;

	public function __construct($width, $height, $font, $color, $size){
		$this->width = $width;
		$this->height = $height;
		$this->font = $font;
		$this->color = $color;
		$this->size = $size;
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
	private $width;
	private $height;

	public function __construct($xmlfile, $width, $height){
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

		$this->width = $width;
		$this->height = $height;

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
		$boxsize = isset($attrs['BOXSIZE']) ? $attrs['BOXSIZE'] : NULL;
		$font = isset($attrs['FONT']) ? $attrs['FONT'] : NULL;

		switch ( $tagname ){
			case 'TEXT':
				$this->add_field(new TextField($name, $align, $font, $position));
				break;
			case 'TEXTAREA':
				$this->add_field(new TextAreaField($name, $align, $boxsize, $font, $position, $this->width, $this->height));
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

	public function render($dst, $data){
		$im  = imagecreatetruecolor($this->width, $this->height);
		$black  = imagecolorallocate($im, 0, 0, 0);
		$white  = imagecolorallocate($im, 255, 255, 255);
		imagefilledrectangle($im, 0, 0, $this->width, $this->height, $black);

		$x = $y = 0;

		foreach ( $this->fields as $field ){
			$this->parse_position($x, $y, $field->position(), $this->width, $this->height);

			$font = $this->default_font;
			if ( $field->font() ){
				$font = $this->fonts[$field->font()];
			}

			$ctx = new RenderContext($this->width, $this->height, $font, $white, 12);

		// 1 is alignment (center)
		// 100 is y-coordinate
		/*$this->render_string_aligned($im, 1, 100, 12, $font, $white, $title);

		$y = 180;
		foreach ( $content as $paragraph ){
			$y = $this->render_string_aligned($im, $field->align(), $y, 12, $font, $white, $paragraph);
		}*/

			switch ( $field->type() ){
				case Field::type_text:
					$this->render_text($im, $field->alignment(), $x, $y, $ctx, $data[$field->name()]);
					break;
				case Field::type_textarea:
					//imagefttext( $im, 24, 0, $x, $y, $white, $font, $data[$field->name()] );
					//$this->render_string_aligned($im, 1, $x, $y, $ctx, $data[$field->name()]);
					$this->render_textarea($im, $field->alignment(), $x, $y, $field->width(), $field->height(), $ctx, $data[$field->name()]);
					break;
				case Field::type_static:
					imagefttext( $im, 12, 0, $x, $y, $white, $font, $field->value() );
					break;
			}
		}

		if ( !$dst ){
			header("Content-Type: image/png");
		}
		imagepng($im, $dst);
	}

	private function render_text($im, $alignment, $x, $y, $ctx, $string){
		$this->render_aligned_string($im, $alignment, $x, $y, $ctx, $string);
	}

	private function render_textarea($im, $alignment, $x, $y, $width, $height, $ctx, $string){
		$x1 = $x - $width / 2;
		$y1 = $y - $height / 2;
		$x2 = $x + $width / 2;
		$y2 = $y + $height / 2;

		imagerectangle($im, $x1, $y1, $x2, $y2, $ctx->color);

		$row = $y1 + (int)($ctx->size * 1.5);

		$lines = explode("\n", $string);
		foreach ($lines as $line){
			if ( strlen(trim($line)) == 0 ){
				continue;
			}

			$word_stack = explode(' ', $line);
			$words = array_shift($word_stack);
			$current_width = $this->get_string_width($ctx->size, $ctx->font, $words);

			while ( count($word_stack) > 0 ){
				$next_word = array_shift($word_stack);
				$next_width = $this->get_string_width($ctx->size, $ctx->font, "$words $next_word");

				if ( $next_width > $width ){
					imagefttext( $im, $ctx->size, 0, $x - $current_width / 2, $row, $ctx->color, $ctx->font, $words );
					$words = $next_word;
					$current_width = $this->get_string_width($ctx->size, $ctx->font, $words);
					$row += (int)($ctx->size * 1.5);
				}

				$words .= ' ' . $next_word;
				$current_width = $next_width;
			}
			imagefttext( $im, $ctx->size, 0, $x - $current_width / 2, $row, $ctx->color, $ctx->font, $words );
			$row += (int)($ctx->size * 2.5);
		}
	}

	private function render_aligned_string($im, $alignment, $x, $y, $ctx, $string){
		$string_width = $this->get_string_width($ctx->size, $ctx->font, $string);

		switch ( $alignment ){
			case Field::align_right:
				imagefttext( $im, $ctx->size, 0, $x - $string_width, $y, $ctx->color, $ctx->font, $string );
				break;
			case Field::align_left:
				imagefttext( $im, $ctx->size, 0, $x, $y, $ctx->color, $ctx->font, $string );
				break;
			case Field::align_center:
				imagefttext( $im, $ctx->size, 0, $x - $string_width / 2, $y, $ctx->color, $ctx->font, $string );
				break;
		}
	}

	private function get_string_width($size, $font, $string){
		$data = imagettfbbox( $size, 0, $font, $string );
		return $data[2] - $data[0];
	}
}

/*class DefaultSlideTemplate extends SlideTemplate implements ISlideTemplate {
	public function __construct(){
		parent::__construct('slide_default_template.xml');
	}
}*/

$template = new SlideTemplate('slide_default_template.xml', 800, 600);
$template->render(NULL, array(
	'title' => 'This is a left aligned title string',
	'title2' => 'This is a right aligned title string',
	'title3' => 'This is a centered title string',
	'content' =>
			"Lorem ipsum dolor sit amet, consectetuer adipiscing elit. Quisque eu nisl. Donec vitae risus vel pede viverra bibendum. Pellentesque sed ante. In nibh arcu, fermentum sed, dictum ac, convallis vitae, justo. Nulla pharetra, metus sit amet adipiscing placerat, est neque ornare massa, id pulvinar purus enim vitae justo. Etiam consequat, velit eu volutpat sollicitudin, lacus ligula faucibus lorem, nec faucibus sapien ante at arcu. In nec mauris vitae quam auctor hendrerit. Aenean dapibus felis sed turpis. Nullam purus. Duis vel mauris. Proin elementum vestibulum velit.\n" .
			"\n" .
			"Donec porta. In faucibus egestas eros. Proin quam nunc, lacinia sed, scelerisque nec, accumsan porta, est. Nullam et ligula. Morbi semper congue dolor. Pellentesque accumsan, urna sit amet mollis porttitor, pede ante pellentesque quam, at dictum ante justo sed est. Nullam et diam. Nullam a libero. Nulla facilisi. Fusce erat justo, adipiscing imperdiet, suscipit vitae, fringilla quis, libero. Nullam auctor tortor at urna. Suspendisse porta, diam quis mollis pellentesque, metus nibh volutpat risus, at mattis diam felis non lectus. Maecenas a erat. Integer diam nibh, lacinia sit amet, adipiscing non, elementum in, elit. Praesent semper nisl ut enim. Cras condimentum, metus a pretium sollicitudin, eros diam lobortis felis, non accumsan urna neque id sapien. Nullam volutpat. Duis aliquam. Nulla facilisi. Etiam at arcu."
	)
);

?>
