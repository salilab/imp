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
      if (!array_key_exists('TOPDIR', $_ENV)) {
         $topdir = '';
      } else {
         $topdir = $_ENV['TOPDIR'];
         if ($topdir == '.' || $topdir == '') {
           $topdir = '';
         } else {
           $topdir = $topdir . '/';
         }
      }
      foreach ($lines as $ln_num => $line) {
         $rline = str_replace('$title', "IMP Community", $line);
         $rline = str_replace('@TOPDIR@', $topdir, $rline);
         echo $rline . "\n";
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
      PrintFile($_ENV['TOPDIR'] . "/header.txt");
      print <<<END
  <div id="container">
   <table class="tcon">
  <tr>
         <td class="twel" colspan="3">
END;
   }

   function print_page_footer() {
      print "   </td></tr>\n";
      print "   </table>\n";
      print "     </div>\n\n";
      print "<div id=\"footer\">\n";
      PrintFile($_ENV['TOPDIR'] . "/footer.txt");
      print <<<END
</div>

</body>
</html>
END;
   }

function example_file($dir, $file) {
  if (!file_exists("$dir/$file")) {
    die("File $dir/$file does not exist");
  }
  $handle = popen("pygmentize -f html -O cssclass=pygments $dir/$file", "r");
  while(!feof($handle)) {
    print fread($handle, 1024);
    flush();
  }
  pclose($handle);
}

?>
