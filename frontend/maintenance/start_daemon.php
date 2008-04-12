<?

chdir("/var/www/mizuki.sidvind.com/maintenance");

require_once("../settings.inc.php");
require_once("../pages/maintenance.php");

$module = new Maintenance;
$module->start();

?>
