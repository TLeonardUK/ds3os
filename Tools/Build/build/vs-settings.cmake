# ================================================================================================
#  DS3OS
#  Copyright (C) 2022 Tim Leonard
# ================================================================================================

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(WIN32)
    # We add this at the high level as these keep sneaking in  
    # in some of the third party library headers. windows.h is a monstrosity.
    add_definitions(-DNOMINMAX -DWIN32_LEAN_AND_MEAN)
endif()

if(WIN32)
    # Use parallel compilation, for some reason this isn't the default with cmake :(
    add_definitions(/MP)
endif()
