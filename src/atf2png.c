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
  #define T32(x) (((x&0xff)<<24)|((x&0xff00)<<8)|((x&0xff0000)>>8)|((x&0xff000000)>>24))
  #define T16(x) (((x&0xff)<<8)|(x>>8))
#else
  #define T32(x) (x)
  #define T16(x) (x)
#endif

static const char PKM[] = { 'P', 'K', 'M', ' ', '1', '0' };
static const uint16_t ETC1_RGB_NO_MIPMAPS = 0;

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

struct texture_s
{
    uint32_t len;
    uint8_t *head;
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
parse_texture(uint8_t *texture, struct atf_s *atf_data,
              struct texture_s *dxt5,
              struct texture_s *pvrtc,
              struct texture_s *etc1, struct texture_s *etc1alpha)
{
    size_t offset;
    uint8_t *head;
    
    head = texture;
    switch (atf_data->fmt)
    {
        // ATFRAWCOMPRESSEDALPHA
        case 5:
            dxt5->len = T32((uint32_t)(*((uint32_t*)texture)));
            printf("dxt5len: 0x%08x, %u\n", dxt5->len, dxt5->len);
            if (dxt5->len > 0)
            {
                dxt5->head = head;
                head += dxt5->len;
            }
            offset = sizeof(uint32_t) + sizeof(uint8_t) * dxt5->len;

            pvrtc->len = T32((uint32_t)(*((uint32_t *)(texture + offset))));
            printf("pvrtclen: 0x%08x, %u\n", pvrtc->len, pvrtc->len);
            if (pvrtc->len > 0)
            {
                pvrtc->head = head;
                head += pvrtc->len;
            }
            offset += sizeof(uint32_t) + sizeof(uint8_t) * pvrtc->len;

            etc1->len = T32((uint32_t)(*((uint32_t *)(texture + offset))));
            etc1->len >>= 1;
            etc1alpha->len = etc1->len;
            printf("etc1len: 0x%08x, %u\n", etc1->len, etc1->len);
            printf("etc1alphalen: 0x%08x, %u\n", etc1alpha->len, etc1alpha->len);
            if (etc1->len > 0)
            {
                etc1->head = head;
                etc1alpha->head = head + etc1->len;
            }
            break;
    }
    return 0;
}

static int
export_texture_etc(const char *filename, struct atf_s *atf_data, uint8_t *texture, uint32_t len)
{
    FILE *fp;
    uint16_t w, h;
    uint16_t ew, eh;

    if ((fp = fopen(filename, "wb")) == NULL)
    {
        fprintf(stderr, "can not open file: %s\n", filename);
        return 1;
    }
    w = 1 << (uint8_t)atf_data->w;
    h = 1 << (uint8_t)atf_data->h;
    ew = T16(w);
    eh = T16(h);
    fwrite(PKM, sizeof(PKM), 1, fp);
    fwrite(&ETC1_RGB_NO_MIPMAPS, sizeof(ETC1_RGB_NO_MIPMAPS), 1, fp);
    fwrite(&ew, sizeof(uint16_t), 1, fp);
    fwrite(&eh, sizeof(uint16_t), 1, fp);
    fwrite(&ew, sizeof(uint16_t), 1, fp);
    fwrite(&eh, sizeof(uint16_t), 1, fp);
    if (len != fwrite(texture, sizeof(uint8_t), len, fp))
    {
        fprintf(stderr, "Error: unable write data into file\n");
        fclose(fp);
        return 1;
    }
    fclose(fp);
    return 0;
}

int
main(int argc, char *argv[])
{
    FILE *fp = NULL;
    struct atf_s atf_data;
    uint8_t *texture = NULL;
    struct texture_s dxt5, pvrtc, etc1, etc1alpha;

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
    
    atf_data.rer = T32(atf_data.rer);
    atf_data.len = T32(atf_data.len) - sizeof(uint32_t);
    atf_data.w = atf_data.w;
    atf_data.h = atf_data.h;
    atf_data.cmap = (atf_data.fmt & 0x80) >> 7;
    atf_data.fmt = atf_data.fmt & 0x7F;

    printf("signture: %c%c%c\n", atf_data.sig[0], atf_data.sig[1], atf_data.sig[2]);
    printf("reserved: 0x%08x\n", atf_data.rer);
    printf("version: %d\n", atf_data.ver);
    printf("datalen: %u\n", atf_data.len);
    printf("cubemap: %d\n", atf_data.cmap);
    printf("format: %d\n", atf_data.fmt);
    printf("log2width: 0x%02x\n", atf_data.w);
    printf("log2height: 0x%02x\n", atf_data.h);
    printf("count: %d\n", atf_data.cnt);

    if (load_texure(fp, &atf_data, &texture) != 0)
    {
        fprintf(stderr, "can not load texture\n");
        goto failed;
    }
    dxt5.head = pvrtc.head = etc1.head = etc1alpha.head = NULL;
    parse_texture(texture, &atf_data, &dxt5, &pvrtc, &etc1, &etc1alpha);
    if (dxt5.head != NULL)
        printf("dxt5: 0x%x\n", dxt5.head);
    if (pvrtc.head != NULL)
        printf("pvrtc: 0x%x\n", pvrtc.head);
    if (etc1.head != NULL)
    {
        printf("etc1: 0x%x\n", etc1.head);
        printf("etc1alpha: 0x%x\n", etc1alpha.head);
        if (export_texture_etc("output.pkm", &atf_data, etc1.head, etc1.len) == 1)
            goto failed;
        if (export_texture_etc("output_alpha.pkm", &atf_data, etc1alpha.head, etc1alpha.len) == 1)
            goto failed;
        printf("export etc1\n");
    }

failed:
    if (texture != NULL) {
        free(texture);
    }
    if (fp != NULL) {
        fclose(fp);
    }

    return 0;
}
