add_executable(game
    src/main.cpp
)

target_link_libraries(game PRIVATE nebula)

# Tells the binary where assets live at compile time
target_compile_definitions(game PRIVATE
    ASSETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/"
)

# Still copy assets to build dir for release builds
add_custom_command(TARGET game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:game>/assets
)
