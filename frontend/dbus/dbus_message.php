<?php
//j// BOF

/*n// NOTE
----------------------------------------------------------------------------
D-BUS PHP Binding
----------------------------------------------------------------------------
(C) direct Netware Group - All rights reserved
http://www.direct-netware.de/redirect.php?dbus

This work is distributed under the W3C (R) Software License, but without any
warranty; without even the implied warranty of merchantability or fitness
for a particular purpose.
----------------------------------------------------------------------------
http://www.direct-netware.de/redirect.php?licenses;w3c
----------------------------------------------------------------------------
$Id: dbus_message.php,v 1.7 2008/02/23 16:50:13 s4u Exp $
v0.1.00
sWG/system/classes/ext_dbus/dbus_message.php
----------------------------------------------------------------------------
NOTE_END //n*/

//j// Functions and classes

if (!defined ("CLASS_direct_dbus_message"))
{
//c// direct_dbus_message
class direct_dbus_message
{
	protected $dvar_dbus_header;
	protected $dvar_dbus_raw;
	public $dvar_debug;
	protected $dvar_debugging;
	protected $dvar_nle;

	//f// direct_dbus_message->__construct () and direct_dbus_message->direct_dbus_message ()
	public function __construct ($f_messages,$f_debug = false)
	{
		$this->dvar_debugging = $f_debug;
		if ($this->dvar_debugging) { $this->dvar_debug = array ("socketdbus/system/classes/ext_dbus/dbus_message.php -dbusmessage->__construct (direct_dbus_message)- (108)"); }
		$this->dvar_dbus_header = NULL;
		$this->dvar_dbus_raw = NULL;
		$this->dvar_nle = $f_messages->dclass_get_nle ();
	}

/*PHP4	function direct_dbus_message ($f_messages,$f_debug = false) { $this->__construct ($f_messages,$f_debug); }
*/
	//f// direct_dbus_message->dclass_get_complete_type (&$f_signature,$f_position,$f_type_count = 1)
	/*PHPe*/ protected function dclass_get_complete_type (&$f_signature,$f_offset,$f_type_count = 1)
	{
		if ($this->dvar_debugging) { $this->dvar_debug[] = "socketdbus/system/classes/ext_dbus/dbus_message.php -dbusmessage->dclass_get_complete_type ($f_signature,$f_offset,$f_type_count)- (137)"; }
		$f_return = "";

		if (((is_string ($f_signature)))&&(strlen ($f_signature) > $f_offset)&&($f_type_count))
		{
			$f_arrays_count = 0;
			$f_dicts_count = 0;
			$f_types_single = array ("b","d","g","i","n","o","q","s","t","u","x","y");

			while (($f_type_count)&&(isset ($f_signature[$f_offset])))
			{
				if (in_array ($f_signature[$f_offset],$f_types_single))
				{
					$f_return .= $f_signature[$f_offset];
					if ((!$f_arrays_count)&&(!$f_dicts_count)) { $f_type_count--; }
				}
				elseif (($f_signature[$f_offset] == "a")||($f_signature[$f_offset] == "v")) { $f_return .= $f_signature[$f_offset]; }
				else
				{
					switch ($f_signature[$f_offset])
					{
					case "(":
					{
						$f_return .= $f_signature[$f_offset];
						$f_arrays_count++;

						break 1;
					}
					case ")":
					{
						$f_return .= $f_signature[$f_offset];
						$f_arrays_count--;

						if ((!$f_arrays_count)&&(!$f_dicts_count)) { $f_type_count--; }
						break 1;
					}
					case "{":
					{
						$f_return .= $f_signature[$f_offset];
						$f_dicts_count++;

						break 1;
					}
					case "}":
					{
						$f_return .= $f_signature[$f_offset];
						$f_dicts_count--;

						if ((!$f_arrays_count)&&(!$f_dicts_count)) { $f_type_count--; }
						break 1;
					}
					}
				}

				$f_offset++;
			}
		}

		if ($this->dvar_debugging) { $this->dvar_debug[] = "socketdbus/system/classes/ext_dbus/dbus_message.php -dbusmessage->dclass_get_complete_type ()- (195) found ".$f_return; }
		return $f_return;
	}

	//f// direct_dbus_message->dclass_get_header ($f_field = "")
	public function dclass_get_header ($f_field = "")
	{
		if ($this->dvar_debugging) { $this->dvar_debug[] = "socketdbus/system/classes/ext_dbus/dbus_message.php -dbusmessage->dclass_get_header ()- (211)"; }
		$f_return = false;

		if ($this->dvar_dbus_header != NULL)
		{
			if ($f_field)
			{
				if (is_numeric ($f_field))
				{
					foreach ($this->dvar_dbus_header[6] as $f_header_field)
					{
						if ((is_bool ($f_return))&&($f_header_field[0] === $f_field)) { $f_return = $f_header_field[1]; }
					}
				}
				else
				{
					switch ($f_field)
					{
					case "endian":
					{
						if ((isset ($this->dvar_dbus_header[0]))&&(($this->dvar_dbus_header[0] == 108)||($this->dvar_dbus_header[0] == 66))) { $f_return = chr ($this->dvar_dbus_header[0]); }
						break 1;
					}
					case "type":
					{
						if (isset ($this->dvar_dbus_header[1]))
						{
							switch ($this->dvar_dbus_header[1])
							{
							case 1:
							{
								$f_return = "method_call";
								break 1;
							}
							case 2:
							{
								$f_return = "method_return";
								break 1;
							}
							case 3:
							{
								$f_return = "error";
								break 1;
							}
							case 4:
							{
								$f_return = "signal";
								break 1;
							}
							default: { $f_return = "unknown"; }
							}
						}

						break 1;
					}
					case "flags":
					{
						if (isset ($this->dvar_dbus_header[2])) { $f_return = $this->dvar_dbus_header[2]; }
						break 1;
					}
					case "protocol":
					{
						if (isset ($this->dvar_dbus_header[3])) { $f_return = $this->dvar_dbus_header[3]; }
						break 1;
					}
					case "body_size":
					{
						if (isset ($this->dvar_dbus_header[4])) { $f_return = $this->dvar_dbus_header[4]; }
						break 1;
					}
					case "serial":
					{
						if (isset ($this->dvar_dbus_header[5])) { $f_return = $this->dvar_dbus_header[5]; }
						break 1;
					}
					}
				}
			}
			else { $f_return = $this->dvar_dbus_header; }
		}

		return $f_return;
	}

	//f// direct_dbus_message->dclass_get_raw ()
	public function dclass_get_raw ()
	{
		if ($this->dvar_debugging) { $this->dvar_debug[] = "socketdbus/system/classes/ext_dbus/dbus_message.php -dbusmessage->dclass_get_raw ()- (304)"; }

		if ($this->dvar_dbus_raw != NULL) { return $this->dvar_dbus_raw; }
		else { return false; }
	}

	//f// direct_dbus_message->dclass_get_raw_body ()
	public function dclass_get_raw_body ()
	{
		if ($this->dvar_debugging) { $this->dvar_debug[] = "socketdbus/system/classes/ext_dbus/dbus_message.php -dbusmessage->dclass_get_raw_body ()- (320)"; }
		$f_return = false;

		if (($this->dvar_dbus_header != NULL)&&($this->dvar_dbus_raw != NULL))
		{
			$f_body_start = 0;

			if (isset ($this->dvar_dbus_header[4]))
			{
				$f_body_start = (strlen ($this->dvar_dbus_raw)) - $this->dvar_dbus_header[4];

				if ($f_body_start >= 16)
				{
					if ($this->dvar_dbus_header[4]) { $f_return = substr ($this->dvar_dbus_raw,$f_body_start); }
					else { $f_return = ""; }
				}
			}
		}

		return $f_return;
	}

	//f// direct_dbus_message->dclass_marshal_array ()
	public function dclass_marshal_array ($f_signature,&$f_data,$f_position = 0)
	{
		if ($this->dvar_debugging) { $this->dvar_debug[] = "socketdbus/system/classes/ext_dbus/dbus_message.php -dbusmessage->dclass_marshal_array ($f_signature,+f_data,$f_position)- (361)"; }
		$f_return = false;

		if ((is_string ($f_signature))&&(is_array ($f_data)))
		{
			$f_data_next = true;
			$f_data_position = 0;
			$f_return = "";
			$f_signature_length = strlen ($f_signature);
			$f_signature_position = 0;

			while (($f_signature_position < $f_signature_length)&&(is_string ($f_return)))
			{
				switch ($f_signature[$f_signature_position])
				{
				case "(":
				{
					$f_position += $this->dclass_marshal_set_boundary ($f_return,$f_position,8);
					$f_sub_signature = $this->dclass_get_complete_type ($f_signature,$f_signature_position);

					if (($f_sub_signature)&&(is_array ($f_data[$f_data_position])))
					{
						$f_sub_signature = substr ($f_sub_signature,1,-1);
						$f_sub_raw = $this->dclass_marshal_array ($f_sub_signature,$f_data[$f_data_position],$f_position);

						if (is_bool ($f_sub_raw)) { $f_return = false; }
						else
						{
							$f_position += strlen ($f_sub_raw);
							$f_signature_position += 1 + strlen ($f_sub_signature);
							$f_return .= $f_sub_raw;
						}
					}
					else { $f_return = false; }

					break 1;
				}
				case ")":
				{
					$f_data_next = false;
					break 1;
				}
				case "{":
				{
					$f_position += $this->dclass_marshal_set_boundary ($f_return,$f_position,8);
					$f_sub_signature = $this->dclass_get_complete_type ($f_signature,$f_signature_position);

					if (($f_sub_signature)&&(is_array ($f_data[$f_data_position])))
					{
						$f_array_element_raw = reset ($f_data[$f_data_position]);
						$f_sub_signature = substr ($f_sub_signature,1,-1);

						if (is_array ($f_array_element_raw)) { $f_array_element_raw = array_merge (array (key ($f_data[$f_data_position])),(array_values ($f_data[$f_data_position]))); }
						else { $f_array_element_raw = array (key ($f_data[$f_data_position]),$f_array_element_raw); }

						$f_sub_raw = $this->dclass_marshal_array ($f_sub_signature,$f_array_element_raw,$f_position);
					}
					else { $f_sub_raw = false; }

					if (is_bool ($f_sub_raw)) { $f_return = false; }
					else
					{
						$f_position += strlen ($f_sub_raw);
						$f_signature_position += 1 + strlen ($f_sub_signature);
						$f_return .= $f_sub_raw;
					}

					break 1;
				}
				case "}":
				{
					$f_data_next = false;
					break 1;
				}
				case "a":
				{
					$f_position += 4 + $this->dclass_marshal_set_boundary ($f_return,$f_position,4);
					$f_sub_signature = $this->dclass_get_complete_type ($f_signature,($f_signature_position + 1));

					if (($f_sub_signature)&&(is_array ($f_data[$f_data_position])))
					{
						reset ($f_data[$f_data_position]);

						$f_array_count = count ($f_data[$f_data_position]);
						$f_array_offset = $f_position;
						$f_array_position = 0;
						$f_sub_raw = "";

						while (($f_return)&&($f_array_position < $f_array_count))
						{
							$f_array_element_raw = array ($f_data[$f_data_position][$f_array_position]);
							$f_array_element_raw = $this->dclass_marshal_array ($f_sub_signature,$f_array_element_raw,$f_position);

							if ((is_string ($f_return))&&(!is_bool ($f_array_element_raw)))
							{
								$f_position += strlen ($f_array_element_raw);
								$f_sub_raw .= $f_array_element_raw;
							}
							else { $f_return = false; }

							$f_array_position++;
						}

						if (is_string ($f_return))
						{
							$f_signature_position += strlen ($f_sub_signature);
							$f_size = strlen ($f_sub_raw);

							$f_size -= ($this->dclass_type_get_position_padding ($f_sub_signature[0],$f_array_offset) - $f_array_offset);
							$f_return .= (pack ("V",$f_size)).$f_sub_raw;
						}
					}
					else { $f_return = false; }

					break 1;
				}
				case "b":
				{
					$f_position += $this->dclass_marshal_set_boundary ($f_return,$f_position,4);

					if (!isset ($f_data[$f_data_position])) { $f_return = false; }
					elseif ($f_data[$f_data_position]) { $f_return .= "\x01\x00\x00\x00"; }
					else { $f_return .= "\x00\x00\x00\x00"; }

					$f_position += 4;
					break 1;
				}
				case "d":
				{
					$f_position += 8 + $this->dclass_marshal_set_boundary ($f_return,$f_position,8);

					if (strlen ("{$f_data[$f_data_position]}") < 9) { $f_return .= pack ("a8","{$f_data[$f_data_position]}"); }
					else { $f_return = false; }

					break 1;
				}
				case "g":
				{
					$f_size = strlen ($f_data[$f_data_position]);
				
					if ((isset ($f_data[$f_data_position]))&&(strlen ($f_data[$f_data_position]) < 256)) { $f_return .= pack ("Ca*x",$f_size,$f_data[$f_data_position]); }
					else { $f_return = false; }

					$f_position += 2 + $f_size;
					break 1;
				}
				case "i":
				{
					$f_position += $this->dclass_marshal_set_boundary ($f_return,$f_position,4);

					if (isset ($f_data[$f_data_position])) { $f_return .= $this->dclass_marshal_set_nle (pack ("L",$f_data[$f_data_position])); }
					else { $f_return = false; }

					$f_position += 4;
					break 1;
				}
				case "n":
				{
					$f_position += $this->dclass_marshal_set_boundary ($f_return,$f_position,2);

					if (isset ($f_data[$f_data_position])) { $f_return .= $this->dclass_marshal_set_nle (pack ("s",$f_data[$f_data_position])); }
					else { $f_return = false; }

					$f_position += 2;
					break 1;
				}
				case "o":
				{
					$f_position += $this->dclass_marshal_set_boundary ($f_return,$f_position,4);

					if (isset ($f_data[$f_data_position]))
					{
						$f_size = strlen ($f_data[$f_data_position]);
						$f_return .= (pack ("V",$f_size)).$f_data[$f_data_position]."\x00";
					}
					else { $f_return = false; }

					$f_position += 5 + $f_size;
					break 1;
				}
				case "q":
				{
					$f_position += $this->dclass_marshal_set_boundary ($f_return,$f_position,2);

					if (isset ($f_data[$f_data_position])) { $f_return .= pack ("v",$f_data[$f_data_position]); }
					else { $f_return = false; }

					$f_position += 2;
					break 1;
				}
				case "s":
				{
					$f_position += $this->dclass_marshal_set_boundary ($f_return,$f_position,4);

					if (isset ($f_data[$f_data_position]))
					{
						$f_size = strlen ($f_data[$f_data_position]);
						$f_return .= (pack ("V",$f_size)).$f_data[$f_data_position]."\x00";
					}
					else { $f_return = false; }

					$f_position += 5 + $f_size;
					break 1;
				}
				case "t":
				{
					$f_position += 8 + $this->dclass_marshal_set_boundary ($f_return,$f_position,8);

					if (strlen ("{$f_data[$f_data_position]}") < 9) { $f_return .= pack ("a8","{$f_data[$f_data_position]}"); }
					else { $f_return = false; }

					break 1;
				}
				case "u":
				{
					$f_position += $this->dclass_marshal_set_boundary ($f_return,$f_position,4);

					if (isset ($f_data[$f_data_position])) { $f_return .= pack ("V",$f_data[$f_data_position]); }
					else { $f_return = false; }

					$f_position += 4;
					break 1;
				}
				case "v":
				{
					if ((is_array ($f_data[$f_data_position]))&&(isset ($f_data[$f_data_position][0])))
					{
						$f_sub_signature = "g".$f_data[$f_data_position][0];
						$f_sub_raw = $this->dclass_marshal_array ($f_sub_signature,$f_data[$f_data_position],$f_position);

						if (is_bool ($f_sub_raw)) { $f_return = false; }
						else
						{
							$f_return .= $f_sub_raw;
							$f_position += strlen ($f_sub_raw);
						}
					}
					else { $f_return = false; }

					$f_data_next = false;
					break 1;
				}
				case "x":
				{
					$f_position += 8 + $this->dclass_marshal_set_boundary ($f_return,$f_position,8);

					if (strlen ("{$f_data[$f_data_position]}") < 9) { $f_return .= pack ("a8","{$f_data[$f_data_position]}"); }
					else { $f_return = false; }

					break 1;
				}
				case "y":
				{

					if ((is_string ($f_data[$f_data_position]))&&(strlen ($f_data[$f_data_position]) == 1)) { $f_return .= $f_data[$f_data_position]; }
					elseif ((is_numeric ($f_data[$f_data_position]))&&($f_data[$f_data_position] < 256)) { $f_return .= pack ("C",$f_data[$f_data_position]); }
					else { $f_return = false; }

					$f_position++;
					break 1;
				}
				default: { $f_return = false; }
				}

				$f_signature_position++;

				if ($f_data_next) { $f_data_position++; }
				else { $f_data_next = true; }
			}
		}
		else { $f_return = false; }

		return $f_return;
	}

	//f// direct_dbus_message->dclass_marshal_set_boundary (&$f_data,$f_position,$f_boundary_spec)
	public function dclass_marshal_set_boundary (&$f_data,$f_position,$f_boundary_spec)
	{
		if ($this->dvar_debugging) { $this->dvar_debug[] = "socketdbus/system/classes/ext_dbus/dbus_message.php -dbusmessage->dclass_marshal_set_boundary (+f_data,$f_position,$f_boundary_spec)- (653)"; }
		$f_return = 0;

		if (((is_string ($f_data)))&&($f_position > 1)&&($f_boundary_spec > 1))
		{
			$f_position = ($f_boundary_spec - ($f_position % $f_boundary_spec));

			if (($f_position)&&($f_position < $f_boundary_spec))
			{
				if ($this->dvar_debugging) { $this->dvar_debug[] = "socketdbus/system/classes/ext_dbus/dbus_message.php -dbusmessage->dclass_marshal_set_boundary ()- (662) added $f_position NUL bytes to conform to the requested boundary"; }
				for ($f_i = 0;$f_i < $f_position;$f_i++) { $f_data .= "\x00"; }
				$f_return = $f_position;
			}
		}

		return $f_return;
	}

	//f// direct_dbus_message->dclass_marshal_set_nle ($f_data)
	protected function dclass_marshal_set_nle ($f_data)
	{
		if ($this->dvar_debugging) { $this->dvar_debug[] = "socketdbus/system/classes/ext_dbus/dbus_message.php -dbusmessage->dclass_marshal_set_nle (+f_data)- (682)"; }

		if ((!$this->dvar_nle)&&(strlen ($f_data) > 1))
		{
			$f_bytes_inverted = array ();
			$f_position = 0;

			for ($f_i = (strlen ($f_data) - 1);$f_i > -1;$f_i--)
			{
				$f_bytes_inverted[$f_position] = $f_data[$f_i];
				$f_position++;
			}

			return implode ("",$f_bytes_inverted);
		}
		else { return $f_data; }
	}

	//f// direct_dbus_message->dclass_set ($f_header,$f_raw,$f_overwrite = false)
	public function dclass_set ($f_header,$f_raw,$f_overwrite = false)
	{
		if ($this->dvar_debugging) { $this->dvar_debug[] = "socketdbus/system/classes/ext_dbus/dbus_message.php -dbusmessage->dclass_set (+f_header,$f_raw,+f_overwrite)- (712)"; }
		$f_return = false;

		if ((is_array ($f_header))&&(is_string ($f_raw))&&(($f_overwrite)||(($this->dvar_dbus_header == NULL)&&($this->dvar_dbus_raw == NULL))))
		{
			$f_return = true;
			$this->dvar_dbus_header = $f_header;
			$this->dvar_dbus_raw = $f_raw;
		}

		return $f_return;
	}

	//f// direct_dbus_message->dclass_type_get_padding ($f_type)
	public function dclass_type_get_padding ($f_type)
	{
		if ($this->dvar_debugging) { $this->dvar_debug[] = "socketdbus/system/classes/ext_dbus/dbus_message.php -dbusmessage->dclass_type_get_padding ($f_type)- (735)"; }
		$f_return = 0;

		if (is_string ($f_type))
		{
			switch ($f_type)
			{
			case "(":
			{
				$f_return = 8;
				break 1;
			}
			case "{":
			{
				$f_return = 8;
				break 1;
			}
			case "a":
			{
				$f_return = 4;
				break 1;
			}
			case "b":
			{
				$f_return = 4;
				break 1;
			}
			case "d":
			{
				$f_return = 8;
				break 1;
			}
			case "i":
			{
				$f_return = 4;
				break 1;
			}
			case "n":
			{
				$f_return = 2;
				break 1;
			}
			case "o":
			{
				$f_return = 4;
				break 1;
			}
			case "q":
			{
				$f_return = 2;
				break 1;
			}
			case "s":
			{
				$f_return = 4;
				break 1;
			}
			case "t":
			{
				$f_return = 8;
				break 1;
			}
			case "u":
			{
				$f_return = 4;
				break 1;
			}
			case "x":
			{
				$f_return = 8;
				break 1;
			}
			}
		}

		return $f_return;
	}

	//f// direct_dbus_message->dclass_type_get_position_padding ($f_type,$f_position)
	public function dclass_type_get_position_padding ($f_type,$f_position)
	{
		if ($this->dvar_debugging) { $this->dvar_debug[] = "socketdbus/system/classes/ext_dbus/dbus_message.php -dbusmessage->dclass_type_get_position_padding ($f_type,$f_position)- (825)"; }

		$f_boundary_spec = $this->dclass_type_get_padding ($f_type);
		$f_return = $f_position;

		if ($f_boundary_spec > 0)
		{
			$f_position = ($f_boundary_spec - ($f_position % $f_boundary_spec));
			if (($f_position)&&($f_position < $f_boundary_spec)) { $f_return += $f_position; }
		}

		return $f_return;
	}

	//f// direct_dbus_message->dclass_unmarshal ($f_le,$f_signature,&$f_data,$f_position = 0)
	public function dclass_unmarshal ($f_le,$f_signature,&$f_data,$f_position = -1)
	{
		if ($this->dvar_debugging) { $this->dvar_debug[] = "socketdbus/system/classes/ext_dbus/dbus_message.php -dbusmessage->dclass_unmarshal (+f_le,$f_signature,+f_data,$f_position)- (861)"; }
		$f_return = false;

		if ((is_string ($f_signature))&&(is_string ($f_data)))
		{
			$f_position_element = false;
			$f_return = array ();
			$f_return_position = 0;
			$f_signature_length = strlen ($f_signature);
			$f_signature_position = 0;

			if ($f_position < 0) { $f_position = 0; }
			else { $f_position_element = true; }

			while (($f_signature_position < $f_signature_length)&&(is_array ($f_return)))
			{
				switch ($f_signature[$f_signature_position])
				{
				case "(":
				{
					$f_position += $this->dclass_unmarshal_get_boundary ($f_position,8);
					$f_sub_signature = $this->dclass_get_complete_type ($f_signature,$f_signature_position);

					if ($f_sub_signature)
					{
						$f_sub_signature = substr ($f_sub_signature,1,-1);
						$f_bytes_unpacked = $this->dclass_unmarshal ($f_le,$f_sub_signature,$f_data,$f_position);

						if ($f_bytes_unpacked)
						{
							if (isset ($f_bytes_unpacked['position']))
							{
								$f_position = $f_bytes_unpacked['position'];
								unset ($f_bytes_unpacked['position']);
							}

							$f_return[$f_return_position] = $f_bytes_unpacked;
						}
						else { $f_return = false; }
					}
					else { $f_return = false; }

					if ($f_return)
					{
						$f_return_position++;
						$f_signature_position += 1 + strlen ($f_sub_signature);
					}

					break 1;
				}
				case ")": break 1;
				case "{":
				{
					$f_position += $this->dclass_unmarshal_get_boundary ($f_position,8);
					$f_sub_signature = $this->dclass_get_complete_type ($f_signature,$f_signature_position);

					if (($f_sub_signature)&&(strlen ($this->dclass_get_complete_type ($f_sub_signature,1)) == 1)&&(strlen ($f_sub_signature) == (strlen ($this->dclass_get_complete_type ($f_sub_signature,2)) + 3)))
					{
						$f_sub_signature = substr ($f_sub_signature,1,-1);
						$f_bytes_unpacked = $this->dclass_unmarshal ($f_le,$f_sub_signature,$f_data,$f_position);

						if ($f_bytes_unpacked)
						{
							if (isset ($f_bytes_unpacked['position']))
							{
								$f_position = $f_bytes_unpacked['position'];
								unset ($f_bytes_unpacked['position']);
							}

							$f_return[$f_return_position] = array ($f_bytes_unpacked[0] => $f_bytes_unpacked[1]);
						}
						else { $f_return = false; }
					}
					else { $f_return = false; }

					if ($f_return)
					{
						$f_return_position++;
						$f_signature_position += 1 + strlen ($f_sub_signature);
					}

					break 1;
				}
				case "}": break 1;
				case "a":
				{
					$f_position += $this->dclass_unmarshal_get_boundary ($f_position,4);
					$f_sub_read = $this->dclass_unmarshal_read ($f_data,$f_position,4);
 
					if (is_bool ($f_sub_read)) { $f_return = false; }
					else
					{
						if ($f_le == "B") { $f_bytes_unpacked = unpack ("N",$f_sub_read); }
						else { $f_bytes_unpacked = unpack ("V",$f_sub_read); }

						$f_position += 4;
						$f_sub_signature = "";

						if ($f_bytes_unpacked) { $f_sub_signature = $this->dclass_get_complete_type ($f_signature,($f_signature_position + 1)); }

						if ($f_sub_signature)
						{
							$f_position = $this->dclass_type_get_position_padding ($f_sub_signature[0],$f_position);
							$f_return[$f_return_position] = array ();
							$f_sub_size = $f_bytes_unpacked[1];

							while (($f_return)&&($f_sub_size))
							{
								$f_bytes_unpacked = $this->dclass_unmarshal ($f_le,$f_sub_signature,$f_data,$f_position);

								if ($f_bytes_unpacked)
								{
									$f_sub_size -= ($f_bytes_unpacked['position'] - $f_position);
									$f_position = $f_bytes_unpacked['position'];
									unset ($f_bytes_unpacked['position']);

									$f_return[$f_return_position] = array_merge ($f_return[$f_return_position],$f_bytes_unpacked);
								}
								else { $f_return = false; }
							}

							if ($f_return)
							{
								$f_return_position++;
								$f_signature_position += strlen ($f_sub_signature);
							}
						}
						elseif ($f_bytes_unpacked) { $f_return = false; }
					}

					break 1;
				}
				case "b":
				{
					$f_position += $this->dclass_unmarshal_get_boundary ($f_position,4);
					$f_sub_read = $this->dclass_unmarshal_read ($f_data,$f_position,4);

					if (is_bool ($f_sub_read)) { $f_return = false; }
					else
					{
						if ($f_le == "B")
						{
							if ($f_sub_read[3] == "\x01") { $f_return[$f_return_position] = true; }
							else { $f_return[$f_return_position] = false; }
						}
						else
						{
							if ($f_sub_read[0] == "\x01") { $f_return[$f_return_position] = true; }
							else { $f_return[$f_return_position] = false; }
						}

						$f_position += 4;
						$f_return_position++;
					}

					break 1;
				}
				case "d":
				{
					$f_position += $this->dclass_unmarshal_get_boundary ($f_position,8);
					$f_sub_read = $this->dclass_unmarshal_read ($f_data,$f_position,8);

					if (is_bool ($f_sub_read)) { $f_return = false; }
					else
					{
						$f_return[$f_return_position] = $this->dclass_unmarshal_set_le ($f_le,"",$f_sub_read);
						$f_return_position++;
						$f_position += 8;
					}

					break 1;
				}
				case "g":
				{
					$f_sub_read = $this->dclass_unmarshal_read ($f_data,$f_position,1);

					if (is_bool ($f_sub_read)) { $f_return = false; }
					else
					{
						$f_bytes_unpacked = unpack ("C",$f_sub_read);
						$f_position++;

						if ($f_bytes_unpacked)
						{
							$f_return[$f_return_position] = $this->dclass_unmarshal_read ($f_data,$f_position,$f_bytes_unpacked[1]);
							$f_return_position++;
							$f_position += 1 + $f_bytes_unpacked[1];
						}
					}

					break 1;
				}
				case "i":
				{
					$f_position += $this->dclass_unmarshal_get_boundary ($f_position,4);
					$f_sub_read = $this->dclass_unmarshal_read ($f_data,$f_position,4);

					if (is_bool ($f_sub_read)) { $f_return = false; }
					else
					{
						$f_return[$f_return_position] = $this->dclass_unmarshal_set_le ($f_le,"L",$f_sub_read);
						$f_return_position++;
						$f_position += 4;
					}

					break 1;
				}
				case "n":
				{
					$f_position += $this->dclass_unmarshal_get_boundary ($f_position,2);
					$f_sub_read = $this->dclass_unmarshal_read ($f_data,$f_position,2);

					if (is_bool ($f_sub_read)) { $f_return = false; }
					else
					{
						$f_return[$f_return_position] = $this->dclass_unmarshal_set_le ($f_le,"s",$f_sub_read);
						$f_return_position++;
						$f_position += 2;
					}

					break 1;
				}
				case "o":
				{
					$f_position += $this->dclass_unmarshal_get_boundary ($f_position,4);
					$f_sub_read = $this->dclass_unmarshal_read ($f_data,$f_position,4);

					if (is_bool ($f_sub_read)) { $f_return = false; }
					else
					{
						if ($f_le == "B") { $f_bytes_unpacked = unpack ("N",$f_sub_read); }
						else { $f_bytes_unpacked = unpack ("V",$f_sub_read); }

						$f_position += 4;

						if ($f_bytes_unpacked)
						{
							$f_return[$f_return_position] = $this->dclass_unmarshal_read ($f_data,$f_position,$f_bytes_unpacked[1]);
							$f_return_position++;
							$f_position += 1 + $f_bytes_unpacked[1];
						}
					}

					break 1;
				}
				case "q":
				{
					$f_position += $this->dclass_unmarshal_get_boundary ($f_position,2);
					$f_sub_read = $this->dclass_unmarshal_read ($f_data,$f_position,2);

					if (is_bool ($f_sub_read)) { $f_return = false; }
					else
					{
						if ($f_le == "B") { $f_return = array_merge ($f_return,(unpack ("n",$f_sub_read))); }
						else { $f_return = array_merge ($f_return,(unpack ("v",$f_sub_read))); }

						$f_return_position++;
						$f_position += 4;
					}

					break 1;
				}
				case "s":
				{
					$f_position += $this->dclass_unmarshal_get_boundary ($f_position,4);
					$f_sub_read = $this->dclass_unmarshal_read ($f_data,$f_position,4);

					if (is_bool ($f_sub_read)) { $f_return = false; }
					else
					{
						if ($f_le == "B") { $f_bytes_unpacked = unpack ("N",$f_sub_read); }
						else { $f_bytes_unpacked = unpack ("V",$f_sub_read); }

						$f_position += 4;

						if ($f_bytes_unpacked)
						{
							$f_return[$f_return_position] = $this->dclass_unmarshal_read ($f_data,$f_position,$f_bytes_unpacked[1]);
							$f_return_position++;
							$f_position += 1 + $f_bytes_unpacked[1];
						}
					}

					break 1;
				}
				case "t":
				{
					$f_position += $this->dclass_unmarshal_get_boundary ($f_position,8);
					$f_sub_read = $this->dclass_unmarshal_read ($f_data,$f_position,8);

					if (is_bool ($f_sub_read)) { $f_return = false; }
					else
					{
						$f_return[$f_return_position] = $this->dclass_unmarshal_set_le ($f_le,"",$f_sub_read);
						$f_return_position++;
						$f_position += 8;
					}

					break 1;
				}
				case "u":
				{
					$f_position += $this->dclass_unmarshal_get_boundary ($f_position,4);
					$f_sub_read = $this->dclass_unmarshal_read ($f_data,$f_position,4);

					if (is_bool ($f_sub_read)) { $f_return = false; }
					else
					{
						if ($f_le == "B") { $f_return = array_merge ($f_return,(unpack ("N",$f_sub_read))); }
						else { $f_return = array_merge ($f_return,(unpack ("V",$f_sub_read))); }

						$f_return_position++;
						$f_position += 4;
					}

					break 1;
				}
				case "v":
				{
					$f_sub_read = $this->dclass_unmarshal_read ($f_data,$f_position,1);

					if (is_bool ($f_sub_read)) { $f_return = false; }
					else
					{
						$f_bytes_unpacked = unpack ("C",$f_sub_read);
						$f_position++;

						if ($f_bytes_unpacked)
						{
							$f_sub_read = $this->dclass_unmarshal_read ($f_data,$f_position,$f_bytes_unpacked[1]);
							$f_position += 1 + $f_bytes_unpacked[1];
						}

						$f_return[$f_return_position] = array ($f_sub_read);
						$f_bytes_unpacked = $this->dclass_unmarshal ($f_le,$f_sub_read,$f_data,$f_position);

						if (is_bool ($f_bytes_unpacked)) { $f_return = false; }
						else
						{
							$f_position = $f_bytes_unpacked['position'];
							unset ($f_bytes_unpacked['position']);
							$f_return[$f_return_position] = array_merge ($f_return[$f_return_position],$f_bytes_unpacked);
						}

						$f_return_position++;
					}

					break 1;
				}
				case "x":
				{
					$f_position += $this->dclass_unmarshal_get_boundary ($f_position,8);
					$f_sub_read = $this->dclass_unmarshal_read ($f_data,$f_position,8);

					if (is_bool ($f_sub_read)) { $f_return = false; }
					else
					{
						$f_return[$f_return_position] = $this->dclass_unmarshal_set_le ($f_le,"",$f_sub_read);
						$f_return_position++;
						$f_position += 8;
					}

					break 1;
				}
				case "y":
				{
					$f_sub_read = $this->dclass_unmarshal_read ($f_data,$f_position,1);

					if (is_bool ($f_sub_read)) { $f_return = false; }
					else
					{
						if ((is_int ($f_le))&&($f_le == $f_signature_position)) { $f_le = $f_sub_read; }
						$f_bytes_unpacked = unpack ("C",$f_sub_read);

						if ($f_bytes_unpacked)
						{
							$f_return[$f_return_position] = $f_bytes_unpacked[1];
							$f_return_position++;
							$f_position++;
						}
						else { $f_return = false; }
					}

					break 1;
				}
				default: { $f_return = false; }
				}

				$f_signature_position++;
			}

			if ((is_array ($f_return))&&($f_position_element)) { $f_return['position'] = $f_position; }
		}
		else { $f_return = false; }

		return $f_return;
	}

	//f// direct_dbus_message->dclass_unmarshal_get_boundary ($f_position,$f_boundary_spec)
	protected function dclass_unmarshal_get_boundary ($f_position,$f_boundary_spec)
	{
		if ($this->dvar_debugging) { $this->dvar_debug[] = "socketdbus/system/classes/ext_dbus/dbus_message.php -dbusmessage->dclass_unmarshal_get_boundary ($f_position,$f_boundary_spec)- (1273)"; }
		$f_return = 0;

		if ($f_boundary_spec > 0)
		{
			$f_position = ($f_boundary_spec - ($f_position % $f_boundary_spec));
			if (($f_position)&&($f_position < $f_boundary_spec)) { $f_return = $f_position; }
		}

		return $f_return;
	}

	//f// direct_dbus_message->dclass_unmarshal_read (&$f_data,$f_length)
	protected function dclass_unmarshal_read (&$f_data,$f_offset,$f_length)
	{
		if ($this->dvar_debugging) { $this->dvar_debug[] = "socketdbus/system/classes/ext_dbus/dbus_message.php -dbusmessage->dclass_unmarshal_read (+f_data,$f_offset,$f_length)- (1297)"; }

		if ((is_string ($f_data))&&(is_numeric ($f_offset))&&(is_numeric ($f_length))&&(strlen ($f_data) >= $f_offset + $f_length)) { return substr ($f_data,$f_offset,$f_length); }
		else { return false; }
	}

	//f// direct_dbus_message->dclass_unmarshal_set_le ($f_position,$f_unpack_mode,$f_data)
	protected function dclass_unmarshal_set_le ($f_le,$f_unpack_mode,$f_data)
	{
		if ($this->dvar_debugging) { $this->dvar_debug[] = "socketdbus/system/classes/ext_dbus/dbus_message.php -dbusmessage->dclass_unmarshal_set_le ($f_le,$f_unpack_mode,+f_data)- (1316)"; }
		$f_return = 0;

		if (is_string ($f_data))
		{
			if ((($f_le == "B")&&($this->dvar_nle))||(($f_le == "l")&&(!$this->dvar_nle)))
			{
				$f_bytes_inverted = array ();
				$f_position = 0;

				for ($f_i = (strlen ($f_data) - 1);$f_i > -1;$f_i--)
				{
					$f_bytes_inverted[$f_position] = $f_data[$f_i];
					$f_position++;
				}

				if ($f_unpack_mode)
				{
					$f_bytes_inverted = unpack ($f_unpack_mode,(implode ("",$f_bytes_inverted)));
					if ($f_bytes_inverted) { $f_return = $f_bytes_inverted[1]; }
				}
				else { $f_return = implode ("",$f_bytes_inverted); }
			}
			elseif ($f_unpack_mode)
			{
				$f_bytes_unpacked = unpack ($f_unpack_mode,$f_data);
				if ($f_bytes_unpacked) { $f_return = $f_bytes_unpacked[1]; }
			}
			else { $f_return = $f_data; }
		}

		return $f_return;
	}
}

define ("CLASS_direct_dbus_message",true);
}

//j// EOF
?>