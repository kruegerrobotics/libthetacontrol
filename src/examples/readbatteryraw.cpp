#include <iostream>

#include "../thetacontrol.hpp"

// this program tries to find a theta v connected via usb and read
// out the battery level via raw a ptp message - the devices must be awake
// the message can be found here: https://api.ricoh/docs/theta-usb-api/

int main() {
  std::cout << "Starting example read battery status" << std::endl;
  ThetaControl thetaCtrl("RICOH THETA V");

  std::cout << "Scanning usb connections to find the ricoh Theta V"
            << std::endl;
  if (thetaCtrl.detectFirstThetaV()) {
    std::cout << "theta found" << std::endl;
  } else {
    std::cout << "Error: no theta found - aborting" << std::endl;
    return -1;
  }

  std::cout << "checking the battery" << std::endl;
  // the battery level is property 0x5001
  int batLevel = thetaCtrl.checkProperty(0x5001);
  std::cout << "The battery is " << batLevel << " %" << std::endl;
  return 0;
}