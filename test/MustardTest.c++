// Copyright (C) 2020-2026  Shihan Zhao and contributors
//
// This file is part of Mustard, an offline software framework for HEP experiments.
//
// Mustard is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// Mustard is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// Mustard. If not, see <https://www.gnu.org/licenses/>.

#include "Mustard/Application/SubprogramLauncher.h++"
#include "Mustard/Testing/TestCLHEPXMT1993732/TestCLHEPXMT1993732.h++"
#include "Mustard/Testing/TestCLHEPXMT1993764/TestCLHEPXMT1993764.h++"
#include "Mustard/Testing/TestCLHEPXXoshiro256StarStar/TestCLHEPXXoshiro256StarStar.h++"
#include "Mustard/Testing/TestCLHEPXXoshiro512StarStar/TestCLHEPXXoshiro512StarStar.h++"
#include "Mustard/Testing/TestCreateTemporaryFile/TestCreateTemporaryFile.h++"
#include "Mustard/Testing/TestExecutor/TestExecutor.h++"
#include "Mustard/Testing/TestExecutorSequential/TestExecutorSequential.h++"
#include "Mustard/Testing/TestFieldTypeName/TestFieldTypeName.h++"
#include "Mustard/Testing/TestFile/TestFile.h++"
#include "Mustard/Testing/TestGaussian/TestGaussian.h++"
#include "Mustard/Testing/TestMT1993732/TestMT1993732.h++"
#include "Mustard/Testing/TestMT1993764/TestMT1993764.h++"
#include "Mustard/Testing/TestMultiRDFEntryProcessor/TestMultiRDFEntryProcessor.h++"
#include "Mustard/Testing/TestMultiRDFEntryReader/TestMultiRDFEntryReader.h++"
#include "Mustard/Testing/TestMultiRDFEventProcessor/TestMultiRDFEventProcessor.h++"
#include "Mustard/Testing/TestMultiRDFEventReader/TestMultiRDFEventReader.h++"
#include "Mustard/Testing/TestPassiveSingleton/TestPassiveSingleton.h++"
#include "Mustard/Testing/TestPhaseSpaceGenerator/TestPhaseSpaceGenerator.h++"
#include "Mustard/Testing/TestRandomNumberDistribution/TestRandomNumberDistribution.h++"
#include "Mustard/Testing/TestSingleRDFEntryProcessor/TestSingleRDFEntryProcessor.h++"
#include "Mustard/Testing/TestSingleRDFEntryReader/TestSingleRDFEntryReader.h++"
#include "Mustard/Testing/TestSingleRDFEventProcessor/TestSingleRDFEventProcessor.h++"
#include "Mustard/Testing/TestSingleRDFEventReader/TestSingleRDFEventReader.h++"
#include "Mustard/Testing/TestSingleton/TestSingleton.h++"
#include "Mustard/Testing/TestUniform/TestUniform.h++"
#include "Mustard/Testing/TestVectorConcept/TestVectorConcept.h++"
#include "Mustard/Testing/TestXoshiro256Plus/TestXoshiro256Plus.h++"
#include "Mustard/Testing/TestXoshiro256PlusPlus/TestXoshiro256PlusPlus.h++"
#include "Mustard/Testing/TestXoshiro256StarStar/TestXoshiro256StarStar.h++"
#include "Mustard/Testing/TestXoshiro512Plus/TestXoshiro512Plus.h++"
#include "Mustard/Testing/TestXoshiro512PlusPlus/TestXoshiro512PlusPlus.h++"
#include "Mustard/Testing/TestXoshiro512StarStar/TestXoshiro512StarStar.h++"

#include <cstdlib>

using namespace Mustard::Testing;

auto main(int argc, char* argv[]) -> int {
    Mustard::Application::SubprogramLauncher launcher;
    launcher.AddSubprogram<TestCLHEPXMT1993732>();
    launcher.AddSubprogram<TestCLHEPXMT1993764>();
    launcher.AddSubprogram<TestCLHEPXXoshiro256StarStar>();
    launcher.AddSubprogram<TestCLHEPXXoshiro512StarStar>();
    launcher.AddSubprogram<TestCreateTemporaryFile>();
    launcher.AddSubprogram<TestExecutor>();
    launcher.AddSubprogram<TestExecutorSequential>();
    launcher.AddSubprogram<TestFieldTypeName>();
    launcher.AddSubprogram<TestFile>();
    launcher.AddSubprogram<TestGaussian>();
    launcher.AddSubprogram<TestMT1993732>();
    launcher.AddSubprogram<TestMT1993764>();
    launcher.AddSubprogram<TestMultiRDFEntryProcessor>();
    launcher.AddSubprogram<TestMultiRDFEntryReader>();
    launcher.AddSubprogram<TestMultiRDFEventProcessor>();
    launcher.AddSubprogram<TestMultiRDFEventReader>();
    launcher.AddSubprogram<TestPassiveSingleton>();
    launcher.AddSubprogram<TestPhaseSpaceGenerator>();
    launcher.AddSubprogram<TestRandomNumberDistribution>();
    launcher.AddSubprogram<TestSingleRDFEntryProcessor>();
    launcher.AddSubprogram<TestSingleRDFEntryReader>();
    launcher.AddSubprogram<TestSingleRDFEventProcessor>();
    launcher.AddSubprogram<TestSingleRDFEventReader>();
    launcher.AddSubprogram<TestSingleton>();
    launcher.AddSubprogram<TestUniform>();
    launcher.AddSubprogram<TestVectorConcept>();
    launcher.AddSubprogram<TestXoshiro256Plus>();
    launcher.AddSubprogram<TestXoshiro256PlusPlus>();
    launcher.AddSubprogram<TestXoshiro256StarStar>();
    launcher.AddSubprogram<TestXoshiro512Plus>();
    launcher.AddSubprogram<TestXoshiro512PlusPlus>();
    launcher.AddSubprogram<TestXoshiro512StarStar>();
    launcher.Launch(argc, argv);
    return EXIT_SUCCESS;
}
