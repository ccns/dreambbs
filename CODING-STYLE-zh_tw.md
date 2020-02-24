# Coding Style

本頁說明 DreamBBS 的 coding style (不含 indentation style)。

Indentation style 的說明，請見 [INDENT](INDENT)。

## 語法
- 語法要符合 10 年前最新的 ISO C 標準 (C99)，但不應使用不被最新 ISO C++ 標準或草案 (C++2a) 所支援的語法
    - 目前 (2019-09-01) 的程式碼，在語法上符合 C99 而已經不符合 C90，不必再繼續支援 C90 語法
- 需要支援 C++ 時，僅考慮過去 10 年內的 ISO C++ 標準 (最舊到 C++11)
- 新的程式碼不能將最新 ISO C++ 標準中的關鍵字當作變數／函數／型別名
    - 目前 (2019-09-01) 沒有轉移使用 C++ 的計畫，不須完全相容 C++ 語法
    - 目前 (2020-02-24) 已基本相容 C++2a 語法，可通過 `g++-8` 或 `clang++-6` 編譯並正常執行
- 可以使用 GNU C extensions；
  但最好在不使用 GNU C extensions 時也能夠編譯
    - 目前 (2019-09-01) 僅使用 GCC 和 Clang 編譯器，而它們都支援 GNU C extensions

## 程式邏輯
- 程式碼不應造成 compiler 發出容易解決的 warning
    - 對於用語言標準難以解決的 compiler warning，如果使用 GNU C extension 可容易解決，就使用 GNU C extension；  
      如果還是難以解決，就暫不解決，等待新的語法標準或 GNU C extensions
- 應透過限制變數的 scope 而非重用變數來節省記憶體的使用量
    - 限制變數 scope 有利於編譯器分析變數的使用狀況，可讓編譯器重新利用不使用的變數的記憶體空間；  
      而重用變數不利於編譯器分析變數的使用狀況
    - 不應一次將所有變數宣告於函數定義的開頭；應善用 block scope 變數以及 C99 的迴圈 scope 變數

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

- 不應為了在邏輯上節省記憶體使用量，而將函數的指標參數所指向的 struct 暫時用作其他型別資料的 buffer

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

- 不要使用全域變數回傳函數執行結果；盡量使用 `return` 或 output arguments

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

- 避免 boilerplate code，以減少 code size
    - 需要增加新功能時，盡量使用既有的函數，不要複製原有函數

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

- 不要使用避免或依賴編譯器優化的 workarounds
- 不要手動優化運算式；現代許多編譯器已經能夠自動優化運算式（`gcc` 及 `clang` 在 `-O0` 下也會優化）

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
int x = (int)floor(y / 32.0);
```
**OK:**
```cpp
int y = get_value();
int x = y >> 5;
```

- 不要定義實作過於複雜的 macro 來處理容易解決的 C 語法問題
    - 例如：不要用 macro 生成 `malloc` 回傳指標的轉型（parse 實作過於複雜），而應直接手寫轉型
- 如果定義了較為複雜的 macro，應該使用註解解釋背後邏輯
    - 參考 `include/cppdef.h`
- 不應為了過舊的編譯環境或編譯器而將程式邏輯複雜化
    - 目前 (2019-09-01) 主要考量的編譯環境為 Linux, OpenBSD, 以及 FreeBSD；  
      考量的 Linux 版本最舊為 3.10，glibc 版本最舊為 2.17
    - 目前 (2019-09-01) 使用的編譯器為 Clang，考量的版本最舊為 3.4

## Binary compatibility
- 目前 (2019-09-01) 考量的編譯環境的系統為 32-bit 及 64-bit x86 架構
- 在會被讀出／寫入 binary file 的資料結構中，不應使用 `long`, `time_t`, 以及其它會因編譯環境架構而有不同大小的資料型別
    - 參見 `include/struct.h`
- 應當標註會被讀出／寫入 binary file 的資料結構
    - 目前 (2020-02-24) 所有相關資料結構都已標註上 `DISKDATA(raw)`

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

- 避免在原始碼中自行宣告函數，統一使用 `#include`
    - 自行宣告容易有型別錯誤，而且用 C++ 編譯時沒有統一 `extern "C"` 的使用時容易發生 linker errors
- 決定一段宣告所屬的 header 時，先依「用途」，再依「語法類型」
    - 泛用的宣告才可僅依「語法類型」決定所屬的 header

## Library 使用
- 應先了解函數各個參數以及回傳值的意義，再使用該函數，以避免誤用而造成邏輯錯誤
- 使用功能相似的 library/system 函數的考量重點
    - 除了以效能或安全為重點的情況下，如果在 BBS 中已經實作了所需要的功能的函數，就使用它
        - 參見 `lib/*.c`
    - 原則上，以在編譯環境中最可能存在的函數為優先
    - 優先度高到低：C standard library 函數 > glibc 函數 = POSIX 系統函數 > *NIX 系統函數 > 外部 library 函數
    - 在一般情況下，如果使用某兩個函數寫出的程式碼差不多一樣複雜，使用優先度高的函數；  
      否則，使用讓程式碼較簡潔的函數；  
      但如果編譯環境可能缺少該函數，就依序使用其它優先度高的函數作為後備

**Good:**
```c
int diff = strncasecmp(str1, str2, LENGTH);
```
- `strncasecmp()` 在 glibc 2.5 前就已存在，可假設編譯環境有此函數

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

-   - 在以效能或安全為重點的情況下，優先使用效能或安全較好的函數；  
      但如果編譯環境可能缺少該函數，就依序使用其它效能或安全較好的函數作為後備，  
      最後應使用在各個編譯環境中都能夠確定存在的函數作為最終後備
    - 如果選擇了多個函數，而所寫出的程式碼會被重複利用，應將該段程式碼獨立定義成函數
- 不應將外部 library 放進 BBS 程式碼中，而應該以 git submodule + symbolic link 的方式引用
    - 不應該維護不是由自己維護的程式碼

## Directory layout
- 盡量讓整個專案結構扁平化；限制 Makefile 的層次在 3 層以下
    - 目前 (2020-02-24) 整個 DreamBBS 專案只有 `scripts/wsproxy/` 一個內層目錄，但沒有自己的 Makefile