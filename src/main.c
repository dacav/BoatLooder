#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "NxtAccess/nxtusb.h"

#define VECTOR_LEN 32
struct act_rec {
    uint8_t vector[VECTOR_LEN];  /* Guest interrupt vector, to load at RAM init */
    struct {
        uint32_t activation;     /* Guest activation address */
        uint32_t sec_data;       /* Section .data */
        uint32_t sec_bss;        /* Section .bss */
        uint32_t sec_stacks;     /* Modes stacks */
    } addr;
};

int main(int argc, char **argv)
{
    nxtusb_t nxt;
    nxterr_t err;
    int luerr;
    struct act_rec rec;
    
    err = nxtusb_new(&nxt, &luerr);
    if (err != NXERR_SUCCESS) {
        printf("%s\n", nxtusb_geterr(err));
    } else {
        rec.addr.activation = 1;
        rec.addr.sec_data = 2;
        rec.addr.sec_bss = 3;
        rec.addr.sec_stacks = 4;
        nxtusb_send(nxt, (void *) &rec, sizeof(struct act_rec), &luerr);
    }
    nxtusb_free(nxt);
    return 0;
}
