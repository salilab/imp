<?php
   #includes
   include("conf.inc.php");

   function ok_path($f) {
     $rp= realpath($f);
     return strncmp($f, $home, strlen($home)) == 0;
   } 

   function PrintFile($f) {
      if (! ok_path(f)) {
       error_log("Bad file path "+f);
       return;
      }
      $co = GetContentsFile('./' . $f);
      $lines = preg_split("/\n/",$co);
      foreach ($lines as $ln_num => $line) {
         echo str_replace('$title', "IMP Community", $line) . "\n";
      }
   }
   
   # Get content of a directory based on ter
   function lsDir($path,$ter) {
      if (! ok_path(f)) {
       error_log("Bad file path "+f);
       return;
      }
      $files = array();
      $dir_handle = @opendir($path) or die("Unable to open $path");
      while ($file = readdir($dir_handle)) {
         if (strchr($file,$ter)) {
            $file = $path."/".$file;
            array_push($files,$file);
         }
      }
      rsort($files);
      closedir($dir_handle);
      return($files);
   }

   function GetContentsFile($f) {
      if (! ok_path(f)) {
       error_log("Bad file path "+f);
       return;
      }
      $fd = fopen($f,"r");
      if (!$fd) {
        print "can't open file ";
        error_log("Can't open file");
      }
      if (filesize($f) > 0) {
        $co = fread($fd,filesize($f));
      } else {
        $co="";
      }
      fclose($fd);
      return($co);
   }

   function print_page_header() {
      PrintFile("header.txt");
      print <<<END
  <div id="container">
   <table class="tcon">
  <tr>
         <td class="twel" colspan="3">
END;
   }

   function print_page_footer() {
      print "   </table>\n";
      print "     </div>\n\n";
      print "<div id=\"footer\">\n";
      PrintFile("footer.txt");
      print <<<END
</div>

</body>
</html>
END;
   }

?>
