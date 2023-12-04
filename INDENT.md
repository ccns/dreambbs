# Indentation Style

This page describes the indentation style for the DreamBBS project.

## Overview
- C & shell code
    - Indent with 4 spaces
    - No tabs

**Good:**
```c
{
    sth
}
```
**Bad:**
- 2-space (Maple3)
```c
{
  sth
}
```
- Hard tab
```c
{
[tab]sth
}
```
-
    - No trailing spaces
    - Use `\n` for line breaks

<p/>

-   - Both K&R and Allman style are allowed, but they should not be mixed within a file, especially within a function
    - Allman style should be used only in existing codes indented using Allman style.
    - From DreamBBS v4 and on, only K&R style will be allowed.

## Line continuation
- The indentation size of continued lines should be 4 spaces
- Argument lists and initializer-lists
    - The elements should either be indented or be aligned with the start of the list
- No wrapping at operators with higher grouping precedence without wrapping at operators with lower grouping precedence

**Good:**
```c
if (sth_long > 42
    && sth_else_long
    )
```
```c
if (sth_long
    > (42 && sth_else_long)
    )
```

- No line breaks between unary, prefix, and suffix operators other than `.` and `->` and their (indexed/called/*etc.*) operand
- If needed, line breaks must come before but not after the suffix operators `.` and `->`

**Good:**
```c
sth_long
    .member++;
```

- If needed, line breaks should come before but not after the binary operators other than assignments and comma

**Good:**
```c
if (sth_long
    && sth_else_long
    )
```

- For the condition operator, if needed, line breaks should come before `:` or both after `?` and before `:`
    - This allows programmers to easily distinguish the conditions from the values

**Good:**
```c
cond ? sth : sth_cond ? sth_else : else_sth
```
-   - Use the above form only if all the condition and result expressions contain no non-empty pairs of `(` & `)`
```c
cond ? (sth & 1)
    : else_cond ? (sth_else | 0x80000000)
    : else_sth
```
-   - Otherwise use the above form only if the condition expressions contain no non-empty pairs of `(` & `)`
```c
(cond_0 || cond_1) ?
    sth
    : (else_cond_0 && else_cond_1) ?
    sth_else
    : else_sth
```
-   - Otherwise use the above form

## Space around operators

- Binary operators and the `?`-`:` operator other than `,` should have a space between the operator and the operands (if on the same line).
- No spaces between unary, prefix, and suffix operators other than `.` and `->` and their (indexed/called/*etc.*) operand
- No spaces between `.` and `->` and their field operand.

## Alignment
- 如果 code 附近的相似 code 已被對齊排版，這段 code 應該對齊排版
- 新的 code 不應該對齊排版，除非附近相似的 code 已被對齊排版

## Labels
- Label 須單獨一行出現，或與多個 labels 在同一行內單獨出現

**Good:**
```c
switch (cond)
{
default: case 1: case 2: case 3:
    sth();
    break;
case -1:
    sth_else();
    break;
case 0:
    ;
}
```

### `goto` labels
- `goto` label 的 indentation 須與所在 function 的 code block 的 `{`/`}` 的所在行相同

**Good:**
```c
int func(void)
{
    if (sth)
    {
sth:
    }
}
```

### `case` labels
- `case` label 的 indentation 須與 `switch` 的 code block 的 `{`/`}` 的所在行相同
- `case` label 內的 code 須 indent
- `default` label 與其它 labels 同在一行時，`default` 須在最前方

**Good:**
```c
switch (sth)
{
case sth:
    code
}
```
```c
switch (sth)
{
case sth:
    if (sth)
    {
default: case sth_else:
        code
    }
}
```

**Bad:**
```c
switch (sth)
{
case sth:
    if (sth)
    {
    case sth_else: default:
        code
    }
}
```

## `{`, `}`, & code blocks
- C code
    - 在 parameter list 及 condition list 後，與 `{` 之間要有空白字元
    - 在 identifier 後，與 `{` 之間要有剛好一個 space
    - 在 identifier 前，與 `}` 之間要有 spaces

<p/>

- 在代表 code block 的 `{` 之後，以及 `}` 之前，要有空白字元

**Good:**
```c
{
    code
}
```
```c
{
}
```
- One-liner
```c
{ code }
```
```c
{ }
```

- 如果 `else`/`else if` 和之前的 `}` 在同一行裡，`else` 後面的 `{` 也要在這一行裡

**Good:**
- K&R style
```c
} else {
```
- K&R style (Stroustrup)
```c
}
else {
```
- Allman style
```c
}
else
{
```

### `do`-`while`
- 必須使用 `{` 與 `}` 作為 code block
- `while` 必須與 `}` 在同一行中
- `while` 後的 `;` 不得單獨一行出現

**Good:**
```c
do {
    sth();
} while (cond);
```

**Bad:**
```c
do {
    sth();
}
while (cond);
```

### `if`/`for`/(`do`-)`while`/`switch` 的 code block
- `{` 與 `}` 不得在同一行中
- 以下任一狀況成立則必須使用 `{` 與 `}`：
    - 為 `do`-`while` 的 code block
    - Condition list 含有換行
    - Code block 內含有 `goto`/`case` labels
    - K&R style is used and the code block is not empty.
- 除了必須使用 `{` 與 `}` 的狀況外……
    - 若 code block 為空，應以單獨一行的 `;` 取代
    - 若使用 Allman style 且僅包含一句不換行的陳述式，可不使用 `{` 與 `}`，但該陳述式須單獨一行出現

**Good:**
```c
while (cond) {
label:
    sth;
}
```
```c
while (cond)
    sth;
```
```c
while (cond)
    ;
```

**Bad:**
```c
while (cond) { sth; }
```
```c
while (cond) sth;
```
```c
while (cond);
```

### Function 的 code block
- 若有 `goto`/`case` labels，則 `{` 與 `}` 必須在不同行中
- 若函式宣告的參數清單無換行，且若 code block 為空或僅包含一句不換行的陳述式，`{` 與 `}` 可在同一行中

**Good:**
```c
int func(int x)
{
    return x;
}
```
```c
int func(int x) { return x; }
```
```c
int func(int x)
{ return x; }
```
```c
void func(void)
{
}
```
```c
void func(void) { }
```
```c
void func(void) {
loop:
    goto loop;
}
```

**Bad:**
```c
void func(void) { loop: goto loop; }
```

### `{`-`}`&ndash;enclosed Initializer-list

- If there are any _non-enclosed_ line wraps within the initializer-list, there should be a line wrap immediately after the `{` and another immediately before the `}`. The `}` should have the same indentation level as the line containing the `{` and should have a following line wrap after immediately following closing brackets and punctuations.
- Otherwise, there should not be any spaces immediately after the `{` or immediately before the `}`.

**Good:**
```c
Conf opts[] = {
    {.type = CONFTYPE_STR, LISTLIT(Option, {
        .name = "sample_configuration",
        {.val_s = "sample text"},
    })},
    {.type = CONFTYPE_INT, LISTLIT(Option, {.name = "answer", {.val_i = 42}})},
};
```

## `(` & `)`

### Condition lists
- `if`/`for`/`while`/`switch` 與 condition list 之間應有剛好一個 space
- _例外_：用來對齊 `else if` 後的 condition list 時，`if` 後可以有 6 個 spaces

**Good:**
```c
[if/for/while/switch] (sth)
```
```c
if      (sth)
...
else if (sth)
```

- Condition list 含有換行時，結尾的 `)` 前須換行，而縮排與 code block 內的縮排相同

**Good:**
```c
if (1 + 1 == 2
    && *p != '\0'
    )
{
    do_sth();
}
```

- Otherwise, if there are any _non-enclosed_ line wraps within a `(`-`)`&ndash;enclosed complete expression, there should be a line wrap immediately after the `(` and another immediately before the `)`. The `)` should have the same indentation level as the line containing the `(` and should have a following line wrap after immediately following closing brackets and punctuations.

**Good:**
```c
z = (
    2 * f((x != 0) ? get_y() / x : get_y(),
        true)
    + g((y != 0) ? get_z() / y : get_z(), &TEMPLVAL(Point2d, {
        .x = get_x(),
        .y = get_y(),
    }))
    + 1
);
```

- 二元非賦值位元運算子，其運算元若直接包含與其不同的二元運算子，應以 `(` 與 `)` 將此運算元包圍
- 二元非賦值運算子，其運算元若直接包含二元位元運算子，應以 `(` 與 `)` 將此運算元包圍

**Good:**
```c
-x | (y & 1) | (z + 3)
```
```c
(-x | (y & 1) | (z + 3)) >= 42
```
```c
res = -x | (y & 1) | (z + 3)
```

- 條件運算子 `?`...`:` 的條件式直接包含二元運算子時，應以 `(` 與 `)` 將其包圍

**Good:**
```c
!x ? 2 * a
    : (b > 42) ? b - 1
    : 2 * c + 1
```

- 條件運算子 `?`...`:` 的結果式直接包含賦值運算子時，應以 `(` 與 `)` 將其包圍

**Good:**
```c
!x ? (a *= 2)
    : (b > 42) ? (b -= 1)
    : (c = 2 * c + 1)
```

- 避免非必要的 `(` 與 `)`

**Good:**
```c
int (*parr)[N] = arr;
++(*parr)[base + idx];
```

**Bad:**
```c
int ((*parr)[N]) = arr;
++((*parr)[base + idx]);
```

- `(` 後與對應的 `)` 前的 space 的使用應該一致，要就不使用 space，要就使用剛好一個 space
- _例外_：在字串中的 `(` 後與對應的 `)` 前，如果一端是全形字，而另一端是半形字，不論全形字的那端有沒有使用 space，半形字的那一端都可以使用一個 space

**Good:**
- `( sth )`
- `(sth)`
- `( )`
- `()`
- `(sth...漢)`
- `( sth...漢 )`
- `(漢...sth)`
- `( 漢...sth )`
- In the strings
    - `"( sth...漢)"`
    - `"(漢...sth )"`

**Bad:**
- `(sth...漢 )`
- `( 漢...sth)`

<br/>

- 函式宣告的參數清單與函式呼叫的引數清單，若含有換行：
    - 每個參數／引數後皆須換行
    - 開頭的 `(` 後換行，結尾的 `)` 前不換行。
    - 結尾的 `)` 後緊接的閉括號與標點之後須換行。
- _例外_：函式呼叫的引數清單，僅有最後一個引數含有換行時，無須另外加入換行。

**Good:**
```c
void log_level(
    const Logger *logger,
    enum LogLevel level,
    const char *format,
    ...)
{
   /* Codes */
}
```
```c
log_level(
    logger,
    LOGLV_DEBUG,
    "Value in decimal: %d\n"
    "Value in hexadecimal: %x\n",
    value,
    value);
```
```c
get_configure(CONFTYPE_INT, &TEMPLVAL(ConfOptionInt, {
    .name = "timeout",
    .value_default = 5,
}));
```

## `[` & `]`

The same rules for the function argument list apply.

```c
return arr[(x >= 0 && x < COUNTOF(arr)) ?
    x
    : 0];
```

## `,`
- C code, in a `{`-`}`&ndash;enclosed initializer-list
    - `,` 不能在被註解掉的 element list 之首，應從註解移出到前一個 element 後
    - If there is a line wrap after the last element, there should be a `,` right before the line wrap.
    - There should not be an optional trailing `,` immediately before the closing `}` without any line wraps in between.
- Code
    - `,` 前不能有額外的 spaces
    - `,` 後和 expression 之間要有至少一個 space
    - `,` 後不能有不用於對齊的額外 spaces
    - 扣除空白字元之後，`,` 不能在一行之首
    - _例外_：特殊功能字串不必符合此規則

**Good:**
```c
sth, sth
```
```c
sth,
sth
```
```c
macro(,,)
```
```c
macro(sth,, sth)
```
```c
int a[] = {sth, sth, /* sth */};
```
```c
int a[] = {
    sth,
    sth,
    /* sth, */
};
```

## `;`
- C & shell code
    - `;` 前不應有 spaces，除非 `;` 前是行內註解
    - `;` 後，與 expression 之間，應有空白字元
    - 扣除空白字元之後，`;` 可以單獨成行
    - _例外_：ANSI escape sequence 相關的註解與字串不必符合此規則

**Good:**
```c
code; sth
```
```c
code;
sth
```
```c
for (;;)
```
```c
for (sth;; sth)
```
```c
for (sth; sth; sth)
```

## Preprocessor directives
- The indentation size should be 2 spaces
- The indentation should be consistent within the whole block
- The indentation can come before or after `#`
- The indentation can be omitted only for short blocks

**Good:**
```c
#if COND
#define STH sth
#endif
```
```c
#if COND
  #define STH sth
#endif
```
```c
#if COND
#  define STH sth
#endif
```
**Bad:**
```c
#if COND
... [Large directive block without indentation]
#endif
```
- The indentation should be independent of the indentation of C code

**Good:**
```c
#if sth
int func(void)
{
    if (sth)
    {
  #if sth_else
        code;
  #endif
    }
}
#endif
```
