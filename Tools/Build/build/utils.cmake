# ================================================================================================
#  DS3OS
#  Copyright (C) 2022 Tim Leonard
# ================================================================================================

# Sets up the folder structure in our ide to match our directory structure
# and custom project nesting group.

macro(util_setup_folder_structure name sources group)
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${${sources}})
    set_target_properties(${name} PROPERTIES FOLDER ${group})
endmacro()


macro(util_copy_all_dlls_to_output name)
    add_custom_command(TARGET ${name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:${name}> $<TARGET_FILE_DIR:${name}>
        COMMAND_EXPAND_LISTS
    )
endmacro()


