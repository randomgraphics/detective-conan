project(detective-conan)

option(DETECTIVE_CONAN_BUILD_DEV "Build tests" ON)

add_subdirectory(src)

if(DETECTIVE_CONAN_BUILD_DEV)
    add_subdirectory(dev)
endif()