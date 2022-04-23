include_directories(include)
add_library(hwLockCtrl include/hwLockCtrl.h src/hwLockCtrl.c)
target_include_directories(hwLockCtrl PUBLIC include)
