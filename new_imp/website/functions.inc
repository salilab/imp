<?php

$sections = array(
  "index" => array(
    "title" => "IMP, an Integrative Modeling Platform",
    "shorttitle" => "About"),
  "contact" => array(
    "title" => "Contact",
    "shorttitle" => "Contact"),
  "developers" => array(
    "title" => "Developers",
    "shorttitle" => "Developers"),
);

function write_header($page) {
  global $sections;
  $title = $sections[$page][title];
  print <<< HEADER
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN"
             "http://www.w3.org/TR/html4/strict.dtd">
<html>
<head>
<link href="/imp/imp.css" rel="stylesheet" type="text/css">
<meta http-equiv="Content-Type" content="text/html;charset=utf-8">
<script type="text/javascript" src="/escramble-new.js"></script>
<title>$title</title>
</head>

<body>

<!-- Auto-generated file: do not edit (any changes will be lost) -->

<div class="header">
<p class="centerimg">
  <img src="imp_banner.jpg" alt="IMP, an Integrative Modeling Platform">
</p>
</div>

<div class="navigation">
HEADER;

  write_navigation($page);
  print "\n<div class=\"content\">\n";
}

function write_navigation($page) {
  global $sections;
  print "<p>\n";
  $links = array();
  foreach ($sections as $key => $value) {
    if ($key == $page) {
      array_push($links, $value[shorttitle]);
    } else {
      array_push($links, "<a href=\"$key.html\">$value[shorttitle]</a>");
    }
  }
  array_push($links, "<a href=\"http://salilab.org/\">Sali Lab</a>");
  print join($links, " |\n");
  print "\n</p>\n</div>\n";
}

function write_footer() {
  print "</div>\n</body>\n</html>\n";
}
