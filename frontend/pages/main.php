<?

require_once('../core/module.inc.php');
require_once('../slides.inc.php');

class Main extends Module {
  function index(){
    Module::set_template('main.tmpl');

    return array(
		  'slides' => get_slides()
		  );
  }
};

?>