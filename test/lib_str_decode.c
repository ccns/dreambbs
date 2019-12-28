#include "config.h"

#include <stdio.h>
#include <assert.h>
#include "dao.h"

int main(void)
{
    char buf[1024] = "=?Big5?B?pl7C0CA6IFtNYXBsZUJCU11UbyB5dW5sdW5nKDE4SzRGTE0pIFtWQUxJ?=\n\t=?Big5?B?RF0=?=";

    str_decode(buf);
    puts(buf);
    assert(!str_cmp(buf, "回覆 : [MapleBBS]To yunlung(18K4FLM) [VALID]"));

    buf[mmdecode("=A7=DA=A4@=AA=BD=B8I=A4=A3=A8=EC=A7=DA=BE=C7=AA=F8", 'q', buf)] = '\0';
    puts(buf);
    assert(!str_cmp(buf, "我一直碰不到我學長"));

    return 0;
}
