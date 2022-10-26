PROJECT(speexdsp LANGUAGES C)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(APPLE)
SET(CMAKE_OSX_DEPLOYMENT_TARGET "10.15")
set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64")
endif()

set(FFT_SOURCES
    libspeexdsp/smallft.c)

add_library(speexdsp STATIC
    libspeexdsp/preprocess.c
    libspeexdsp/jitter.c
    libspeexdsp/mdf.c
    libspeexdsp/fftwrap.c
    libspeexdsp/filterbank.c
    libspeexdsp/resample.c
    libspeexdsp/buffer.c
    libspeexdsp/scal.c
    ${FFT_SOURCES})
set_property(TARGET speexdsp
    PROPERTY PUBLIC_HEADER
    include/speex/speex_buffer.h
    include/speex/speex_echo.h
    include/speex/speex_jitter.h
    include/speex/speex_preprocess.h
    include/speex/speex_resampler.h
    include/speex/speexdsp_types.h
    include/speex/speexdsp_config_types.h)
target_include_directories(speexdsp
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/speex>)
if(APPLE)
target_compile_definitions(speexdsp
    PRIVATE 
    FLOATING_POINT
    USE_SMALLFT
    EXPORT=)
else()
target_compile_definitions(speexdsp
    PRIVATE 
    FLOATING_POINT
    USE_SSE
    USE_SMALLFT
    EXPORT=)
endif()
install(TARGETS speexdsp
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    PUBLIC_HEADER DESTINATION include)
