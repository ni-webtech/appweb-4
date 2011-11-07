<?
        foreach ($_POST as $key => $value) {
            echo "_POST[$key] = \"$value\n\"";
        }
        foreach ($_GET as $key => $value) {
            echo "_GET[$key] = \"$value\"\n";
        }
        foreach ($_SERVER as $key => $value) {
            echo "_SERVER[$key] = \"$value\"\n";
        }
        foreach ($_ENV as $key => $value) {
            echo "_ENV[$key] = \"$value\"\n";
        }
?>
