# Script for macOS code signing

if(NOT DEFINED APP_DIR)
  set(APP_DIR "${CPACK_TEMPORARY_INSTALL_DIRECTORY}/ALL_IN_ONE")
endif()

execute_process(COMMAND codesign --force --sign - "${APP_DIR}/LoopAuditioneer.app")
message("Checking code signature...")
execute_process(COMMAND codesign --verify --deep "${APP_DIR}/LoopAuditioneer.app")
