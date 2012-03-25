<html><head>form.php</head>
<body>
    <pre>
        <?php
            if (isset($_POST['submit'])) {
                header('X-Private-Header: 123');
            }
            foreach ($_SERVER as $key => $value) {
                echo "_SERVER $key is $value <br/>\n";
            }
            foreach ($_POST as $key => $value) {
                echo "_POST $key is $value <br/>\n";
            }
            foreach ($_GET as $key => $value) {
                echo "_GET $key is $value <br/>\n";
            }
        ?></pre>
	<form name="details" method="post" action="form.php">
		Name <input type="text" name="name" value="<?php echo $_POST['name'] ?>">
		Address <input type="text" name="address" value="<?php echo $_POST['address'] ?>">
		<input type="submit" name="submit" value="OK">
	</form>
</body>
</html>
