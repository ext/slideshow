<?

function get_log($lines, $show_debug){
  global $log_file, $debug_log_file;

  //  $grep_expr = $show_debug ? "\"\"" : "-v \"(DD)\"";

  //  $lines = explode("\n", `tail -n $lines $log_file $grep_cmd`);
  $filename = $show_debug ? $debug_log_file :  $log_file;
  $lines = explode("\n", `tail -n $lines $filename`);

  $log = array();

  foreach ( $lines as $line ){
    $severity = 'normal';
    if ( strncmp( $line, "(WW)", 4) == 0 ){
      $severity = 'warning';
    } else if ( strncmp( $line, "(!!)", 4) == 0 ){
      $severity = 'fatal';
    }

    $log[] = array(
		   'severity' => $severity,
		   'content' => str_replace(" ", "&nbsp;", $line)
		   );
  }   

  return $log;
}

?>