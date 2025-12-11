#!/usr/bin/env python3
# rmw_test_fixture를 rmw 의존성 없이 빌드되도록 수정

cmake_file = "/mnt/c/Users/thpark/SynologyDrive/6k2jvr_work/prj_ESP32/esp32_micro_hub/components/micro_ros_espidf_component/micro_ros_dev/src/ament_cmake_ros/rmw_test_fixture/CMakeLists.txt"

with open(cmake_file, 'r') as f:
    lines = f.readlines()

# 6번째 줄 (index 5): find_package(rmw REQUIRED)
lines[5] = '# ' + lines[5]

# 14-16번째 줄 (index 13-15): target_link_libraries
lines[13] = '# ' + lines[13]
lines[14] = '# ' + lines[14]
lines[15] = '# ' + lines[15]

# 32번째 줄 (index 31): ament_export_dependencies(rmw)
lines[31] = '# ' + lines[31]

with open(cmake_file, 'w') as f:
    f.writelines(lines)

print(f"Modified: {cmake_file}")
print("rmw dependency removed")
