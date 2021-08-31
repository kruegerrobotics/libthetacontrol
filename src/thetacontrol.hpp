#ifndef THETACONTROL_H_TY54LDG
#define THETACONTROL_H_TY54LDG

#include "ptp-helpers.hpp"
#include <iostream>
#include <string>

class ThetaControl {
private:
  std::string deviceName;

  // specific datastrucutes from libusb and libptp
  // usb_bus * usbBusses = nullptr;
  PTPParams ptpParams;
  PTP_USB ptpUSB;
  struct usb_device
      *thetaDev; // this is the specific usb device which is the theta

  bool isSessionOpen = false;

  // initialises the libusb api
  // stores the usb to private member variable
  // void initUSB();

public:
  ThetaControl(std::string deviceName) : deviceName(deviceName) {}

  ~ThetaControl();

  // detects if a theta v/respective the first theta that is connected via USB
  // returns true if a device was found
  bool detectFirstThetaV();

  // puts the theta from sleeo mode to awake mode
  int disableSleepMode();

  // returns true if the device is sleeping
  bool isSleeping();

  // returns true if the device is in streaming mode
  bool isInStreamingMode();

  bool switchToStreamingMode();

  bool wakeUp();

  // RAW functions to be debated if they should in the public
  // return value for selected property
  // returns negative value (TODO check if APIs would only return positive
  // values)
  int checkProperty(uint32_t propertyCode);

  int setProperty(uint32_t propertyCode, uint16_t value);
};

#endif