<?

function get_slides(){
  $slides = array();

  $set = array(
	       q("SELECT id, fullpath FROM immediate ORDER BY id"),
	       q("SELECT id, fullpath FROM files ORDER BY id")
	       );

  foreach( $set as $result ){
    while ( ( $row = mysql_fetch_assoc($result) ) ){
      $item = array(
		    'id' => $row['id'],
		    'fullpath' => $row['fullpath'],
		    'name' => basename($row['fullpath'])
		    );
      $slides[] = $item;
    }
  }

  return $slides;
}

?>