<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">

<head>
	<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
	<title>bajs</title>
</head>

<body>

<div>
	<h2>Upload new item</h2>
	<h3>Image</h3>

	<form action="upload_image.php" method="post" enctype="multipart/form-data">
		<fieldset>
			<input type="file" id="filename" name="filename" />
			<input type="submit" value="Upload" />
		</fieldset>
	</form>

	<h3>Text</h3>

	<form action="upload_text.php" method="post">
		<fieldset>
			<label for="title">Title</label><br/>
			<input type="text" id="title" name="title" /><br/>
			<label for="content">Content</label><br/>
			<textarea id="content" name="content" cols="10" rows="10"></textarea><br/>
			<input type="submit" value="Upload" />
		</fieldset>
	</form>
</div>

</body>
</html>
