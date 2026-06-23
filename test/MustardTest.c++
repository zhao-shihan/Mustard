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
#include "Mustard/Testing/TestEstimate/TestEstimate.h++"
#include "Mustard/Testing/TestExecutor/TestExecutor.h++"
#include "Mustard/Testing/TestExecutorSequential/TestExecutorSequential.h++"
#include "Mustard/Testing/TestFieldMap/TestFieldMap.h++"
#include "Mustard/Testing/TestFieldTypeName/TestFieldTypeName.h++"
#include "Mustard/Testing/TestFile/TestFile.h++"
#include "Mustard/Testing/TestGaussian/TestGaussian.h++"
#include "Mustard/Testing/TestHelixAxisPOCA/TestHelixAxisPOCA.h++"
#include "Mustard/Testing/TestHelixLinePOCA/TestHelixLinePOCA.h++"
#include "Mustard/Testing/TestHelixPointPOCA/TestHelixPointPOCA.h++"
#include "Mustard/Testing/TestLineLinePOCA/TestLineLinePOCA.h++"
#include "Mustard/Testing/TestLinePointPOCA/TestLinePointPOCA.h++"
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
#include "Mustard/Testing/TestStatistic/TestStatistic.h++"
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
    launcher.AddSubprogram<TestEstimate0>();
    launcher.AddSubprogram<TestEstimate1>();
    launcher.AddSubprogram<TestEstimate2>();
    launcher.AddSubprogram<TestEstimate3>();
    launcher.AddSubprogram<TestEstimate4>();
    launcher.AddSubprogram<TestEstimate5>();
    launcher.AddSubprogram<TestEstimate6>();
    launcher.AddSubprogram<TestEstimate7>();
    launcher.AddSubprogram<TestEstimate8>();
    launcher.AddSubprogram<TestEstimate9>();
    launcher.AddSubprogram<TestEstimate10>();
    launcher.AddSubprogram<TestEstimate11>();
    launcher.AddSubprogram<TestEstimate12>();
    launcher.AddSubprogram<TestEstimate13>();
    launcher.AddSubprogram<TestEstimate14>();
    launcher.AddSubprogram<TestEstimate15>();
    launcher.AddSubprogram<TestEstimate16>();
    launcher.AddSubprogram<TestEstimate17>();
    launcher.AddSubprogram<TestEstimate18>();
    launcher.AddSubprogram<TestEstimate19>();
    launcher.AddSubprogram<TestEstimate20>();
    launcher.AddSubprogram<TestEstimate21>();
    launcher.AddSubprogram<TestEstimate22>();
    launcher.AddSubprogram<TestEstimate23>();
    launcher.AddSubprogram<TestEstimate24>();
    launcher.AddSubprogram<TestEstimate25>();
    launcher.AddSubprogram<TestEstimate26>();
    launcher.AddSubprogram<TestEstimate27>();
    launcher.AddSubprogram<TestEstimate28>();
    launcher.AddSubprogram<TestEstimate29>();
    launcher.AddSubprogram<TestEstimate30>();
    launcher.AddSubprogram<TestEstimate31>();
    launcher.AddSubprogram<TestEstimate32>();
    launcher.AddSubprogram<TestEstimate33>();
    launcher.AddSubprogram<TestExecutor>();
    launcher.AddSubprogram<TestExecutorSequential>();
    launcher.AddSubprogram<TestFieldMap>();
    launcher.AddSubprogram<TestFieldTypeName>();
    launcher.AddSubprogram<TestFile>();
    launcher.AddSubprogram<TestGaussian>();
    launcher.AddSubprogram<TestHelixAxisPOCA>();
    launcher.AddSubprogram<TestHelixLinePOCA>();
    launcher.AddSubprogram<TestHelixPointPOCA>();
    launcher.AddSubprogram<TestLineLinePOCA>();
    launcher.AddSubprogram<TestLinePointPOCA>();
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
    launcher.AddSubprogram<TestStatistic0>();
    launcher.AddSubprogram<TestStatistic1>();
    launcher.AddSubprogram<TestStatistic2>();
    launcher.AddSubprogram<TestStatistic3>();
    launcher.AddSubprogram<TestStatistic4>();
    launcher.AddSubprogram<TestStatistic5>();
    launcher.AddSubprogram<TestStatistic6>();
    launcher.AddSubprogram<TestStatistic7>();
    launcher.AddSubprogram<TestStatistic8>();
    launcher.AddSubprogram<TestStatistic9>();
    launcher.AddSubprogram<TestStatistic10>();
    launcher.AddSubprogram<TestUniform>();
    launcher.AddSubprogram<TestVectorConcept>();
    launcher.AddSubprogram<TestXoshiro256Plus>();
    launcher.AddSubprogram<TestXoshiro256PlusPlus>();
    launcher.AddSubprogram<TestXoshiro256StarStar>();
    launcher.AddSubprogram<TestXoshiro512Plus>();
    launcher.AddSubprogram<TestXoshiro512PlusPlus>();
    launcher.AddSubprogram<TestXoshiro512StarStar>();
    return launcher.Launch(argc, argv);
}
