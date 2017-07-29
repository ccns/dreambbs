CCNS Eazy-Reader and RSS feed for BBS System

這是一個讓BBS的文章能方便且完整的在網路上呈現，並且能紀錄文章點擊次數，還有新文章時能自動的發布到RSS。

系統需求：PHP 5.3.3 以上
            * php5-sqlit3
          Apache(需修改設定 User Group 皆需修改成bbs)
            * mod_rewrite

文件結構：

.
|-- ANSIcolorcover.php
|-- click2open.gif
|-- config.php
|-- cover.php
|-- db.php
|-- favicon.ico
|-- JS
|   |-- ansi.js
|   |-- blink.js
|   |-- jquery.lazyload.mini.js
|   \-- screen_width.js
|-- JS_unmin(無壓縮之js)
|   |-- ansi.js
|   |-- blink.js
|   `-- screen_width.js
|-- LICENSES
|-- README
|-- robots.txt
|-- RSS.php
|-- start.sql
|-- style
|   \-- style.css
|-- uaocode.php
\-- ucs2.txt

配置說明：
1.將縮需要的系統需求安裝起來後，記得將apache的httpd.conf將User Group皆設定成bbs才可以讀取bbs所產生的檔案。

2.利用指令將sqlite配置好並且將其移動到你想要的地方
  sqlite3 [db filename] < start.sql

3.設定config.php 將db路徑設定好 即可

CCNS Dream BBS 2010 是以 Wind's Top BBS 為起始，並參考諸位前輩的智慧結晶改版而來，所有財產均屬於原作者。

本模組適用於 itoc 所維護的 Maple-itoc 及 gaod 所維護的 Wind's Top所衍伸發展之 BBS 系統, 並應可經小部分修改後在其他版本之 BBS 系統上使用。

附上 BBS 部分的程式版本資訊(族譜)

Phoenix Bulletin Board System           Version: 3.00-4.0
Copyright (C) 1993, 1994                Ming-Feng Chen, Ji-Tzay Yang
                                        Tsun-Yi Wen

秘密情人資訊站                          Version: 3.1-4.0
Copyright (C) 1994                      簡顯鑑, 劉佳峰

MapleBridge Bulletin Board System       Version: 2.36
Copyright (C) 1994-1995                 Jeng-Hermes Lin, Hung-Pin Chen
                                        SoC, Xshadow

SunOfBeach Bulletin Board System        Version: 2.36
Copyright (C) 1996                      Kaede, woju

Wind's Top Bulletin Board System        Version: 2.3 Rev. 981101
Copyright (C) 1998                      Yu-Chung Tsao, virtu

Wind's Top Bulletin Board System        Version: 3.02 Rev. 20040424
Copyright (C) 2000-2004                 visor, statue, Jerics, yab, verit

Wind's Top Bulletin Board System        Version: 3.10 Rev. 20081002
Copyright (C) 2004-2008                 Hung-Yi Chen (gaod)

DreamLand Bulletin Board System         Version: 3.10 Rev. 20091120
Copyright (C) 2010                      Pang-Wei Tsai (cache)

開發理念

主要分成兩個部分, PHP 的模組以及 BBS 端部分
可以依照看板的屬性(公開,好友,秘密)決定是否要允許文章好讀
板主可以從看板設定中設定是否要開啟該看板的 RSS feed 功能

[範例] 含有圖片超鏈結的文章
http://bbs.ccns.cc/DLbbs-Change/A16C8LJV

[範例] 含有 Youtube 超鏈結的文章
http://bbs.ccns.cc/DLbbs-Change/A16C8LVH

[範例] RSS feed
http://bbs.ccns.cc/DLbbs-Change.xml

會有這個想法，主要是想在無須轉寄或者重新複製貼上的狀況下
，保留 bbs 的風貌在網路上。網頁部分亦轉換成 utf-8 的編碼
，並結合了 RSS 提供訂閱。

好讀網頁上亦有 "推到 Plurk/推到 Facebook/推到 Twitter" 
與社群網站結合的功能

未來是否可以變成雙向bbs，此部份還在構思當中。
