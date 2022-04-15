# ================================================================================================
#  DS3OS
#  Copyright (C) 2021 Tim Leonard
# ================================================================================================

include_external_msproject(
        Loader
        "${CMAKE_CURRENT_SOURCE_DIR}/Loader.csproj"
)

util_setup_folder_structure(Loader SOURCES "DS3OS")