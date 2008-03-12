<?

class Path {
  private $path;

  function Path(){
    if ( isset($_SERVER['PATH_INFO']) ){
      $path = $_SERVER['PATH_INFO'];

      if ( $path == '/' ){
	$this->path = array ( 'main', 'index' );
	return;
      }

      $this->path = array_slice(explode("/",$_SERVER['PATH_INFO']),1);

      $section_missing = count($this->path) == 1;
      if ( $section_missing ){
	$this->path[1] = 'index';
      }

      $section_invalid = $this->path[1] == '';
      if ( $section_invalid ){
	$this->path[1] = 'index';
      }

    } else {
      $this->path = array( 'main', 'index' );
    }
  }

  function module(){
    ///@todo Check for valid modules
    return $this->path[0];
  }

  function section(){
    return $this->path[1];
  }
};

?>
