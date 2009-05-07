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

uint8_t buffer[] = {
    1,2,3,4,0x1b,5,6,7
};

int main(int argc, char **argv)
{
    nxtusb_t nxt;
    nxterr_t err;
    int luerr;

    err = nxtusb_new(&nxt, &luerr);
    if (err != NXERR_SUCCESS) {
        printf("%s\n", nxtusb_geterr(err));
    } else {
        nxtusb_send(nxt, (void *)buffer, sizeof(buffer), &luerr);
    }

    nxtusb_free(nxt);
    return 0;
}
