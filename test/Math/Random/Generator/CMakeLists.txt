# Copyright (C) 2020-2025  The Mustard development team
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

add_executable(MT1993732 MT1993732.c++)
target_link_libraries(MT1993732 Mustard::Mustard)

add_executable(MT1993764 MT1993764.c++)
target_link_libraries(MT1993764 Mustard::Mustard)

add_executable(Xoshiro256Plus Xoshiro256Plus.c++)
target_link_libraries(Xoshiro256Plus Mustard::Mustard)

add_executable(Xoshiro256PP Xoshiro256PP.c++)
target_link_libraries(Xoshiro256PP Mustard::Mustard)

add_executable(Xoshiro256SS Xoshiro256SS.c++)
target_link_libraries(Xoshiro256SS Mustard::Mustard)

add_executable(Xoshiro512Plus Xoshiro512Plus.c++)
target_link_libraries(Xoshiro512Plus Mustard::Mustard)

add_executable(Xoshiro512PP Xoshiro512PP.c++)
target_link_libraries(Xoshiro512PP Mustard::Mustard)

add_executable(Xoshiro512SS Xoshiro512SS.c++)
target_link_libraries(Xoshiro512SS Mustard::Mustard)
