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

add_library(class_lib src/optimize.cpp src/constraint_function.cpp src/sqlAction.cpp src/SQL.cpp src/new2D.cpp src/scheduling_parameter.cpp)
# add the executable
# executable by 50 household CEMS
add_executable(LHEMS src/hems_test.cpp )
target_link_libraries(LHEMS 
class_lib
${LIBMYSQLCLIENT_LIBRARIES} 
/usr/local/lib/libglpk.a
)
add_executable(LHEMS2 src/hems_test2.cpp)
target_link_libraries(LHEMS2 
class_lib
${LIBMYSQLCLIENT_LIBRARIES} 
/usr/local/lib/libglpk.a
)
add_executable(LHEMS3 src/hems_test3.cpp)
target_link_libraries(LHEMS3 
class_lib
${LIBMYSQLCLIENT_LIBRARIES} 
/usr/local/lib/libglpk.a
)
add_executable(LHEMS4 src/hems_test4.cpp)
target_link_libraries(LHEMS4 
class_lib
${LIBMYSQLCLIENT_LIBRARIES} 
/usr/local/lib/libglpk.a
)
add_executable(LHEMS5 src/hems_test5.cpp)
target_link_libraries(LHEMS5 
class_lib
${LIBMYSQLCLIENT_LIBRARIES} 
/usr/local/lib/libglpk.a
)
add_executable(LHEMS6 src/hems_test6.cpp)
target_link_libraries(LHEMS6 
class_lib
${LIBMYSQLCLIENT_LIBRARIES} 
/usr/local/lib/libglpk.a
)
add_executable(LHEMS7 src/hems_test7.cpp)
target_link_libraries(LHEMS7 
class_lib
${LIBMYSQLCLIENT_LIBRARIES} 
/usr/local/lib/libglpk.a
)
add_executable(LHEMS8 src/hems_test8.cpp)
target_link_libraries(LHEMS8 
class_lib
${LIBMYSQLCLIENT_LIBRARIES} 
/usr/local/lib/libglpk.a
)
add_executable(LHEMS9 src/hems_test9.cpp)
target_link_libraries(LHEMS9 
class_lib
${LIBMYSQLCLIENT_LIBRARIES} 
/usr/local/lib/libglpk.a
)
add_executable(LHEMS10 src/hems_test10.cpp)
target_link_libraries(LHEMS10 
class_lib
${LIBMYSQLCLIENT_LIBRARIES} 
/usr/local/lib/libglpk.a
)

add_executable(GHEMS src/cems_test.cpp)
target_link_libraries(GHEMS 
class_lib
${LIBMYSQLCLIENT_LIBRARIES} 
/usr/local/lib/libglpk.a
)
