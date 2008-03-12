<?

require_once('../settings.inc.php');
require_once('../db_functions.inc.php');
require_once('../core/module.inc.php');

class Video extends Module {
  function index(){
    Module::set_template('video.tmpl');
    global $video_dir;

    $files = array();

    if ($dh = opendir($video_dir)) {
      while (($file = readdir($dh)) !== false) {
	if ( $file[0] == "." ){
	  continue;
	}
	$files[] = $file;
      }
      closedir($dh);
    }

    return array(
		 'files' => $files
		 );
  }

  function play(){
    global $video_dir;
    $fullpath = "$video_dir/$_GET[name]";

    $this->send_signal("PlayVideo", "s", array($fullpath));
    Module::redirect('/index.php');
  }
};

?>