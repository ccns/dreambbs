# Coding Style

本頁說明 DreamBBS 的 coding style (不含 indentation style)。

Indentation style 的說明，請見 [[INDENT]]。

## 語法
- 語法要符合 10 年前最新的 ISO C 標準 (C11)，但不應使用不被最新 ISO C++ 標準或草案 (C++23) 所支援的語法
    - 至 2019-09-01 為止的程式碼，在語法上符合 C99 而已經不符合 C90，已不必再繼續維持 C90 語法
- 需要支援 C++ 時，僅考慮過去 10 年內的 ISO C++ 標準 (最舊到 C++14)，不必考慮更久遠的標準
- 新的程式碼不能將最新 ISO C++ 標準中的關鍵字當作變數／函式／型別名
    - 至 2019-09-01 為止沒有轉移使用 C++ 的計畫，不須完全相容標準 C++ 語法
    - 不過，至 2020-02-24 為止已基本相容 C++20 語法，可通過 `g++-8` 或 `clang++-6` 編譯並正常執行
- 可以使用 GNU C extensions；
  但最好在不使用 GNU C extensions 時也能夠編譯
    - 目前 (2022-03-19) 僅使用 GCC 和 Clang 編譯器，而它們都支援 GNU C extensions

## 人類語言的使用

### 程式碼與註解
- 原則上一律使用英文
- 不應使用其它語言的拉丁化文字
- 為求用法的一致，遇到用詞上有英式與美式之別時，原則上一律使用美式用法
- 如用詞包含附加符號，原則上不省略附加符號

但以下情況例外：

- 用於顯示的字串
    - 目前此專案尚待支援介面上的國際化
- 從其它程式專案引進的註解
- 舊有註解
- 直接引用的文字註解
- 尚未有正式英文名稱或尚不能以英文精準描述的概念
- 官方名稱原文並非英文的專有名詞 (但有英文名稱時須列出補充)

上述例外情況中，若有必要，可再補充人工翻譯至英文的文字。

如使用的非英文語言使用拉丁字母系統作為文字，則須標明所使用的語言。

### Identifiers
基本上適用程式碼與註解的規則，再加上以下規則：
- 僅能使用 7-bit ASCII 字元

### 說明文件
- 可使用其它語言，惟使用語言非英文時，原則上應標明文件所使用的語言
    - 例外：舊有說明文件可不標明所使用的語言

## 區域變數的使用
- 應透過限制變數的 scope 而非重用變數來節省記憶體的使用量
    - 限制變數 scope 有利於編譯器分析變數的使用狀況，利於讓編譯器重新利用不使用的變數的記憶體空間；  
      而重用變數不利於編譯器分析變數的使用狀況
- 不應一次將所有變數宣告於函式定義的開頭
    - 應善用 block scope 變數以及 C99 的迴圈 scope 變數
    - 可依可讀性的需要，而在須使用之處時再定義變數

**Good:**
```cpp
for (int i = 0, n = get_n(sth); i < n; ++i) {
    code;
}
```
**Bad:**
```cpp
int i, n;
n = get_n(sth);
for (i = 0; i < n; ++i) {
    code;
}
```

- 在維持易讀性的前提下，儘可能不要定義暫時變數，尤其是不要定義未使用的變數；既有的未使用變數則應移除
- 非得使用暫時變數時，則儘可能使用 `const`

**Good:**
```cpp
char buf[32];
const char *str = "<anonymous>";
const char *const name = get_name();
if (name) {
    strlcpy(buf, name, sizeof(buf));
    str = buf;
}
process(str);
```
**Bad:**
```cpp
char buf[32];
char *str = "<anonymous>";
char *name = get_name();
size_t len;
if (name)
    len = strlcpy(str = buf, name, sizeof(buf));
process(str);
```

## 全域變數的使用
- 欲僅宣告全域變數並於稍後定義時，應使用 `extern`
- 減少與避免全域變數的使用
    - 不要使用全域變數回傳函式執行結果；盡量使用 `return` 或 output arguments

**Good:**
```cpp
bool func(void)
{
    if (do_task() == TASK_SUCCESS)
        return true;
    return false;
}

void process(void)
{
    if (func())
        do_sth();
    else
        do_sth_else();
}
```
**Bad:**
```cpp
static bool ok = false;

void func(void)
{
    if (do_task() == TASK_SUCCESS)
        ok = true;
    else
        ok = false;
}

void process(void)
{
    func();
    if (ok)
        do_sth();
    else
        do_sth_else();
}
```

-   - 如未能完全避免全域變數的使用，則應將用於同一功能的全域變數以 struct 組織起來

## 可讀性與可移植性
- 程式碼不應造成 compiler 發出容易解決的 warning
    - 對於用語言標準難以解決的 compiler warning，如果使用 GNU C extension 可容易解決，就使用 GNU C extension；  
      如果還是難以解決，就暫不解決，等待新的語法標準或新的 GNU C extensions
- 不應假設函式的回傳值必為某值
- 不應為了節省記憶體的使用，而將函式的指標參數所指向的 struct 暫時用作其他型別資料的 buffer
    - 此能避免改動相關程式後出現 buffer 大小不足的狀況

**Good:**
```cpp
int func(Struct *obj)
{
    FILE *fp;
    {
        char path[LENGTH];
        get_path(path);
        if (!(fp = fopen(path, "r")))
            return 1;
    }
    code_about_obj;
    fclose(fp);
    return 0;
}
```
**Bad:**
```cpp
int func(Struct *obj)
{
    FILE *fp;
    get_path((char *)obj);
    if (!(fp = fopen((char *)obj, "r")))
        return 1;
    code_about_obj;
    fclose(fp);
    return 0;
}
```

- 不要使用避免或依賴編譯器最佳化的 workarounds
- 不要以破壞可讀性的方式手動最佳化運算式
    - 現代許多編譯器已經能夠自動最佳化運算式（`gcc` 及 `clang` 在 `-O0` 下也會最佳化）

**Good:**
```cpp
int y = get_value();
int x = 31 * y;
```
**Bad:**
```cpp
int y = get_value();
int x = (y << 5) - y;
```

-   - 例外：可以用 arithmetic right shift 實作除數為 2 的冪次的 integer floored division/modulo
-   -   - `lhs >> rhs` 的 `lhs` 為負時，依據 C99 及 C++11 標準會產生 implementation-defined 的結果；依據 C++20 標準則會產生 arithmetic right shift 的結果

**Good:**
```cpp
int y = get_value();
int x = y >> 5;
```
**OK:**
```cpp
int y = get_value();
int x = (int)floor(y / 32.0);
```

- 避免撰寫不必要的程式分支
    - 避免 control hazard

**Good:**
```cpp
x = 0;
```
**Bad:**
```cpp
if (x != 0)
    x = 0;
```

**Good:**
```cpp
free(ptr);
```
**Bad:**
```cpp
if (!ptr)
    free(ptr);
```

- 根據 ISO C 與 ISO C++ 標準，`free(NULL)` 不具有任何作用，無須手動進行空指標檢查。

**Good:**
```cpp
flag &= (int)~FLAG_X; // 確保 `FLAG_X` 為無號整數且寬度不比 `unsigned int` 窄時，bit mask 有 sign extension
```
**Good:**
```cpp
flag &= ~((flag & 0) | FLAG_X); // 確保 bit mask 的寬度不比 `flag` 窄
```
**Bad:**
```cpp
if (flag & FLAG_X)
    flag ^= FLAG_X;
```

- 避免 boilerplate code，以減少 code size
    - 需要增加新功能時，盡量使用或擴充既有的函式，不要複製原有函式

**Good:**
```cpp
void func(const char *str_task)
{
    do_sth(str_task);
}
```
**Bad:**
```cpp
void func(void)
{
    do_sth("sth");
}

void func2(void)
{
    do_sth("sth_else");
}
```

## Macro 的使用
- 不要定義實作過於複雜的 macro 來處理容易解決的 C 語法問題
    - 例如：不要用 macro 生成 `malloc` 回傳指標的轉型（parse 實作過於複雜），而應直接手寫轉型
- 如果定義了較為複雜的 macro，應該使用註解解釋背後邏輯
    - 參考 `include/cppdef.h`
- 不應為了過舊的編譯環境或編譯器而將程式邏輯複雜化
    - 目前 (2022-03-19) 主要考量的編譯環境為 Linux；  
      考量的 Linux 版本最舊為 4.18，glibc 版本最舊為 2.28
    - 目前 (2022-03-19) 所考量的編譯器，Clang 版本最舊為 13，GCC 版本最舊為 11

## Binary compatibility
- 目前 (2022-03-19) 考量的編譯環境的系統為 32-bit 及 64-bit x86 架構
- 在會被讀出／寫入 binary file 的資料結構中，不應使用 `long`, `time_t`, 以及其它會因編譯環境架構而有不同大小的資料型別
    - 參見 `include/struct.h`
    - 目前 (2022-03-19) 已無在相關資料結構中使用這些資料型別
- 應當標註會被讀出／寫入 binary file 的資料結構
    - 至 2020-02-24 為止，所有相關資料結構都已標註上 `DISKDATA(raw)`
- 在會被讀出／寫入 binary file 或是 shared memory 的資料結構中，不應使用指標型別
    - 目前 (2022-03-19) 已無在相關資料結構中使用指標型別

## Header 的使用
- 不同支程式使用的 header 應該分開，以方便控制特定程式的編譯環境

**Good:**

`a.h`:
```cpp
#include "lib.h"
void do_sth(void);
```
`b.h`:
```cpp
#include "lib.h"
```
`lib.h`:
```cpp
void do_lib(void);
```
`a.c`:
```cpp
#include "a.h"

void do_sth(void) { }

int main(int argc, char *argv[]) { }
```
`b.c`:
```cpp
#include "b.h"

static void do_sth(void) { }

int main(int argc, char *argv[]) { }
```
`lib.c`:
```cpp
void do_lib(void) { }
```

**Bad:**

`main.h`:
```cpp
void do_lib(void);
#ifdef A_C
void do_sth(void);
#endif
```
`a.c`:
```cpp
#define A_C
#include "main.h"

void do_sth(void) { }

int main(int argc, char *argv[]) { }
```
`b.c`:
```cpp
#define B_C
#include "main.h"

static void do_sth(void) { }

int main(int argc, char *argv[]) { }
```
`lib.c`: 同前

- 避免在原始碼中自行宣告函式；統一使用 `#include`
    - 自行宣告容易有型別錯誤，而且用 C++ 編譯時沒有統一 `extern "C"` 的使用時容易發生 linker errors
- 決定一段宣告所屬的 header 時，先依循「用途」，再依循「語法類型」
    - 泛用的宣告才可僅依循「語法類型」決定其所屬的 header

## Library 使用
- 應先了解函式各個參數以及回傳值的意義，再使用該函式，以避免誤用而造成邏輯錯誤
- 使用功能相似的 library/system 函式的考量重點
    - 除了以效能或安全為重點的情況下，如果在 BBS 中已經實作了所需要的功能的函式，就使用它
        - 參見 `lib/*.c`
    - 原則上，以在編譯環境中最可能存在的函式為優先
    - 優先度高到低：C standard library 函式 > GCC built-in 函式 > glibc 專有函式 = POSIX 系統函式 > *NIX 系統函式 > 外部 library 函式
    - 在一般情況下，如果使用某兩個函式寫出的程式碼差不多一樣複雜，使用優先度高的函式；  
      否則，使用讓程式碼較簡潔的函式；  
      但如果編譯環境可能缺少該函式，就依序使用其它優先度高的函式作為後備

**Good:**
```c
int diff = strncasecmp(str1, str2, LENGTH);
```
- `strncasecmp()` 在 glibc 2.5 前就已存在，可假設編譯環境有此函式

**Bad:**
```c
int diff = 0;
const char *ptr1 = str1;
const char *ptr2 = str2;
int len = LENGTH;
while (len--) {
    char ch1 = *ptr1;
    char ch2 = *ptr2;
    if (ch1 >= 'A' && ch1 <= 'Z')
        ch1 += 'a' - 'A';
    if (ch2 >= 'A' && ch2 <= 'Z')
        ch2 += 'a' - 'A';
    diff = ch1 - ch2;
    if (diff || !*ptr1 || !*ptr2)
        break;
    ++ptr1;
    ++ptr2;
}
```

- 冗長

**Worse:**
```c
int diff;
char buf1[LENGTH+1], buf2[LENGTH+1];
strncpy(buf1, str1, LENGTH);
buf1[LENGTH] = '\0';
strncpy(buf2, str2, LENGTH);
buf2[LENGTH] = '\0';
for (char *ptr = buf1; *ptr; ptr++)
    *ptr = tolower(*ptr);
for (char *ptr = buf2; *ptr; ptr++)
    *ptr = tolower(*ptr);
diff = strncmp(buf1, buf2, LENGTH);
```

- 又冗長又浪費記憶體，而且不使用 variable length array (C99 或 GNUC++ 之功能) 的話，字串長度會有限制

-   - 在以效能或安全為重點的情況下，優先使用效能或安全較好的函式；  
      但如果編譯環境可能缺少該函式，就依序使用其它效能或安全較好的函式作為後備，  
      最後應使用在各個編譯環境中都能夠確定存在的函式作為最終後備
    - 如果選擇了多個函式，而所寫出的程式碼會被重複利用，應將該段程式碼獨立定義成函式
- 不應將外部 libraries 放進 BBS 程式碼中，而應該以 git submodule + symbolic link 的方式引用
    - 原則上，不維護不是由自己維護的程式碼

## Directory layout
- 盡量保持整個專案結構的扁平；限制 Makefile 的層次在 3 層以下
    - 目前 (2022-03-19) 整個 DreamBBS 專案只有 `scripts/wsproxy/` 一個內層目錄，但沒有自己的 Makefile