#include <stdio.h>
#include <limits.h>
#include "miniz.h"

int main(int argc, char *argv[])
{
    FILE *pInfile, *pOutfile;
    uint infile_size;
    int p = 1;
    const char *pSrc_filename;
    const char *pDst_filename;
    long file_loc;

    printf("miniz.c version: %s\n", MZ_VERSION);
    if (argc < 2)
    {
        printf("Usage: miniz_gzip infile outfile\n");
        return EXIT_FAILURE;
    }

    pSrc_filename = argv[p++];
    pDst_filename = argv[p++];

    // Open input file.
    pInfile = fopen(pSrc_filename, "rb");
    if (!pInfile)
    {
        printf("Failed opening input file!\n");
        return EXIT_FAILURE;
    }

    // Determine input file's size.
    fseek(pInfile, 0, SEEK_END);
    file_loc = ftell(pInfile);
    fseek(pInfile, 0, SEEK_SET);

    infile_size = (uint)file_loc;

    // Open output file.
    pOutfile = fopen(pDst_filename, "wb");
    if (!pOutfile)
    {
        printf("Failed opening output file!\n");
        return EXIT_FAILURE;
    }

    printf("Input file size: %u\n", infile_size);

    uint8_t *s_inbuf = malloc(infile_size);
    if (!s_inbuf)
    {
        printf("Failed allocating memory for input file!\n");
        return EXIT_FAILURE;
    }
    if (fread(s_inbuf, 1, infile_size, pInfile) != infile_size)
    {
        printf("Failed reading from input file!\n");
        return EXIT_FAILURE;
    }

    uint8_t *s_outbuf = malloc(infile_size + 18);
    if (!s_outbuf)
    {
        printf("Failed allocating memory for output file!\n");
        return EXIT_FAILURE;
    }

    unsigned long zsize = compressBound(infile_size);
    s_outbuf[0] = 0x1F;
    s_outbuf[1] = 0x8B;
    s_outbuf[2] = 8;
    s_outbuf[3] = 0;
    s_outbuf[4] = 0;
    s_outbuf[5] = 0;
    s_outbuf[6] = 0;
    s_outbuf[7] = 0;
    s_outbuf[8] = 0;
    s_outbuf[9] = 0xFF;
    int err = compress2(s_outbuf + 10, &zsize, s_inbuf, infile_size, 10);
    if (err != Z_OK)
    {
        printf("compress2 failed %d!\n", err);
        return EXIT_FAILURE;
    }
    int footerStart = (int)zsize + 10;
    mz_ulong crc = mz_crc32(MZ_CRC32_INIT, s_inbuf, infile_size);
    s_outbuf[footerStart] = crc & 0xFF;
    s_outbuf[footerStart + 1] = (crc >> 8) & 0xFF;
    s_outbuf[footerStart + 2] = (crc >> 16) & 0xFF;
    s_outbuf[footerStart + 3] = (crc >> 24) & 0xFF;
    s_outbuf[footerStart + 4] = infile_size & 0xFF;
    s_outbuf[footerStart + 5] = (infile_size >> 8) & 0xFF;
    s_outbuf[footerStart + 6] = (infile_size >> 16) & 0xFF;
    s_outbuf[footerStart + 7] = (infile_size >> 24) & 0xFF;

    if (fwrite(s_outbuf, 1, zsize + 10 + 8, pOutfile) != (zsize + 10 + 8))
    {
        printf("Failed writing to output file!\n");
        return EXIT_FAILURE;
    }

    printf("Output file size: %lu\n", zsize + 10 + 8);

    fclose(pInfile);
    fclose(pOutfile);
    
    free(s_inbuf);
    free(s_outbuf);
    return 0;
}
