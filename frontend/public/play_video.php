<?

require_once("../settings.inc.php");
require_once("../db_functions.inc.php");

$fullpath = "$video_dir/$_GET[name]";

connect();
q("INSERT INTO immediate (fullpath) VALUES ('$fullpath')");
disconnect();

header('Location: index.php');

?>