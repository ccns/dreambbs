<?php

/****************************************************

      Big5UAO To UTF-8 Conventer And雙色字偵測

 ****************************************************
 作者：renn999.bbs <AT> bbs.ccns.ncku.edu.tw
 
 運行需求: PHP 5.3 以上
 註譯：本文件參考並修改至ZTrem，ucs2.txt也是
 usage：b2u( $b5_str )
 return:array
  str:UTF-8str
  dcw:boolean 是否有雙色字
 ****************************************************/

function b2u( $b5_str ) {
  global $uaotable;
  if ( strlen($uaotable) < 64382 ) {
    $handle = fopen ( "ucs2.txt", 'rb' );
    $uaotable = fread( $handle, 64382 );
    if ( strlen($uaotable) != 64382 ) {
      die ("Unicode-At-On Table Loading Error!");
    }
    fclose($handle);
  }
  $i=0;
  $utf_str['str'] ='';
  $utf_str['dcw']=false;

  while ($i < strlen($b5_str)):
    if(ord($b5_str[$i]) < 128):
      $utf_str['str'] .= str_replace('@','&#64;',$b5_str[$i]);
    else:
      unset($i1,$i2,$shift);
      $i1=ord($b5_str[$i]);
      if(!isset($b5_str[++$i])){
        $utf_str['dcw']=true;
        break;
      }
      $i2=ord($b5_str[$i]);
      $shift = (($i1 << 8) | $i2) - 33088;
      $i1=ord($uaotable[$shift*2]);
      $i2=ord($uaotable[$shift*2+1]);
      $c=$i1 << 8 | $i2;
      if($c>=128 && $c <= 2047):
      //2code utf-8
        $utf_str['str'] .= chr(192 | ($c >> 6));
        $utf_str['str'] .= chr(128 | ($c & 63));
      elseif($c >= 2048 && $c <= 65535):
      //3code utf-8
        $utf_str['str'] .= chr(224 | ($c >> 12));
        $utf_str['str'] .= chr(128 | (($c >> 6) & 63));
        $utf_str['str'] .= chr(128 | ($c & 63));
      /*elseif($c >= 65536 && $c <= 1114111):
      //4code utf-8 不大可能出現 先註解掉
        $utf_str['str'] .= chr(240 | ($c >> 18));
        $utf_str['str'] .= chr(128 | (($c >> 12) & 63));
        $utf_str['str'] .= chr(128 | (($c>> 6) & 63));
        $utf_str['str'] .= chr(128 | ($c & 63));*/
      else:
        $utf_str['str'] .= '?';
      endif;
    endif;
    ++$i;
  endwhile;
  $utf_str['str'] = preg_replace('/([\x{00A1}-\x{00FF}\x{FF80}-\x{FF9F}])/u','$1 ',$utf_str['str']);
  return $utf_str;
}

function preg_callback_url($matches){
  $url = $matches[0];
  $url_rc=preg_replace('/\\x1b\\[([0-9;]*?)m/','',htmlspecialchars_decode(str_replace('&#64;','@',$url)));
  if(preg_match('!^http://www\.youtube\.com/watch\?(?:.*&)?v=([^&]*)(&.*)?$!',$url_rc,$i)){
    return '<object width="425" height="344"><param name="movie" value="http://www.youtube.com/v/'.$i['1'].'?fs=1&amp;hl=en_US&amp;rel=0"></param><param name="allowFullScreen" value="true"></param><param name="allowscriptaccess" value="always"></param><embed src="http://www.youtube.com/v/'.$i['1'].'?fs=1&amp;hl=en_US&amp;rel=0" type="application/x-shockwave-flash" allowscriptaccess="always" allowfullscreen="true" width="425" height="344"></embed></object><br /><a href="'.$url_rc.'" rel="nofollow">'.$url.'</a>';
  }elseif(preg_match('!^http://.*\.(?:jp(?:e?g|e)|png|w?bmp|gif|tif{1,2})!i',$url_rc)){
    return '<img src="'.$url_rc.'" /><br /><a href="'.$url_rc.'" rel="nofollow">'.$url.'</a>';
  }elseif(preg_match('!^http://.*\.swf!i',$url_rc)){
    return '<object classid="clsid:D27CDB6E-AE6D-11cf-96B8-444553540000" codebase="http://download.macromedia.com/pub/shockwave/cabs/flash/swflash.cab#version=5,0,0,0"><param name="movie" value="'.$url_rc.'"><param name="quality" value="high"><param name="wmode" value="transparent"><embed src="'.$url_rc.'" quality="high" pluginspage="http://www.macromedia.com/shockwave/download/index.cgi?P1_Prod_Version=ShockwaveFlash" type="application/x-shockwave-flash"></object><br /><a href="'.$url_rc.'" rel="nofollow">'.$url.'</a>';
  }else{
    return '<a href="'.$url_rc.'" rel="nofollow">'.$url.'</a>';
  }
}

function preg_callback_url_index($matches){
  $url = $matches[0];
  $url_rc=preg_replace('/\\x1b\\[([0-9;]*?)m/','',htmlspecialchars_decode(str_replace('&#64;','@',$url)));
  return '<a href="'.$url_rc.'" rel="nofollow">'.$url.'</a>';
}

function header_call_back($matches){
  $str  = '<div id="header" style="overflow:hidden">';
  $str .= '<div id=\'headerANSI\' class=\'bg44\'>';
  if(preg_match('/^(.*) (看板|站內): ([0-9A-Za-z-._]+)/',$matches['1'],$i)){
    $str .= '<span style=\'float:right\'><span class=\'fg34 bg47\'>&nbsp;'.$i['2'].'&nbsp;</span><span class=\'fg37\'>&nbsp;'. $i['3'] .'&nbsp;</span></span>';
    $matches['1']=$i['1'];
  }
  $str .= '<span class=\'fg34 bg47\'>&nbsp;作者&nbsp;</span><span class=\'fg37\'>&nbsp;'.$matches['1'].'</span><br />';
  $str .= '<span class=\'fg34 bg47\'>&nbsp;標題&nbsp;</span><span class=\'fg37\'>&nbsp;'.$matches['2'].'</span><br />';
  $str .= '<span class=\'fg34 bg47\'>&nbsp;時間&nbsp;</span><span class=\'fg37\'>&nbsp;'.$matches['3'].'</span></div>';
  $str .= '<div id="headerNOANSI">'.str_replace("\n",'<br />',$matches['0']).'</div>';
  $str .= '</div><!--[if lte IE 7]><br /><![endif]-->';//處理ie 和 ff等其他瀏覽器說不清楚的關係...
  
  return $str;
}

function stripnull($str){
  if(($pos=strpos($str,chr(0)))!==FALSE)
    $str=substr($str,0,$pos);
    
  return $str;
}
