<html>
<body>

<!-- This upload is configured to use the Appweb upload filter and not the PHP upload filter. -->

    <h2>File Upload to PHP using the Appweb Upload Filter</h2>

<?php

    /*
        echo "<pre>";
        print_r($_FILES);
        print_r($_POST);
        print_r($HTTP_POST_VARS);
        echo "</pre>";
    */

/*
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
*/
?>

<form enctype="multipart/form-data" action="/upload/upload.php" method="POST">
    <input type="hidden" name="MAX_FILE_SIZE" value="134217728" /> 
    <table>
        <tr><td>File</td><td><input name="userfile" type="file" /></td>
    </table>
    <input type="submit" value="Send File" />
</form> 
<body>
<html>
