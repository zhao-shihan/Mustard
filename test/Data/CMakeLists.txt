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

add_executable(TestFieldTypeName TestFieldTypeName.c++)
target_link_libraries(TestFieldTypeName Mustard::Mustard)

add_executable(TestSingleDataEntryIO TestSingleDataEntryIO.c++)
target_link_libraries(TestSingleDataEntryIO Mustard::Mustard)

add_executable(TestSingleDataEventIO TestSingleDataEventIO.c++)
target_link_libraries(TestSingleDataEventIO Mustard::Mustard)

add_executable(TestMultiDataEntryIO TestMultiDataEntryIO.c++)
target_link_libraries(TestMultiDataEntryIO Mustard::Mustard)

add_executable(TestMultiDataEventIO TestMultiDataEventIO.c++)
target_link_libraries(TestMultiDataEventIO Mustard::Mustard)

add_executable(TestSingleDataEntryProcessor TestSingleDataEntryProcessor.c++)
target_link_libraries(TestSingleDataEntryProcessor Mustard::Mustard)

add_executable(TestSingleDataEventProcessor TestSingleDataEventProcessor.c++)
target_link_libraries(TestSingleDataEventProcessor Mustard::Mustard)

add_executable(TestMultiDataEntryProcessor TestMultiDataEntryProcessor.c++)
target_link_libraries(TestMultiDataEntryProcessor Mustard::Mustard)

add_executable(TestMultiDataEventProcessor TestMultiDataEventProcessor.c++)
target_link_libraries(TestMultiDataEventProcessor Mustard::Mustard)
