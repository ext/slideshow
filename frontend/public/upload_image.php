<?

require_once("../settings.inc.php");
require_once("../db_functions.inc.php");

$name = $_FILES['filename']['name'];
$hash = crc32(uniqid());
$fullpath = "$image_dir/{$hash}_$name";

move_uploaded_file($_FILES['filename']['tmp_name'], $fullpath);

connect();
q("INSERT INTO files (fullpath) VALUES ('$fullpath')");
disconnect();

header('Location: index.php');

?>