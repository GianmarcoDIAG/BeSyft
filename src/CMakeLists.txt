add_subdirectory(parser)
add_subdirectory(synthesis)

include_directories(${PARSER_INCLUDE_PATH} ${SYNTHESIS_INCLUDE_PATH} ${EXT_INCLUDE_PATH})
add_executable(BeSyft main.cpp)

target_link_libraries(BeSyft ${PARSER_LIB_NAME} ${SYNTHESIS_LIB_NAME})

install(TARGETS BeSyft
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	)

INSTALL (
  DIRECTORY ${CMAKE_SOURCE_DIR}/src/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h*")
