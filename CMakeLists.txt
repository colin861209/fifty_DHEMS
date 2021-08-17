cmake_minimum_required(VERSION 3.10)

# set the project name
set (CMAKE_CXX_STANDARD 17)
project(CmakeTest)
include(FindPkgConfig)
pkg_check_modules(LIBMYSQLCLIENT REQUIRED mysqlclient)


# include dirs
include_directories(
    include
    ${LIBMYSQLCLIENT_INCLUDE_DIRS}
    ${CATKIN_PACKAGE_INCLUDE_DESTINATION}
    # ${GLPK_INCLUDE_DIRS}
    # /home/hems/glpk-4.65/src
)

link_directories(${LIBMYSQLCLIENT_LIBRARY_DIRS})
add_library(common_lib src/new2D.cpp src/SQLFunction.cpp src/scheduling_parameter.cpp)

add_library(LHEMS_lib src/fifty_LHEMS_function.cpp src/LHEMS_constraint.cpp)
link_libraries(LHEMS_lib common_lib)
add_library(GHEMS_lib src/GHEMS_function.cpp src/GHEMS_constraint.cpp)
link_libraries(GHEMS_lib common_lib)
# add the executable
# executable by 50 household CEMS
add_executable(LHEMS src/LHEMS.cpp )
target_link_libraries(LHEMS 
LHEMS_lib
${LIBMYSQLCLIENT_LIBRARIES} 
/usr/local/lib/libglpk.a
)
add_executable(LHEMS2 src/LHEMS2.cpp)
target_link_libraries(LHEMS2 
LHEMS_lib
${LIBMYSQLCLIENT_LIBRARIES} 
/usr/local/lib/libglpk.a
)
add_executable(LHEMS3 src/LHEMS3.cpp)
target_link_libraries(LHEMS3 
LHEMS_lib
${LIBMYSQLCLIENT_LIBRARIES} 
/usr/local/lib/libglpk.a
)
add_executable(LHEMS4 src/LHEMS4.cpp)
target_link_libraries(LHEMS4 
LHEMS_lib
${LIBMYSQLCLIENT_LIBRARIES} 
/usr/local/lib/libglpk.a
)
add_executable(LHEMS5 src/LHEMS5.cpp)
target_link_libraries(LHEMS5 
LHEMS_lib
${LIBMYSQLCLIENT_LIBRARIES} 
/usr/local/lib/libglpk.a
)
add_executable(LHEMS6 src/LHEMS6.cpp)
target_link_libraries(LHEMS6 
LHEMS_lib
${LIBMYSQLCLIENT_LIBRARIES} 
/usr/local/lib/libglpk.a
)
add_executable(LHEMS7 src/LHEMS7.cpp)
target_link_libraries(LHEMS7 
LHEMS_lib
${LIBMYSQLCLIENT_LIBRARIES} 
/usr/local/lib/libglpk.a
)
add_executable(LHEMS8 src/LHEMS8.cpp)
target_link_libraries(LHEMS8 
LHEMS_lib
${LIBMYSQLCLIENT_LIBRARIES} 
/usr/local/lib/libglpk.a
)
add_executable(LHEMS9 src/LHEMS9.cpp)
target_link_libraries(LHEMS9 
LHEMS_lib
${LIBMYSQLCLIENT_LIBRARIES} 
/usr/local/lib/libglpk.a
)
add_executable(LHEMS10 src/LHEMS10.cpp)
target_link_libraries(LHEMS10 
LHEMS_lib
${LIBMYSQLCLIENT_LIBRARIES} 
/usr/local/lib/libglpk.a
)

add_executable(GHEMS src/GHEMS.cpp)
target_link_libraries(GHEMS 
GHEMS_lib
${LIBMYSQLCLIENT_LIBRARIES} 
/usr/local/lib/libglpk.a
)