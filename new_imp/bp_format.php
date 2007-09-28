<?php
// what you want each indent to be (e.g. "\t" or "   ")
$indent_tab = "   ";

/* whether you want braces flush with the lines they enclose
(e.g.
	if (a == b)
		{
		a = 1;
		}
 ... instead of ...
	if (a == b)
	{
		a = 1;
	}
*/
$flush_braces = true;

/*
whether you want to open brace on separate line 
e.g.
	if (a == b)
		{
		a = 1;
		}
 ... instead of ...
	if (a == b) {
		a = 1;
		}
*/
$open_brace_on_separate_line = true;


$indent_level = 0;
$indent_str = "";

/*****************************************
SetIndentStr:
	Set the line lead string for the current level of
indentation.
******************************************/

function SetIndentStr()
	{
	global $indent_str, $indent_tab, $indent_level;
	
	$indent_str = "";
	for ($i = 0; $i < $indent_level; $i++)
		$indent_str .= $indent_tab;
	}
	
	
/*****************************************
FormatLine:
	Return a line formatted according to the
current rules.
******************************************/

function FormatLine($line, $line_start)
	{
	global $indent_level, $indent_str, $flush_braces, $open_brace_on_separate_line;
	
	SetIndentStr();
	
	// remove leading white space
	preg_match("/\s*(.*)/", $line, $match);
	$line = $match[1];
	
	if (preg_match ("/namespace.*{/", $line))
		{
		print "ignore namespace\n";
		
		$line = $line_start . $indent_str . $line;
		}
	
	else if (preg_match ("/}\selse\s+{/", $line))
		{
		if ($open_brace_on_separate_line)
			{
			if ($flush_braces)
				{
				$line = $line_start . "$indent_str}";
				$indent_level--;
				SetIndentStr();
				$line .= "\n$indent_str"."else";
				$indent_level++;
				SetIndentStr();
				$line .= "\n$indent_str{";
				}
			else
				{
				$indent_level--;
				SetIndentStr();
				$line = $line_start . "$indent_str}\n$indent_str"."else\n$indent_str{";
				$indent_level++;
				}
			}
		else
			{
			$indent_level--;
			SetIndentStr();
			$line = $line_start . "$indent_str} else {";
			$indent_level++;
			}
		}
		
	else if (preg_match ("/^\s*{\s*$/", $line))
		{
		$indent_level++;
		print "indent level: $indent_level\n";

		if ($flush_braces)
			{
			SetIndentStr();
			}

		if ($open_brace_on_separate_line)
			{
			$line = $line_start . $indent_str . "{";
			}
		else
			{
			$line = " {";
			}
		}
	
	else if (preg_match ("/(.*)\s*{\s*$/", $line, $match))
		{
		$indent_level++;
		print "indent level: $indent_level\n";

		if ($open_brace_on_separate_line)
			{
			$line = $line_start . $indent_str . $match[1] . "\n";
			if ($flush_braces)
				{
				SetIndentStr();
				}
	
			$line .= $indent_str . "{";
			}
		else
			{
			$line = $line_start . $indent_str . $match[1] . " {";
			}
		}
		
	else if (preg_match ("/^\s*}\s*$/", $line))
		{
		$indent_level--;
		print "indent level: $indent_level\n";
		
		if (!$flush_braces)
			{
			SetIndentStr();
			}

		$line = $line_start . $indent_str . "}";
		}
		
	else 
		{
		$line = $line_start . $indent_str . $line;
		}
		
	return $line;
	}
	
	
/*****************************************
FormatFile:
	Read in all lines from given file pointer,
format them, and write them to the other given
file pointer.
******************************************/

function FormatFile($fpr, $fpw)
	{
	$line_start = "";
	while (!feof($fpr))
		{
		$next_line = fgets($fpr);
		fputs($fpw, FormatLine($next_line, $line_start));
		$line_start = "\n";
		}
		
	fputs($fpw, $line_start);
	}
	
	
/*****************************************
ModifyFiles:
	Reformat all .cpp files from the given
directory and store them in the second given
directory.
******************************************/

function ModifyFiles($src_dir, $dst_dir)
	{
	global $indent_level;
	
	if ($dp = opendir($src_dir))
		{
		while (false !== ($file = readdir($dp)))
			{
			if (preg_match ("/.+\.cpp/", $file))
				{
				$fpr = fopen ("$src_dir/$file", "r");
				
				if (file_exists ("$dst_dir/$file"))
					{
					print "File $dst_dir/$file already exists! It cannot be overwritten.\n";
					}
				else
					{
					print "Reformatting $src_dir/$file in $dst_dir/$file ...\n";
					
					$fpw = fopen ("$dst_dir/$file", "w");
					
					$indent_level = 0;
					FormatFile($fpr, $fpw);
					
					fclose($fpr);
					fclose($fpw);
					print "... reformatting done.\n";
					}
				}
			}
			
		closedir($dp);
		}
	else
		{
		print "Unable to open directory $src_dir\n";
		}
	}
	
// #################### main rountine #####################

print "BP_Format\n";
if ($argc != 3)
	{
	print "Formats .cpp files based on parameters that indicate whether you want:\n";
	print "  1) braces flush with the code they encompass;\n";
	print "  2) the open brace on a separate line;\n";
	print "  3) tabs or some number of spaces for indents.\n\n";
	print "Usage: php bp_format.php src_directory dst_directory\n";
	exit(0);
	}
	
$src_dir = $argv[1];
$dst_dir = $argv[2];

for ($i = 0; $i < $argc; $i++)
	{
	print "$i: $argv[$i]\n";
	}
	
ModifyFiles($src_dir, $dst_dir);

?>
