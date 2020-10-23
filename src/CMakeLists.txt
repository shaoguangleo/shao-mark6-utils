cmake_minimum_required(VERSION 3.2)

set(SOURCE_FILES dboss.c dqa.c gather.c dpstat.c dspeed.c scatspeed.c)

include_directories("${PROJECT_BINARY_DIR}/include")

include_directories(../include)


add_executable(shao_mark6_dboss dboss.c miscellany.c)
target_link_libraries(shao_mark6_dboss ${CMAKE_THREAD_LIBS_INIT})

add_executable(shao_mark6_dpstat dpstat.c)

add_executable(shao_mark6_dqa dqa.c)

add_executable(shao_mark6_dspeed dspeed.c)

add_executable(shao_mark6_gather gather.c)
set(GCC_GATHER_LINK_FLAGS "-lrt")
add_definitions(${GCC_GATHER_LINK_FLAGS})
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -lrt")
set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -laio")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lrt")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -laio")
target_link_libraries(shao_mark6_gather "-lrt")

add_executable(shao_mark6_scatspeed scatspeed.c scat_write.c writer.c)
target_link_libraries(shao_mark6_scatspeed ${CMAKE_THREAD_LIBS_INIT})

install(TARGETS shao_mark6_dboss DESTINATION bin)
install(TARGETS shao_mark6_dpstat DESTINATION bin)
install(TARGETS shao_mark6_dqa DESTINATION bin)
install(TARGETS shao_mark6_dspeed DESTINATION bin)
install(TARGETS shao_mark6_gather DESTINATION bin)
install(TARGETS shao_mark6_scatspeed DESTINATION bin)