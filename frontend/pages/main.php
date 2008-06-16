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

require_once('../core/module.inc.php');
require_once('../models/bin_collection.php');

class Main extends Module {
	function __construct(){
		connect();
	}

	function __destruct(){
		disconnect();
	}

	function index(){
		global $settings;

		Module::set_template('main.tmpl');

		$available_bins = array( 0 => 'Unsorted' );
		$ret = q('SELECT id, name FROM bins');
		while ( ( $row = mysql_fetch_assoc($ret) ) ){
			$available_bins[ $row['id'] ] = $row['name'];
		}

		$ret = array(
			'collection' => new BinCollection(),
			'active_bin' => $settings->current_bin(),
			'available_bins' => $available_bins
		);

		$motd = $settings->motd();
		if ( strlen(trim($motd)) > 0 ){
			$ret['motd'] = $motd;
		}

		return $ret;
	}
};

?>