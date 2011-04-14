<?PHP
   include("conf.inc.php");
   # Navigator on top of page
   function GetLinks () {
      echo "<ul>
               <li><a href=\"".$GLOBALS["home"]."\">imp</a></li>
               <li><a href=\"doc.html\" class=\"link\">doc</a></li>
               <li><a href=\"http://salilab.org/imp/wiki/\" class=\"link\">wiki</a></li>
               <li><a href=\"groups.html\">groups</a></li>
               <li><span class=\"e-mail\">imp at salilab.org | contact</span></li>
            </ul>";   
   }

   function GetDocumentation() {
print <<< END
<ul>
<li>IMP 1.0 stable release
  <ul>
  <li><a href="1.0/tutorial/">Introductory tutorial</a></li>
  <li><a href="1.0/doc/html/">Manual</a></li>
  </ul>
</li>

<li>Latest nightly build
  <ul>
  <li><a href="http://salilab.org/imp/nightly/doc/html/">Manual</a></li>
  </ul>
</li>
</ul>
END;
   }
   
?>
