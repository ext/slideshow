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
$Id: dbus_session.php,v 1.9 2008/02/21 08:46:13 s4u Exp $
v0.1.00
sWG/system/classes/ext_dbus/dbus_session.php
----------------------------------------------------------------------------
NOTE_END //n*/

//j// Functions and classes

if (!defined ("CLASS_direct_dbus_session"))
{
//c// direct_dbus_session
class direct_dbus_session
{
	protected $dvar_dbus_callbacks;
	protected $dvar_dbus_callback_listeners;
	protected $dvar_dbus_guid;
	protected $dvar_dbus_messages;
	public $dvar_debug;
	protected $dvar_debugging;
	protected $dvar_nle;
	protected $dvar_socket_available;
	protected $dvar_socket_dbus;
	protected $dvar_socket_path;
	protected $dvar_socket_timeout;

	//f// direct_dbus_session->__construct () and direct_dbus_session->direct_dbus_session ()
	public function __construct ($f_path,$f_ext_dbus_path = "",$f_debug = false)
	{
		$this->dvar_debugging = $f_debug;

		if ($this->dvar_debugging) { $this->dvar_debug = array ("socketdbus/system/classes/ext_dbus/dbus_session.php -dbussession->__construct (direct_dbus_session)- (134)"); }
		$this->dvar_dbus_callbacks = array ();
		$this->dvar_dbus_callback_listeners = "\n";
		$this->dvar_dbus_guid = "";
		$this->dvar_dbus_messages = NULL;

		if (pack ("S",1) == "\x01\x00") { $this->dvar_nle = true; }
		else { $this->dvar_nle = false; }

		$this->dvar_socket_available = function_exists ("fsockopen");
		$this->dvar_socket_dbus = NULL;

		if (strlen ($f_ext_dbus_path)) { $f_ext_dbus_path .= "/"; }

		if ((@include_once ($f_ext_dbus_path."dbus_message.php"))&&(@include_once ($f_ext_dbus_path."dbus_messages.php")))
		{
			if (stripos ($f_path,"unix:abstract://") === 0) { $this->dvar_socket_path = preg_replace ("#unix:abstract:\/\/#i","unix://\x00",$f_path); }
			else { $this->dvar_socket_path = $f_path; }
		}

		$this->dvar_socket_timeout = 15;
	}

	//f// direct_dbus_session->__destruct ()
	public function __destruct () { $this->dclass_disconnect (); }

	//f// direct_dbus_session->dclass_auth_hex ()
	protected function dclass_auth_hex ($f_data,$f_unpack = true)
	{
		if ($this->dvar_debugging) { $this->dvar_debug[] = "socketdbus/system/classes/ext_dbus/dbus_session.php -dbussession->dclass_auth_hex ($f_data,+f_unpack)- (188)"; }
		$f_return = false;

		if ($f_unpack)
		{
			$f_return = @unpack("H*hexdata",$f_data);
			if (isset ($f_return['hexdata'])) { $f_return = $f_return['hexdata']; }
		}
		else { $f_return = @pack("H*",$f_data); }

		return $f_return;
	}

	//f// direct_dbus_session->dclass_auth_read ()
	protected function dclass_auth_read ()
	{
		if ($this->dvar_debugging) { $this->dvar_debug[] = "socketdbus/system/classes/ext_dbus/dbus_session.php -dbussession->dclass_auth_read ()- (210)"; }
		$f_return = false;

		if (is_resource ($this->dvar_socket_dbus))
		{
			$f_data_read = "";
			$f_stream_check = array ($this->dvar_socket_dbus);
			$f_stream_ignored = NULL;
			$f_timeout_time = time () + $this->dvar_socket_timeout;

			while ((!feof ($this->dvar_socket_dbus))&&(strpos ($f_data_read,"\r\n") < 1)&&($f_timeout_time > (time ())))
			{
				stream_select ($f_stream_check,$f_stream_ignored,$f_stream_ignored,$this->dvar_socket_timeout);
				$f_data_read .= fread ($this->dvar_socket_dbus,4096);
			}

			if (strpos ($f_data_read,"\r\n") > 0)
			{
				if ($this->dvar_debugging) { $this->dvar_debug[] = "socketdbus/system/classes/ext_dbus/dbus_session.php -dbussession->dclass_auth_read ()- (228) read ".$f_data_read; }
				$f_return = trim ($f_data_read);
			}
		}

		return $f_return;
	}

	//f// direct_dbus_session->dclass_read_parse_response ($f_return_response = false)
	protected function dclass_auth_read_parse_response ($f_return_response = false)
	{
		if ($this->dvar_debugging) { $this->dvar_debug[] = "socketdbus/system/classes/ext_dbus/dbus_session.php -dbussession->dclass_auth_read_parse_response (+f_return_response)- (248)"; }

		$f_return = false;
		$f_data_read = $this->dclass_auth_read ();

		if ($f_data_read)
		{
			$f_data = explode (" ",$f_data_read,2);

			if (count ($f_data) == 2)
			{
				if ($f_return_response) { $f_return = $f_data; }
				elseif (($f_data[0] == "DATA")||($f_data[0] == "OK")) { $f_return = true; }
			}
		}

		return $f_return;
	}

	//f// direct_dbus_session->dclass_auth_write ($f_data)
	protected function dclass_auth_write ($f_data)
	{
		if ($this->dvar_debugging) { $this->dvar_debug[] = "socketdbus/system/classes/ext_dbus/dbus_session.php -dbussession->dclass_auth_write ($f_data)- (278)"; }
		$f_return = false;

		if (is_resource ($this->dvar_socket_dbus))
		{
			$f_data = str_replace ((array ("\r","\n")),"",$f_data);
			$f_return = $this->dclass_write ($f_data."\r\n");
		}

		return $f_return;
	}

	//f// direct_dbus_session->dclass_auth_write_parse_response ($f_data,$f_return_response = false)
	protected function dclass_auth_write_parse_response ($f_data,$f_return_response = false)
	{
		if ($this->dvar_debugging) { $this->dvar_debug[] = "socketdbus/system/classes/ext_dbus/dbus_session.php -dbussession->dclass_auth_write_parse_response ($f_data,+f_return_response)- (304)"; }

		$f_return = false;
		if ($this->dclass_auth_write ($f_data)) { $f_return = $this->dclass_auth_read_parse_response ($f_return_response); }

		return $f_return;
	}

	//f// direct_dbus_session->dclass_callback (&$f_message,$f_body)
	public function dclass_callback (&$f_message,$f_body)
	{
		if ($this->dvar_debugging) { $this->dvar_debug[] = "socketdbus/system/classes/ext_dbus/dbus_session.php -dbussession->dclass_callback (+f_message,+f_body)- (324)"; }

		if ((is_object ($f_message))&&(is_array ($f_body)))
		{
			$f_header_array = $f_message->dclass_get_header (5);
			$f_type = $f_message->dclass_get_header ("type");
			$f_continue_check = is_string ($f_type);

			if (($f_continue_check)&&(($f_type == "method_return")||($f_type == "error")))
			{
				if (($f_header_array)&&(isset ($this->dvar_dbus_callbacks[$f_header_array[1]])))
				{
					$this->dclass_callback_caller ($this->dvar_dbus_callbacks[$f_header_array[1]],$f_message,$f_body);
					unset ($this->dvar_dbus_callbacks[$f_header_array[1]]);
				}
			}

			if ($f_continue_check)
			{
				$f_listener = "(\*|".(preg_quote ($f_type)).")";

				for ($f_i = 1;$f_i < 4;$f_i++)
				{
					$f_header_array = $f_message->dclass_get_header ($f_i);

					if ($f_header_array) { $f_listener .= "\:(\*|".(preg_quote ($f_header_array[1])).")"; }
					else { $f_listener .= "\:\*"; }
				}

				$f_listener = "($f_listener)";
			}

			if (($f_continue_check)&&(preg_match_all ("#^$f_listener$#im",$this->dvar_dbus_callback_listeners,$f_listeners,PREG_SET_ORDER)))
			{
				foreach ($f_listeners as $f_listener)
				{
					if (isset ($this->dvar_dbus_callbacks[$f_listener[1]])) { $this->dclass_callback_caller ($this->dvar_dbus_callbacks[$f_listener[1]],$f_message,$f_body); }
				}
			}
		}
	}

	//f// direct_dbus_session->dclass_callback_caller ($f_callbacks,&$f_message,$f_body)
	protected function dclass_callback_caller ($f_callbacks,&$f_message,$f_body)
	{
		if ($this->dvar_debugging) { $this->dvar_debug[] = "socketdbus/system/classes/ext_dbus/dbus_session.php -dbussession->dclass_callback_caller (+f_callbacks,+f_message,+f_body)- (381)"; }

		if ((is_array ($f_callbacks))&&(is_object ($f_message))&&(is_array ($f_body)))
		{
			foreach ($f_callbacks as $f_callback)
			{
				if ((is_string ($f_callback))&&(function_exists ($f_callback))) { $f_callback ($f_message,$f_body); }
				elseif ((is_array ($f_callback))&&(count ($f_callback) == 2)&&(isset ($f_callback[0]))&&(isset ($f_callback[1]))&&(method_exists ($f_callback[0],$f_callback[1]))) { $f_callback[0]->{$f_callback[1]} ($f_message,$f_body); }
			}
		}
	}

	//f// direct_dbus_session->dclass_callback_check ($f_callback)
	public function dclass_callback_check ($f_callback)
	{
		if ($this->dvar_debugging) { $this->dvar_debug[] = "socketdbus/system/classes/ext_dbus/dbus_session.php -dbussession->dclass_callback_check (+f_callback)- (404)"; }
		$f_return = false;

		if ((is_string ($f_callback))&&(function_exists ($f_callback))) { $f_return = true; }
		elseif ((is_array ($f_callback))&&(count ($f_callback) == 2)&&(isset ($f_callback[0]))&&(isset ($f_callback[1]))) { $f_return = method_exists ($f_callback[0],$f_callback[1]); }

		return $f_return;
	}

	//f// direct_dbus_session->dclass_callback_listen ($f_timeout = 0,$f_messages = 0)
	public function dclass_callback_listen ($f_timeout = 0,$f_messages = 0)
	{
		if ($this->dvar_debugging) { $this->dvar_debug[] = "socketdbus/system/classes/ext_dbus/dbus_session.php -dbussession->dclass_callback_listen ($f_timeout,$f_messages)- (426)"; }

		if ((is_resource ($this->dvar_socket_dbus))&&(is_object ($this->dvar_dbus_messages))) { return $this->dvar_dbus_messages->dclass_callback_listen ($f_timeout,$f_messages); }
		else { return false; }
	}

	//f// direct_dbus_session->dclass_callback_listener_id ($f_type,$f_path,$f_interface,$f_member)
	protected function dclass_callback_listener_id ($f_type,$f_path,$f_interface,$f_member)
	{
		if ($this->dvar_debugging) { $this->dvar_debug[] = "socketdbus/system/classes/ext_dbus/dbus_session.php -dbussession->dclass_callback_listener_id ($f_type,$f_path,$f_interface,$f_member,+f_callback)- (446)"; }

		if ((is_string ($f_type))&&(is_string ($f_path))&&(is_string ($f_interface))&&(is_string ($f_member)))
		{
			if (strlen ($f_type)) { $f_return = $f_type; }
			else { $f_return = "*"; }

			if (strlen ($f_path)) { $f_return .= ":".$f_path; }
			else { $f_return .= ":*"; }

			if (strlen ($f_interface)) { $f_return .= ":".$f_interface; }
			else { $f_return .= ":*"; }

			if (strlen ($f_member)) { $f_return .= ":".$f_member; }
			else { $f_return .= ":*"; }
		}
		else { $f_return = false; }

		return $f_return;
	}

	//f// direct_dbus_session->dclass_callback_register_listener ($f_type,$f_path,$f_interface,$f_member,$f_callback)
	public function dclass_callback_register_listener ($f_type,$f_path,$f_interface,$f_member,$f_callback)
	{
		if ($this->dvar_debugging) { $this->dvar_debug[] = "socketdbus/system/classes/ext_dbus/dbus_session.php -dbussession->dclass_callback_register_listener ($f_type,$f_path,$f_interface,$f_member,+f_callback)- (484)"; }

		$f_return = $this->dclass_callback_check ($f_callback);
		if ($f_return) { $f_return = $this->dclass_callback_listener_id ($f_type,$f_path,$f_interface,$f_member); }

		if (is_string ($f_return))
		{
			if (!isset ($this->dvar_dbus_callbacks[$f_return])) { $this->dvar_dbus_callbacks[$f_return] = array (); }

			if (is_string ($f_callback))
			{
				$this->dvar_dbus_callbacks[$f_return][$f_callback] = $f_callback;
				$this->dvar_dbus_callback_listeners = (str_replace ("\n$f_return\n","\n",$this->dvar_dbus_callback_listeners)).$f_return."\n";
			}
			elseif (is_array ($f_callback))
			{
				$f_callback_class = get_class ($f_callback[0]);
				$this->dvar_dbus_callbacks[$f_return][$f_callback_class.".".$f_callback[1]] = $f_callback;
				$this->dvar_dbus_callback_listeners = (str_replace ("\n$f_return\n","\n",$this->dvar_dbus_callback_listeners)).$f_return."\n";
			}
			else { $f_return = false; }
		}

		return $f_return;
	}

	//f// direct_dbus_session->dclass_callback_register_serial ($f_serial,$f_callback)
	public function dclass_callback_register_serial ($f_serial,$f_callback)
	{
		if ($this->dvar_debugging) { $this->dvar_debug[] = "socketdbus/system/classes/ext_dbus/dbus_session.php -dbussession->dclass_callback_register_serial ($f_serial,+f_callback)- (522)"; }

		if (!isset ($this->dvar_dbus_callbacks[$f_serial])) { $this->dvar_dbus_callbacks[$f_serial] = array (); }

		if (is_string ($f_callback)) { $this->dvar_dbus_callbacks[$f_serial][$f_callback] = $f_callback; }
		elseif (is_array ($f_callback))
		{
			$f_callback_class = get_class ($f_callback[0]);
			if (is_string ($f_callback[1])) { $this->dvar_dbus_callbacks[$f_serial][$f_callback_class.".".$f_callback[1]] = $f_callback; }
		}
	}

	//f// direct_dbus_session->dclass_callback_unregister_listener ($f_type,$f_path,$f_interface,$f_member,$f_callback)
	public function dclass_callback_unregister_listener ($f_type,$f_path,$f_interface,$f_member,$f_callback)
	{
		if ($this->dvar_debugging) { $this->dvar_debug[] = "socketdbus/system/classes/ext_dbus/dbus_session.php -dbussession->dclass_callback_unregister_listener ($f_type,$f_path,$f_interface,$f_member,+f_callback)- (552)"; }

		$f_return = $this->dclass_callback_check ($f_callback);
		if ($f_return) { $f_return = $this->dclass_callback_listener_id ($f_type,$f_path,$f_interface,$f_member); }

		if (is_string ($f_return))
		{
			if (is_string ($f_callback))
			{
				if (isset ($this->dvar_dbus_callbacks[$f_return][$f_callback])) { unset ($this->dvar_dbus_callbacks[$f_return][$f_callback]); }
			}
			elseif (is_array ($f_callback))
			{
				$f_callback_class = get_class ($f_callback[0]);
				if (isset ($this->dvar_dbus_callbacks[$f_return][$f_callback_class.".".$f_callback[1]])) { unset ($this->dvar_dbus_callbacks[$f_return][$f_callback_class.".".$f_callback[1]]); }
			}
			else { $f_return = false; }

			$this->dvar_dbus_callback_listeners = str_replace ("\n$f_return\n","\n",$this->dvar_dbus_callback_listeners);
			if (empty ($this->dvar_dbus_callbacks[$f_return])) { unset ($this->dvar_dbus_callbacks[$f_return]); }
		}

		return $f_return;
	}

	//f// direct_dbus_session->dclass_connect ($f_sync_timeout = 3)
	public function dclass_connect ($f_sync_timeout = 3)
	{
		if ($this->dvar_debugging) { $this->dvar_debug[] = "socketdbus/system/classes/ext_dbus/dbus_session.php -dbussession->dclass_connect ($f_sync_timeout)- (589)"; }

		$f_return = false;

		if (is_resource ($this->dvar_socket_dbus)) { $f_return = false; }
		elseif ($this->dvar_socket_available)
		{
			$f_error_code = 0;
			$f_error = "";

			if (stripos ($this->dvar_socket_path,"unix://") === 0) { $this->dvar_socket_dbus = @fsockopen ($this->dvar_socket_path,0,$f_error_code,$f_error,$this->dvar_socket_timeout); }
			elseif (preg_match ("#^(.+?)\:(\d+)$#",$this->dvar_socket_path,$f_port_array)) { $this->dvar_socket_dbus = @fsockopen ($f_port_array[1],$f_port_array[2],$f_error_code,$f_error,$this->dvar_socket_timeout); }

			if (($f_error_code)||($f_error)||(!is_resource ($this->dvar_socket_dbus)))
			{
				if ($this->dvar_debugging) { $this->dvar_debug[] = "socketdbus/system/classes/ext_dbus/dbus_session.php -dbussession->dclass_connect ()- (604) reports: $f_error_code:".$f_error; }
				$this->dvar_socket_dbus = NULL;
			}
			else
			{
				$f_return = true;
				@stream_set_blocking ($this->dvar_socket_dbus,0);
				@stream_set_timeout ($this->dvar_socket_dbus,$this->dvar_socket_timeout);

				$f_auth_response = $this->dclass_auth_write_parse_response ("\x00AUTH ",true);
				if (is_array ($f_auth_response)) { $f_auth_response = explode (" ",$f_auth_response[1]); }
			}

			if (($f_return)&&(in_array ("EXTERNAL",$f_auth_response)))
			{
				if (function_exists ("posix_getuid")) { $f_uid = posix_getuid (); }
				else { $f_uid = getmyuid (); }

				if (is_numeric ($f_uid))
				{
					$f_auth_login_response = $this->dclass_auth_write_parse_response ("AUTH EXTERNAL ".($this->dclass_auth_hex ($f_uid)),true);
					if ((is_array ($f_auth_login_response))&&($f_auth_login_response[0] == "OK")) { $this->dvar_dbus_guid = $f_auth_login_response[1]; }
				}
			}

			if (($f_return)&&(!$this->dvar_dbus_guid)&&(in_array ("ANONYMOUS",$f_auth_response)))
			{
				$f_auth_response = $this->dclass_auth_write_parse_response ("AUTH ANONYMOUS",true);
				if ((is_array ($f_auth_response))&&($f_auth_response[0] == "OK")) { $this->dvar_dbus_guid = $f_auth_response[1]; }
			}

			if ($this->dvar_dbus_guid)
			{
				$f_return = $this->dclass_auth_write ("BEGIN ");
				if ($this->dvar_debugging) { $this->dvar_debug[] = "socketdbus/system/classes/ext_dbus/dbus_session.php -dbussession->dclass_connect ()- (638) started binary protocol"; }
				$this->dvar_dbus_messages = new direct_dbus_messages ($this,$f_sync_timeout,$this->dvar_debugging);
			}
			else { $f_return = false; }
		}

		return $f_return;
	}

	//f// direct_dbus_session->dclass_disconnect ()
	public function dclass_disconnect ()
	{
		if ($this->dvar_debugging) { $this->dvar_debug[] = "socketdbus/system/classes/ext_dbus/dbus_session.php -dbussession->dclass_disconnect ()- (656)"; }
		$f_return = false;

		if (is_resource ($this->dvar_socket_dbus))
		{
			$f_return = fclose ($this->dvar_socket_dbus);
			$this->dvar_dbus_callbacks = array ();
			$this->dvar_dbus_callback_listeners = "\n";
			$this->dvar_dbus_guid = "";
			$this->dvar_dbus_messages = NULL;
			$this->dvar_socket_dbus = NULL;
			$this->dvar_socket_path = "";
		}

		return $f_return;
	}

	//f// direct_dbus_session->dclass_get_guid ()
	public function dclass_get_guid ()
	{
		if ($this->dvar_debugging) { $this->dvar_debug[] = "socketdbus/system/classes/ext_dbus/dbus_session.php -dbussession->dclass_get_guid ()- (682)"; }

		if (is_resource ($this->dvar_socket_dbus)) { return $this->dvar_dbus_guid; }
		else { return false; }
	}

	//f// direct_dbus_session->dclass_get_handle ()
	public function &dclass_get_handle ()
	{
		if ($this->dvar_debugging) { $this->dvar_debug[] = "socketdbus/system/classes/ext_dbus/dbus_session.php -dbussession->dclass_get_handle ()- (697)"; }

		if ((is_resource ($this->dvar_socket_dbus))&&(is_object ($this->dvar_dbus_messages))) { $f_return =& $this->dvar_socket_dbus; }
		else { $f_return = false; }

		return $f_return;
	}

	//f// direct_dbus_session->dclass_get_nle ()
	public function dclass_get_nle ()
	{
		if ($this->dvar_debugging) { $this->dvar_debug[] = "socketdbus/system/classes/ext_dbus/dbus_session.php -dbussession->dclass_get_nle ()- (714)"; }
		return $this->dvar_nle;
	}

	//f// direct_dbus_session->dclass_read ()
	public function dclass_read ($f_length,$f_timeout,$f_length_forced = false)
	{
		if ($this->dvar_debugging) { $this->dvar_debug[] = "socketdbus/system/classes/ext_dbus/dbus_session.php -dbussession->dclass_read ($f_length,$f_timeout,+f_length_forced)- (730)"; }
		$f_return = "";

		if ((is_resource ($this->dvar_socket_dbus))&&($f_length))
		{
			$f_length_read = 0;
			$f_length_last_read = 0;
			$f_stream_check = array ($this->dvar_socket_dbus);
			$f_stream_ignored = NULL;
			$f_timeout_time = time () + $f_timeout;

			do
			{
				stream_select ($f_stream_check,$f_stream_ignored,$f_stream_ignored,$f_timeout);
				$f_data_read = fread ($this->dvar_socket_dbus,$f_length);
				$f_length_last_read = strlen ($f_data_read);

				$f_return .= $f_data_read;
				$f_length_read += $f_length_last_read;
			}
			while ((!feof ($this->dvar_socket_dbus))&&($f_length_read < $f_length)&&($f_timeout_time > (time ()))&&(($f_length_last_read > 0)||($f_length_forced)));
		}

		if ($this->dvar_debugging) { $this->dvar_debug[] = "socketdbus/system/classes/ext_dbus/dbus_session.php -dbussession->dclass_read ()- (753) read ".$f_return; }
		return $f_return;
	}

	//f// direct_dbus_session->dclass_resource_check ()
	public function dclass_resource_check ()
	{
		if ($this->dvar_debugging) { $this->dvar_debug[] = "socketdbus/system/classes/ext_dbus/dbus_session.php -dbussession->dclass_resource_check ()- (766)"; }
		return is_resource ($this->dvar_socket_dbus);
	}

	//f// direct_dbus_session->dclass_send_method_call ($f_path,$f_interface,$f_member,$f_destination = "",$f_flags = NULL,$f_signature = "",$f_parameter = NULL)
	public function dclass_send_method_call ($f_path,$f_interface,$f_member,$f_destination = "",$f_flags = NULL,$f_signature = "",$f_parameter = NULL)
	{
		if ($this->dvar_debugging) { $this->dvar_debug[] = "socketdbus/system/classes/ext_dbus/dbus_session.php -dbussession->dclass_send_method_call ($f_path,$f_interface,$f_member,$f_destination,+flags,$f_signature,+f_parameter)- (789)"; }

		if ((is_resource ($this->dvar_socket_dbus))&&(is_object ($this->dvar_dbus_messages))) { return $this->dvar_dbus_messages->dclass_send_method_call ($f_path,$f_interface,$f_member,$f_destination,$f_flags,$f_signature,$f_parameter); }
		else { return false; }
	}

	//f// direct_dbus_session->dclass_send_method_call_async_response ($f_callback,$f_path,$f_interface,$f_member,$f_destination = "",$f_flags = NULL,$f_signature = "",$f_parameter = NULL)
	public function dclass_send_method_call_async_response ($f_callback,$f_path,$f_interface,$f_member,$f_destination = "",$f_flags = NULL,$f_signature = "",$f_parameter = NULL)
	{
		if ($this->dvar_debugging) { $this->dvar_debug[] = "socketdbus/system/classes/ext_dbus/dbus_session.php -dbussession->dclass_send_method_call_async_response (+f_callback,$f_path,$f_interface,$f_member,$f_destination,+flags,$f_signature,+f_parameter)- (817)"; }

		if ((is_resource ($this->dvar_socket_dbus))&&(is_object ($this->dvar_dbus_messages))) { return $this->dvar_dbus_messages->dclass_send_method_call_async_response ($f_callback,$f_path,$f_interface,$f_member,$f_destination,$f_flags,$f_signature,$f_parameter); }
		else { return false; }
	}

	//f// direct_dbus_session->dclass_send_method_call_sync_response ($f_path,$f_interface,$f_member,$f_destination = "",$f_flags = NULL,$f_signature = "",$f_parameter = NULL)
	public function dclass_send_method_call_sync_response ($f_path,$f_interface,$f_member,$f_destination = "",$f_flags = NULL,$f_signature = "",$f_parameter = NULL)
	{
		if ($this->dvar_debugging) { $this->dvar_debug[] = "socketdbus/system/classes/ext_dbus/dbus_session.php -dbussession->dclass_send_method_call_sync_response ($f_path,$f_interface,$f_member,$f_destination,+flags,$f_signature,+f_parameter)- (842)"; }

		if ((is_resource ($this->dvar_socket_dbus))&&(is_object ($this->dvar_dbus_messages))) { return $this->dvar_dbus_messages->dclass_send_method_call_sync_response ($f_path,$f_interface,$f_member,$f_destination,$f_flags,$f_signature,$f_parameter); }
		else { return false; }
	}

	//f// direct_dbus_session->dclass_send_signal ($f_path,$f_interface,$f_member,$f_flags = NULL,$f_signature = "",$f_parameter = NULL)
	public function dclass_send_signal ($f_path,$f_interface,$f_member,$f_flags = NULL,$f_signature = "",$f_parameter = NULL)
	{
		if ($this->dvar_debugging) { $this->dvar_debug[] = "socketdbus/system/classes/ext_dbus/dbus_session.php -dbussession->dclass_send_signal ($f_path,$f_interface,$f_member,+flags,$f_signature,+f_parameter)- (865)"; }

		if ((is_resource ($this->dvar_socket_dbus))&&(is_object ($this->dvar_dbus_messages))) { return $this->dvar_dbus_messages->dclass_send_signal ($f_path,$f_interface,$f_member,$f_flags,$f_signature,$f_parameter); }
		else { return false; }
	}

	//f// direct_dbus_session->dclass_set_flag ($f_flag = "",$f_status = NULL,$f_flags = "")
	public function dclass_set_flag ($f_flag = "",$f_status = NULL,$f_flags = "")
	{
		if ($this->dvar_debugging) { $this->dvar_debug[] = "socketdbus/system/classes/ext_dbus/dbus_session.php -dbussession->dclass_set_flag ($f_flag,+f_status,+f_flags)- (884)"; }

		if (strlen ($f_flags)) { $f_return = $f_flags; }
		else { $f_return = 0; }

		switch ($f_flag)
		{
		case "no_reply_expected":
		{
			$f_flag = 1;
			break 1;
		}
		case "no_auto_start":
		{
			$f_flag = 2;
			break 1;
		}
		default: { $f_flag = NULL; }
		}

		if ($f_flag != NULL)
		{
			if (is_bool ($f_status))
			{
				if ($f_status) { $f_return |= $f_flag; }
				else { $f_return &= ~$f_flag; }
			}
			else { $f_return ^= $f_flag; }
		}

		return $f_return;
	}

	//f// direct_dbus_session->dclass_write ($f_data)
	public function dclass_write ($f_data)
	{
		if ($this->dvar_debugging) { $this->dvar_debug[] = "socketdbus/system/classes/ext_dbus/dbus_session.php -dbussession->dclass_write ($f_data)- (927)"; }
		$f_return = false;

		if ((is_resource ($this->dvar_socket_dbus))&&(!empty ($f_data)))
		{
			if (fwrite ($this->dvar_socket_dbus,$f_data)) { $f_return = true; }
		}

		return $f_return;
	}
}

define ("CLASS_direct_dbus_session",true);
}

//j// EOF
?>