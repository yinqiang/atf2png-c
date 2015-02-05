/*
** ATF to PNG
** Copyright Yinqiang Zhu 2015 (yinqiang.zhu<at>gmail.com)
**
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  #define T(x) (((x&0xff)<<24)|((x&0xff00)<<8)|((x&0xff0000)>>8)|((x&0xff000000)>>24))
#else
  #define T(x) (x)
#endif

struct atf_s
{
    uint8_t sig[3]; // signture
    uint32_t rer;   // reserved
    uint8_t ver;    // version
    uint32_t len;   // length
    int8_t cmap;    // cubemap
    int8_t fmt;     // format
    uint8_t w;      // log2width
    uint8_t h;      // log2height
    uint8_t cnt;    // count
};

/*
static int
is_little()
{
    int x = 1;
    char *p = (char *)&x;
    return (*p ? 1 : 0);
}
*/

static int
load_texure(FILE *fp, struct atf_s *atf_data, uint8_t **texture)
{
    size_t len = sizeof(uint8_t) * atf_data->len;
    *texture = malloc(len);
    memset(*texture, 0, len);
    if (atf_data->len != fread(*texture, sizeof(uint8_t), atf_data->len, fp))
    {
        return 1;
    }
    return 0;
}

static int
parse_texture(const uint8_t *texture, struct atf_s *atf_data)
{
    uint32_t dxt5len = 0, pvrtclen = 0, etc1len = 0;
    //uint8_t *dxt5, *pvrtc, *etc1;
    size_t offset;
    
    switch (atf_data->fmt)
    {
        // ATFRAWCOMPRESSEDALPHA
        case 5:
            dxt5len = T((uint32_t)(*((uint32_t*)texture)));
            printf("dxt5len: %08x\n", T(dxt5len));
            
            offset = sizeof(uint32_t) + sizeof(uint8_t) * dxt5len;
            printf("offset: %d\n", offset);
            pvrtclen = (uint32_t)(*((uint32_t *)(texture + offset)));
            printf("pvrtclen: %08x\n", T(pvrtclen));
            
            offset += sizeof(uint32_t) + sizeof(uint8_t) * pvrtclen;
            printf("offset: %d\n", offset);
            etc1len = (uint32_t)(*((uint32_t *)(texture + offset)));
            printf("etc1len: %08x\n", T(etc1len));
            break;
    }
    return 0;
}

int
main(int argc, char *argv[])
{
    FILE *fp = NULL;
    struct atf_s atf_data;
    uint8_t *texture = NULL;

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

    fread(atf_data.sig, sizeof(uint8_t), 3, fp);
    fread(&atf_data.rer, sizeof(uint32_t), 1, fp);
    fread(&atf_data.ver, sizeof(uint8_t), 1, fp);
    fread(&atf_data.len, sizeof(uint32_t), 1, fp);
    fread(&atf_data.fmt, sizeof(uint8_t), 1, fp);
    fread(&atf_data.w, sizeof(uint8_t), 1, fp);
    fread(&atf_data.h, sizeof(uint8_t), 1, fp);
    fread(&atf_data.cnt, sizeof(uint8_t), 1, fp);
    
    atf_data.rer = T(atf_data.rer);
    atf_data.len = T(atf_data.len) - sizeof(uint32_t);
    atf_data.cmap = (atf_data.fmt & 0x80) >> 7;
    atf_data.fmt = atf_data.fmt & 0x7F;

    printf("signture: %c%c%c\n", atf_data.sig[0], atf_data.sig[1], atf_data.sig[2]);
    printf("reserved: %08x\n", atf_data.rer);
    printf("version: %d\n", atf_data.ver);
    printf("datalen: %u\n", atf_data.len);
    printf("cubemap: %d\n", atf_data.cmap);
    printf("format: %d\n", atf_data.fmt);
    printf("log2width: %d\n", atf_data.w);
    printf("log2height: %d\n", atf_data.h);
    printf("count: %d\n", atf_data.cnt);

    if (load_texure(fp, &atf_data, &texture) != 0)
    {
        fprintf(stderr, "can not load texture\n");
        goto failed;
    }
    parse_texture(texture, &atf_data);

failed:
    if (texture != NULL) {
        free(texture);
    }
    if (fp != NULL) {
        fclose(fp);
    }

    return 0;
}
