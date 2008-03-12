<?

function post($name, $default = ""){
  return array_get($_POST, $name, $default);
}

function get($name, $default = ""){
  return array_get($_GET, $name, $default);
}

function array_get(array $array, $name, $default){
  $data = $default;
  if ( isset ( $array[$name] ) ){
    $data = $array[$name];
  }

  if ( get_magic_quotes_gpc() ){
    $data = stripslashes($data);
  }

  return $data;
}