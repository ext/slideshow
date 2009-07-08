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

class Slide {
	private $id;
	private $name;
	private $fullpath;
	private $active;
	private $type;
	private $title;

	function __construct( $row ){
		$this->id = $row['id'];
		$this->name = basename($row['fullpath']);
		$this->fullpath = $row['fullpath'];
		$this->active = $row['active'];
		$this->type = $row['type'];
		$this->title = $row['title'];
	}

	function id(){
		return $this->id;
	}

	function name(){
		return $this->name;
	}

	function image_url(){
		return 'image/' . basename($this->fullpath()) . '/samples/1280x1024.png';
	}

	function thumbnail_url(){
		return 'image/' . basename($this->fullpath()) . '/samples/200x200.png';
	}

	function fullpath(){
		return $this->fullpath;
	}

	function active(){
		return $this->active != 0;
	}

	function type(){
		return $this->type;
	}

	function title(){
		return $this->title;
	}
}

class Bin implements Iterator {
	private $slides;
	private $id;
	private $name;
	private $current = 0;

	function __construct( array $slides, $id, $name ){
		$this->slides = $slides;
		$this->id = $id;
		$this->name = $name;
	}

	function nr_of_slides(){
		return count( $this->slides );
	}

	function id(){
		return $this->id;
	}

	function name(){
		return $this->name;
	}

	public function rewind(){
		$this->current = 0;
	}

	public function next(){
		return $this->slides[ $this->current++ ];
	}

	public function current(){
		return $this->valid() ? $this->slides[ $this->current ] : false;
	}

	public function valid(){
		return isset( $this->slides[ $this->current ] );
	}

	public function key(){
		return $this->current;
	}
}

class BinCollection implements Iterator, ArrayAccess {
	private $collection;
	private $current = 0;

	function __construct( ){
		$result = q('
			SELECT slides.id, fullpath, bins.id as bid, bin_id, active, name, sortorder, type, title
				FROM slides
				LEFT JOIN bins
					ON slides.bin_id = bins.id
			UNION
			SELECT slides.id, fullpath, bins.id as bid, bin_id, active, name, sortorder, type, title
				FROM slides
				RIGHT JOIN bins
					ON slides.bin_id = bins.id
			ORDER BY bid, bin_id, sortorder, id'
		);

		$this->collection = array();

		$bin = array();
		$prevbin_id = 0;
		$bin_name = 'Unsorted';
		while ( ( $row = mysql_fetch_assoc($result) ) ){
			$binid = $row['bin_id'] != NULL ? $row['bin_id'] : $row['bid'];

			if ( $binid != $prevbin_id ){
				$this->collection[] = new Bin( $bin, $prevbin_id, $bin_name );
				$bin = array();
				$prevbin_id = $binid;
				$bin_name = $row['name'];
			}

			if ( $row['id'] != NULL ){
				$bin[] = new Slide( $row );
			}
		}

		$this->collection[] = new Bin( $bin, $prevbin_id, $bin_name );
	}

	function nr_of_bins(){
		return count( $this->collection );
	}

	public function rewind(){
		$this->current = 0;
	}

	public function next(){
		return $this->collection[ $this->current++ ];
	}

	public function current(){
		return $this->valid() ? $this->collection[ $this->current ] : false;
	}

	public function valid(){
		return isset( $this->collection[ $this->current ] );
	}

	public function key(){
		return $this->current;
	}

	public function offsetExists($offset) {
		return isset( $this->collection[$offset] );
	}

	public function offsetGet($offset) {
		return $this->collection[$offset];
	}

	public function offsetSet($offset, $value) {
		return false;
	}

	public function offsetUnset($offset) {
		return false;
	}
};

?>
