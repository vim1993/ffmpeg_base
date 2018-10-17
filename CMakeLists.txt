PROJECT(FFMPEG_PLAYER)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.7)

#####################################
#videoPlayer: compiler video player OBJ
#audioPlayer: compiler audio player OBJ
#Player: compiler player OBJ
#####################################
SET(OBJ_PRODUCT "videoPlayer")

#set include path
INCLUDE_DIRECTORIES(./include)

#set src path
AUX_SOURCE_DIRECTORY(. MAIN_SRC)
IF(OBJ_PRODUCT STREQUAL "videoPlayer")
AUX_SOURCE_DIRECTORY("./src/videoPlay/" PLAYER_SRC)
ELSE(OBJ_PRODUCT STREQUAL "audioPlayer")
AUX_SOURCE_DIRECTORY("./src/audioPlay/" PLAYER_SRC)
ENDIF()

#set lib path
LINK_DIRECTORIES(./lib)
LINK_DIRECTORIES(/usr/local/lib)
#executable
ADD_EXECUTABLE(${PROJECT_NAME} ${PLAYER_SRC} ${MAIN_SRC})

#link lib
TARGET_LINK_LIBRARIES(${PROJECT_NAME} SDL2 avcodec avdevice avfilter avformat avutil swresample swscale)


