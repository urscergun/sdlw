# sdlwEmbed.cmake - helper to bake a binary file into a compiled C byte array.
#
# Provided both in the source tree and in the installed package, so apps that
# consume sdlw (via find_package) can embed their own fonts/assets the same way
# the bundled example does.
#
#   sdlw_embed_asset(<input-file> <symbol> <out-var>)
#     Generates "<symbol>.c" defining:
#         const unsigned char <symbol>[]     = { ... };
#         const unsigned int  <symbol>_len   = <N>;
#     and returns the generated .c path in <out-var>. Add it to your target's
#     sources and declare the symbols with extern "C".

# Locate bin2c.cmake: installed next to this file, or in the source tree's tools/.
if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/bin2c.cmake")
    set(SDLW_BIN2C_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/bin2c.cmake")
else()
    set(SDLW_BIN2C_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/../tools/bin2c.cmake")
endif()

function(sdlw_embed_asset input symbol out_var)
    set(gen_dir "${CMAKE_CURRENT_BINARY_DIR}/sdlw_generated")
    file(MAKE_DIRECTORY "${gen_dir}")
    set(out "${gen_dir}/${symbol}.c")
    add_custom_command(
        OUTPUT  "${out}"
        COMMAND ${CMAKE_COMMAND}
                -DINPUT=${input} -DOUTPUT=${out} -DSYMBOL=${symbol}
                -P "${SDLW_BIN2C_SCRIPT}"
        DEPENDS "${input}" "${SDLW_BIN2C_SCRIPT}"
        COMMENT "Embedding ${input}"
        VERBATIM)
    set(${out_var} "${out}" PARENT_SCOPE)
endfunction()
