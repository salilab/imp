<?php

# Populate the HTTP GET array from a command line containing 'key=value'
# arguments
foreach ($argv as $arg) {
  list($key, $val) = explode("=", $arg);
  $_GET[trim($key)] = trim($val);
}

?>
