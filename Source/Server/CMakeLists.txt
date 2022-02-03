set(PROJECT_NAME Server)

set(Protobuf_USE_STATIC_LIBS ON)

find_package(Protobuf REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)
find_package(CURL CONFIG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(OpenSSL REQUIRED)

PROTOBUF_GENERATE_CPP(PROTOBUF_SOURCES PROTO_HEADER
        "${CMAKE_SOURCE_DIR}/Protobuf/FpdLogMessage.proto"
        "${CMAKE_SOURCE_DIR}/Protobuf/Frpg2PlayerData.proto"
        "${CMAKE_SOURCE_DIR}/Protobuf/Frpg2RequestMessage.proto")

# FIXME: should be a manual list
file(GLOB_RECURSE CLIENT_SOURCES ${CMAKE_CURRENT_SOURCE_DIR} Client/*.cpp)
file(GLOB_RECURSE CONFIG_SOURCES ${CMAKE_CURRENT_SOURCE_DIR} Config/*.cpp)
file(GLOB_RECURSE SERVER_SOURCES ${CMAKE_CURRENT_SOURCE_DIR} Server/*.cpp)
file(GLOB_RECURSE CORE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR} Core/*.cpp)

if (WIN32)
    set(PLATFORM_SOURCES Platform/Win32/Win32Platform.cpp)
elseif (UNIX)
    set(PLATFORM_SOURCES Platform/Linux/LinuxPlatform.cpp)
endif()

add_executable(${PROJECT_NAME}
        ${PROTOBUF_SOURCES}
        ${CLIENT_SOURCES}
        ${CONFIG_SOURCES}
        ${CORE_SOURCES}
        ${SERVER_SOURCES}
        ${PLATFORM_SOURCES}
        Entry.cpp)


set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 17
        CXX_EXTENSIONS OFF)

target_include_directories(${PROJECT_NAME} PRIVATE
        "${CMAKE_CURRENT_BINARY_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty")

target_compile_options(${PROJECT_NAME} PRIVATE -fpermissive)
# FIXME: vcpkg targets make this incompatible with system packages, use CURL_LBRARIES, etc. instead.
target_link_libraries(${PROJECT_NAME} PRIVATE
        lib_generic_c
        aes_modes
        steam_api
        protobuf::libprotobuf-lite
        unofficial::sqlite3::sqlite3
        ZLIB::ZLIB
        CURL::libcurl
        OpenSSL::SSL
        OpenSSL::Crypto
        )

