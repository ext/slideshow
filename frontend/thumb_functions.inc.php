<?

require_once("../settings.inc.php");

function video_thumbnail($filename){
  global $video_dir, $tmp_dir;

  $needs_escape = array(" ", "&", "-");
  $escaped = array("\\ ", "\\&", "\\-");

  $escaped_filename = str_replace($needs_escape, $escaped, $filename);

  if ( !file_exists( "video_thumbs/$filename.gif" ) ){
    $tmp = array();

    for ( $i = 0; $i < 5; $i++ ){
      $name = tempnam($tmp_dir,"thumb");
      $tmp[] = $name;
      shell_exec("ffmpeg -i $video_dir/$escaped_filename -vframes 1 -ss ".($i*5)." -f image2 $name");
    }

    echo shell_exec("convert -delay 50 $tmp[0] $tmp[1] $tmp[2] $tmp[3] $tmp[4] -loop 0 video_thumbs/$escaped_filename.gif");

    for ( $i = 0; $i < 5; $i++ ){
      unlink($tmp[$i]);
    }
  }

  return "video_thumbs/$filename.gif";	
}