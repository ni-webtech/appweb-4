<html>
<head>
	<title>Test PHP Page</title>
</head>

<body>

	Welcome <?php echo $_POST["name"]; ?>.<br />
	Favorite color <?php echo $_POST["color"]; ?>
<?
	echo "TOP ";
	foreach ($_POST  as $key => $value) {
		echo "<p>POST $key is $value </p>\n";
	}
	foreach ($HTTP_POST_VARS as $key => $value) {
		echo "<p>HTTP_POST $key is $value </p>\n";
	}
	foreach ($HTTP_GET_VARS as $key => $value) {
		echo "<p>GET $key is $value </p>\n";
	}
	foreach ($_REQUEST as $key => $value) {
		echo "<p>REQUEST $key is $value </p>\n";
	}
	$v = $_REQUEST['ok'];
	echo "Value: $v";
	$v = $_POST;
	echo "POST is $v";

	if (isset($_REQUEST['ok'])) {
		echo "ISSET";
	}
?>

<form action="form.php" method="post">
Name: <input type="text" name="name" />
Color: <input type="text" name="color" />
<input type="submit" />
</form>

</body>
</html>
