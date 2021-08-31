#include "thetacontrol.hpp"

#include <cstring>
#include <iostream>

#include <usb.h>

#include "ptp-helpers.hpp"

ThetaControl::~ThetaControl() {
  std::cout << "closing Theta" << std::endl;
  if (isSessionOpen) {
    int res = ptp_closesession(&ptpParams);
    if (res != PTP_RC_OK) {
      std::cout << "unable to close ptp session" << std::endl;
    }
    close_usb(&ptpUSB, thetaDev);
  }
}

int ThetaControl::setProperty(uint32_t propertyCode, uint16_t value) {
  PTPContainer ptp;
  char *dpv = (char *)&value;
  memset(&ptp, 0, sizeof(ptp));

  ptp.Code = PTP_OC_SetDevicePropValue;
  ptp.Param1 = propertyCode;
  ptp.Nparam = 1;

  // little trick
  ptpParams.data = &ptpUSB;

  int res =
      ptp_transaction(&ptpParams, &ptp, PTP_DP_SENDDATA, sizeof(value), &dpv);
  if (res != PTP_RC_OK) {
    ptp_perror(&ptpParams, res);
    std::cout << "could not set device property" << std::endl;
    return -1;
  }
  if (dpv) {
    printf("%x is set to: %08x\n", propertyCode, *((uint8_t *)dpv));
  }
  return 0;
}

int ThetaControl::checkProperty(uint32_t propertyCode) {
  PTPContainer ptp;
  char *dpv = NULL;
  memset(&ptp, 0, sizeof(ptp));

  ptp.Code = PTP_OC_GetDevicePropValue;
  ptp.Param1 = propertyCode;
  ptp.Nparam = 1;

  // little trick
  ptpParams.data = &ptpUSB;

  int res = ptp_transaction(&ptpParams, &ptp, PTP_DP_GETDATA, 0, &dpv);
  if (res != PTP_RC_OK) {
    ptp_perror(&ptpParams, res);
    fprintf(stderr, "ERROR: "
                    "Could not get device property description!\n"
                    "Try to reset the camera.\n");
    return -1;
  }
  if (dpv) {
    printf("%x is set to: %08x\n", propertyCode, *((uint8_t *)dpv));
    return (int)*dpv;
  } else {
    return -1;
  }
}

bool ThetaControl::detectFirstThetaV() {
  struct usb_device *dev;
  struct usb_bus *bus;
  int found = 0;

  int count = 0;
  usb_init();
  usb_find_busses();
  usb_find_devices();
  bus = usb_get_busses();
  while (bus != NULL) {
    dev = static_cast<struct usb_device *>(bus->devices);

    while (dev != NULL) {
      /* if it's a PTP device try to talk to it */
      if (dev->config) {
        if ((dev->config->interface->altsetting->bInterfaceClass ==
             USB_CLASS_PTP)) {
          if (dev->descriptor.bDeviceClass != USB_CLASS_HUB) {
            PTPParams params;
            PTP_USB ptp_usb;
            PTPDeviceInfo deviceinfo;
            int res;

            find_endpoints(dev, &ptp_usb.inep, &ptp_usb.outep, &ptp_usb.intep);
            init_ptp_usb(&params, &ptp_usb, dev);

            res = ptp_opensession(&params, 1);
            if (res != PTP_RC_OK) {
              std::cout << "Could not open ptp session" << std::endl;
              usb_release_interface(
                  ptp_usb.handle,
                  dev->config->interface->altsetting->bInterfaceNumber);
              return false;
            }

            res = ptp_getdeviceinfo(&params, &deviceinfo);
            if (res != PTP_RC_OK) {
              std::cout << "Could not get device info " << std::endl;
              usb_release_interface(
                  ptp_usb.handle,
                  dev->config->interface->altsetting->bInterfaceNumber);
              return false;
            }

            printf("%s/%s\t0x%04X/0x%04X\t%s\n", bus->dirname, dev->filename,
                   dev->descriptor.idVendor, dev->descriptor.idProduct,
                   deviceinfo.Model);

            // if we wind the theta V - expection to have "RICOH THETA V" as
            // deviceinfo.Model
            const char *thetaVDescritpion = "RICOH THETA V";
            if (strcmp(deviceinfo.Model, thetaVDescritpion) == 0) {
              // this means we have found a theta
              // we store the device infos in the data structs
              thetaDev = dev;
              ptpUSB = ptp_usb;
              ptpParams = params;
              isSessionOpen = true;
              return true;
            } else {
              // not the theta but we need to close the session
              int res = ptp_closesession(&params);
              if (res != PTP_RC_OK) {
                std::cout << "Could not close session" << std::endl;
              }
              close_usb(&ptp_usb, dev);
            }
          }
        }
      }
      dev = dev->next;
    }
    bus = bus->next;
  }
  // loop through the bus
  return false;
}

int ThetaControl::disableSleepMode() {
  // sleep mode property 0xD80E
  // value 0x00 not sleeping / 0x01 sleeping
  // https://api.ricoh/docs/theta-usb-api/property/sleep_mode/
  const uint32_t sleepMode = 0xD80E;
  int res = setProperty(sleepMode, 0x00);
  return -1;
}

bool ThetaControl::isInStreamingMode() { // 0x5013 StillCaptureMode
  // https://api.ricoh/docs/theta-usb-api/property/still_capture_mode/
  // streaming mode is 0x8005
  int res = checkProperty(0x5013);
  if (res == 5) {
    return true;
  } else {
    return false;
  }
  // std::cout << __FUNCTION__ << " - result: " << res << std::endl;
}

bool ThetaControl::isSleeping() {
  int res = checkProperty(0xD80E);
  if (res == 1) {
    return true;
  } else {
    return false;
  }
}

bool ThetaControl::switchToStreamingMode() {
  setProperty(0x5013, 0x8005);
  return true;
}

bool ThetaControl::wakeUp() {
  setProperty(0xD80E, 0x00);
  return true;
}