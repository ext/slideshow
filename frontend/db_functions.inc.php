<?
function connect(){
  global $db_user, $db_pass, $db_host, $db_name;
  mysql_connect($db_host, $db_user, $db_pass)
    or die("Could not connect to database: " . mysql_error());
  mysql_select_db($db_name);
}

function disconnect(){
  mysql_close();
}

function q($query){
  $res = mysql_query($query) or
    die("Mysql error: ".mysql_error()."\n<br/>Executing \"$query\"\n");
  return $res;
}
?>