# ================================================================================================
#  DS3OS
#  Copyright (C) 2022 Tim Leonard
# ================================================================================================


add_subdirectory(ThirdParty)

if (WIN32)
    add_subdirectory(Loader)
endif()

add_subdirectory(Server)

