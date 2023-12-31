# ================================================================================================
#  DS3OS
#  Copyright (C) 2021 Tim Leonard
# ================================================================================================

project(Shared C CXX)

SET(SOURCES   
    Core/Crypto/Cipher.h
    Core/Crypto/CWCCipher.cpp
    Core/Crypto/CWCCipher.h
    Core/Crypto/CWCClientUDPCipher.cpp
    Core/Crypto/CWCClientUDPCipher.h
    Core/Crypto/CWCServerUDPCipher.cpp
    Core/Crypto/CWCServerUDPCipher.h
    Core/Crypto/RSACipher.cpp
    Core/Crypto/RSACipher.h
    Core/Crypto/RSAKeyPair.cpp
    Core/Crypto/RSAKeyPair.h
    Core/Network/NetConnection.h
    Core/Network/NetConnectionTCP.cpp
    Core/Network/NetConnectionTCP.h
    Core/Network/NetConnectionUDP.cpp
    Core/Network/NetConnectionUDP.h
    Core/Network/NetHttpRequest.cpp
    Core/Network/NetHttpRequest.h
    Core/Network/NetIPAddress.cpp
    Core/Network/NetIPAddress.h
    Core/Network/NetUtils.cpp
    Core/Network/NetUtils.h
    Core/Utils/Compression.cpp
    Core/Utils/Compression.h
    Core/Utils/DebugCounter.cpp
    Core/Utils/DebugCounter.h
    Core/Utils/DebugObjects.cpp
    Core/Utils/DebugObjects.h
    Core/Utils/DebugObjects.inc
    Core/Utils/DebugTimer.cpp
    Core/Utils/DebugTimer.h
    Core/Utils/DiffTracker.cpp
    Core/Utils/DiffTracker.h
    Core/Utils/Endian.h
    Core/Utils/Enum.h
    Core/Utils/EnumDef.inc
    Core/Utils/Event.h
    Core/Utils/File.cpp
    Core/Utils/File.h
    Core/Utils/Logging.cpp
    Core/Utils/Logging.h
    Core/Utils/Random.cpp
    Core/Utils/Random.h
    Core/Utils/Strings.cpp
    Core/Utils/Strings.h
    Core/Utils/WinApi.cpp
    Core/Utils/WinApi.h
    Core/Utils/Rtti.cpp
    Core/Utils/Rtti.h
    Core/Utils/Protobuf.cpp
    Core/Utils/Protobuf.h

    
    Game/GameType.cpp
    Game/GameType.h

    Platform/Platform.h
)
 
if (WIN32)
    SET(PLATFORM_SOURCES
        Platform/Win32/Win32Platform.cpp
    )
    SET(PLATFORM_LIBRARIES         
        dbghelp.lib 
        Rpcrt4.lib
    )
elseif (UNIX)
    SET(PLATFORM_SOURCES
        Platform/Linux/LinuxPlatform.cpp
    )
    SET(PLATFORM_LIBRARIES         
        uuid
    )
endif()
 
add_library(${PROJECT_NAME} STATIC ${SOURCES} ${PLATFORM_SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../")

target_compile_definitions(${PROJECT_NAME} PRIVATE -D_CRT_SECURE_NO_WARNINGS -D_WINSOCK_DEPRECATED_NO_WARNINGS -D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)

target_link_libraries(
    ${PROJECT_NAME}
    aes_modes
    lib_generic_c
    zlib
    libcurl
    crypto
    ssl
    libprotobuf-lite
    ${PLATFORM_LIBRARIES}
)

util_setup_folder_structure(${PROJECT_NAME} SOURCES "DS3OS")
