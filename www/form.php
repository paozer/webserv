<!DOCTYPE html>
<html>
<body>
<h1>Form!</h1>
<br />
<form action="action.php" method="get">
	<p>Name: <input type="text" name="name" /></p>
	<p>Age: <input type="text" name="age" /></p>
	<p><input type="submit" value="OK"></p>
</form>

<?php
$output = shell_exec('ls');
echo "<pre>$output</pre>";
?>

<br />
<?php
phpinfo(INFO_VARIABLES);
?>
<p><a href="index.html">Back to index</a></p>
<p><a href="page.php">page.php</a></p>
<p><a href="auth.php">auth.php</a></p>
<p><a href="connect.php">connect.php</a></p>
<p><a href="form.php">form.php</a></p>


</body>
</html>
