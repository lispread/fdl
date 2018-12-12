cmake_minimum_required(VERSION 3.8)
include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)
project(NONE)

zephyr_include_directories(src/dloader)

target_sources(app PRIVATE src/main.c)
target_sources(app PRIVATE src/dloader/dl_cmd_download.c)
#target_sources(app PRIVATE src/dloader/dl_stdio.c)
target_sources(app PRIVATE src/dloader/dl_channel.c)
target_sources(app PRIVATE src/dloader/dl_crc.c)
target_sources(app PRIVATE src/dloader/dl_packet.c)
target_sources(app PRIVATE src/dloader/dl_cmd_proc.c)
