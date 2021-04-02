#include "config.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "dao.h"

int main(void)
{
    char buf[1024] = "=?Big5?B?pl7C0CA6IFtNYXBsZUJCU11UbyB5dW5sdW5nKDE4SzRGTE0pIFtWQUxJ?=\n\t=?Big5?B?RF0=?=";

    mmdecode_str(buf);
    puts(buf);
    assert(!strcmp(buf, "�^�� : [MapleBBS]To yunlung(18K4FLM) [VALID]"));

    buf[mmdecode("=A7=DA=A4@=AA=BD=B8I=A4=A3=A8=EC=A7=DA=BE=C7=AA=F8", 'q', buf)] = '\0';
    puts(buf);
    assert(!strcmp(buf, "�ڤ@���I����ھǪ�"));

    return 0;
}
