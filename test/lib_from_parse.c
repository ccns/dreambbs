#include "config.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "dao.h"

int main(void)
{
    char addr[48];
    char nick[32];

#define CHECK_CASE(input, addr_exp, nick_exp, ret_exp) \
    do { \
        char buf[] = input; \
        printf("`%s`: ", buf); \
        assert(from_parse(buf, addr, nick) == ret_exp); \
        printf("Address: `%s`, nick name: `%s`\n", addr, nick); \
        assert(!strcmp(addr, addr_exp)); \
        assert(!strcmp(nick, nick_exp)); \
    } while (0)

    CHECK_CASE("user@domain", "user@domain", "", 0);
    CHECK_CASE("user@domain (nick)", "user@domain", "nick", 0);
    CHECK_CASE("user@domain (\"nick\")", "user@domain", "nick", 0);
    CHECK_CASE("nick <user@domain>", "user@domain", "nick", 0);
    CHECK_CASE("\"nick\" <user@domain>", "user@domain", "nick", 0);
    CHECK_CASE("@", "@", "", 0);
    CHECK_CASE("<", "<", "", -1);
    CHECK_CASE("", "", "", -1);

#undef CHECK_CASE

    return 0;
}
