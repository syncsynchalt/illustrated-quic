#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <openssl/evp.h>

// "c_cid" or "s_cid" in our examples
#define SHORT_PKT_CID_LEN 5

void die(const char *msg)
{
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    exit(1);
}

void die_usage(const char *msg)
{
    if (msg) {
        fprintf(stderr, "%s\n", msg);
    }
    fprintf(stderr, "Usage: hp_removal key < in > out\n");
    exit(1);
}

void debug_print(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "HP DEBUG: ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
}

bool is_long_packet(const char *pkt)
{
    return (pkt[0] & 0x80) == 0x80;
}

bool is_initial_packet(const char *pkt)
{
    return (pkt[0] & 0xf0) == 0xc0;
}

void compute_mask(const uint8_t *key, size_t keylen, const uint8_t *sample, size_t samplelen, uint8_t *mask_out)
{
    // Using AES-128-ECB
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();

    uint8_t out[256];
    int outlen = 0;
    const EVP_CIPHER *cipher = EVP_get_cipherbyname("AES-128-ECB");
    EVP_EncryptInit_ex(ctx, cipher, NULL, key, NULL);
    EVP_EncryptUpdate(ctx, out, &outlen, sample, samplelen);
    if (outlen < 5) {
        die("Unexpected short write");
    }
    memcpy(mask_out, out, 5);
    EVP_CIPHER_CTX_free(ctx);
    return;
}

size_t get_pn_offset(char *pkt)
{
    size_t pn_offset = 0;
    if (is_initial_packet(pkt)) {
        size_t dcidlen = pkt[5];
        size_t ccidlen = pkt[6 + dcidlen];
        size_t token_section = 1; // assumption that's always true in our packets
        uint8_t payload_length_b1 = pkt[7 + dcidlen + ccidlen + token_section];
        size_t payload_length_len = ((payload_length_b1 & 0xC0) >> 6) + 1;
        pn_offset = 7 + dcidlen + ccidlen + token_section + payload_length_len;
    } else if (is_long_packet(pkt)) {
        size_t dcidlen = pkt[5];
        size_t ccidlen = pkt[6 + dcidlen];
        uint8_t payload_length_b1 = pkt[7 + dcidlen + ccidlen];
        size_t payload_length_len = ((payload_length_b1 & 0xC0) >> 6) + 1;
        pn_offset = 7 + dcidlen + ccidlen + payload_length_len;
    } else {
        pn_offset = 1 + SHORT_PKT_CID_LEN;
    }
    debug_print("pn_offset %d", (int)pn_offset);
    return pn_offset;
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        die_usage(NULL);
    }

    uint8_t key[16] = { 0 };
    const char *hexkey = argv[1];
    if (strlen(hexkey) != 32) {
        die_usage("Incorrect key length");
    }
    if (strspn(hexkey, "0123456789abcdefABCDEF") != 32) {
        die_usage("Key must be in hex");
    }
    for (size_t i = 0; i < 16; i++) {
        char bb[3] = { 0 };
        strncat(bb, hexkey + 2*i, 2);
        key[i] = (uint8_t)strtol(bb, NULL, 16);
    }

    char buf[512];
    size_t nr = fread(buf, 1, sizeof(buf), stdin);

    uint8_t sample[16];
    size_t pn_offset = get_pn_offset(buf);
    size_t sample_offset = pn_offset + 4;
    memcpy(sample, buf + sample_offset, 16);
    debug_print("sample [%02x%02x..%02x%02x]", sample[0], sample[1], sample[14], sample[15]);

    uint8_t mask[5];
    compute_mask(key, sizeof(key), sample, sizeof(sample), mask);
    debug_print("mask [%02x%02x%02x%02x%02x]", mask[0], mask[1], mask[2], mask[3], mask[4]);

    // unprotect the first byte
    if (is_long_packet(buf)) {
        // Long header: 4 bits masked
        buf[0] ^= mask[0] & 0x0f;
    } else {
        // Short header: 5 bits masked
        buf[0] ^= mask[0] & 0x1f;
    }
    debug_print("unmasked byte[0] to %02x", (uint8_t)buf[0]);

    // unprotect the packet number
    size_t pn_length = (buf[0] & 0x03) + 1;
    for (size_t i = 0; i < pn_length; i++) {
        buf[pn_offset + i] ^= mask[i+1];
        debug_print("unmasked len[%d] to %02x", (int)i, (uint8_t)buf[pn_offset + i]);
    }

    if (fwrite(buf, 1, nr, stdout) != nr) {
        die("short write");
    }

    // copy the remainder from stdin to stdout
    while (!feof(stdin)) {
        size_t nr = fread(buf, 1, sizeof(buf), stdin);
        if (fwrite(buf, 1, nr, stdout) != nr) {
            die("short write");
        }
    }

    exit(0);
}
