cmake_minimum_required(VERSION 2.8.9)

#Bring the headers
include_directories(include)
include_directories("include")

#Can manually add the sources using the set command as follows:
#set(SOURCES src/mainapp.cpp src/Student.cpp)


#However, the file(GLOB...) allows for wildcard additions:
file(GLOB SOURCES "src/*.c")


link_directories("")
link_libraries()

add_executable(szhq_demo ${SOURCES})

target_compile_options(szhq_demo PRIVATE -std=gnu99)

