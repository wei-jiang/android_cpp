# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# can not use project(xxx)
set(CXXLIB "cxxlib")

set(SWIG_I_FILE "src/main/cpp/cpp_net.i")
set(JAVA_GEN_PACKAGE "my.free.net")
string(REPLACE "." "/" JAVA_GEN_SUBDIR ${JAVA_GEN_PACKAGE})
set(JAVA_GEN_DIR ${Project_SOURCE_DIR}/src/main/java/${JAVA_GEN_SUBDIR})

# Sets the minimum version of CMake required to build the native library.
set(CMAKE_VERBOSE_MAKEFILE 1) # Use 1 for debugging, 0 for release
cmake_minimum_required(VERSION 3.5.0)

# Sets some warning/error conditions and which version of C/C++ to use
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++17")
# set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -D__ANDROID_API__=21")

set(BOOST_ROOT "/data/cpp_libs/android_libs/boost")
set(BOOST_INCLUDEDIR "${BOOST_ROOT}/include/boost-1_67")
set(BOOST_LIBRARYDIR "${BOOST_ROOT}/lib")
FOREACH(LIB 
    libboost_date_time-clang-mt-s-a32-1_67.a
    libboost_atomic-clang-mt-s-a32-1_67.a
    libboost_filesystem-clang-mt-s-a32-1_67.a
    libboost_iostreams-clang-mt-s-a32-1_67.a
    libboost_system-clang-mt-s-a32-1_67.a
    libboost_timer-clang-mt-s-a32-1_67.a
)
    LIST(APPEND Boost_LIBRARIES "${BOOST_LIBRARYDIR}/${LIB}")
ENDFOREACH(LIB)

set(SOURCE_FILES
    src/main/cpp/common.cpp    
    src/main/cpp/cpp_net.cpp
    src/main/cpp/http_svr.cpp
    src/main/cpp/udp.cpp
    src/main/cpp/ws_svr.cpp
    src/main/cpp/util.cpp
)

# Include a location to the header files
include_directories(src/main/cpp)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.
add_library( # Sets the name of the library.
    ${CXXLIB}

    # Sets the library as a shared library.
    SHARED

    # Provides the list of files to compile.
    ${SOURCE_FILES} )

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.
find_library( # Sets the name of the path variable.
              log-lib
              # Specifies the name of the NDK library that you want CMake to locate.
              log )
              
find_library( zlib z )
# Fail the build if SWIG is not found - Ensure SWIG is installed and on the system path
# TODO: Does this run for EACH architecture?
find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})

# Remove old java files, in case we don't need to generate some of them anymore
file(REMOVE_RECURSE ${JAVA_GEN_DIR})

# Ensure file recognized as C++ (otherwise, exported as C file)
set_property(SOURCE src/main/cpp/cpp_net.i PROPERTY CPLUSPLUS ON)

# Setup SWIG flags and locations
set(CMAKE_SWIG_FLAGS -c++ -package ${JAVA_GEN_PACKAGE})
set(CMAKE_SWIG_OUTDIR ${JAVA_GEN_DIR})

# Export a wrapper file to Java, and link with the created C++ library
swig_add_module(cpp_lib java ${SWIG_I_FILE})
# 3.8.2 above
# swig_add_library(cpp_lib LANGUAGE java SOURCES ${SWIG_I_FILE} TYPE static)
swig_link_libraries(cpp_lib ${CXXLIB})

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.
target_link_libraries( # Specifies the target library.
                    ${CXXLIB}
                    # Links the target library to the log library
                    # included in the NDK.
                    ${log-lib} ${zlib})

                       
include_directories(${BOOST_INCLUDEDIR})
target_link_libraries(${CXXLIB} ${Boost_LIBRARIES})


set(HEADER_DIR 
    "/data/cpp_libs/android_libs/libmagic/include"
    "/data/cpp_libs/android_libs/openssl/include"
    "/data/cpp_libs/Simple-Web-Server"
    "/data/cpp_libs/Simple-WebSocket-Server"   
)
target_include_directories(${CXXLIB} PRIVATE ${HEADER_DIR})

set(libmagic "/data/cpp_libs/android_libs/libmagic/lib/libmagic.a")
set(OPENSSL_LIBRARYDIR "/data/cpp_libs/android_libs/openssl/lib")
FOREACH(LIB 
    libcrypto.a
    libssl.a
)
    LIST(APPEND OpenSSL_LIBRARIES "${OPENSSL_LIBRARYDIR}/${LIB}")
ENDFOREACH(LIB)

set(IncludeDir 
    ${BOOST_INCLUDEDIR}
    "/data/cpp_libs/spdlog/include"
)
include_directories(${IncludeDir})
target_link_libraries(${CXXLIB} ${Boost_LIBRARIES} ${OpenSSL_LIBRARIES} ${libmagic})