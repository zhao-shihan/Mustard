# Copyright (C) 2020-2025  Mustard developers
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

add_executable(Xoshiro256PlusPlus Xoshiro256PlusPlus.c++)
target_link_libraries(Xoshiro256PlusPlus Mustard::Mustard)

add_executable(Xoshiro256StarStar Xoshiro256StarStar.c++)
target_link_libraries(Xoshiro256StarStar Mustard::Mustard)

add_executable(Xoshiro512Plus Xoshiro512Plus.c++)
target_link_libraries(Xoshiro512Plus Mustard::Mustard)

add_executable(Xoshiro512PlusPlus Xoshiro512PlusPlus.c++)
target_link_libraries(Xoshiro512PlusPlus Mustard::Mustard)

add_executable(Xoshiro512StarStar Xoshiro512StarStar.c++)
target_link_libraries(Xoshiro512StarStar Mustard::Mustard)
