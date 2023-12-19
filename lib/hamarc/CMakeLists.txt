add_library(hamarc HamArc.cpp)
target_link_libraries(hamarc PRIVATE parser)
target_include_directories(hamarc PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(hamarc PRIVATE haming)