<?php
ob_start();
$time = microtime(true);
include('config.php');
include('ANSIcolorcover.php');
//引入big5uao To UTF-8外加雙色字偵測的function php
$socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);                                                     
socket_connect($socket, SITE_DOMAIN, 23);
$out='';
while(1){
  $out.=socket_read($socket,256,PHP_BINARY_READ);
  if(preg_match("/\\x1b\[22\;13H/",$out)){
    $out=str_replace("\x1b[22;13H",'',$out);
    socket_close($socket);
    break;
  }
}
$out=str_replace("\r",'',$out);
$out=explode("\n",$out);

$post='';
foreach($out as $i => $inside){
  if (!$i){
    $inside=substr($inside,23);
  }
  $i++;
  $inside=str_replace(array("\n","\r","\x1b[2J","\x1b[K"),'',$inside);
  $inside=preg_replace(array("/\x1b\[[0-9;]+H/","/\x1b\[[0-9;]+f/"),"\n\n",$inside);
  $inside=htmlspecialchars($inside);
  $inside=preg_replace_callback('!(?:http?|ftp|https):/{2}[\w\.\\x1b\\[\;]{2,}[/\w\:\-\.\?\&\=\#\%\\x1b\\[\;\~\,]*!i','preg_callback_url_index', $inside);
  $post .= ANSI($inside); 
}
unset($out);
$post = str_replace('請輸入代號：             ','',$post);
header("Content-type: text/html; charset=utf-8");
?>
<html>
  <head>
    <link type="text/css" href="http://<?=WEBSITE_DOMAIN?>/style/style.css"  rel="stylesheet" />
    <meta name="title" content="<?=SITE_NAME?>bbs" />
    <meta name="robots" content="noindex,nofollow" />
    <title><?=SITE_NAME?>bbs</title>
    <script type="text/javascript" src="http://ajax.googleapis.com/ajax/libs/jquery/1.4.3/jquery.min.js"></script>
    <script type="text/javascript" src="http://<?=WEBSITE_DOMAIN?>/JS/blink.js"></script>
    <script type="text/javascript" src="http://<?=WEBSITE_DOMAIN?>/JS/screen_width.js"></script>
    <script type="text/javascript" src="http://<?=WEBSITE_DOMAIN?>/JS/ansi.js"></script>

	
</head>
  <body>
    <div id='pbody'>
      <div style="font-weight: bold; text-align: right; width: 100%; font-size: 0.8em;padding:20px 0px;height:20px">
      <button style="float:left" id="removeANSI">純文字模式</button>
      <button style="float:left" id="ANSIcolor">預設格式化內容</button>
    </div>
      <pre><?php
echo $post;
unset($post);
?>
</pre><small>
<span class="fg37">powered by renn999&lt;AT&gt;bbs.ccns.cc DreamLand BBS 2010 共耗時 <?php echo microtime(true) - $time;?>s</span>
</small>
    </div>
  </body>
<script type="text/javascript">

  var _gaq = _gaq || [];
  _gaq.push(['_setAccount', 'UA-18550616-1']);
  _gaq.push(['_setDomainName', '.ccns.cc']);
  _gaq.push(['_trackPageview']);

  (function() {
       var ga = document.createElement('script'); ga.type = 'text/javascript'; ga.async = true;
       ga.src = ('https:' == document.location.protocol ? 'https://ssl' : 'http://www') + '.google-analytics.com/ga.js';
       var s = document.getElementsByTagName('script')[0]; s.parentNode.insertBefore(ga, s);
   })();

</script>
</html>
<?php
ob_end_flush();
?>
