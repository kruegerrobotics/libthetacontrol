cmake_minimum_required(VERSION 3.18)
project(libthetacontrol)

#TODO
#find package libptp2
#find package libusb

#the library
set(LIB_HEADERS src/thetacontrol.hpp src/ptp-helpers.hpp)
set(LIB_SOURCES src/thetacontrol.cpp src/ptp-helpers.cpp)
add_library(thetacontrol STATIC ${LIB_HEADERS} ${LIB_SOURCES})
target_link_libraries(thetacontrol ptp2 usb)

#the examples
add_executable(switchToLivestream src/examples/switchtolivestream.cpp)
target_link_libraries(switchToLivestream thetacontrol)

add_executable(readBatteryRaw src/examples/readbatteryraw.cpp)
target_link_libraries(readBatteryRaw thetacontrol)

#installation (only lib)
#todo only one header should suffice --> only thetacontrol.hpp
install(FILES src/thetacontrol.hpp DESTINATION include)
install(FILES src/ptp-helpers.hpp DESTINATION include)
install(TARGETS thetacontrol DESTINATION lib)
