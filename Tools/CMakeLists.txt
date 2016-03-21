
add_subdirectory(OPSCompilerLib)

add_subdirectory(IDLParser)

add_subdirectory(OPSReflectionLib)

option(OPS_BUILD_OPSCOMPILER "Enable to build OPSCompiler." OFF)
if(OPS_BUILD_OPSCOMPILER)
  message(WARNING " The OPSCompiler codebase is hopelessly outdated. Are you sure?")
  add_subdirectory(IDLCompiler)
endif()

add_subdirectory(opsc)


# install the latest template files
set(TMPL_PATH NBOPSIDLSupport/src/ops/netbeansmodules/idlsupport/templates)
set(OPS_TEMPLATE_FILES
  ${TMPL_PATH}/cppenumtemplate.tpl
  ${TMPL_PATH}/cpppublishertemplate.tpl
  ${TMPL_PATH}/cppsubscribertemplate.tpl
  ${TMPL_PATH}/cpptemplate.tpl
  ${TMPL_PATH}/cpptypefactorytemplate.tpl
  ${TMPL_PATH}/csenumtemplate.tpl
  ${TMPL_PATH}/cspublishertemplate.tpl
  ${TMPL_PATH}/cssubscribertemplate.tpl
  ${TMPL_PATH}/cstemplate.tpl
  ${TMPL_PATH}/cstypefactorytemplate.tpl
  ${TMPL_PATH}/debugproj.tpl
  ${TMPL_PATH}/javaconfig.tpl
  ${TMPL_PATH}/javaenumtemplate.tpl
  ${TMPL_PATH}/javapublishertemplate.tpl
  ${TMPL_PATH}/javasubscribertemplate.tpl
  ${TMPL_PATH}/javatemplate.tpl
  ${TMPL_PATH}/javatypefactorytemplate.tpl
  ${TMPL_PATH}/vs_pub_cpp.tpl
  ${TMPL_PATH}/vs_pub_proj.tpl
  ${TMPL_PATH}/vs_sln.tpl
  ${TMPL_PATH}/vs_sub_cpp.tpl
  ${TMPL_PATH}/vs_sub_proj.tpl
)

source_group("Template Files" FILES ${OPS_TEMPLATE_FILES})

# install template files in share
install(FILES ${OPS_TEMPLATE_FILES}
  DESTINATION share/ops/templates
)

add_jar(IDLTemplates ${OPS_TEMPLATE_FILES}
  VERSION ${OPS_VERSION}
  OUTPUT_DIR ${CMAKE_BINARY_DIR}/bin
)

install_jar(IDLTemplates ${OPS_JAR_REL_INSTALL_PATH})