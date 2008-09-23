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

function pretty_json($json){
	$indent = 0;
	$no_escape = false;
	$in_string = false;

	//$json = trim($json);
	$len = strlen($json);

	$pretty = "";

	for ( $i = 0; $i < $len; $i++ ){

		// Remove any escaped forward slashes since JSON handles them anyway.
		// It is not correct to escape them anyway and makes reading it harder.
		if ( $in_string && $json[$i] == '\\' && $json[$i+1] == '/' ){
			continue;
		}

		$pretty .=  $json[$i];

		if ( $i > 0 && $json[$i] == '"' && $json[$i-1] != '\\' ){
			$in_string = !$in_string;
			continue;
		}

		if ( $in_string ){
			continue;
		}

		switch ( $json[$i] ){
		case '"':
		case '[':
		case ']':
			$no_escape = !$no_escape;
		}

		if ( $no_escape ){
			continue;
		}

		switch ( $json[$i] ){
		case '{':
			$indent++;
			$pretty .= "\n" . str_repeat( "\t", $indent );
			break;

		case '}':
			$indent--;

			$pretty[strlen($pretty)-1] = "\n";
			$pretty .= str_repeat( "\t", $indent ) . '}';
			break;

		case ',':
			if ( $indent == 1 ){
				$pretty .= "\n";
			}

			$pretty .= "\n" . str_repeat( "\t", $indent );



			break;

		case ':':
			$pretty .= ' ';
			break;

		}
	}

	$pretty .= "\n";

	return $pretty;
}
?>