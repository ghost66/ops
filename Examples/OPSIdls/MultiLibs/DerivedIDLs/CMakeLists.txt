
# list of source files that, when changed, require regeneration of target
set(CURRENT_SRCS
    opsproject/project.properties
    src/DerivedIDLs/FooData.idl
    )

# the output file that we want
set(JAR_OUTPUT ${CMAKE_BINARY_DIR}/bin/DerivedIDLs.jar)

add_custom_command(OUTPUT ${JAR_OUTPUT}
  COMMAND ${OPSC_COMPILER} -P ${CMAKE_CURRENT_SOURCE_DIR} -g ALL -gpr ../../../../../../Ada/
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/Generated/Java/DerivedIDLs.jar ${JAR_OUTPUT}
  DEPENDS ${CURRENT_SRCS}
  COMMENT "Generating ${JAR_OUTPUT} from ${CMAKE_CURRENT_SOURCE_DIR}"
  VERBATIM
  )

add_custom_target(DerivedIDLs ALL DEPENDS ${JAR_OUTPUT})

add_dependencies(DerivedIDLs BaseIDLs)
