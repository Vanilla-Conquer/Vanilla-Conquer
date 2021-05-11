add_executable(mixtool makemix.cpp  mixnamedb.cpp mixnamedb.h gmixdb.c gmixdb.h crc32.cpp crc32.h fastini.cpp fastini.h)
target_link_libraries(mixtool PUBLIC common miniposix)

if (WIN32)
    target_compile_definitions(mixtool PRIVATE -DNOMINMAX)
endif()