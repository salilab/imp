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
<li><a href="http://salilab.org/imp/1.0/doc/html/">1.0 release</a></li>
<li><a href="http://salilab.org/imp/nightly/doc/html/">Latest nightly
build</a></li>
</ul>
END;
   }
   
?>
