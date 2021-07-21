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

# add the executable
# executable by 50 household CEMS
add_executable(LHEMS src/LHEMS.cpp src/new2D.cpp src/SQLFunction.cpp src/scheduling_parameter.cpp src/fifty_LHEMS_function.cpp src/LHEMS_constraint.cpp)
target_link_libraries(LHEMS 
${LIBMYSQLCLIENT_LIBRARIES} 
/usr/local/lib/libglpk.a
)
add_executable(LHEMS2 src/LHEMS2.cpp src/new2D.cpp src/SQLFunction.cpp src/scheduling_parameter.cpp src/fifty_LHEMS_function.cpp src/LHEMS_constraint.cpp)
target_link_libraries(LHEMS2 
${LIBMYSQLCLIENT_LIBRARIES} 
/usr/local/lib/libglpk.a
)
add_executable(LHEMS3 src/LHEMS3.cpp src/new2D.cpp src/SQLFunction.cpp src/scheduling_parameter.cpp src/fifty_LHEMS_function.cpp src/LHEMS_constraint.cpp)
target_link_libraries(LHEMS3 
${LIBMYSQLCLIENT_LIBRARIES} 
/usr/local/lib/libglpk.a
)
add_executable(LHEMS4 src/LHEMS4.cpp src/new2D.cpp src/SQLFunction.cpp src/scheduling_parameter.cpp src/fifty_LHEMS_function.cpp src/LHEMS_constraint.cpp)
target_link_libraries(LHEMS4 
${LIBMYSQLCLIENT_LIBRARIES} 
/usr/local/lib/libglpk.a
)
add_executable(LHEMS5 src/LHEMS5.cpp src/new2D.cpp src/SQLFunction.cpp src/scheduling_parameter.cpp src/fifty_LHEMS_function.cpp src/LHEMS_constraint.cpp)
target_link_libraries(LHEMS5 
${LIBMYSQLCLIENT_LIBRARIES} 
/usr/local/lib/libglpk.a
)
add_executable(LHEMS6 src/LHEMS6.cpp src/new2D.cpp src/SQLFunction.cpp src/scheduling_parameter.cpp src/fifty_LHEMS_function.cpp src/LHEMS_constraint.cpp)
target_link_libraries(LHEMS6 
${LIBMYSQLCLIENT_LIBRARIES} 
/usr/local/lib/libglpk.a
)
add_executable(LHEMS7 src/LHEMS7.cpp src/new2D.cpp src/SQLFunction.cpp src/scheduling_parameter.cpp src/fifty_LHEMS_function.cpp src/LHEMS_constraint.cpp)
target_link_libraries(LHEMS7 
${LIBMYSQLCLIENT_LIBRARIES} 
/usr/local/lib/libglpk.a
)
add_executable(LHEMS8 src/LHEMS8.cpp src/new2D.cpp src/SQLFunction.cpp src/scheduling_parameter.cpp src/fifty_LHEMS_function.cpp src/LHEMS_constraint.cpp)
target_link_libraries(LHEMS8 
${LIBMYSQLCLIENT_LIBRARIES} 
/usr/local/lib/libglpk.a
)
add_executable(LHEMS9 src/LHEMS9.cpp src/new2D.cpp src/SQLFunction.cpp src/scheduling_parameter.cpp src/fifty_LHEMS_function.cpp src/LHEMS_constraint.cpp)
target_link_libraries(LHEMS9 
${LIBMYSQLCLIENT_LIBRARIES} 
/usr/local/lib/libglpk.a
)
add_executable(LHEMS10 src/LHEMS10.cpp src/new2D.cpp src/SQLFunction.cpp src/scheduling_parameter.cpp src/fifty_LHEMS_function.cpp src/LHEMS_constraint.cpp)
target_link_libraries(LHEMS10 
${LIBMYSQLCLIENT_LIBRARIES} 
/usr/local/lib/libglpk.a
)

add_executable(GHEMS src/GHEMS.cpp src/new2D.cpp src/SQLFunction.cpp src/scheduling_parameter.cpp src/GHEMS_function.cpp src/GHEMS_constraint.cpp)
target_link_libraries(GHEMS 
${LIBMYSQLCLIENT_LIBRARIES} 
/usr/local/lib/libglpk.a
)