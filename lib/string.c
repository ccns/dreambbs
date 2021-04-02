#include "config.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dao.h"
#include "global_def.h"

/* Copy string `src` to buffer `dst` and return the new string end of `dst` (formerly `str_add`) */
GCC_NONNULLS
GCC_RET_NONNULL char *str_pcpy(char *dst, const char *src)
{
    const size_t len = strlen(src);
    memmove(dst, src, len + 1);
    return dst + len;
}

/* Strip ANSI escapes from string `str` and output the result string to buffer `dst`
 * At most `max - 1` characters is output to `dst`, and a `'\0'` string end is always added.
 * If `max` == `1`, output an empty string to `dst`
 * If `max` <= 0, do nothing */
GCC_NONNULLS
void str_ansi(char *dst, const char *str, int max)
{
    const char *tail = dst + max - 1;
    bool ansi = false;

    if (dst < tail)
    {
        for (int ch; (ch = *str); ++str)
        {
            if (ch == '\n')
            {
                /* Skip the rest of `str` */
                tail = dst;
                break;
            }
            else if (ch == '\x1b')
            {
                ansi = true;
            }
            else if (ansi)
            {
                if ((ch < '0' || ch > '9') && ch != ';' && ch != '[')
                    ansi = false;
            }
            else
            {
                *dst++ = ch;
                if (dst >= tail)
                    break;
            }
        }
    }
    if (dst <= tail)
        *dst = '\0';
}

/* Concatenate strings `s1` & `s2` and output the result string to buffer `dst` */
GCC_NONNULLS
void str_cat(char *dst, const char *s1, const char *s2)
{
    const size_t len1 = strlen(s1);
    const size_t len2 = strlen(s2);
    memmove(dst, s1, len1);
    memmove(dst + len1, s2, len2 + 1);
}

/* Compare two strings `s1` and `s2`, ignoring case differences (formerly `str_cmp`)
 * Returns the value of difference between the first different bytes of the two strings or `0` if the strings do not differ */
GCC_NONNULLS
GCC_PURE int str_casecmp(const char *s1, const char *s2)
{
    for (;;)
    {
        int c1 = *s1++;
        if (c1 >= 'A' && c1 <= 'Z')
            c1 |= 0x20;

        int c2 = *s2++;
        if (c2 >= 'A' && c2 <= 'Z')
            c2 |= 0x20;

        const int diff = c1 - c2;
        if (diff || !c1)
            return diff;
    }
}

/* Compare two strings `s1` and `s2`, ignoring case differences and handling DBCS characters
 * Returns the value of difference between the first different bytes of the two strings or `0` if the strings do not differ */
GCC_NONNULLS
GCC_PURE int str_casecmp_dbcs(const char *s1, const char *s2)
{
    bool in_dbcs1 = false;
    bool in_dbcs2 = false;
    for (;;)
    {
        int c1 = *s1++;
        if (in_dbcs1 || IS_DBCS_HI(c1))
            in_dbcs1 = !in_dbcs1;
        else if (c1 >= 'A' && c1 <= 'Z')
            c1 |= 0x20;

        int c2 = *s2++;
        if (in_dbcs2 || IS_DBCS_HI(c2))
            in_dbcs2 = !in_dbcs2;
        else if (c2 >= 'A' && c2 <= 'Z')
            c2 |= 0x20;

        const int diff = c1 - c2;
        if (diff || !c1)
            return diff;
    }
}

/* Split string `src` with spaces and copy the first line of the second splitted item to `dst` (formerly `str_cut`) */
GCC_NONNULLS
void str_split_2nd(char *dst, const char *src)
{
    src = strchr(src, ' ');
    if (!src)
    {
        *dst = '\0';
        return;
    }

    do
    {
        ++src;
    } while (*src == ' ');

    {
        const size_t len = strcspn(src, " \n\r");
        memmove(dst, src, len);
        dst[len] = '\0';
    }
}

/* Not boundary-checking version of `dbcs_nstate()`
 * `idx` must smaller than the length of `str`, otherwise buffer overrun may occur */
GCC_NONNULLS
GCC_PURE enum DbcsState dbcs_state(const char *str, size_t idx)
{
    return dbcs_nstate(str, idx, idx + strlen(str + idx));
}

/* Return the DBCS state at byte position `idx` in the string `str` of maximum length `len` */
GCC_NONNULLS
GCC_PURE enum DbcsState dbcs_nstate(const char *str GCC_NONSTRING, size_t idx, size_t len)
{
    if (idx >= len)
        return DBCS_ASCII; /* Invalid; defaults to `DBCS_ASCII` */

    /* H: High byte; L: Low byte; A: ASCII byte */
    const char *ptr = str + idx;
    const bool is_hi = IS_DBCS_HI(*ptr);

    /* "L"/"A" || "?A" || "LL"/"AL" => ASCII */
    /*  ^   ^       ^       ^    ^ptr */
    if (!is_hi && (ptr <= str || !IS_DBCS_LOW(*ptr) || !IS_DBCS_HI(ptr[-1])))
        return DBCS_ASCII;

    /* "H" * (2n+1) => LEAD || "H" * 2n => TRAIL  */
    /* "H" * 2n + "L" => ASCII || "H" * (2n+1) + "L" => TRAIL */
    bool is_head = true;
    while (--ptr >= str && IS_DBCS_HI(*ptr))
        is_head = !is_head;
    /* "H" is head: "HH" => LEAD || "HL" => LEAD || "HA" => ASCII */
    /* Check the next byte of the byte at position `idx` */
    if (is_head && is_hi)
        return (idx + 1 < len && (IS_DBCS_HI(str[idx + 1]) || IS_DBCS_LOW(str[idx + 1]))) ? DBCS_LEAD : DBCS_ASCII;
    /* Otherwise the head is ASCII */
    return (is_head) ? DBCS_ASCII : DBCS_TRAIL;
}

/* Not boundary-checking version of `dbcs_nstate_ansi()`
 * `idx` must smaller than the length of `str`, otherwise buffer overrun may occur */
GCC_NONNULLS
GCC_PURE enum DbcsState dbcs_state_ansi(const char *str, size_t idx)
{
    return dbcs_nstate_ansi(str, idx, idx + strlen(str + idx));
}

/* Return the DBCS state at byte position `idx` in the string `str` of maximum length `len`, ignoring ANSI escapes
 * If `idx` is larger than the length of `str`, return DBCS_ASCII */
GCC_NONNULLS
GCC_PURE enum DbcsState dbcs_nstate_ansi(const char *str GCC_NONSTRING, size_t idx, size_t len)
{
    /* H: High byte; L: Low byte; A: ASCII byte */
    size_t cur = idx;
    const bool is_hi = IS_DBCS_HI(str[cur]);

    /* "L"/"A" || "?A" || "LL"/"AL" => ASCII */
    /*  ^   ^       ^       ^    ^cur */
    size_t prev = str_nmove_ansi(str, cur, -1, len);
    if (!is_hi && (!(prev < cur && (prev || str[prev] != '\x1b')) || !IS_DBCS_LOW(str[cur]) || !IS_DBCS_HI(str[prev])))
        return DBCS_ASCII;

    /* "H" * (2n+1) => LEAD || "H" * 2n => TRAIL  */
    /* "H" * 2n + "L" => ASCII || "H" * (2n+1) + "L" => TRAIL */
    bool is_head = true;
    bool interesc = prev + 1 < cur;
    while (prev < cur && (prev || str[prev] != '\x1b') && IS_DBCS_HI(str[prev]))
    {
        is_head = !is_head;
        cur = prev;
        prev = str_nmove_ansi(str, cur, -1, len);
    }
    /* Also check whether escapes occur between the leading byte and the trailing byte for trailing bytes */
    /* "H" is head: "HH" => LEAD || "HL" => LEAD || "HA" => ASCII */
    if (is_head && is_hi)
    {
        /* Check the next non-ANSI escape byte of the byte at position `idx` */
        int next = str_nmove_ansi(str, idx, 1, len);
        interesc = (str[next] && next > idx + 1);
        return (str[next] && (IS_DBCS_HI(str[next]) || IS_DBCS_LOW(str[next]))) ? ((interesc) ? DBCS_LEAD_INTERESC : DBCS_LEAD) : DBCS_ASCII;
    }
    /* Otherwise the head is ASCII */
    return (is_head) ? DBCS_ASCII : ((interesc) ? DBCS_TRAIL_INTERESC : DBCS_TRAIL);
}

/* Return a string allocated using `malloc` with the content of string `src`
 * `pad` is the size in bytes of the space after the string, including the `'\0'` string end
 * If `pad` <= `0`, the space for the `'\0'` string end is still included and is set to `'\0'` */
GCC_NONNULLS
GCC_RET_NONNULL char *str_dup(const char *src, int pad)
{
    const size_t len = strlen(src);
    char *const dst = (char *)malloc(len + BMAX(pad, 1));
    memcpy(dst, src, len + 1);
    return dst;
}

/* Similar to the `setdirpath` below, but for some `folder`, use the parent directory as the base directory instead (formerly `str_folder`)
 * When `folder` is nether a path to a `.DIR` or `.GEM` file nor ends with `'/'`,
 *     and the name of the containing directory is a single character, then the parent directory is used as the base directory
 * If `folder` does not contain any `'/'` characters, `folder` is used as the base directory
 * Otherwise, the characters after the last `'/'` (can be empty) is taken as the name of a file in the containing directory
 * E.g., `fname`: `"file"`
 * - `folder`: `"gem/brd/test/.DIR"` -> `fpath`: `"gem/brd/test/file"`
 * - `folder`: `"gem/brd/test/.GEM"` -> `fpath`: `"gem/brd/test/file"`
 * - `folder`: `"gem/brd/test/Q/F1FRGANQ"` -> `fpath`: `"gem/brd/test/file"` (cf. `setdirpath`)
 * - `folder`: `"gem/brd/test/@/@class"` -> `fpath`: `"gem/brd/test/file"` (cf. `setdirpath`)
 * - `folder`: `"gem/brd/test/@/"` -> `fpath`: `"gem/brd/test/@/file"`
 * - `folder`: `"gem/brd/test/@"` -> `fpath`: `"gem/brd/test/file"`
 * - `folder`: `"gem/brd/test/"` -> `fpath`: `"gem/brd/test/file"`
 * - `folder`: `"gem/brd/t/@"` -> `fpath`: `"gem/brd/file"` (cf. `setdirpath`)
 * - `folder`: `"gem/brd/t/"` -> `fpath`: `"gem/brd/t/file"`
 * - `folder`: `"gem/.DIR"` -> `fpath`: `"gem/file"`
 * - `folder`: `"gem/U/F0S8JULU"` -> `fpath`: `"gem/file"` (cf. `setdirpath`)
 * - `folder`: `"gem/"` -> `fpath`: `"gem/file"`
 * - `folder`: `"gem"` -> `fpath`: `"gem/file"`
 * - `folder`: `"g/@"` -> `fpath`: `"file"` (cf. `setdirpath`)
 * - `folder`: `"g/"` -> `fpath`: `"g/file"`
 * - `folder`: `"/"` -> `fpath`: `"/file"` */
GCC_NONNULLS
void setdirpath_root(char *fpath, const char *folder, const char *fname)
{
    const char *tail = strrchr(folder, '/');
    if (tail)
    {
        ++tail;
        if (!(*tail == '.' || !*tail) && ((tail - 3 >= folder && tail[-3] == '/') || tail - 2 >= folder))
            tail -= 2;

        const size_t len = tail - folder;
        memmove(fpath, folder, len);
        strcpy(fpath + len, fname);
    }
    else
    {
        const size_t len = strlen(folder);
        memmove(fpath, folder, len);
        fpath[len] = '/';
        strcpy(fpath + len + 1, fname);
    }
}

/* Output the path string to `fname` with the containing directory of the file specified with `direct` as the base directory to `fpath`
 * If `direct` does not contain any `'/'` characters, `direct` is taken as the containing directory
 * Otherwise, the characters after the last `'/'` (can be empty) is taken as the name of a file in the containing directory
 * E.g., `fname`: `"file"`
 * - `direct`: `"brd/test/.DIR"` -> `fpath`: `"brd/test/file"`
 * - `direct`: `"brd/test/P/A1E443BP"` -> `fpath`: `"brd/test/P/file"`
 * - `direct`: `"brd/test/"` -> `fpath`: `"brd/test/file"`
 * - `direct`: `"brd"` -> `fpath`: `"brd/file"` */
GCC_NONNULLS
void setdirpath(char *fpath, const char *direct, const char *fname)
{
    const char *const delim = strrchr(direct, '/');
    if (delim)
    {
        const size_t len = delim - direct + 1;
        memmove(fpath, direct, len);
        strcpy(fpath + len, fname);
    }
    else
    {
        const size_t len = strlen(direct);
        memmove(fpath, direct, len);
        fpath[len] = '/';
        strcpy(fpath + len + 1, fname);
    }
}

/* ----------------------------------------------------  */
/* E-mail address format                                 */
/* ----------------------------------------------------  */
/* 1. user@domain                                        */
/* 2. <user@domain>                                      */
/* 3. user@domain (nick)                                 */
/* 4. user@domain ("nick")                               */
/* 5. nick <user@domain>                                 */
/* 6. "nick" <user@domain>                               */
/* ----------------------------------------------------  */

/* Parse string `from` and copy the email address into buffer `addr`, and the nickname into buffer `nick` (formerly `str_from`)
 * Returns `-1` if `from` does not contain any email addresses; `from` is copied into `addr` and an empty string is copied into `nick`
 * Returns `0` otherwise */
GCC_NONNULLS
int from_parse(const char *from, char *addr, char *nick)
{
    const char *at = NULL;
    const char *langle = NULL;
    const char *from_end;

    /* Assume that `addr` and `nick` both point to buffers with a size adequate to hold the string `from` */
    const size_t from_sz = strlen(from) + 1;

    *nick = '\0';

    {
        int cc;
        for (from_end = from; (cc = *from_end); ++from_end)
        {
            if (cc == '<')
                langle = from_end;
            else if (cc == '@')
                at = from_end;
        }
    }

    if (at == NULL)
    {
        strcpy(addr, from);
        return -1;
    }

    if (langle && langle < at && from_end[-1] == '>')
    {
        /* case 2/5/6 : name <mail_addr> */

        if (langle > from)
        {
            const char *nick_end = langle - 1;
            const char *nick_head = from;
            if (*nick_head == '(' && nick_end[-1] == ')')
            {
                ++nick_head;
                --nick_end;
            }
            if (*nick_head == '"' && nick_end[-1] == '"')
            {
                ++nick_head;
                --nick_end;
            }
            str_sncpy(nick, nick_head, from_sz, nick_end - nick_head);
            mmdecode_str(nick);
        }

        --from_end;
        from = langle + 1;
    }
    else
    {
        /* case 1/3/4 */

        const char *nick_end = from_end - 1;
        if (*nick_end == ')')
        {
            const char *nick_head = strchr(from, '(');
            if (nick_head)
            {
                from_end = nick_head - 1;

                ++nick_head;
                if (*nick_head == '"' && nick_end[-1] == '"')
                {
                    ++nick_head;
                    --nick_end;
                }
                str_sncpy(nick, nick_head, from_sz, nick_end - nick_head);
                mmdecode_str(nick);
            }
        }
    }

    str_sncpy(addr, from, from_sz, from_end - from);
    return 0;
}


/* Return whether string `tag` is present in the token list string `list`, whose items are delimited with `'/'` characters */
GCC_NONNULLS
GCC_PURE bool str_has(const char *list, const char *tag)
{
    const size_t len = strlen(tag);
    for (;;)
    {
        const size_t len_pat = strcspn(list, "/");
        if (len_pat == len && !str_ncasecmp(list, tag, len))
            return true;
        list += len_pat;
        if (!*list)
            return false;
        ++list;
    }
}

/* Return a polynomial rolling hash for string `str` with specifying the initial value to be `seed` and the multiplier to be `mult_base`
 * The most significant bit of the result is set to `0` */
GCC_NONNULLS
GCC_PURE int str_hash_mult(const char *str, unsigned int seed, unsigned int mult_base)
{
    for (unsigned int cc; (cc = *str); ++str)
        seed = mult_base * seed + cc;
    return (seed & 0x7fffffff);
}

GCC_NONNULLS
GCC_PURE int str_hash(const char *str, unsigned int seed)
{
    return str_hash_mult(str, seed, 31);
}

GCC_NONNULLS
GCC_PURE int str_hash2(const char *str, unsigned int seed)
{
    return str_hash_mult(str, seed, 127);
}

GCC_NONNULLS
GCC_PURE int hash32(const char *str)
{
    const unsigned int seed = 1048583; /* a big prime number */
    return str_hash(str, seed);
}

/* Return the length of string `str` without spaces (formerly `str_len) */
GCC_NONNULLS
GCC_PURE int str_len_nospace(const char *str)
{
    int len = 0;

    for (int cc; (cc = *str); ++str)
    {
        if (cc != ' ')
            ++len;
    }

    return len;
}

/* Convert string `str` to lowercase and output the result to buffer `dst` */
GCC_NONNULLS
void str_lower(char *dst, const char *src)
{
    for (;;)
    {
        int ch = *src++;
        if (ch >= 'A' && ch <= 'Z')
            ch |= 0x20;
        *dst++ = ch;
        if (!ch)
            return;
    }
}

/* Convert string `str` to lowercase with DBCS characters handled and output the result to buffer `dst` (formerly `str_lowest`) */
GCC_NONNULLS
void str_lower_dbcs(char *dst, const char *src)
{
    bool in_dbcs = false;          /* 1: 前一碼是中文字 */

    for (;;)
    {
        int ch = *src++;
        if (in_dbcs || IS_DBCS_HI(ch))
            in_dbcs = !in_dbcs;
        else if (ch >= 'A' && ch <= 'Z')
            ch |= 0x20;
        *dst++ = ch;
        if (!ch)
            return;
    }
}

GCC_CONSTEXPR static inline bool is_ansi_param(int ch)
{
    return (ch >= '0' && ch <= '9') || ch == ';' || ch == ':';
}

/* Return the resulting byte position of moving from the byte position `idx` in the string `str` of maximum length of `len` by diff bytes, skip ANSI escapes
 * The result is unspecified if `idx` is inside of a ANSI escape except for the leading `ESC` byte
 * The result is bounded between `0` and the length of `str`:
 *     - `0` is returned for moving to positions before the first non-ANSI-escape bytes
 *     - The length of `str` is returned for moving to positions after the last non-ANSI-escape bytes */
GCC_NONNULLS
GCC_PURE size_t str_nmove_ansi(const char *str, size_t idx, ssize_t diff, size_t len)
{
    const char *ptr = str + BMIN(idx, len);

    /* `diff > 0`: Move afterwards; `diff < 0`: Move backwards */
    if (diff > 0)
    {
        do
        {
            const size_t len_rest = str_nlen(ptr, BMIN(diff, len));
            const char *const ptr_esc = memchr(ptr, '\x1b', len_rest);
            const ssize_t step = (ptr_esc) ? ptr_esc - ptr : len_rest;
            ptr += step;
            diff -= step;
            if (ptr_esc)
            {
                /* step: 01     000     0000 */
                /*      "A*sB" "A**sB" "A*[*sB" (* == ESC) */
                /*        ^ptr    ^ptr     ^ptr */
                /*       (-1)   (-0)    (-0) */
                if (step)
                    ++diff; /* ESC does not count */
                ++ptr;
                if (*ptr == '[')
                {
                    /* "A*[1;31mB" */
                    /*    ^ptr */
                    do
                    {
                        ++ptr;
                    } while (*ptr && is_ansi_param(*ptr));
                    /* "A*[1;31mB" */
                    /*         ^ptr */
                }
                /* step: 0001 */
                /*      "A*sB" */
                /*         ^ptr */
            }
            /* else: "str" */
            /*           ^ptr (*ptr == '\0') */
        } while (*ptr && diff > 0);
        /* Skip the trailing ANSI escapes */
        while (*ptr == '\x1b')
        {
            ++ptr;
            if (*ptr == '[')
            {
                do
                {
                    ++ptr;
                } while (*ptr && is_ansi_param(*ptr));
            }
            /* Skip the ANSI escape final byte */
            if (*ptr != '\x1b')
                ++ptr;
        }
    }
    else if (diff < 0)
    {
        /* Assume `*ptr` has a value not for the param bytes of ANSI escape */
        const char *final = ptr;
        while (ptr > str)
        {
            --ptr;
            ++diff;
            if (*ptr == '[' && ptr > str && ptr[-1] == '\x1b')
            {
                --ptr;
                /*              .final    .final */
                /* step:  66543210      11010 */
                /*      "1*[1;31mA"   "1*[*sA" */
                /*        ^ptr          ^ptr */
                /*        (-6)          (-1) */
                diff -= final - ptr - ((*final == '\x1b') ? 1 : 0); /* Add back */
                final = ptr;
            }
            else if (*ptr == '\x1b')
            {
                /*         .final   .final   .final */
                /* step:  210      1000     10000 */
                /*      "1*sA"   "1**sA"  "1**[mA" */
                /*        ^ptr     ^ptr     ^ptr */
                /*       (-2)      (-1)     (-1) */
                diff -= 2 - ((*final == '\x1b') ? 1 : 0); /* Add back */
                final = ptr;
            }
            else if (!is_ansi_param(*ptr))
            {
                /*          .      .       .final      .final     .final    .final   +.final */
                /*      "*[[A"  "[10"   "2[0"    "1*[1mA"    "1*[mA"    "1*sA"    "*sBA" */
                /*         ^ptr  ^ptr     ^ptr        ^ptr       ^ptr      ^ptr     +^ptr */
                /* ansi:   io    ooo      oo          io         io        io       ioo (in/out) */
                if (diff == 1 && ptr + 1 == final && *final != '\x1b')
                    break; /* The previous byte is out of ANSI escapes */
                final = ptr;
            }
            /* else:        .final */
            /*      "1*[1;31m0" */
            /*            ^ptr */
        }
        if (diff > 0)
            ptr += diff; /* No possible ANSI escape heads found; backtract */
    }

    return ptr - str;
}

/* Compare two strings `s1` and `s2` for at most `n` bytes, ignoring case differences (formerly `str_ncmp`)
 * Returns the value of difference between the first different bytes of the two strings or `0` if the first `n` bytes of the two strings do not differ */
GCC_NONNULLS
GCC_PURE int str_ncasecmp(const char *s1, const char *s2, int n)
{
    while (n--)
    {
        int c1 = *s1++;
        if (c1 >= 'A' && c1 <= 'Z')
            c1 |= 0x20;

        int c2 = *s2++;
        if (c2 >= 'A' && c2 <= 'Z')
            c2 |= 0x20;

        const int diff = c1 - c2;
        if (diff || !c1)
            return diff;
    }

    return 0;
}

/* Compare two strings `s1` and `s2` for at most `n` bytes, ignoring case differences and handling DBCS characters
 * Returns the value of difference between the first different bytes of the two strings or `0` if the first `n` bytes of the two strings do not differ */
GCC_NONNULLS
GCC_PURE int str_ncasecmp_dbcs(const char *s1, const char *s2, int n)
{
    bool in_dbcs1 = false;
    bool in_dbcs2 = false;
    while (n--)
    {
        int c1 = *s1++;
        if (in_dbcs1 || IS_DBCS_HI(c1))
            in_dbcs1 = !in_dbcs1;
        else if (c1 >= 'A' && c1 <= 'Z')
            c1 |= 0x20;

        int c2 = *s2++;
        if (in_dbcs2 || IS_DBCS_HI(c2))
            in_dbcs2 = !in_dbcs2;
        else if (c2 >= 'A' && c2 <= 'Z')
            c2 |= 0x20;

        const int diff = c1 - c2;
        if (diff || !c1)
            return diff;
    }

    return 0;
}

/* Remove trailing spaces and tabs from the string end `str` in-place (formerly `str_strip`)
 * There must be at least 1 character which is neither space nor tab before `str`, otherwise a buffer overrun occurs */
GCC_NONNULLS
void str_rstrip_tail(char *str)
{
    for (;;)
    {
        const int ch = *--str;
        if (!(ch == ' ' || ch == '\t'))
            break;
    }
    str[1] = '\0';
}

/* Thor.980921: str_ncpy 含 0 */
/* Thor.980921: str_ncpy 已含 0之空間 */
/* Thor.980921: 已包含 0 */
/* Thor.980921: str_ncpy與一般 strncpy有所不同, 特別注意 */
/*
 * str_scpy() - similar to strncpy(3) but terminates string always with '\0' if siz != 0,
 * and doesn't do padding (formerly `str_ncpy`)
 * It behaves like `strlcpy`, except that it does not read more than `siz` bytes from `src`.
 * It behaves the same as `strscpy` from the Linux Kernel API, except that it return `-1` when truncation occurs.
 */
GCC_NONNULLS
ssize_t str_scpy(char *dst, const char *src GCC_NONSTRING, size_t siz)
{
    /* Copy as many bytes as will fit */
    const size_t slen = (siz > 0) ? str_nlen(src, siz) : 0;
    if (slen < siz)
    {
        memmove(dst, src, slen + 1); /* Copy the `'\0'` end */
        return slen;
    }
    else if (siz > 0)
    {
        memmove(dst, src, siz - 1);

        /* Not enough room in dst, add NUL */
        dst[siz - 1] = '\0';    /* NUL-terminate dst */
    }
    return -1;
}

/* Copy at most `n` bytes from string `src` into buffer `dst` with size of `siz` bytes
 * `dst` will always be `'\0'`-terminated unless `siz` is `0`
 * Return `-1` when truncation occurs
 * Return the number of non-`'\0'` characters copied otherwise
 * It does not read more than `siz` bytes nor `n` bytes from `src`. */
GCC_NONNULLS
ssize_t str_sncpy(char *dst, const char *src GCC_NONSTRING, size_t siz, size_t n)
{
    /* Copy as many bytes as will fit */
    const size_t slen = ((siz = BMIN(siz, n + 1)) > 0) ? str_nlen(src, siz) : 0;
    if (slen < siz)
    {
        memmove(dst, src, slen + 1); /* Copy the `'\0'` end */
        return slen;
    }
    else if (siz > 0)
    {
        memmove(dst, src, siz - 1);

        /* Not enough room in dst, add NUL */
        dst[siz - 1] = '\0';    /* NUL-terminate dst */
    }
    return -1;
}

/* Return a string allocated using `malloc` with the content of the first `len` characters from string `src` */
GCC_NONNULLS
GCC_RET_NONNULL char *str_ndup(const char *src GCC_NONSTRING, int len)
{
    const size_t slen = (len > 0) ? str_nlen(src, len) : 0;
    char *const dst = (char *)malloc(slen + 1);
    memcpy(dst, src, slen);
    dst[slen + 1] = '\0';
    return dst;
}

/* Return the length of string `str` or `maxlen` if `str` contains more than `maxlen` non-'\0' characters */
GCC_NONNULLS
GCC_PURE size_t str_nlen(const char *str GCC_NONSTRING, size_t maxlen)
{
    const char *const end = str + maxlen;
    for (const char *p = str; p < end; ++p)
    {
        if (!*p)
            return p - str;
    }
    return maxlen;
}

/* str_pat: String match with wildcard pattern, supporting wildcard characters `?`, `*`, & `\`
 * Returns `true` when string `str` matches the pattern specified with string `pat` */
GCC_NONNULLS
GCC_PURE bool str_pat(const char *str, const char *pat)
{
    const char *xstr = NULL;
    const char *xpat = NULL;

    for (int cs; (cs = *str); ++str)
    {
        int cp = *pat++;
        if (cp == '*')
        {
            do
            {
                cp = *pat++;
                if (!cp)
                    return true;
            } while (cp == '*');
            xpat = pat;
            xstr = str;
        }

        if (cp == '?')
            continue;

        if (cp == '\\')
            cp = *pat++;

#ifdef  CASE_IN_SENSITIVE
        if (cp >= 'A' && cp <= 'Z')
            cp |= 0x20;
        if (cs >= 'A' && cs <= 'Z')
            cs |= 0x20;
#endif
        if (cp == cs)
            continue;

        if (xpat)
        {
            pat = xpat;
            str = xstr++;
            continue;
        }

        return false;
    }

    {
        int cp;
        while ((cp = *pat) == '*')
            ++pat;
        if (cp)
            return false;
    }

    return true;
}


/* Reverse the string `src`, output the result to a buffer from the buffer end `dst`, and return the string head of `dst` (formerly `str_rev`) */
GCC_NONNULLS
GCC_RET_NONNULL char *str_rev_tail(char *dst, const char *src)
{
    *dst = '\0';
    for (int cc; (cc = *src); ++src)
        *--dst = cc;
    return dst;
}

/* Encode string `str` in-place using a form of run-length encoding and return the encoded string length (formerly `str_rle`)
 * Encoding scheme: 4+ repeated bytes (value > `'\b'`) => `'\b' <repeat length in 8-bit binary>` */
GCC_NONNULLS
int rle_encode(char *str)
{
    const char *src = str;
    char *dst = str;

    for (int cc; (cc = *src); ++src)
    {
        if (cc > 8 && cc == src[1] && cc == src[2] && cc == src[3])
        {
            int rl = 4;
            src += 4;
            while (rl < 0xff && *src == cc)
            {
                ++rl;
                ++src;
            }
            --src;

            *dst++ = 8;
            *dst++ = rl;
        }
        *dst++ = cc;
    }

    *dst = '\0';
    return dst - str;
}

/* Find and return the first occurrence of pattern `tag` in string `str` or `NULL` if not found, ignoring case differences (formerly `str_str`)
 * `tag` is a non-empty string */
GCC_NONNULLS
GCC_PURE char *str_casestr(const char *str, const char *tag)
{
    const int c2_head = *tag;

    for (int c1_head; (c1_head = *str); ++str)
    {
        if (c1_head >= 'A' && c1_head <= 'Z')
            c1_head |= 0x20;

        if (c1_head == c2_head)
        {
            const char *p1 = str + 1;
            const char *p2 = tag + 1;

            for (;;)
            {
                int c2 = *p2++;
                if (!c2)
                    return (char *)str;
                if (c2 >= 'A' && c2 <= 'Z')
                    c2 |= 0x20;

                int c1 = *p1++;
                if (c1 >= 'A' && c1 <= 'Z')
                    c1 |= 0x20;

                if (c1 != c2)
                    break;
            }
        }
    }

    return NULL;
}

/* Find and return the first occurrence of pattern `tag` in string `str` or `NULL` if not found, ignoring case differences and handling DBCS characters (formerly `str_sub`)
 * `tag` is an non-empty string and may contain DBCS characters */
GCC_NONNULLS
GCC_PURE char *str_casestr_dbcs(const char *str, const char *tag)
{
    const int c2_head = *tag;
    bool in_dbcs = false;               /* 1: 前一碼是中文字 */

    for (int c1_head; (c1_head = *str); ++str)
    {
        if (in_dbcs)
        {
            in_dbcs = !in_dbcs;
            continue;
        }

        if (IS_DBCS_HI(c1_head))
            in_dbcs = !in_dbcs;
        else if (c1_head >= 'A' && c1_head <= 'Z')
            c1_head |= 0x20;

        if (c1_head == c2_head)
        {
            const char *p1 = str + 1;
            const char *p2 = tag + 1;
            bool in_dbcs1 = in_dbcs; /* 1: 前一碼是中文字 */
            bool in_dbcs2 = IS_DBCS_HI(c2_head);

            for (;;)
            {
                int c2 = *p2++;
                if (!c2)
                    return (char *)str;
                if (in_dbcs2 || IS_DBCS_HI(c2))
                    in_dbcs2 = !in_dbcs2;
                else if (c2 >= 'A' && c2 <= 'Z')
                    c2 |= 0x20;

                int c1 = *p1++;
                if (in_dbcs1 || IS_DBCS_HI(c1))
                    in_dbcs1 = !in_dbcs1;
                else if (c1 >= 'A' && c1 <= 'Z')
                    c1 |= 0x20;

                if (c1 != c2)
                    break;
            }
        }
    }

    return NULL;
}

/* Return the string end of string `str` */
GCC_NONNULLS
GCC_PURE GCC_RET_NONNULL char *str_tail(const char *str)
{
    return (char *)strchr(str, '\0');
}

/* Remove trailing spaces from string `buf` in-place (formerly `str_trim`) */
GCC_NONNULLS
void str_rtrim(char *buf)
{
    char *p = strchr(buf, '\0');
    while (--p >= buf && *p == ' ')
        *p = '\0';
}

typedef struct {
    const char *mark;
    enum HdrMode mode;
} TitleMark;

static const TitleMark str_ttl_marks[] = {
    {STR_REPLY, HDRMODE_REPLY},
    {STR_FORWARD, HDRMODE_FORWARD},
    {STR_FORWARD_TAG, HDRMODE_FORWARD},
    {STR_ANNOUNCE, HDRMODE_ANNOUNCE},
    {STR_ANNOUNCE_EN, HDRMODE_ANNOUNCE},
};

/* Return the begin of article title in string `title` with article type tags skipped and output the article type to `pmode`
 * If the tag begins with a `'['`, the tag is not skipped
 * If `pmode` is `NULL`, the article type is discard and is not output to `pmode` */
GCC_NONNULL(1)
GCC_RET_NONNULL char *str_ttl_hdrmode(const char *title, enum HdrMode *pmode)
{
    if (pmode)
        *pmode = HDRMODE_NORMAL;

    for (size_t i = 0; i < COUNTOF(str_ttl_marks); ++i)
    {
        const TitleMark *const mark = &str_ttl_marks[i];
        if (!str_ncasecmp_dbcs(title, mark->mark, strlen(mark->mark)))
        {
            /* Do not skip the initial `[]` tag */
            if (mark->mark[0] != '[')
            {
                title += strlen(mark->mark);
                if (*title == ' ')
                    ++title;
            }
            if (pmode)
                *pmode = mark->mode;
            break;
        }
    }

    return (char *)title;
}

/* Return the begin of article title in string `title` with article type tags skipped */
GCC_NONNULLS
GCC_PURE GCC_RET_NONNULL char *str_ttl(const char *title)
{
    return str_ttl_hdrmode(title, NULL);
}

/*-------------------------------------------------------*/
/* lib/str_xor.c     ( NTHU CS MapleBBS Ver 3.10 )       */
/*-------------------------------------------------------*/
/* author : thor.bbs@bbs.cs.nthu.edu.tw                  */
/* target : included C for str xor-ing (signed mail)     */
/* create : 99/03/30                                     */
/* update :   /  /                                       */
/*-------------------------------------------------------*/

/* Thor: 結果是將src xor到dst上, 若有0結果, 則不變,
         所以dst長度必大於等於 src(以字串而言) */
/* Perform bitwise `xor` on binary data `dst` in-place with string `src`
 * The string length of `src` is the length of bytes where `xor` is performed
 * Bytes with `xor` result `0` are kept with their original values */
GCC_NONNULLS
void str_xor(char *dst GCC_NONSTRING, /* Thor.990409: 任意長度任意binary seq, 至少要 src那麼長 */
             const char *src    /* Thor.990409: 任意長度str, 不含 \0 */
    )
{
    for (int cc; (cc = *src); ++src)
    {
        const int cxor = cc ^ *dst;
        if (cxor)
             *dst = cxor;
        ++dst;
    }
}

/* strlcat based on OpenBSDs strlcat */

/*
 * Appends src to string dst of size siz (unlike strncat, siz is the
 * full size of dst, not space left).  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz <= strlen(dst)).
 * Returns strlen(src) + MIN(siz, strlen(initial dst)).
 * If retval >= siz, truncation occurred.
 */
GCC_NONNULLS
size_t strlcat(char *dst, const char *src, size_t siz)
{
    /* Find the end of dst and adjust bytes left but don't go past end */
    const size_t dlen = str_nlen(dst, siz);
    if (dlen == siz)
        return siz + strlen(src);

    char *const d = dst + dlen;
    const size_t slen = str_nlen(src, siz - 1 - dlen);
    memmove(d, src, slen);
    d[slen] = '\0';

    return dlen + slen + strlen(src + slen); /* count does not include NUL */
}

/* strlcpy based on OpenBSDs strlcpy */
/*
 * Copy src to string dst of size siz.  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz == 0).
 * Returns strlen(src); if retval >= siz, truncation occurred.
 */
GCC_NONNULLS
size_t strlcpy(char *dst, const char *src, size_t siz)
{
    /* Copy as many bytes as will fit */
    const size_t slen = str_nlen(src, siz);
    if (slen != siz)
    {
        memmove(dst, src, slen + 1); /* Copy the `'\0'` end */
    }
    else if (siz != 0)
    {
        memmove(dst, src, siz - 1);

        /* Not enough room in dst, add NUL */
        dst[siz - 1] = '\0';    /* NUL-terminate dst */
    }

    return slen + strlen(src + slen); /* count does not include NUL */
}
