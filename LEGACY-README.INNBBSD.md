```
innbbsd Guide                                                  [WindTopBBS]
=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

                          WindTopBBS Ver 3.02

                    BBS <==> News 轉信程式 innbbsd 介紹

               Modified by 元智資工  沈俊興(statue.bbs@bbs.yzu.edu.tw)

=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

                        目      錄

                (一) 轉信 source 修改

                     [a] 設定 src/innbbsd/Makefile
                     [b] 重編執行檔

                (二) 轉信設定檔修改

                     [a] 設定 bbsname.bbs
                     [b] 設定 nodelist.bbs
                     [c] 設定 newsfeeds.bbs
                     [d] 設定 activefile
                     [e] 跑 bbsnnrp


─────────────────────────────────────
(一) 轉信 source 修改
─────────────────────────────────────

[a] 設定 src/innbbsd/Makefile
        Row:14
        ADMINUSER= statue.bbs@bbs.yzu.edu.tw
        Row:20
        BBSADDR = bbs.yzu.edu.tw

[b] 重編執行檔
    cd /home/bbs/src/innbbsd
        make clean
        make link
        make freebsd install


─────────────────────────────────────
(二) 轉信設定檔修改
─────────────────────────────────────

[a] 修改 bbsname.bbs
        bbs.yzu

[b] 設定 nodelist.bbs
   innbbsd 由 nodelist.bbs 控制連線機器.
   bbs.yzu 要和 bbsname.bbs 一樣，自己的機器設定 IHAVE(7777)
        #bbsname        host                    port            ps.
        bbs.yzu         bbs.yzu.edu.tw          IHAVE(7777)     元智大學 BBS
        news.yzu        news.yzu.edu.tw         POST(119)       元智大學 News
        news.nthu       news.cs.nthu.edu.tw     POST(119)       楓橋 News
              ^^^^^^^^^^                ^^^^^^^^         ^^^^^^^
                 tab                      tab              tab
        #自己的 bbsname 要跟 bbsname.bbs 一樣

[c] 設定 newsfeeds.bbs
        #newsgroup                      board           server
        #------------------------------ --------------- ---------
        tw.bbs.admin.installbbs         Installbbs      news.yzu
        yzu.bbs.windtop                 WindTop         news.yzu
        nthu.cs.bbs.plan                MaplePlan       news.nthu
                        ^^^^^^^^^^^^^^^^       ^^^^^^^^^
                              tab                 tab
        #newsgroup 是新聞群組
        #board 是站上的 board name..大小寫注意
        #server 則是 nodelist.bbs 中的 BBSname
    PS.以上三個當有變動時就要 /home/bbs/innd/ctlinnbbsd reload
       編寫時到最後一行一定要多按一個 enter 鍵把游標移至下一行

[d] 設定 active file
        #newsgroup                  站上的最後一封    不管
        tw.bbs.admin.installbbs 0000002014 0000001224 y
        yzu.bbs.windtop 0000000011 0000000001 y
        nthu.cs.bbs.plan 0000003867 0000003692 y
                        ^          ^          ^
                        空白       空白       空白
    PS.如果要加新的 newsgroup 則先 joe 一個 test
        tw.bbs.rec.mud                  0000000000 0000000001 y
    然後 /home/bbs/innd/bbsnnrp -c news.yzu.edu.tw /home/bbs/innd/test
    他就會把那個變成
        tw.bbs.rec.mud                  0000023700 0000019821 y
    PS.-c 是指 reset active files only; don't receive articles
        如果不想這樣用..telnet news.yzu.edu.tw 119 > groups
                       list
                       quit
        然後看看 groups 中的內容
    PS.最後一個參數要用絕對路徑 or 相對於 /home/bbs/
       /home/bbs/innd/bbslink 主動把 out.bntp 中的文章對外送信..

[e] 跑 bbsnnrp

這個 client gateway 讓你 "亂拿 news".
同時可
1. 到 A server 取 a group, 到 B server 取 b group.
   a 和 b 可以相同或不同.
   例如, 到 news.yzu.edu.tw 取 tw.bbs.admin.installbbs, yzu.bbs.windtop
         到 news.cs.nthu.edu.tw 取 nthu.cs.bbs.plan
   設好不同的 active file, 如 news.yzu.act, news.nthu.act
   跑
   innd/bbsnnrp news.yzu.edu.tw /home/bbs/innd/news.yzu.act
   innd/bbsnnrp news.cs.nthu.edu.tw /home/bbs/innd/news.nthu.act

當然大量讀取時必須禮貌地徵求 server 同意.

關於較詳細的說明，請參考 src/innbbsd/README
若想與 nthu.cs.bbs.plan 轉信，請來信給 news@news.cs.nthu.edu.tw。

--
                                                    元智資工 沈俊興
                                        E-Mail: <statue.bbs@bbs.yzu.edu.tw>
```
