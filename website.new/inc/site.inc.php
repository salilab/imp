<?PHP
   include("inc/conf.inc.php");
   # Navigator on top of page
   function GetLinks () {
      echo "<ul>
               <li><a href=\"".$GLOBALS["home"]."\">imp</a></li>
               <li><a href=\"https://salilab.org/imp/doc/doxygen/\" class=\"link\">doc</a></li>
               <li><a href=\"https://salilab.org/imp/wiki/\" class=\"link\">wiki</a></li>
               <li><a href=\"?page=pubs\">publications</a></li>
               <li><a href=\"?page=groups\">groups</a></li>
               <li><span class=\"e-mail\">imp at salilab.org | contact</span></li>
            </ul>";   
   }
   # <li><a href=\"?page=imp\" class=\"implink\">IMP</a></li>

   # GET PUBLICATIONS FOR IMP
   function GetPublications () {
      dl("dom.so");
      dl("xmlreader.so");
      $artypes = array ('citations','methods','applications','related');
      foreach ($artypes as $at_num => $at) {
        echo "<a href=\"#$at\">$at</a>&nbsp;&nbsp;";
      }
      foreach ($artypes as $at_num => $at) {
         $dir = $GLOBALS['home_dir']."/pubs/".$at; 
         if ($at == "citations")    { echo "<a name=\"$at\"></a><h4>To cite IMP use these articles:</h4>"; }
         if ($at == "methods")      { echo "<a name=\"$at\"></a><h4>IMP methods:</h4>"; }
         if ($at == "applications") { echo "<a name=\"$at\"></a><h4>IMP applications:</h4>"; }
         if ($at == "related")      { echo "<a name=\"$at\"></a><h4>Articles related to IMP:</h4>"; }
         $files = lsDir($dir,".xml");
         #echo "Files are $files";
         foreach ($files as $f_num => $file) {
	    #echo $file;
	    $parsed=PubMedParser($file);
            #echo "begin parsed";
	    #echo $parsed;
	    #echo "end parsed";
            echo "<div id=\"pub$at\">".$parsed."</div>";
         }
      }
   }
      
   function PubMedParser($file) {
      #ini_set('display_errors',1);
      #error_reporting(E_ALL|E_STRICT);
      $reader = new XMLReader();
      #return("");
      $nauthors = 0;
      if(!$reader->open($file)){
        error_log("Can't open file");
	return("");
      }
      $mesh='';
      $author_list='';
      $keywordlist='';
      $key='';
      $pubdate=1;
      $lastname="";
      $forename="";
      $title="";
      $journal="";
      $pages="";
      $sillyness="";
      while ($reader->read()) {
         if($reader->nodeType == XMLReader::ELEMENT ) { $name = $reader->name; }
         if (in_array($reader->nodeType, array(XMLReader::TEXT, XMLReader::CDATA, XMLReader::WHITESPACE, XMLReader::SIGNIFICANT_WHITESPACE)) && $name!='') {
            $value= $reader->value;
         }
         if($reader->value != '') {
           #error_log($value);
            if($name == 'PMID' and $key == "") { $key = $value; }           
            if($name == 'ArticleTitle'){ $title =  $value; }
            if($name == 'Title') { $journal = $value; }
            if($name == 'PubDate') { $pubdate = 1; }
            if($name == 'Year' && $pubdate == 1){ $year = $value;; $pubdate = 0; }       
            if($name == 'Volume') { $volume = $value; }
            if($name == 'Issue') { $number = $value; }
            if($name == 'MedlinePgn') { $pages = $value; }
            if($name == 'Affiliation') { $note = 'Affiliaton: ' . $value; }
            if($name == 'Language') { $language = $value; }
            if($name == 'ISSN') { $isbn = $value; }
            if($name == 'AbstractText') { $Abstract = $value; }
            if($name == 'DescriptorName') { $mesh .= $value . '; '; }           
            if($name == 'Keyword') { $keywordlist .= $value . '; '; }
            if($name == 'LastName') { $lastname = $value; }
            if($name == 'ForeName') { $forename = $value; }
            if($lastname != '' && $forename != '') {
               $nauthors++;
               if ($nauthors == 1) { $first_author = $lastname.', '.substr($forename,0,1); }
               $author_list .= $lastname.', '.substr($forename,0,1).'; ';
               $lastname = '';
               $forename = '';
               $initials = '';
            }       
         }
         if ($reader->nodeType == XMLReader::END_ELEMENT) {
            $name = '';
            $value = '';
         }
      }
      $title = "<a href=\"http://www.ncbi.nlm.nih.gov/pubmed/".$key."\">$title</a>";
      $reference = "$author_list. ($year) &#8220;$title&#8221; <i>$journal</i> <b>$volume</b>:$pages";
      #$reference = $file;
      return($reference);
   }    
   
?>
