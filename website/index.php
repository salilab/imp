<?PHP
   include("conf.inc.php");
   include("site.inc.php");
   include("misc.inc.php");
   $page = $_GET['page'];
   if (!$page) { $page = "home"; }
?>


<?PHP
   if ($page == "home") {
   } elseif ($page == "imp") {
   } elseif ($page == "groups") {
   } elseif ($page == "doc") {
   } elseif ($page == "download") {
   } else {
     #ini_set('display_errors',1);
     #error_reporting(E_ALL|E_STRICT);
     header("HTTP/1.1 404 Not Found");
     echo "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">";
     echo "<html><head>";
     echo "<title>404 Not Found</title>";
     echo "</head><body>";
     echo "<h1>Not Found</h1>";
     echo "<p>The requested page ";
     # should be OK since it is just being sent back to the user
     echo $page;
     echo " was not found on this server.</p>";
     echo "<hr>";
     echo "</body></html>";

     exit;
   }
?>

<?PHP PrintFile("header.txt"); ?>
  <div id="container">
   <table class="tcon">
  <tr>
         <td class="twel" colspan="3">
            <?PHP
               if ($page == "home") {
                  echo "<h1>Welcome</h1>
                        <div id=\"goal\">";
                  PrintFile("goal.txt");    
                  echo "</div>";
               } elseif ($page == "imp") {
                  #echo "<h2>Integrative Modeling Platform</h2>
                  echo "<div id=\"imp\">";
                  PrintFile("imp.txt");              
                  echo "</div>"; 
               } elseif ($page == "groups") {
                  echo "<h1>Participating groups</h1>
                        <div id=\"groups\">";
                  PrintFile("groups.txt");              
                  echo "</div>"; 
               } elseif ($page == "doc") {
                  echo "<h1>Documentation</h1>
                        <div id=\"docs\">";
                  GetDocumentation();              
                  echo "</div>"; 
               } elseif ($page == "download") {
                  echo "<h1>Download</h1>
                        <div id=\"download\">";
                  PrintFile("download.txt");              
                  echo "</div>"; 
               } else {
                  error_log("Invalid page snuck through"+$page);
               }
            ?>
         </td>
      </tr>
   <?PHP
      if ($page == "home") {
      echo "<tr>
               <td class=\"tnews\">
                  <h3>News</h3>
                  <div class=\"high\">";
                     PrintFile("news.txt");
      echo "      </div>
               </td>
               <td class=\"thres\">
                  <h3>Highlighted research</h3>
                  <div class=\"high\">";
                     PrintFile("hresearch.txt");
      echo "      </div>
               </td>
               <td class=\"thdev\">
                  <h3>Highlighted development</h3>
                  <div class=\"high\">";
                     PrintFile("hdevelopment.txt");
      echo "      </div>
               </td>
            </tr>";
      }
   ?>
   </table>
     </div>

<div id="footer">
   <?PHP PrintFile("footer.txt"); ?>
</div>

</body>
</html>
