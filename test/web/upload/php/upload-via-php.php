<html>
<body>

<!--
    This upload is configured to use PHP's own file uploader and NOT the Appweb upload filter.
    You MUST not use this inside a location block which has the upload filter enabled. Otherwise the upload filter
    will get this script will not be looking for where Appweb put's it. -->

    <h2>File Upload using PHP's own file upload</h2>
<?php

    /*
        echo "<pre>";
        print_r($_FILES);
        print_r($_POST);
        print_r($HTTP_POST_VARS);
        echo "</pre>";
    */

	$uploaddir = '/tmp/';
	$uploadfile = '/tmp/' . basename($_FILES['userfile']['name']);

	if (isset($_POST['MAX_FILE_SIZE'])) {
		foreach ($_FILES as $key => $value) {
		    echo "FILES $key is $value <br />\n";
		}

		echo "<p>Upload temp file " . $_FILES['userfile']['tmp_name'] . "</p>";

		if (move_uploaded_file($_FILES['userfile']['tmp_name'], $uploadfile)) {
			echo "<p><b>File saved as " . $uploadfile . "</b></p>";
		} else {
		   echo "<b>Could not move uploaded file</b>\n";
		}
	}

?>

<form enctype="multipart/form-data" action="/upload/upload.php" method="POST">
    <input type="hidden" name="MAX_FILE_SIZE" value="134217728" /> 
    Send this file: <input name="userfile" type="file" />
    <input type="submit" value="Send File" />
</form> 
<body>
<html>
