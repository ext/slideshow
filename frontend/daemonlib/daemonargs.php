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

class DaemonArguments {
	private $password = false;
	private $logfile = false;
	private $basepath = false;
	private $arguments = "--daemon --fullscreen --stdin";

	public function set_database_settings($host, $name, $username, $password){
		$this->arguments .= " --db_host $host --db_name $name --db_user $username";
		$this->db_password = $password;
	}

	public function set_basepath($path){ $this->basepath = $path; }
	public function set_logfile($filename){ $this->logfile = $filename; }
	public function set_resolution($resolution){ $this->arguments .= " --resolution $resolution"; }
	public function set_bin_id($id){ $this->arguments .= " --collection-id $id"; }

	public function as_string(){ return $this->arguments; }
	public function basepath(){ return $this->basepath; }
	public function password(){ return $this->db_password; }
	public function logfile(){ return $this->logfile; }
}
