cmake_minimum_required(VERSION 3.16)
add_compile_definitions(NOMINMAX=1)
add_compile_definitions(THREADING_ENABLE=true)
add_compile_definitions(__CLI__=true)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CPP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../cpp)
set(PLUGIN_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../cpp/plugins)
set(CLI_PLUGIN_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../plugins)

add_subdirectory(${CPP_ROOT} oVnSensor)

if(EXISTS "${PLUGIN_ROOT}/Calibration")
	add_subdirectory(${PLUGIN_ROOT}/Calibration CalibrationPlugin)
	set(__CALIBRATION__ TRUE)
	add_compile_definitions(__CALIBRATION__)
endif()

if(EXISTS "${PLUGIN_ROOT}/DataExport")
	add_subdirectory(${PLUGIN_ROOT}/DataExport DataExportPlugin)
	set(__DATA_EXPORT__ TRUE)
	add_compile_definitions(__DATA_EXPORT__)
endif()

if(EXISTS "${PLUGIN_ROOT}/FirmwareProgrammer")
	add_subdirectory(${PLUGIN_ROOT}/FirmwareProgrammer FirmwareProgrammerPlugin)
	set(__FIRMWARE_PROGRAMMER__ TRUE)
	add_compile_definitions(__FIRMWARE_PROGRAMMER__)
endif()

if(EXISTS "${PLUGIN_ROOT}/RegisterScan")
	add_subdirectory(${PLUGIN_ROOT}/RegisterScan RegisterScanPlugin)
	set(__REGISTER_SCAN__ TRUE)
	add_compile_definitions(__REGISTER_SCAN__)
endif()

if(EXISTS "${PLUGIN_ROOT}/Logger")
	set(__LOGGER__ TRUE)
	add_compile_definitions(__LOGGER__)
endif()

if(EXISTS "${PLUGIN_ROOT}/Math")
	set(__MATH__ TRUE)
	add_compile_definitions(__MATH__)
endif()

if(EXISTS "${CPP_ROOT}/tests")
	set(__TESTS__ TRUE)
	add_compile_definitions(__TESTS__)
endif()

set(SOURCES
	AssemblyInfo.cpp
	CLI_Sensor.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_link_libraries(${PROJECT_NAME} oVnSensor)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_include_directories(${PROJECT_NAME} PRIVATE oVnSensor)

if(__CALIBRATION__)
	target_include_directories(${PROJECT_NAME} PRIVATE CalibrationPlugin)
	target_include_directories(${PROJECT_NAME} PUBLIC ${CLI_PLUGIN_ROOT})
	target_link_libraries(${PROJECT_NAME} CalibrationPlugin)
endif()

if(__DATA_EXPORT__)
	target_include_directories(${PROJECT_NAME} PRIVATE DataExportPlugin)
	target_include_directories(${PROJECT_NAME} PUBLIC ${CLI_PLUGIN_ROOT})
	target_link_libraries(${PROJECT_NAME} DataExportPlugin)
endif()

if(__FIRMWARE_PROGRAMMER__)
	target_include_directories(${PROJECT_NAME} PRIVATE FirmwareProgrammerPlugin)
	target_include_directories(${PROJECT_NAME} PUBLIC ${CLI_PLUGIN_ROOT})
	target_link_libraries(${PROJECT_NAME} FirmwareProgrammerPlugin)
endif()

if(__REGISTER_SCAN__)
	target_include_directories(${PROJECT_NAME} PRIVATE RegisterScanPlugin)
	target_include_directories(${PROJECT_NAME} PUBLIC ${CLI_PLUGIN_ROOT})
	target_link_libraries(${PROJECT_NAME} RegisterScanPlugin)
endif()

if(__LOGGER__)
	target_include_directories(${PROJECT_NAME} PRIVATE ${PLUGIN_ROOT}/Logger)
	target_include_directories(${PROJECT_NAME} PUBLIC ${CLI_PLUGIN_ROOT})
endif()

if(__MATH__)
	target_include_directories(${PROJECT_NAME} PRIVATE ${PLUGIN_ROOT}/Math)
	target_include_directories(${PROJECT_NAME} PUBLIC ${CLI_PLUGIN_ROOT})
endif()


SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES COMMON_LANGUAGE_RUNTIME "")
STRING(REPLACE "/EHsc" "/EHa" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
STRING(REPLACE "/RTC1" "" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})

# Copy output dll to "net" directory
set(DLL_OUTPUT_DIR ${CPP_ROOT}/../net/)

if(NOT EXISTS "${DLL_OUTPUT_DIR}")
	file(MAKE_DIRECTORY "${DLL_OUTPUT_DIR}")
endif()

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:VnSdk_Net> ${DLL_OUTPUT_DIR})
