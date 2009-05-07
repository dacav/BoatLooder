#ifndef __NXTUSB_H__
#define __NXTUSB_H__

#include <stdlib.h>

typedef enum {
    NXERR_SUCCESS = 0,
    NXERR_SAMBA = 1,
    NXERR_NOTFOUND = 2,
    NXERR_LIBUSB = 3
} nxterr_t;
typedef struct nxtusb * nxtusb_t;

nxterr_t nxtusb_new(nxtusb_t *nxt, int *libusb_err);
void nxtusb_free(nxtusb_t e);
nxterr_t nxtusb_send(nxtusb_t nxt, void *buffer, size_t len, int *libusb_err);
const char *nxtusb_geterr(nxterr_t e);

#endif /* __NXTUSB_H__ */
