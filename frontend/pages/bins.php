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

require_once('../models/bin_collection.php');

class Bins extends Module {
	function __construct(){
		connect();
	}

	function __destruct(){
		disconnect();
	}

	function index(){
		return array(
			'collection' => new BinCollection,
		);
	}

	function rename( $id ){
		$ret = r('SELECT name FROM bins WHERE id = ' . (int)$id);

		return array(
			'id' => $id,
			'name' => $ret['name']
		);
	}

	function delete( $id ){
		if ( array_key_exists('confirm', $_GET) === true ){
			q("UPDATE files SET bin_id = 0 WHERE bin_id = $id");
			q("DELETE FROM bins WHERE id = $id");
			$this->send_signal("Reload");
			Module::log("Removed bin with id $id");
			Module::redirect('/index.php/bins');
		}

		$ret = r('SELECT name FROM bins WHERE id = ' . (int)$id);

		return array(
			'id' => $id,
			'name' => $ret['name']
		);
	}

	function perform_rename(){
		q('UPDATE bins SET name = \'' . mysql_real_escape_string($_POST['name']) . '\' WHERE id = ' . (int)$_POST['id']);
		Module::redirect('/index.php/bins');
	}

	function create(){
		q('INSERT INTO bins (name) VALUES (\'' . mysql_real_escape_string($_POST['name']) . '\')');
		Module::redirect('/index.php/bins');
	}
}
