<?php
/****************************************************

                   ansi色彩轉換

 ****************************************************
 作者：renn999.bbs <AT> bbs.ccns.ncku.edu.tw
       cache.bbs   <AT> bbs.ee.ncku.edu.tw 
 
 運行需求: PHP 5.3 以上
           Apache(需修改設定 User Group 皆需修改成bbs)
 附加說明：ANSIcolorcover.php ansi色彩轉換
           請盡量不要使用到global var
 ****************************************************/
$prev_dcw='';
$bg='40';
$fg='37';
$light_fc=FALSE;
$sbk=FALSE;

function ANSI($inside){
  global $prev_dcw, $bg, $fg, $light_fc, $sbk, $cc;
  $ansi='';
  $inside=explode("\x1b\x5b",$inside);
  
  foreach($inside as $key=> &$value){
    if($key){
      $value=explode('m',$value,2);
    }else{
      $value=array('',$value);
    }
  }
  unset($value);

  foreach($inside as $key => $value){
    //當$key等於0時不處理色碼 採繼承
    if($key!=0){
      $color = explode(';',$value['0']);
      foreach($color as $color_value){
        switch($color_value):
          case '':
          case '0':
            $light_fc=FALSE;
            $bg='40';
            $fg='37';
            $sbk=FALSE;
            break;
          case '1':
            $light_fc=TRUE;
            break;
          case '5':
            $sbk=TRUE;
            break;
          case ($color_value >= 40 && $color_value <= 47):
            $bg=$color_value;
            break;
          case ($color_value >= 30 && $color_value <= 37):
            $fg=$color_value;
            break;
        endswitch;
      }
    }      
    
    if( $value['1'] ==null)
      continue;

	unset($cc);
    $cc['fg']='fg'.(($light_fc)?'l':'').$fg;
    $cc['bg']='bg'.$bg;
    
    if($sbk)
      $cc['sb']='sbk';
    
    if($prev_dcw!=''){//把雙色字接好
      $value['1'] = substr($value['1'],1);
      $str=b2u($value['1']);
	  $str['str']=$prev_dcw.$str['str'];
    }else{
      $str=b2u($value['1']);
	}
    //處理雙色字
    if($str['dcw']){
      $dcw_f = substr($inside[$key]['1'],-1).substr($inside[$key+1]['1'],0,1);
      $dcw_f = b2u($dcw_f);
      $prev_dcw=$dcw_f['str'];
      $str['str'].='<span class="dcw">'.$dcw_f['str'].'</span>';
    }else{
      $prev_dcw='';
    }
    if(($cc['fg']!='fg37' or $cc['bg']!='bg40' or $sbk) and $str['str']!=''){
    //if($str['str']!=''){
      $str['str']='<span class="'.join(' ',$cc).'">'.$str['str'].'</span>';
    }//clear dummy null span

    $ansi .= $str['str'];
    unset($str,$dcw_f);
  }
  $ansi .= "\n";
  return $ansi;
}
?>
