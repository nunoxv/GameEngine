# xengine_copy_shaders(<target>)
# Copies the engine's GLSL shaders next to the executable after each build; the engine
# looks for "<executable dir>/shaders" by default.
function(xengine_copy_shaders target)
  add_custom_command(TARGET ${target} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${XENGINE_SHADER_DIR}" "$<TARGET_FILE_DIR:${target}>/shaders"
    COMMENT "Copying engine shaders")
endfunction()
