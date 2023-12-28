# ================================================================================================
#  DS3OS
#  Copyright (C) 2022 Tim Leonard
# ================================================================================================

add_subdirectory(ThirdParty)

if (WIN32)
    add_subdirectory(Loader)
    add_subdirectory(Injector)
endif()

add_subdirectory(Server)
add_subdirectory(Server.DarkSouls3)
add_subdirectory(Server.DarkSouls2)
add_subdirectory(Shared)

