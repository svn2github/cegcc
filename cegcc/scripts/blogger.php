<?php
error_reporting(E_ALL);
require_once('class.atomapi.php');

function read_atom_xml($filename) {
   $chunksize = 1*(1024*1024);
   $buffer = '';
   $handle = fopen($filename, 'rb');
   if ($handle === false) {
       return false;
   }
   while (!feof($handle)) {
       $buffer = fread($handle, $chunksize);
       flush();
   }
   $status = fclose($handle);
   return $buffer;
}

$atom_filename = 'atom.xml';

echo "<a name=\"blogs\"></a><h2>News;</h2>\n";

/**********************************************************************
  CONSTRUCT A FEED OBJECT AND ENTRY OBJECTS FOR ALL AVAILABLE ENTRIES
**********************************************************************/
// Collect entries from the first feed
$rfeed = read_atom_xml($atom_filename);
if ($rfeed === false)
{
  echo "could not read atom feed\n";
  return false;
}

$feed = new AtomFeed(false, false, $rfeed);

// Create objects from all available entries
$entries = $feed->get_entries();

// Get the first available entry object
$entry = $entries[0];

$maxdisplay = 2;
echo '<h2>Last ' . $maxdisplay . ' Entries</h2>';

$inum = 0;
foreach ($entries as $entry) {
  if (++$inum > $maxdisplay)
    break;

	$link = $entry->get_links('type', 'text/html');
	$link = $link[0]['href'];
	
	// Get the title
	$title = $entry->get_title();
	$title = $title['title'];
	
	$content = $entry->get_content();
	$content = $content['content'];
	$author = $entry->get_author();
	$email = "fixme";
	$author = $author['name'];
	
  $date = $entry->get_modified();

$table1 = <<<PARA
<table width="99%" cellpadding="2" cellspacing="0" border="0" id="table1">
  <tr>
    <td valign="top" bgcolor="#c8c8ee">&nbsp;&nbsp; <font size="4" color="#222222" face="arial,helvetica">
        <b><a>$title</a> </b></font>
    </td>
  </tr>
</table>
<b>posted by <a href="mailto: $email">
    $author</a> on $date </b>
<br>
    $content
<br>
<a href=" $link "><b>Read More...</b></a><br>
<p></p>
PARA;

echo $table1;

}

?>
