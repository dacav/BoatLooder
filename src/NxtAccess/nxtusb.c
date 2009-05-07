#include "nxtusb.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <libusb-1.0/libusb.h>

/* Lego NXT keys, used by NxOS as well */
static const uint16_t nxt_vendor_id = 0x0694;
static const uint16_t nxt_product_id = 0xff00;

/* SAM-BA keys */
static const uint16_t samba_vendor_id = 0x03eb;
static const uint16_t samba_product_id = 0x6124;

/* Transmission constants */
static const int tx_endpoint = 1;
static const int tx_timeout = 0;

/* Escapes */
static const uint8_t esc = 0x1b;
static const uint8_t eot = 0x04;

/* Nxt buffer size */
static const uint32_t usb_buflen = 5;

struct nxtusb {
    struct libusb_context *context;
    struct libusb_device_handle *handle;
    uint8_t *buffer;
};

static const char *errmsg[] = {
    "Success",
    "The NXT uses SAM-BA",
    "NXT not found",
    "USB access error"
};

const char *nxtusb_geterr(nxterr_t e)
{
    return errmsg[e];
}

static inline int send_raw(struct libusb_device_handle *handle,
                           uint8_t *buffer, size_t len, int *transf)
{
    return libusb_bulk_transfer(handle, tx_endpoint, buffer, len, transf,
                                tx_timeout);
}

static inline void chunk(struct libusb_device_handle *handle,
                         uint8_t *buffer, size_t *fill)
{
    size_t f;
    int32_t t;

    f = *fill;
    if (f % usb_buflen == 0 && f != 0) {
        send_raw(handle, buffer, f, &t);
        *fill = 0;
    }
}

nxterr_t nxtusb_send(nxtusb_t nxt, void *buffer, size_t len,
                     int *libusb_err)
{
    uint32_t i, j;
    int32_t t;
    uint8_t *out, val;
    struct libusb_device_handle *handle;

    out = nxt->buffer;
    handle = nxt->handle;

    for (i = 0, j = 0; i < len; i++) {
        val = ((uint8_t *)buffer)[i];
        if (val == esc || val == eot) {
            out[j++] = esc;
            chunk(handle, out, &j);
        }
        out[j++] = val;
        chunk(handle, out, &j);
    }
    out[j++] = eot;
    send_raw(handle, out, j, &t);
    return NXERR_SUCCESS;
}

nxterr_t nxtusb_new(nxtusb_t *nxt, int *libusb_err)
{
    nxtusb_t ret;
    int err;
    libusb_device_handle *handle;

    ret = malloc(sizeof(struct nxtusb));
    assert(ret != NULL);
    ret->buffer = malloc(sizeof(uint8_t) * usb_buflen);
    assert(ret->buffer != NULL);

    if ((err = libusb_init(&ret->context)) != 0) {
        *libusb_err = err;
        err = NXERR_LIBUSB;
        goto fail0;
    }

    /* Assigning handle */
    handle = libusb_open_device_with_vid_pid(ret->context,
                                             nxt_vendor_id,
                                             nxt_product_id);
    if (handle == NULL) {
        /* No such usb device. Checking if there's a SAM-BA device */
        handle = libusb_open_device_with_vid_pid(ret->context,
                                                 samba_vendor_id,
                                                 samba_product_id);
        if (handle != NULL) {
            err = NXERR_SAMBA;
            libusb_close(handle);
        } else {
            err = NXERR_NOTFOUND;
        }
        goto fail1;
    }
    ret->handle = handle;
    *nxt = ret;
    return NXERR_SUCCESS;

  fail1:
    libusb_exit(ret->context);
  fail0:
    free(ret);
    *nxt = NULL;
    return err;
}

void nxtusb_free(nxtusb_t u)
{
    if (u == NULL)
        return;
    libusb_close(u->handle);
    libusb_exit(u->context);
    free(u->buffer);
    free(u);
}

