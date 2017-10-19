cmake_minimum_required(VERSION 3.4.1)

file(GLOB_RECURSE SRC_FILES FOLLOW_SYMLINKS
     ../../../native/PushUpPal/src/*.cpp
     ../../../native/PushUpPal/glue-code/jni/*.cpp
     ../../../deps/djinni/support-lib/*.cpp)

include_directories(../../../native/PushUpPal/src
                    ../../../native/PushUpPal/glue-code/interfaces/generated
                    ../../../native/PushUpPal/glue-code/jni
                    ../../../deps/djinni/support-lib
                    ../../../deps/djinni/support-lib/jni
                    ../../../deps/OpenCV-android-sdk/sdk/native/jni/include)

add_library(lib-opencv SHARED IMPORTED)
set_target_properties(lib-opencv PROPERTIES IMPORTED_LOCATION
                      ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libopencv_java3.so)

add_library(native-pushuppal SHARED ${SRC_FILES})

target_link_libraries(native-pushuppal
                      lib-opencv)