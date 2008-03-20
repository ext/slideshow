<?
function connect(){
  global $Database;
  mysql_connect($Database['Host'], $Database['Username'], $Database['Password'])
    or die("Could not connect to database: " . mysql_error());
  mysql_select_db($Database['Name']);
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