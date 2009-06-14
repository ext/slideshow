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

require('../core/xml_settings.php');
require('../models/xquery.php');

class configuration extends Module {
	public function index(){
		global $settings;

		$xmlsettings = new XMLSettings('../settings.xml');
		$xmlsettings->merge_settings('../settings.json');

		$xquery = new xquery( $settings->xquery_executable() );
		$displays = $xquery->get_displays();

		// This isn't really good as if the users switches display the list
		// should update available resolutions. Perhaps we should send an array
		// with display as key and resolution as value.
		// See #89'
		$resolution = $xquery->resolutions_for_display($settings->display());

		return array(
			'settings' => $xmlsettings->as_array(),
			'environment' => $settings->environment(),
			'displays' => $displays,
			'resolution' => $resolution
		);
	}

	public function save(){
		$xmlsettings = new XMLSettings('../settings.xml');
		$xmlsettings->merge_flat_array($_POST);
		$new_settings = new Settings($xmlsettings->as_settings_array());

		$environment = explode("\n", $_POST['Env']);
		foreach($environment as $line){
			$line = trim($line);
			if ( empty($line) ){
				continue;
			}
			$new_settings->add_environment($line);
		}

		$password = $new_settings->database_password();
		if ( empty($password) ){
			global $settings;
			$new_settings->set_database_password($settings->database_password());
		}

		$new_settings->persist('../settings.json');
		$this->redirect('/index.php');
	}
}

?>