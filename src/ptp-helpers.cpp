
/* This code in this file is heavily based, in other words copied from libptp2
   the functions uses here from the ptpcam which is part of the same library.
   The orignal can be found here: http://libptp.sourceforge.net/ and this is
   under the GNU Public License which required that this reuse (if I understand
   correct) also is required to stand under the same license. Thus, as from the
   excerpt from the original the below applies.
*/

/* 	Excerp from the original:*/
/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#include "ptp-helpers.hpp"
#include <cstdio>
#include <cstring>

#include <usb.h>

void find_endpoints(struct usb_device *dev, int *inep, int *outep, int *intep) {
  int i, n;
  struct usb_endpoint_descriptor *ep;

  ep = dev->config->interface->altsetting->endpoint;
  n = dev->config->interface->altsetting->bNumEndpoints;

  for (i = 0; i < n; i++) {
    if (ep[i].bmAttributes == USB_ENDPOINT_TYPE_BULK) {
      if ((ep[i].bEndpointAddress & USB_ENDPOINT_DIR_MASK) ==
          USB_ENDPOINT_DIR_MASK) {
        *inep = ep[i].bEndpointAddress;

        fprintf(stderr, "Found inep: 0x%02x\n", *inep);
      }
      if ((ep[i].bEndpointAddress & USB_ENDPOINT_DIR_MASK) == 0) {
        *outep = ep[i].bEndpointAddress;
        fprintf(stderr, "Found outep: 0x%02x\n", *outep);
      }
    } else if ((ep[i].bmAttributes == USB_ENDPOINT_TYPE_INTERRUPT) &&
               ((ep[i].bEndpointAddress & USB_ENDPOINT_DIR_MASK) ==
                USB_ENDPOINT_DIR_MASK)) {
      *intep = ep[i].bEndpointAddress;
      fprintf(stderr, "Found intep: 0x%02x\n", *intep);
    }
  }
}

void init_ptp_usb(PTPParams *params, PTP_USB *ptp_usb, struct usb_device *dev) {
  usb_dev_handle *device_handle;

  params->write_func = ptp_write_func;
  params->read_func = ptp_read_func;
  params->check_int_func = ptp_check_int;
  params->check_int_fast_func = ptp_check_int;
  params->error_func = ptpcam_error;
  params->debug_func = ptpcam_debug;
  params->sendreq_func = ptp_usb_sendreq;
  params->senddata_func = ptp_usb_senddata;
  params->getresp_func = ptp_usb_getresp;
  params->getdata_func = ptp_usb_getdata;
  params->data = ptp_usb;
  params->transaction_id = 0;
  params->byteorder = PTP_DL_LE;

  if ((device_handle = usb_open(dev))) {
    if (!device_handle) {
      perror("usb_open()");
      throw "failed to open usb connection";
    }
    ptp_usb->handle = device_handle;
    usb_set_configuration(device_handle, dev->config->bConfigurationValue);
    usb_claim_interface(device_handle,
                        dev->config->interface->altsetting->bInterfaceNumber);
  }
}

void close_usb(PTP_USB *ptp_usb, struct usb_device *dev) {
  // clear_stall(ptp_usb);
  usb_release_interface(ptp_usb->handle,
                        dev->config->interface->altsetting->bInterfaceNumber);
  usb_reset(ptp_usb->handle);
  usb_close(ptp_usb->handle);
}

static short ptp_write_func(unsigned char *bytes, unsigned int size,
                            void *data) {
  int result;
  PTP_USB *ptp_usb = (PTP_USB *)data;

  result = usb_bulk_write(ptp_usb->handle, ptp_usb->outep, (char *)bytes, size,
                          ptpcam_usb_timeout);
  if (result >= 0)
    return (PTP_RC_OK);
  else {
    perror("usb_bulk_write");
    return PTP_ERROR_IO;
  }
}

static short ptp_read_func(unsigned char *bytes, unsigned int size,
                           void *data) {
  int result = -1;
  PTP_USB *ptp_usb = (PTP_USB *)data;
  int toread = 0;
  signed long int rbytes = size;

  do {
    bytes += toread;
    if (rbytes > PTPCAM_USB_URB)
      toread = PTPCAM_USB_URB;
    else
      toread = rbytes;
    result = usb_bulk_read(ptp_usb->handle, ptp_usb->inep, (char *)bytes,
                           toread, ptpcam_usb_timeout);
    /* sometimes retry might help */
    if (result == 0)
      result = usb_bulk_read(ptp_usb->handle, ptp_usb->inep, (char *)bytes,
                             toread, ptpcam_usb_timeout);
    if (result < 0)
      break;
    rbytes -= PTPCAM_USB_URB;
  } while (rbytes > 0);

  if (result >= 0) {
    return (PTP_RC_OK);
  } else {
    perror("usb_bulk_read");
    return PTP_ERROR_IO;
  }
}

static short ptp_check_int(unsigned char *bytes, unsigned int size,
                           void *data) {
  int result;
  PTP_USB *ptp_usb = (PTP_USB *)data;

  result = usb_bulk_read(ptp_usb->handle, ptp_usb->intep, (char *)bytes, size,
                         ptpcam_usb_timeout);
  if (result == 0)
    result = usb_bulk_read(ptp_usb->handle, ptp_usb->intep, (char *)bytes, size,
                           ptpcam_usb_timeout);
  fprintf(stderr, "USB_BULK_READ returned %i, size=%i\n", result, size);

  if (result >= 0) {
    return result;
  } else {
    perror("ptp_check_int");
    return result;
  }
}

void ptpcam_debug(void *data, const char *format, va_list args) {
  // hopefully not needed
  if (true < 2)
    return;
  vfprintf(stderr, format, args);
  fprintf(stderr, "\n");
  fflush(stderr);
}

void ptpcam_error(void *data, const char *format, va_list args) {
  vfprintf(stderr, format, args);
  fprintf(stderr, "\n");
  fflush(stderr);
}
