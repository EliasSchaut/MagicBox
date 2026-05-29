#
# PlatformIO pre-build script: inject a unique FIRMWARE_BUILD_ID on every build.
#
# Used by the EEPROM progress-save (src/story.cpp): the running firmware stores
# this id next to the saved story node. On power-up the saved id is only honored
# if it matches the firmware's id, so:
#   - power loss + same firmware  -> ids match  -> resume from saved node
#   - new firmware flashed         -> ids differ -> ignore save, start over
#
import time

Import("env")  # noqa: F821  (provided by PlatformIO/SCons)

build_id = int(time.time()) & 0xFFFFFFFF
env.Append(CPPDEFINES=[("FIRMWARE_BUILD_ID", "%uUL" % build_id)])
print("inject_build_id: FIRMWARE_BUILD_ID = %u" % build_id)
