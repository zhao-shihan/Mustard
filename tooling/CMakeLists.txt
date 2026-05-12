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

find_package(Python3 QUIET)

if(Python3_FOUND)
    # Run generate-test-scripts.py to produce GitHub workflow YAMLs and run-tests.sh
    add_custom_target(
        GenerateTestScripts ALL
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/generate-test-scripts.py"
            > "${CMAKE_CURRENT_BINARY_DIR}/generate-test-scripts.log" 2>&1
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        COMMENT "Generating local test scripts and GitHub test workflows"
    )
    # Copy run-tests.sh to the build/test directory
    add_custom_command(
        TARGET GenerateTestScripts POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/run-tests.sh"
            "${CMAKE_BINARY_DIR}/test/run-tests.sh"
        COMMAND chmod +x "${CMAKE_BINARY_DIR}/test/run-tests.sh"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/parexec.sh"
            "${CMAKE_BINARY_DIR}/test/parexec.sh"
        COMMENT "Copying local test scripts"
    )
else()
    message(WARNING "Python3 not found, skipping generation of test scripts")
endif()
