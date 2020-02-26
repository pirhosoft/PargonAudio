set(MODULE_NAME Audio)
set(TARGET_NAME Pargon${MODULE_NAME})
set(MAIN_HEADER Include/Pargon/${MODULE_NAME}.h)

#Audio.XAudio2.h

set(PUBLIC_HEADERS
	Include/Pargon/Audio/AudioDevice.h
	Include/Pargon/Audio/AudioPlayer.h
	Include/Pargon/Audio/Mixer.h
	Include/Pargon/Audio/Sound.h
)

set(SOURCES
	Source/Core/AudioDevice.cpp
	Source/Core/Mixer.cpp
	Source/Core/Sound.cpp
)

set(DEPENDENCIES
	PargonApplication
	PargonContainers
	PargonFiles
	PargonTypes
)

set(XAUDIO2_SOURCES
	Source/XAudio2/XAudio2Player.cpp
	Source/XAudio2/XAudio2Player.h
)

include(CheckIncludeFileCXX)
CHECK_INCLUDE_FILE_CXX(Xaudio2.h HAS_XAUDIO2)

if(HAS_XAUDIO2)
	list(APPEND SOURCES ${XAUDIO2_SOURCES})
endif()

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Include/Pargon/" FILES ${MAIN_HEADER})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Include/Pargon/${MODULE_NAME}/" PREFIX Include FILES ${PUBLIC_HEADERS})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Source/" FILES ${SOURCES})

add_library(${TARGET_NAME} STATIC)
target_compile_features(${TARGET_NAME} PUBLIC cxx_std_17)
target_include_directories(${TARGET_NAME} PUBLIC Include)
target_include_directories(${TARGET_NAME} PRIVATE Source)
target_link_libraries(${TARGET_NAME} PUBLIC ${DEPENDENCIES})
target_sources(${TARGET_NAME} PRIVATE "${MAIN_HEADER}" "${PUBLIC_HEADERS}" "${SOURCES}")
