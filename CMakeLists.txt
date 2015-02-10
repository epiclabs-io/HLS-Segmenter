project(httplvs-sd)
cmake_minimum_required(VERSION 2.6)

include(detectFFMpeg.cmake)


add_executable(segmenter segmenter.c options_parsing.c helpers.c )
target_link_libraries(segmenter -lm -lbz2 -lz -liconv -lavformat -lavcodec -lavutil ${FFMPEG_LIBS})
