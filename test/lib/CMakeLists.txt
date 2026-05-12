# Copyright (C) 2020-2026  Shihan Zhao and contributors
#
# This file is part of Mustard, an offline software framework for HEP experiments.
#
# Mustard is free software: you can redistribute it and/or modify it under the
# terms of the GNU General Public License as published by the Free Software
# Foundation, either version 3 of the License, or (at your option) any later
# version.
#
# Mustard is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
# A PARTICULAR PURPOSE. See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along with
# Mustard. If not, see <https://www.gnu.org/licenses/>.

file(GLOB_RECURSE mustard_testing_src ${CMAKE_CURRENT_SOURCE_DIR}/Mustard/*.c++)
add_library(MustardTesting STATIC ${mustard_testing_src})
target_include_directories(MustardTesting PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(MustardTesting Mustard::Mustard)
