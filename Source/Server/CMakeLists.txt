set(PROJECT_NAME Server)

set (Protobuf_USE_STATIC_LIBS ON)

find_package(Protobuf REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)
find_package(curl CONFIG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(OpenSSL REQUIRED)

PROTOBUF_GENERATE_CPP(PROTO_SRC PROTO_HEADER
        "${CMAKE_SOURCE_DIR}/Protobuf/FpdLogMessage.proto"
        "${CMAKE_SOURCE_DIR}/Protobuf/Frpg2PlayerData.proto"
        "${CMAKE_SOURCE_DIR}/Protobuf/Frpg2RequestMessage.proto")

# FIXME: should be a manual list
file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)

add_executable(${PROJECT_NAME} ${SOURCES} ${PROTO_SRC})

set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 17
        CXX_EXTENSIONS OFF)

target_include_directories(${PROJECT_NAME} PRIVATE
        "${CMAKE_CURRENT_BINARY_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty")

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

