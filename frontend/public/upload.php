<?
/**
 * This file is part of Slideshow.
 * Copyright (C) 2008 David Sveningsson <ext@sidvind.com>
 * 
 * Slideshow is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Slideshow is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Slideshow.  If not, see <http://www.gnu.org/licenses/>.
 */
?>
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
