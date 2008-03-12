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
$Id: dbus_messages.php,v 1.8 2008/02/22 10:18:22 s4u Exp $
v0.1.00
sWG/system/classes/ext_dbus/dbus_messages.php
----------------------------------------------------------------------------
NOTE_END //n*/

//j// Functions and classes

if (!defined ("CLASS_direct_dbus_messages"))
{
//c// direct_dbus_messages
class direct_dbus_messages
{
	protected $dvar_dbus_guid;
	protected $dvar_dbus_name;
	protected $dvar_dbus_broken_data_header;
	protected $dvar_dbus_broken_data_read;
	protected $dvar_dbus_broken_length;
	protected $dvar_dbus_requests;
	protected $dvar_dbus_session;
	protected $dvar_dbus_sync_timeout;
	public $dvar_debug;
	protected $dvar_debugging;
	protected $dvar_nle;

	//f// direct_dbus_messages->__construct () and direct_dbus_messages->direct_dbus_messages ()
	public function __construct ($f_session,$f_sync_timeout = 3,$f_debug = false)
	{
		$this->dvar_debugging = $f_debug;
		if ($this->dvar_debugging) { $this->dvar_debug = array ("socketdbus/system/classes/ext_dbus/dbus_messages.php -dbusmessages->__construct (direct_dbus_messages)- (136)"); }
		$this->dvar_dbus_guid = $f_session->dclass_get_guid ();

		if ($this->dvar_dbus_guid)
		{
			$this->dvar_dbus_broken_data_header = array ();
			$this->dvar_dbus_broken_data_read = "";
			$this->dvar_dbus_broken_length = 0;
			$this->dvar_dbus_requests = 1;
			$this->dvar_dbus_session = $f_session;
			$this->dvar_dbus_sync_timeout = $f_sync_timeout;
			$this->dvar_dbus_name = "";
			$this->dvar_dbus_name = $this->dclass_get_name ();
			$this->dvar_nle = $f_session->dclass_get_nle ();
		}
	}

/*PHP4	function direct_dbus_messages ($f_session,$f_sync_timeout = 3,$f_debug = false) { $this->__construct ($f_session,$f_sync_timeout,$f_debug); }
*/
	//f// direct_dbus_messages->dclass_callback ($f_le,&$f_message)
	/*PHPe*/ protected function dclass_callback ($f_le,&$f_message)
	{
		if ($this->dvar_debugging) { $this->dvar_debug[] = "socketdbus/system/classes/ext_dbus/dbus_messages.php -dbusmessages->dclass_callback ($f_le,+f_message)- (179)"; }
		$f_return = is_object ($this->dvar_dbus_session);

		if ($f_return)
		{
			$f_header_array = $f_message->dclass_get_header (6);

			if (($this->dvar_dbus_name)&&($f_header_array))
			{
				if ($f_header_array[1] != $this->dvar_dbus_name) { $f_return = false; }
			}

			if ($f_return)
			{
				$f_header_array = $f_message->dclass_get_header (8);

				if ($f_header_array)
				{
					$f_signature = $f_header_array[1];
					$f_body = $f_message->dclass_unmarshal ($f_le,$f_signature,($f_message->dclass_get_raw_body ()));
					if (is_bool ($f_body)) { $f_return = false; }
				}
				elseif ($f_message->dclass_get_header ("body_size")) { $f_return = false; }
				else { $f_body = array (); }
			}

			if ($f_return) { $this->dvar_dbus_session->dclass_callback ($f_message,$f_body); }
		}

		return $f_return;
	}

	//f// direct_dbus_messages->dclass_callback_listen ($f_timeout = 0,$f_messages = 0)
	public function dclass_callback_listen ($f_timeout = 0,$f_messages = 0)
	{
		if ($this->dvar_debugging) { $this->dvar_debug[] = "socketdbus/system/classes/ext_dbus/dbus_messages.php -dbusmessages->dclass_callback_listen ($f_timeout,$f_messages)- (224)"; }

		$f_return = is_object ($this->dvar_dbus_session);

		if ($f_return)
		{
			if ($f_timeout == 0) { $f_timeout = $this->dvar_dbus_sync_timeout; }
			elseif ($f_timeout < 10000) { $f_timeout = 0.01; }
			else { $f_timeout /= 1000000; }

			$f_continue_check = true;
			$f_timeout_time = microtime (true) + $f_timeout;
			$f_timeout_seconds = ceil ($f_timeout / 1000000);
		}

		while ((is_bool ($f_return))&&($f_return)&&($f_continue_check)&&($f_timeout_time > (microtime (true))))
		{
			$f_continue_check = false;
			$f_message =& $this->dclass_read ($f_timeout_seconds);

			if (is_object ($f_message))
			{
				$f_le = $f_message->dclass_get_header ("endian");
				if (is_string ($f_le)) { $f_continue_check = true; }
			}

			if ($f_continue_check) { $f_return = $this->dclass_callback ($f_le,$f_message); }

			if ($f_messages > -1)
			{
				$f_messages--;
				if ($f_messages == 0) { $f_continue_check = false; }
			}
		}

		return $f_return;
	}

	//f// direct_dbus_messages->dclass_get_name ()
	public function dclass_get_name ()
	{
		if ($this->dvar_debugging) { $this->dvar_debug[] = "socketdbus/system/classes/ext_dbus/dbus_messages.php -dbusmessages->dclass_get_name ()- (280)"; }
		$f_return = false;

		if ($this->dvar_dbus_name) { $f_return = $this->dvar_dbus_name; }
		else
		{
			$f_response = $this->dclass_send_method_call_sync_response ("/org/freedesktop/DBus","org.freedesktop.DBus","Hello","org.freedesktop.DBus");

			if ((is_array ($f_response))&&(isset ($f_response['body'][0])))
			{
				$this->dvar_dbus_name = $f_response['body'][0];
				$f_return = $f_response['body'][0];
			}
		}

		return $f_return;
	}

	//f// direct_dbus_messages->dclass_get_nle ()
	public function dclass_get_nle ()
	{
		if ($this->dvar_debugging) { $this->dvar_debug[] = "socketdbus/system/classes/ext_dbus/dbus_messages.php -dbusmessages->dclass_get_nle ()- (307)"; }
		return $this->dvar_nle;
	}

	//f// direct_dbus_messages->dclass_read ($f_timeout)
	public function &dclass_read ($f_timeout)
	{
		if ($this->dvar_debugging) { $this->dvar_debug[] = "socketdbus/system/classes/ext_dbus/dbus_messages.php -dbusmessages->dclass_read ($f_timeout)- (326)"; }

		$f_continue_check = true;
		$f_response = new direct_dbus_message ($this,$this->dvar_debugging);
		$f_return = is_object ($this->dvar_dbus_session);
		$f_timeout_time = time () + $f_timeout;

		while ((is_bool ($f_return))&&($f_return)&&(($f_continue_check)||($this->dvar_dbus_broken_length))&&($f_timeout_time > (time ())))
		{
			$f_continue_check = false;

			if ($this->dvar_dbus_broken_length)
			{
				$f_length_unread = ($this->dvar_dbus_broken_length - strlen ($this->dvar_dbus_broken_data_read));

				if ($f_length_unread) { $f_data_read = $this->dvar_dbus_session->dclass_read ($f_length_unread,$f_timeout); }
				else { $f_data_read = ""; }

				if (is_bool ($f_data_read)) { $f_return = false; }
				else
				{
					$this->dvar_dbus_broken_data_read .= $f_data_read;

					if ($f_length_unread == strlen ($f_data_read))
					{
						if (empty ($this->dvar_dbus_broken_header))
						{
							$f_continue_check = true;
							$f_data_read = $this->dvar_dbus_broken_data_read;
						}
						else
						{
							if ($f_response->dclass_set ($f_response->dclass_unmarshal (0,"yyyyuua(yv)",$this->dvar_dbus_broken_data_read),$this->dvar_dbus_broken_data_read)) { $f_return =& $f_response; }
							else { $f_return = false; }

							$this->dvar_dbus_broken_data_read = "";
							$this->dvar_dbus_broken_header = array ();
							$this->dvar_dbus_broken_length = 0;
						}
					}
				}
			}
			else
			{
				$f_data_read = $this->dvar_dbus_session->dclass_read (16,$f_timeout);

				if (is_bool ($f_data_read)) { $f_return = false; }
				else
				{
					if (strlen ($f_data_read) == 16) { $f_continue_check = true; }
					else
					{
						$this->dvar_dbus_broken_data_read = $f_data_read;
						$this->dvar_dbus_broken_length = 16;
					}
				}
			}

			if ($f_continue_check)
			{
				$f_continue_check = false;

				$this->dvar_dbus_broken_data_read = $f_data_read;
				$this->dvar_dbus_broken_length = 16;

				$this->dvar_dbus_broken_header = $f_response->dclass_unmarshal (0,"yyyyuuu",$f_data_read);

				if ($this->dvar_dbus_broken_header)
				{
					if ($this->dvar_dbus_broken_header[6])
					{
						$this->dvar_dbus_broken_length += $this->dvar_dbus_broken_header[6];

						$f_length_boundary = $this->dvar_dbus_broken_length % 8;
						if (($f_length_boundary)&&($f_length_boundary < 8)) { $this->dvar_dbus_broken_length += (8 - $f_length_boundary); }
					}

					if ($this->dvar_dbus_broken_header[4]) { $this->dvar_dbus_broken_length += $this->dvar_dbus_broken_header[4]; }
				}
				else { $f_return = false; }
			}
		}

		return $f_return;
	}

	//f// direct_dbus_messages->dclass_send_build_message ($f_type,$f_path,$f_interface,$f_member,$f_destination = "",$f_flags = NULL,$f_signature = "",$f_parameter = NULL)
	protected function dclass_send_build_message ($f_type,$f_path,$f_interface,$f_member,$f_destination = "",$f_flags = NULL,$f_signature = "",$f_parameter = NULL)
	{
		if ($this->dvar_debugging) { $this->dvar_debug[] = "socketdbus/system/classes/ext_dbus/dbus_messages.php -dbusmessages->dclass_send_build_message ($f_type,$f_path,$f_interface,$f_member,$f_destination,+flags,$f_signature,+f_parameter)- (434)"; }
		$f_return = is_object ($this->dvar_dbus_session);

		if (($f_return)&&(is_string ($f_path))&&(strlen ($f_path))&&(is_string ($f_interface))&&(is_string ($f_member))&&(strlen ($f_member))&&(is_string ($f_destination)))
		{
			$f_body_exists = false;
			$f_body_raw_length = 0;
			$f_continue_check = true;
			$f_return = new direct_dbus_message ($this);

			if ((is_string ($f_signature))&&(strlen ($f_signature)))
			{
				if ($f_parameter == NULL) { $f_continue_check = false; }
				$f_body_exists = true;
			}

			if ($f_continue_check)
			{
$f_header_array = array (
array (1,(array ("o",$f_path))),
array (3,(array ("s",$f_member))),
);

				if (strlen ($f_interface)) { $f_header_array[] = array (2,(array ("s",$f_interface))); }
				if (strlen ($f_destination)) { $f_header_array[] = array (6,(array ("s",$f_destination))); }
				if ($this->dvar_dbus_name) { $f_header_array[] = array (7,(array ("s",$this->dvar_dbus_name))); }
			}

			if (($f_continue_check)&&($f_body_exists))
			{
				$f_body_raw = $f_return->dclass_marshal_array ($f_signature,$f_parameter);

				if (is_bool ($f_body_raw)) { $f_continue_check = false; }
				else
				{
					$f_header_array[] = array (8,(array ("g",$f_signature)));
					$f_body_raw_length = strlen ($f_body_raw);
				}
			}

			if ($f_continue_check)
			{
				if ($f_flags == NULL) { $f_flags = 0; }
				$f_header_array = array ("l",$f_type,$f_flags,1,$f_body_raw_length,$this->dvar_dbus_requests,$f_header_array);

				$f_header_raw = $f_return->dclass_marshal_array ("yyyyuua(yv)",$f_header_array);

				if (is_bool ($f_header_raw)) { $f_continue_check = false; }
				else
				{
					$f_return->dclass_marshal_set_boundary ($f_header_raw,(strlen ($f_header_raw)),8);
					if ($f_body_exists) { $f_header_raw .= $f_body_raw; }
				}
			}

			if ($f_continue_check) { $f_return->dclass_set ($f_header_array,$f_header_raw); }
			else { $f_return = false; }
		}

		return $f_return;
	}

	//f// direct_dbus_messages->dclass_send_method_call ($f_path,$f_interface,$f_member,$f_destination = "",$f_flags = NULL,$f_signature = "",$f_parameter = NULL)
	public function dclass_send_method_call ($f_path,$f_interface,$f_member,$f_destination = "",$f_flags = NULL,$f_signature = "",$f_parameter = NULL)
	{
		if ($this->dvar_debugging) { $this->dvar_debug[] = "socketdbus/system/classes/ext_dbus/dbus_messages.php -dbusmessages->dclass_send_method_call ($f_path,$f_interface,$f_member,$f_destination,+flags,$f_signature,+f_parameter)- (517)"; }

		if ($f_flags == NULL) { $f_flags = 1; }
		else { $f_flags |= 1; }

		$f_return = $this->dclass_send_build_message (1,$f_path,$f_interface,$f_member,$f_destination,$f_flags,$f_signature,$f_parameter);

		if (is_object ($f_return))
		{
			$f_return = $f_return->dclass_get_raw ();

			if ((is_string ($f_return))&&(strlen ($f_return))) { $f_return = $this->dvar_dbus_session->dclass_write ($f_return); }
			else { $f_return = false; }
		}

		return $f_return;
	}

	//f// direct_dbus_messages->dclass_send_method_call_async_response ($f_callback,$f_path,$f_interface,$f_member,$f_destination = "",$f_flags = NULL,$f_signature = "",$f_parameter = NULL)
	public function dclass_send_method_call_async_response ($f_callback,$f_path,$f_interface,$f_member,$f_destination = "",$f_flags = NULL,$f_signature = "",$f_parameter = NULL)
	{
		if ($this->dvar_debugging) { $this->dvar_debug[] = "socketdbus/system/classes/ext_dbus/dbus_messages.php -dbusmessages->dclass_send_method_call_async_response (+f_callback,$f_path,$f_interface,$f_member,$f_destination,+flags,$f_signature,+f_parameter)- (561)"; }

		$f_return = $this->dvar_dbus_session->dclass_callback_check ($f_callback);
		if ($f_return) { $f_return = $this->dclass_send_build_message (1,$f_path,$f_interface,$f_member,$f_destination,$f_flags,$f_signature,$f_parameter); }

		if (is_object ($f_return))
		{
			$f_return = $f_return->dclass_get_raw ();

			if ((is_string ($f_return))&&(strlen ($f_return)))
			{
				$f_return = $this->dvar_dbus_session->dclass_write ($f_return);

				if ($f_return)
				{
					$this->dvar_dbus_session->dclass_callback_register_serial ($this->dvar_dbus_requests,$f_callback);

					if ($this->dvar_dbus_requests < 4294967296) { $this->dvar_dbus_requests++; }
					else { $this->dvar_dbus_requests = 1; }
				}
			}
			else { $f_return = false; }
		}

		return $f_return;
	}

	//f// direct_dbus_messages->dclass_send_method_call_sync_response ($f_path,$f_interface,$f_member$f_flags = NULL,$f_signature = "",$f_parameter = NULL)
	public function dclass_send_method_call_sync_response ($f_path,$f_interface,$f_member,$f_destination = "",$f_flags = NULL,$f_signature = "",$f_parameter = NULL)
	{
		if ($this->dvar_debugging) { $this->dvar_debug[] = "socketdbus/system/classes/ext_dbus/dbus_messages.php -dbusmessages->dclass_send_method_call_sync_response ($f_path,$f_interface,$f_member,$f_destination,+flags,$f_signature,+f_parameter)- (608)"; }
		$f_return = false;

		$f_message = $this->dclass_send_build_message (1,$f_path,$f_interface,$f_member,$f_destination,$f_flags,$f_signature,$f_parameter);
		if (is_object ($f_message)) { $f_return = $this->dclass_send_sync_response ($f_message); }

		return $f_return;
	}

	//f// direct_dbus_messages->dclass_send_signal ($f_path,$f_interface,$f_member,$f_flags = NULL,$f_signature = "",$f_parameter = NULL)
	public function dclass_send_signal ($f_path,$f_interface,$f_member,$f_flags = NULL,$f_signature = "",$f_parameter = NULL)
	{
		if ($this->dvar_debugging) { $this->dvar_debug[] = "socketdbus/system/classes/ext_dbus/dbus_messages.php -dbusmessages->dclass_send_signal ($f_path,$f_interface,$f_member,+flags,$f_signature,+f_parameter)- (636)"; }

		if ($f_flags == NULL) { $f_flags = 1; }
		else { $f_flags |= 1; }

		if (strlen ($f_interface)) { $f_return = true; }
		else { $f_return = false; }

		if ($f_return) { $f_return = $this->dclass_send_build_message (4,$f_path,$f_interface,$f_member,"",$f_flags,$f_signature,$f_parameter); }

		if (is_object ($f_return))
		{
			$f_return = $f_return->dclass_get_raw ();

			if ((is_string ($f_return))&&(strlen ($f_return))) { $f_return = $this->dvar_dbus_session->dclass_write ($f_return); }
			else { $f_return = false; }
		}

		return $f_return;
	}

	//f// direct_dbus_messages->dclass_send_sync_response (&$f_message)
	protected function dclass_send_sync_response (&$f_message)
	{
		if ($this->dvar_debugging) { $this->dvar_debug[] = "socketdbus/system/classes/ext_dbus/dbus_messages.php -dbusmessages->dclass_send_sync_response (+f_message)- (674)"; }

		$f_continue_check = true;
		$f_return = is_object ($this->dvar_dbus_session);

		if (is_object ($f_message)) { $f_message_raw = $f_message->dclass_get_raw (); }
		else { $f_return = false; }

		if (($f_return)&&(is_string ($f_message_raw))&&(strlen ($f_message_raw)))
		{
			$f_return = $this->dvar_dbus_session->dclass_write ($f_message_raw);
			unset ($f_message_raw);

			$f_timeout_time = (time () + $this->dvar_dbus_sync_timeout);
			$f_serial = $this->dvar_dbus_requests;

			if ($this->dvar_dbus_requests < 4294967296) { $this->dvar_dbus_requests++; }
			else { $this->dvar_dbus_requests = 0; }
		}
		else { $f_return = false; }

		while ((is_bool ($f_return))&&($f_return)&&($f_continue_check)&&($f_timeout_time > (time ())))
		{
			$f_continue_check = false;
			$f_response =& $this->dclass_read ($this->dvar_dbus_sync_timeout);

			if (is_object ($f_response))
			{
				$f_le = $f_response->dclass_get_header ("endian");
				if (is_string ($f_le)) { $f_continue_check = true; }
			}

			if ($f_continue_check)
			{
				$f_header_array = $f_response->dclass_get_header (5);
				if (($f_header_array)&&($f_header_array[1] == $f_serial)) { $f_continue_check = false; }

				if ($f_continue_check) { $this->dclass_callback ($f_le,$f_response); }
				else
				{
					$f_type = $f_response->dclass_get_header ("type");
					if (is_bool ($f_type)) { $f_return = false; }

					if ($f_return)
					{
						$f_header_array = $f_response->dclass_get_header (6);

						if (($this->dvar_dbus_name)&&($f_header_array))
						{
							if ($f_header_array[1] == $this->dvar_dbus_name) { $f_continue_check = true; }
						}
						else { $f_continue_check = true; }
					}
					else { $f_continue_check = true; }

					if ($f_continue_check)
					{
						if (($f_return)&&(($f_type == "method_return")||($f_type == "error")))
						{
							$f_header_array = $f_response->dclass_get_header (8);

							if ($f_header_array)
							{
								$f_signature = $f_header_array[1];
								$f_body = $f_response->dclass_unmarshal ($f_le,$f_signature,($f_response->dclass_get_raw_body ()));
								if (is_bool ($f_body)) { $f_return = false; }
							}
							elseif ($f_response->dclass_get_header ("body_size")) { $f_return = false; }
							else { $f_body = array (); }
						}
						else { $f_return = false; }

						if ($f_return) { $f_return = array ("message" => $f_response,"body" => $f_body); }
					}
					else { $f_continue_check = true; }
				}
			}
		}

		return $f_return;
	}
}

define ("CLASS_direct_dbus_messages",true);
}

//j// EOF
?>