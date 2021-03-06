# FreeRDS: FreeRDP Remote Desktop Services (RDS)
#
# Copyright 2013 Marc-Andre Moreau <marcandre.moreau@gmail.com>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

include(ExternalProject)

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/xorg-server")
set(EXTERNAL_PROJECTS_BASE "${CMAKE_CURRENT_BINARY_DIR}/external")
set_property(DIRECTORY PROPERTY "EP_BASE" ${EXTERNAL_PROJECTS_BASE})

execute_process(COMMAND "X" -version ERROR_VARIABLE XORG_SERVER_VERSION_OUTPUT)

STRING(REGEX MATCH "[0-9]+.[0-9]+.[0-9]+" XORG_SERVER_VERSION "${XORG_SERVER_VERSION_OUTPUT}")

message(STATUS "Using xorg-server sources version ${XORG_SERVER_VERSION}")

set(EXTERNAL_PROJECT "xorg-server")
set(${EXTERNAL_PROJECT}_VERSION "${XORG_SERVER_VERSION}")
set(${EXTERNAL_PROJECT}_NAME "${EXTERNAL_PROJECT}-${${EXTERNAL_PROJECT}_VERSION}")
set(${EXTERNAL_PROJECT}_EXT ".tar.bz2")
set(${EXTERNAL_PROJECT}_FILE "${${EXTERNAL_PROJECT}_NAME}${${EXTERNAL_PROJECT}_EXT}")
set(${EXTERNAL_PROJECT}_URL "http://xorg.freedesktop.org/releases/individual/xserver/${${EXTERNAL_PROJECT}_FILE}")
set(${EXTERNAL_PROJECT}_CONFIGURE "--prefix=/usr;--with-sha1=libcrypto;--disable-dpms")
set(${EXTERNAL_PROJECT}_UPDATE "")
set(${EXTERNAL_PROJECT}_MAKE "")

ExternalProject_Add(${EXTERNAL_PROJECT}
	URL ${${EXTERNAL_PROJECT}_URL}
	UPDATE_COMMAND ${${EXTERNAL_PROJECT}_UPDATE}
	CONFIGURE_COMMAND "${EXTERNAL_PROJECTS_BASE}/Source/${EXTERNAL_PROJECT}/configure" ${${EXTERNAL_PROJECT}_CONFIGURE}
	BUILD_IN_SOURCE 1
	INSTALL_COMMAND "")
endif()
	
add_subdirectory(rdp)

