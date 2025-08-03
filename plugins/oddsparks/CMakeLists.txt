# Copyright The Mumble Developers. All rights reserved.
# Use of this source code is governed by a BSD-style license
# that can be found in the LICENSE file at the root of the
# Mumble source tree or at <https://www.mumble.info/LICENSE>.

add_library(oddsparks SHARED
	"oddsparks.cpp"

	"../Module.cpp"
	"../ProcessBase.cpp"
	"../ProcessWindows.cpp")

if(WIN32)
	target_sources(oddsparks PRIVATE "../HostWindows.cpp")
else()
	target_sources(oddsparks PRIVATE "../HostLinux.cpp")
endif()
