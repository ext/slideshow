<?

function get_log($lines, $show_debug){
  global $Files, $BasePath;

  //  $grep_expr = $show_debug ? "\"\"" : "-v \"(DD)\"";

  //  $lines = explode("\n", `tail -n $lines $log_file $grep_cmd`);
  $filename = $show_debug ? $Files['Log']['Debug'] :  $Files['Log']['Base'];
  $lines = explode("\n", `tail -n $lines $BasePath/$filename`);

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