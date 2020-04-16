```
作者: benjamin (哈~) 看板: benjamin
標題: NoCeM 簡介
時間: Fri Mar  5 05:03:38 1999


                        NoCeM 簡介

1.NoCeM簡介

  NoCeM的發音為(No See 'Em, 也就是No See Them), 其主要的目的是處理現在
  常見的 SPAM, 適用的對像相當廣泛, 從 news server 的管理員到一般閱讀
  NEWS 的使用者都可加入.

  每位使用者可將他在網路上認為不值得閱讀(或值得閱讀的文章), 整理出來,
  按照一定的格式 (NoCeM有特別規定格式), 將相關的資料(message-id)製成一
  篇文章 (在NoCeM中稱為notice), 利用各種管道將這些資料散播給其他的使用
  者, 散播的管道並沒有限制, mailing list, web, news都是常見的方式.

  其他的使用者則可接受這些notice, 接受發文者的建議, 閱讀或者拒看文章,
  使用者有絕對的權利可以選擇是否接受這些notice, 如果使用者信任發出
  notice的使用者, 那麼可以選擇接受他的建議, 拒看某些文章, 反之, 使用者
  也有充份的自由可以選擇不加理會.

  特別重要的一點, 所有的notice必須經過發文者的 PGP 簽名, 如此可以確定
  notice不會被任何人偽造或竄改.

2.實際的做法及應用

  上面提到使用者接到 notice 後可以接受建議拒看文章, 實際的應用可分:
  一般使用者與 news server 管理者來說明

  一般使用者:

  每位使用者可以自行設定接受那些人送出的 notice, 當使用者在閱讀文章之前
  先取得 nocem notice, 然後針對使用者的設定做 PGP 簽名的檢查, 檢查通過
  後才繼續進行下一個步驟.

  剛才提到 notice 可分為建議閱讀(show), 或拒看(hide), 來更動使用者的
  .newsrc檔, 凡是被歸為hide的文章, 會在 .newsrc上做上標記, 使得使用者接下
  來閱讀文章時, 該文章會自動被過濾掉或被設為已讀, 至於歸類為show的文章
  也做特別標記將其顯示出來

  以目前來講, hide是較常被使用的, 所有的spam都會被歸類為hide.

  因此, 在使用者閱讀時, 就已經看不到這篇文章, 不過, 這篇文章依然存在 news
  server之中, 並不會被刪除

  現在已有發展相關的程式, 供使用者選用, 不過只限定支援有.newsrc檔的 news
  reader  程式, 如:tin......
  一般使用者除了可以指定接收那些人送出的notice外, 也可設定這些notice所
  能影響及控制的newsgroup.

  news server管理者:

  主要的原理和上面所寫的大致相同, 不過原來修改.newsrc檔的動作, 現在變成
  直接將文章從 news server 中刪除, 同樣地, 管理者有絕對的權利選擇是否加入
  NoCeM 或是加入後只接受某些人的notice.

3.NoCeM的優缺點

  A.節省網路頻寬

    與cancel message比較, NoCeM佔了較少的頻寬及空間; cancel message
    的特性是一對一, 一篇spam就要對應一篇cancel message, 對硬碟來說
    spam被刪除了, 但卻多了一篇cancel message; 對網路頻寬來說, spam
    與cancel message都被傳遞, 兩者都浪費了頻寬, 如果從 news 流量來分析,
    spam與cancel message的量是接近 1:1 的

    不過NoCeM則不相同, 一篇notice中可以包含相當多的message-id, 與
    cancel相比, 省去了大量傳遞所佔的頻寬

  B.較少爭議, 更多彈性

    cancel message最常引人爭議的是其流傳範圍廣(當然, 經過適當的設定
    後, 可以改善),  甚至有少數使用者將之視為網路上的白色恐怖,  冠上一頂
    限制言論自由的大帽子, 為了免除這些顧慮, NoCeM可算是一種不錯的解法
    管理者可自由選擇是否加入NoCeM, 或是選擇接收何人的文章, 沒有人強迫全
    世界的管理者都要參加, 因為這是每個人的選擇

    當然, 或許有使用者要問, 如果我的news server管理者加入NoCeM, 那麼
    他不就間接的影響到我所讀的文章??  的確, 當管理者接受notice刪除 news
    server 中的文章後, 使用者的確無法在這個server上看到這篇文章, 不過
    使用者還是有充份的選擇權, 可以選擇別的news server閱讀, 甚至,
    換一個ISP!!

4.NoCeM的延伸

  NoCeM notice可以由任何人發出, 也可以被任何人接收, 根據這樣的概念, 就發
  展出pseudo moderator; 當某個使用者對某些特定的討論區有著較多的經驗時,
  可以自告奮勇的送出notice, 針對特定的討論區內的文章作一篩選, 建議其他的
  那幾篇可以詳加閱讀, 那幾篇不值得一看, 而其他的讀者, 如果相信他的經驗和
  判斷, 就可以設定接收他的notice, 這就是pseudo moderator.

  一般的newsgroup moderator產生的方法較為嚴謹, 所有的文章必需經過moderator
  認同後, 才能送上newsgroup, 因此並不是所有的newsgroup都有moderator來為文
  章品質把關.

  事實上pseudo moderator的觀念, 就像一般BBS中每個板的板主一樣, 只不過
  pseudo moderator並沒有權利刪除任何文章, 除非讀者接受他送出的notice, 因
  此, 較不易發生 "板主管理不公"  "板主濫砍文章",  諸如此類的情形, 只要讀
  者懷疑或不能接受pseudo moderator的判斷, 隨時可以停止接受pseudo moderator
  送出的notice, 讀者可以自由的做出決定.

5.NoCeM的推廣與應用

  目前NoCeM的機制已有一定的規模, 而且在USENET上以推行有一段時間,
  但是應用程式的普及才能使更多使用者受惠, 應用程式的部份可以分程兩部份

一般使用者:

  目前主要還是提供給使用.newsrc檔的news reader程式使用, 如tin....
  其他像是netscape outlook等, 目前似乎沒有看到相關的支援

  除此之外, tw.* 有不少的文章是由 BBS 送出, 不可否認的 BBS 也應被視為
  使用者發表文章的管道與工具, 它所帶來的改變也影響了不少使用者參與 NEWS
  討論的習慣, 目前 NoCeM 與 BBS(innbbsd) 的整合已經有了初步的結果,
  Leeym 已經使得 innbbsd 可以支援 NoCeM, 並且正在測試, 為 NoCeM 與 BBS
  的結合與催生踏出一大步

news server管理者:

  這方面相關的程式已經頗多, 也支援數種不同的news server程式, 以inn為例
  http://sites.inka.de/~bigred/sw/c-nocem.html
  c-nocem可支援inn2.x以下的板本

6.其他

  A.目前 NoCeM notice大部份都是透過 news 傳送, alt.nocem.misc是主要傳送
    這些notice的newsgroup

7.參考

  http://www.cm.org
  http://sites.inka.de/~bigred/sw/c-nocem.html
  http://cae.ce.ntu.edu.tw/~leeym/nocem/ncm-innbbsd-latest.tgz
  news:news.admin.nocem
  news:alt.nocem.misc

--
※ Origin: 網際新世界 ◆ From: 163.28.64.246
```
