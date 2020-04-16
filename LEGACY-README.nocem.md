```
# Author:       Yen-Ming Lee <leeym@cae.ce.ntu.edu.tw>
# Start Date:   Thu Feb 25 1999 +0800
# Project:      INNBBSD - NoCeM
# File:         nocem.c nocem.h
#
# Copyright:    Copyright (c) 2000 by Yen-Ming Lee
#
#               Permission to use, copy, modify, and distribute this
#               software for any purpose with or without fee is hereby
#               granted, provided that the above copyright notice and this
#               permission notice appear in all copies.


        innbbsd 由 skhuang@csie.nctu.edu.tw 編寫
        本程式 NoCeM-innbbsd 只是 patch

        本程式感謝 clcheng@CCCA.NCTU.edu.tw 及 stevel@bbs.ee.ntu.edu.tw
        提供寶貴意見.

        NoCeM 之設定檔在 ~bbs/innd/ncmperm.bbs, 格式為 "Issuer Type Perm"
        各欄位 *必須* 使用 tab 隔開. 第一次載入時程式會自動產生設定檔.

        如願接收某一名 Issuer 的 ncm-notice, 請將最後一欄 no 從改為 yes
        然後 ctlinnbbsd reload 重新載入資料即可.
        *請勿* 使用 ve 編輯 ncmperm.bbs , 因為 ve 會將 tab 全部轉為 space

        本程式需配合 pgp5 使用, 安裝 NoCeM-innbbsd-patch 前, 請先安裝 pgp5
        否則請修改 nocem.c , #undef PGP5

        pgp5 的中文文件可在底下幾處找到.
        http://bbs.ee.ntu.edu.tw/boards/BSD/4/62.html
        http://www.sinica.edu.tw/cc/netsrv/dawei/pgp5.0/

        NoCeM 資料請參考 http://www.cm.org
        及 benjamin.bbs@com.neto.net (clcheng) 所寫的說明文件.

--
 Yen-Ming Lee [李彥明]          | http://cae.ce.ntu.edu.tw/~leeym/
 CAE Group, Civil Engineering, NTU, Taipei, Taiwan
```
