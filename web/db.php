<?
function hit_count(){
  global $brd,$pid;
  $db = new SQLite3(DB_PATH);
  $db_result = $db->query("SELECT * FROM brd WHERE brd LIKE '$brd'");
  if (!($bid = $db_result->fetchArray())){
    $db->query("INSERT INTO brd (brd) VALUES ('$brd')");
    $bid[0] = $db->lastInsertRowID();
  }
  unset($db_result);
  $db_result = $db->query('SELECT counter FROM counter WHERE BID LIKE \''.$bid['0'] .'\' AND PID LIKE \''.$pid.'\'');
  if ($count = $db_result->fetchArray()) {
    $db->query('UPDATE counter SET counter=counter+1 where BID=\''.$bid['0'].'\' and PID=\''.$pid.'\'');
	return ++$count['0'];
  }else{
    $db->query('INSERT INTO counter (BID,PID,counter) VALUES (\''.$bid['0'].'\',\''.$pid.'\',1)');
	return '1';
  }
  unset($db_result,$db);
}

?>
