```
NoCeM-innbbsd-patch (以下簡稱 ncm-innbbsd)
CHANGES

ver 0.80
1-9-04	支援 GPG
	(感謝 AlanSung)

ver 0.71
5-23-01	補上一些錯誤訊息, 修正 NCMparse()

ver 0.70
5-11-01	修正 NCMparse 的 "END NCM BODY" 部分
	(感謝 lcr.bbs@server.old-castle.org)

ver 0.69
11-5-00	修正 NCMparse() 安全性問題
	(感謝 DarkKiller.bbs@abpe.org)

ver 0.68
11-4-00	修正 NCMparse() 安全性問題
	(感謝 DarkKiller.bbs@abpe.org)

ver 0.67
11-3-00	修正 NCMparse() 安全性問題
	(感謝 DarkKiller.bbs@abpe.org)
	預設改回使用 PGP 驗證, 不用 PGP 者請自行 #undef PGP5
	(感謝 Amis.bbs@bbs.ee.nthu.edu.tw)

ver 0.66
5-19-00	用 stdarg.h 取代 varargs.h, 修正 SOLARIS 的 va_start() 問題
	(感謝 edwardc@concorde.upma.net)

ver 0.65
4-26-00	加入 search_issuer_type() , 分別接收同一 issuer 不同 type 的 notice
	增加 MAXSPAMMID 到 10000
	新增 LINELEN, 設定為 512
	預設改為不使用 PGP 驗證.
	NCMregister() 完成階段性統計任務, 不想加入統計, 或者連至 TANet 速度
	不快的站台, 可在 nocem.h 設定 #define DONT_REGISTER 不列入統計

ver 0.63
2-13-00 加入對 M3-innbbsd 的 patch
        (感謝 mat.bbs@fall.twbbs.org)
        M3 的 BODY 已將 \r 處理掉, 所以原先抓行尾的 \r 將會抓不到
        更正 v0.6 文件 INSTALL.nocem 4.b 錯誤的地方
        * 對使用 v0.6 而無法運作的 M3 站長們致歉 *

ver 0.62
1-24-00 加入 tcpcommand() 的 Solaris patch
        (感謝 edwardc.bbs@drogon.seed.net.tw)
        加入 PGP5 的 #define 以及 #if .. #endif, 可選擇關閉 PGP5 驗證

ver 0.6
1-17-00 重新整理一下 NCMregister(), 改用 gethostname()
        免得有 bbsadm 連 Makefile 的 ADMINUSER 都懶得改
        加入對 Maple3 的安裝說明

ver 0.51
5-21-99 新增 NCMregister(), 紀錄 NoCeM-innbbsd 使用情形.

ver 0.5
3-6-99  更新文件
3-5-99  將 strcasestr() 換成 strstr().
3-4-99  de 掉一個很嚴重的 bug: SPAMMID 不限制陣列大小的話將會超用,
        蓋掉其他變數.. 造成 innbbsd 掛掉.
        (特別像是吃到 nocem@newsgate.nctu.edu.tw 一篇 notice 的 Count 七百多)
        整理程式碼, Beta release..

ver 0.43
3-3-99  修改 receive_nocem(), 發現不是 notice 即改 receive_article()

ver 0.42
3-2-99  修改 cancel_article_front(), 允許 Issuer 砍信, 並配合修正說明文件

ver 0.41
3-2-99  修改 NCMverify();

ver 0.4
3-1-99  細部整合, 變數處理, 改用 NCMfunction() 統一格式提高程式可讀性

ver 0.3
2-28-99 新增 PGP verify 之簽名檢查
        新增 NCMupdate 自動更新設定檔
        改由 search_group 來比對文章, 減少 DBfetch 的次數, 大幅提高效能.

ver 0.2
2-27-99 新增 ncmperm.bbs 之 Issuer 權限控制

ver 0.1
2-26-99 編寫 ncm-innbbsd 主程式
        分析 NCM 0.9 之 NCM HEADERS
        分析 NCM 之 NCM BODY
        分析 NCM 0.93 之 NCM HEADERS
2-25-99 參考 ncmspool-0.92.3b 及 c-nocem-3.2 決定 ncm-innbbsd 之運作方式
```
