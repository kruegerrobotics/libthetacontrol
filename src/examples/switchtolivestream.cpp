#include <iostream>

#include "../thetacontrol.hpp"

// this program tries to find a theta v connected via usb and switches
// switches its mode to "live streaming" - the devices must be awake

int main() {
  std::cout << "Starting example switch to stream mode" << std::endl;
  ThetaControl thetaCtrl("RICOH THETA V");

  std::cout << "Scanning usb connections to find the ricoh Theta V"
            << std::endl;
  if (thetaCtrl.detectFirstThetaV()) {
    std::cout << "theta found" << std::endl;
  } else {
    std::cout << "Error: no theta found - aborting" << std::endl;
    return -1;
  }

  std::cout << "check if theta is in streaming mode" << std::endl;
  const int maxRetries = 3;
  int loopCount = 1;

  // this is dangerous when the device switches modes the USB connection and id
  // changes device changes from usb, id and type
  while (!thetaCtrl.isInStreamingMode()) {
    if (loopCount <= maxRetries) {
      std::cout << "theta is not in streaming mode - initiating mode switch - "
                   "attempt: "
                << loopCount << "/" << maxRetries << std::endl;
      thetaCtrl.switchToStreamingMode();
      loopCount++;
    } else {
      std::cout << "Error: theta is not ready to stream" << std::endl;
      return -1;
    }
  }
  std::cout << "theta is ready to stream" << std::endl;
  return 0;
}