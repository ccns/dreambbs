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

    - Both K&R and Allman style are allowed, but they should not be mixed within a file, especially within a function

**Bad:**
- Mix Allman style with K&R style
```
if (cond)
{
} else {
}
```

## Line continuation
- The indentation size of continued lines should be 4 spaces
- Argument lists and initializer-lists
    - The elements should be indented or be aligned with the start of the list
    - The end of list without elements in the same line should be indented less that the elements
- No wrapping at operators with higher grouping precedence without wrapping at operators with lower grouping precedence

**Good:**
```c
if (sth_long > 42
    && sth_else_long)
```
```c
if (sth_long
    > (42 && sth_else_long))
```
**Bad:**
```c
if (sth_long
    > 42 && sth_else_long)
```
```c
if (sth_long > (42
    && sth_else_long))
```

- No line breaks between unary, prefix, and suffix operators other than `.` and `->` and their operand
- If needed, line breaks must come before the suffix operators `.` and `->`

**Good:**
```c
sth_long
    .member++;
```
**Bad:**
```c
sth_long.
    member++;
```

- If needed, line breaks should come before the binary operators other than assignments and comma

**Good:**
```c
if (sth_long
    && sth_else_long)
```
**Bad:**
```c
if (sth_long &&
    sth_else_long)
```

- For the condition operator, if needed, line breaks should come before `:` or both after `?` and before `:`
    - This allows programmers to easily distinguish the conditions from the values

**Good:**
```c
cond ? sth : sth_cond ? sth_else : else_sth
```
```c
cond ? sth
    : else_cond ? sth_else
    : else_sth
```
```c
cond ?
    sth
    : else_cond ?
    sth_else
    : else_sth
```
**Bad:**
```c
cond ?
    sth : else_cond ?
    sth_else : else_sth
```
```c
cond
    ? sth : else_cond
    ? sth_else : else_sth
```
**Worse:**
```c
cond
    ? sth :
    else_cond
    ? sth_else :
    else_sth
```

## Alignment
- 如果 code 附近的相似 code 已被對齊排版，這段 code 應該對齊排版
- 新的 code 不應該對齊排版，除非附近相似的 code 已被對齊排版

## Condition lists
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

## `goto` label
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

**Bad:**
```c
int func(void)
{
    sth:
}
```
```c
int func(void)
{
    if (sth)
    {
    sth:
    }
}
```

**Worse:**
```c
int func(void)
{
    if (sth)
    {
        sth:
    }
}
```

## `switch`
- `case` label 的 indentation 須與 `switch` 的 code block 的 `{`/`}` 的所在行相同
- `case` label 內的 code 須 indent

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
case sth:
        code
    }
}
```

**Bad:**
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
code
}
```
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
    case sth:
        code
    }
}
```

## `{` & `}`
- C code
    - 在 parameter list 及 condition list 後，與 `{` 之間要有空白字元
    - 在 identifier 後，與 `{` 之間要有剛好一個 space
    - 在 identifier 前，與 `}` 之間要有 spaces

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

**Bad:**
```c
} else
{
```

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

**Bad:**
```c
{code}
```
```c
{ code}
```
```c
{code }
```
```c
{}
```

## `(` & `)`
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
- `( sth)`
- `(sth )`
- `(sth...漢 )`
- `( 漢...sth)`

## `,`
- C code
    - 在 initializer-list 中，`,` 不能在被註解掉的 element list 之首，應從註解移出到前一個 element 後
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

**Bad:**
```c
sth,sth
```
```c
sth , sth
```
```c
macro(, ,)
```
```c
macro(sth, , sth)
```
```c
int a[] = {sth, sth /* , sth */};
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

**Bad:**
```c
code;sth
```
```c
code ; sth
```
```c
for (; ;)
```
```c
for (sth; ; sth)
```
```c
for (s;t;h)
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
**Bad:**
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

## Auto indentation commands
- The commands for performing auto indentation for MapleBBS-itoc
    - Outdated for DreamBBS

```
find . -name "*.c" -execdir sh -c "indent -st -bap -bli0 -i4 -l79 -ncs -npcs -npsl -fca -lc79 -fc1 -ts4 {} > {}.new " \;
find . -name "*.c.new" -exec prename -f -v 's/\.new//' {} \;
find . -name "*.h" -execdir sh -c "indent -st -bap -bli0 -i4 -l79 -ncs -npcs -npsl -fca -lc79 -fc1 -ts4 {} > {}.new " \;
find . -name "*.h.new" -exec prename -f -v 's/\.new//' {} \;
find . -name "*.h" -exec sed -i 's/\t/    /g' {} \;
find . -name "*.c" -exec sed -i 's/\t/    /g' {} \;
```
