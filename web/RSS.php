<?php
/****************************************************

          BBS Rss Feed

 ****************************************************
 作者：renn999.bbs <AT> bbs.ccns.ncku.edu.tw
       cache.bbs   <AT> bbs.ee.ncku.edu.tw 
 
 運行需求: PHP 5.3 以上
           Apache(需修改設定 User Group 皆需修改成bbs)
           (並將此文件的權限修改為755 OR 777)
 ****************************************************/

date_default_timezone_set("Asia/Taipei");
include('config.php');
$bbs_path='/home/bbs/brd/';
$brd=$_GET['brd'];
//$brd = $argv['1'];
//for cli mode debug
if($brd==""){
  exit('LOSS_BROAD_NAME');
}elseif(!preg_match("/^[a-zA-Z0-9.\-_]+$/",$brd)){
  exit("THIS_IS_NOT_A_VIALD_BROAD");
}elseif(!is_dir($bbs_path.$brd)){
  exit('NO_SUCH_BROAD_NAME');
}

$filename = '/home/bbs/.BRD';
$handle = fopen ( $filename, 'rb' );
$brd_num = filesize($filename)/256;
for($i=0;$i<$brd_num;$i++){
  fseek($handle, $i*256);
  $brd_list  = fread($handle,116);
  $brd_id    = stripnull(substr($brd_list,0,12));
  if($brd==$brd_id){
    if(ord($brd_list['107']) & 128){
      exit('THIS_BRD_IS_NOT_PUBLIC');
    }
    if(!(ord($brd_list['115']) & 32)){
      exit('THIS_BRD_RSS_FEED_CLOSED');
    }
    $brd_disp=substr($brd_list,16,40);
    $brd_disp=b2u(stripnull($brd_disp));
	$have_brd=true;
    break;
  }
  unset($brd_id,$brd_list);
}
if(!$have_brd)
  exit('NO_SUCH_BROAD');
unset($filename,$handle,$brd_num,$brd_id,$brd_list,$pos);

$filename=$bbs_path.$brd.'/.DIR';
if (!file_exists($filename)) {
  exit('READ_BROAD_ERROR');
}

$all_post=filesize($filename)/256;
$fp=fopen($filename,'rb');

$post_num=($all_post<10)?$all_post:10;

for($i=1; $i <= $post_num ; ++$i){
  fseek($fp, ($all_post-$i)*256);
  $post_list  = fread($fp,256);
  $post_under = ord($post_list['7']);
  if((ord($post_list['7']) & 4) or (ord($post_list['6']) & 48) or (ord($post_list['4']) & 128)){
    ++$post_num;
    continue;
  }
  $post_id[]    = substr($post_list,12,8);
  $post_title[] = b2u(htmlspecialchars(stripnull(substr($post_list,183,73))));
}

header('Content-type: application/xml; charset=utf-8');
echo '<?xml version="1.0" encoding="utf-8" ?>',"\n";
?>
<rss version="2.0"
xmlns:content="http://purl.org/rss/1.0/modules/content/"
xmlns:wfw="http://wellformedweb.org/CommentAPI/"
xmlns:dc="http://purl.org/dc/elements/1.1/"
xmlns:atom="http://www.w3.org/2005/Atom"
xmlns:sy="http://purl.org/rss/1.0/modules/syndication/"
xmlns:slash="http://purl.org/rss/1.0/modules/slash/"
>

  <channel>
  <title><?php echo SITE_NAME;?>RSS - <?=$brd?>版</title>
  <description><?=$brd_disp['str']?></description>
  <link>http://<?=WEBSITE_DOMAIN?>/<?=$brd?>.xml</link>
  <atom:link href="http://<?=WEBSITE_DOMAIN?>/<?=$brd?>.xml" rel="self" type="application/rss+xml" />
  <language>zh-tw</language>
  <generator><?=SITE_NAME?>RSS Ver0.1</generator>
<?php
foreach($post_id as $key => $id):
  $post_path  =$bbs_path.$brd.'/'.substr($id, -1).'/'.$id;
  $fpp        =fopen($post_path,'rb');
  $post = '';
  while($current_line = fgets($fpp)){
    $current_line = str_replace("\r",'',$current_line);
    $current_line = preg_replace('/\\x1b\\[([0-9;]*?)m/','',$current_line);//清掉所有色碼
    $current_line = b2u($current_line);//轉換BIG5->UTF8
    $post .= $current_line['str'];
    if(preg_match('/^(時間|發信站): .*? ?\(?([A-Za-z0-9: ]+)\)?/',$current_line['str'],$time)){
      $post_time[$key][] = $time['2'];
    }
    if (preg_match('/^※ Origin.*◆ From.*$/',$current_line['str'])){
      break;
    }
    unset($current_line);
  }
?>
  <item>
    <title><?php echo htmlspecialchars(stripnull($post_title[$key]['str']));?></title>
    <pubDate><?php echo date(DATE_RSS, strtotime(trim($post_time[$key][0])));?></pubDate>
    <link><?php echo 'http://',WEBSITE_DOMAIN,'/',$brd,'/',$id ;?></link>
    <guid isPermaLink="false"><?=$id?></guid>
    <description><?php echo'<![CDATA[',str_replace(array("\n","\r"),'',htmlspecialchars($post)),']]>';?></description>
    <content:encoded><?php echo '<![CDATA[',str_replace(array(' ',"\n","\r"),array('&nbsp;','<br />',''),htmlspecialchars($post)),']]>';?></content:encoded>
  </item>
<?php endforeach;?>
  </channel>
</rss>
