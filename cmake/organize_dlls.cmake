if (NOT DEFINED TARGET_DIR)
    message(FATAL_ERROR "TARGET_DIR is not defined")
endif()

set(DLL_DIR "${TARGET_DIR}/dlls")
file(MAKE_DIRECTORY "${DLL_DIR}")

file(GLOB ROOT_DLLS "${TARGET_DIR}/*.dll")
foreach(DLL_FILE IN LISTS ROOT_DLLS)
    get_filename_component(DLL_NAME "${DLL_FILE}" NAME)
    file(RENAME "${DLL_FILE}" "${DLL_DIR}/${DLL_NAME}")
endforeach()
