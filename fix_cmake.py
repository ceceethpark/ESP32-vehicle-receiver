#!/usr/bin/env python3

cmake_file = r"C:\Users\thpark\SynologyDrive\6k2jvr_work\prj_ESP32\esp32_micro_hub\components\micro_ros_espidf_component\micro_ros_src\src\Micro-XRCE-DDS-Client\CMakeLists.txt"

with open(cmake_file, 'r') as f:
    lines = f.readlines()

# Remove lines 321-324 (VERSION, ${PROJECT_VERSION}, SOVERSION, ${...})
# These are the lines with VERSION and SOVERSION, not C_STANDARD
new_lines = []
for i, line in enumerate(lines):
    line_num = i + 1
    # Skip lines 321, 322, 323, 324 (VERSION and SOVERSION properties)
    if line_num in [321, 322, 323, 324]:
        print(f"Removing line {line_num}: {line.strip()}")
        continue
    new_lines.append(line)

with open(cmake_file, 'w') as f:
    f.writelines(new_lines)

print(f"\nModified {cmake_file}")
print("Removed VERSION/SOVERSION properties (lines 321-324)")
