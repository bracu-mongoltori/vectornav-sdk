cmake_minimum_required(VERSION 3.16)
project(SaveNonMatchingConfiguration VERSION 1.0 LANGUAGES CSharp)

set(NET_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../../../net)
if (NOT NET_SDK_PATH)
  set(NET_SDK_PATH ${NET_ROOT_PATH}/VnSdk_Net.dll)
endif()

add_executable(${PROJECT_NAME} ${DESIGNER_FILES} ${XAML_FILES} SaveNonMatchingConfiguration.cs)

set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DOTNET_REFERENCES "System")

set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DOTNET_REFERENCE_VnSdk_Net ${NET_SDK_PATH})
