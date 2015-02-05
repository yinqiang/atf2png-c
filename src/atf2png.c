/*
** ATF to PNG
** Copyright Yinqiang Zhu 2015 (yinqiang.zhu<at>gmail.com)
**
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

struct atf_s
{
    uint8_t sig;   // signture
    uint32_t rer;  // reserved
    uint8_t ver;   // version
    uint32_t len;  // length
    int8_t cmap;   // cubemap
    int8_t fmt;    // format
    uint8_t w;     // log2width
    uint8_t h;     // log2height
    uint8_t cnt;   // count
};

int
main(int argc, char *argv[])
{
    FILE *fp;
    struct atf_s

    if (argc < 2)
    {
        fprintf(stderr, "Usage: atf2png path_to/file.atf\n");
        return 1;
    }
    if ((fp = fopen(argv[1], "rb")) == NULL)
    {
        fprintf(stderr, "can not open file: %s\n", argv[1]);
        return 1;
    }


}
