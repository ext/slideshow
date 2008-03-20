<?

if ( !file_exists("../settings.json") ){
	die("settings.json not found!");
}

$json_string = file_get_contents("../settings.json");
$data = json_decode( $json_string, true );

foreach ( $data['Path'] as $name => $value ){
	$data['Path'][$name] = "{$data['BasePath']}/$value";
}

extract($data);

?>
