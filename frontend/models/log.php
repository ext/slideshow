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

class Log {
	function __construct( $filename ){
		if ( !( file_exists($filename) && is_readable($filename) ) ){
			throw new Exception("Could not open log `$filename'");
		}

		$this->_filename = $filename;
		$this->_filters = array();
	}

	function read_lines( $nr_of_lines ){

		///@todo Not portable
		$lines = explode("\n", `tail -n $nr_of_lines $this->_filename`);

		$log = array();
		foreach ( $lines as $line ){
			$severity = 'normal';

			foreach ( $this->_filters as $filter ){
				if ( preg_match( $this->_filters['pattern'], $line ) > 0 ){
					$severity = $this->_filters['severity'];
				}
			}

			$log[] = array(
				'severity' => $severity,
				'content' => str_replace(" ", "&nbsp;", $line)
			);
		}

		return $log;
	}
};

?>
