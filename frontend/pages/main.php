<?

require_once('../core/module.inc.php');
require_once('../slides.inc.php');

class Main extends Module {
  function index(){
    Module::set_template('main.tmpl');

    $ret = array();

    $ret['slides'] = get_slides();

    global $BasePath;
    $motd_file = "$BasePath/motd";

    if ( file_exists($motd_file) ){
      $file = fopen("$BasePath/motd", "r");
      $content = str_replace("\n", "<br/>\n", fread($file, filesize($motd_file)));

      if ( strlen($content) > 0 ){
	$ret['motd'] = $content;
      }
    } 

    return $ret;
  }
};

?>