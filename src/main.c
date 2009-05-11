#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "NxtAccess/nxtusb.h"

void dump(uint8_t A[], size_t len)
{
    int i;
    for (i = 0; i < len; i++)
        printf("0x%x ", A[i]);
    putchar(10);
}

void init(uint32_t buffer[], size_t len)
{
    int i;
    for (i = 0; i < len; i++)
        buffer[i] = 0x12345600 + i;
}

static const int BUFLEN = 70;

static inline void print_status(const char *str)
{
    fprintf(stderr, "\x1b[42;30m%-80s\x1b[0m\n", str);
}

int main(int argc, char **argv)
{
    nxtusb_t nxt;
    nxterr_t err;
    int luerr;
    uint32_t buffer[BUFLEN];
    
    err = nxtusb_new(&nxt, &luerr);
    if (err != NXERR_SUCCESS) {
        printf("%s\n", nxtusb_geterr(err));
    } else {
        init(buffer, BUFLEN);
        print_status("Starting...");
        if (nxtusb_send(nxt, (void *)buffer, BUFLEN, &luerr) != 0)
            print_status("Transmission error");
        print_status("Done");
    }
    nxtusb_free(nxt);
    return 0;
}
