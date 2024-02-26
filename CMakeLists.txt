###################################################################################################
##
##      project info 
##
###################################################################################################

cmake_minimum_required(VERSION 3.14)
project(GridObjects VERSION 1.0.0 LANGUAGES CXX)

get_directory_property(IS_SUBPROJECT PARENT_DIRECTORY)

###################################################################################################
##
##      project options
##
###################################################################################################

if (IS_SUBPROJECT AND NOT DEFINED GROB_TESTING)
    message(STATUS "GridObjects not testing in submodule mode")
    set(GROB_TESTING OFF)
endif()

if (IS_SUBPROJECT AND NOT DEFINED GROB_DOC)
    message(STATUS "autodoc for GridObjects in submodule mode disabled")
    set(GROB_DOC OFF)
endif()

option(GROB_TESTING "enable testing" OFF)
option(GROB_COVERAGE "coverage" OFF)
option(GROB_DOC "enable doc" OFF)



###################################################################################################
##
##      headers lib
##
###################################################################################################

add_library(grob_headers INTERFACE)
target_include_directories(grob_headers INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(grob_headers INTERFACE cxx_std_14)

add_library(Grob::grob ALIAS grob_headers)

###################################################################################################
##
##      install
##
###################################################################################################

install(DIRECTORY include/grob DESTINATION include)
install(TARGETS grob_headers EXPORT GrobConfig)
install(EXPORT GrobConfig NAMESPACE Grob:: DESTINATION share/Grob/cmake)

include(CMakePackageConfigHelpers)
write_basic_package_version_file("${PROJECT_BINARY_DIR}/GrobConfigVersion.cmake"
    VERSION
        ${PROJECT_VERSION}
    COMPATIBILITY
        AnyNewerVersion
)
install(FILES "${PROJECT_BINARY_DIR}/GrobConfigVersion.cmake" DESTINATION share/Grob/cmake)

###################################################################################################
##
##      tests
##
###################################################################################################

if(NOT MYLIB_TESTING)
    message(STATUS "testing Mylib disabled")
else()
    add_subdirectory(tests)
endif()

###################################################################################################
##
##      documentation
##
###################################################################################################

if(NOT GROB_DOC)
    message(STATUS "Mylib docs disabled")
else()
    add_subdirectory(doc)
endif()
