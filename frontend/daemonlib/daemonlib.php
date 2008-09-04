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

class SlideshowInst {
	private $pid;

	const StatusStopped = 0;
	const StatusRunning = 1;
	const StatusCrashed = 2;

	public function __construct($pidfile){
		$this->pid = 0;

		if ( file_exists($pidfile) ){
			///@todo Not portable
			$this->pid = (int)`cat $pidfile`;
		}
	}

	function get_status(){
		// If $pid is zero the process isn't available.
		if ( $this->pid == 0 ){
			return SlideshowInst::StatusStopped;
		}

		// Check if the process accepts signals
		///@todo Not portable
		if ( posix_kill($this->pid, 0) ){
			return SlideshowInst::StatusRunning;
		}

		// The process does not exist or is in a unresponsive state,
		// either way, it is malfunctioning.
		return SlideshowInst::StatusCrashed;
	}
}