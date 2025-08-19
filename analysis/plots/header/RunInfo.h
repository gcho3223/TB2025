#pragma once

#include <TString.h>

#include <vector>
#include <map>

namespace TB2025 {
  std::vector<int> vec_calibRun = {
    12171, 12172, 12183, 12182,  // -- M1, T1-4
    12174, 12175, 12181, 12180,  // -- M2, T1-4
    12176, 12177, 12179, 12178,  // -- M3, T1-4
    12184, 12185, 12195, 12194,  // -- M4, T1-4
    12186, 12187, 12193, 12192,  // -- M5, T1-4
    12188, 12189, 12191, 12190,  // -- M6, T1-4
    12196, 12197, 12207, 12206,  // -- M7, T1-4
    12198, 12199, 12205, 12204,  // -- M8, T1-4
    12200, 12201, 12203, 12202   // -- M9, T1-4
  };

  std::map<TString, int> GetMap_CalibRun()
  {
    std::map<TString, int> map_calibRun;
    int nRun = (int)vec_calibRun.size();
    for(int i_run=0; i_run<nRun; ++i_run)
    {
      int moduleNum = (int)(i_run/4)+1;
      int towerNum = (i_run%4) + 1;

      TString tag = TString::Format("M%d-T%d", moduleNum, towerNum);
      map_calibRun.insert( std::make_pair(tag, vec_calibRun[i_run]) );
    }

    // -- print
    for(auto& pair : map_calibRun )
      printf("%s --> %d\n", pair.first.Data(), pair.second);

    return map_calibRun;
  }

  // -- Leakage counter
  std::map<TString, int> map_muBeamRun = {
    {"Top",           0.0},
    {"Bottom",        0.0},
    {"Left(Jura)",    0.0},
    {"Right(Saleve)", 0.0},
  };

  std::vector<double> vec_emScanE = {6, 10, 20, 30, 40, 60, 80, 100, 120};  // M5-T2: 10GeV 데이터 없음
  std::vector<double> vec_emScanE_M4 = {10, 20, 30, 40, 60, 80, 100, 120};  // M4-T2: 6GeV 데이터 없음  
  std::vector<int> vec_emScanRun = {};
  
  std::map<TString, std::vector<int>> map_emScanRunM5 = {
    {"120GeV", {12263, 12264, 12267, 12268, 12279, 12280}},
    {"100GeV", {12260, 12261}},
    {"80GeV", {12265, 12266}},
    {"60GeV", {12269, 12270}},
    {"40GeV", {12271, 12272}},
    {"30GeV", {12273, 12274}},
    {"20GeV", {12275, 12276}},
    {"10GeV", {12277}},
    {"6GeV", {12278}},
  };
  std::map<TString, std::vector<int>> map_emScanRunM4 = {
    {"120GeV", {12285, 12293, 12294, 12295, 12296, 12297, 12298, 12299, 12300}},
    {"100GeV", {12301, 12302, 12303}},
    {"80GeV", {12304, 12305, 12306}},
    {"60GeV", {12307, 12308, 12309}},
    {"40GeV", {12310, 12311, 12312}},
    {"30GeV", {12313, 12314, 12315}},
    {"20GeV", {12316, 12317, 12318}},
    {"10GeV", {12286, 12292, 12319}},
  };
  std::map<TString, std::vector<int>> map_emScanRun_merged = {
    //center, GeV
    {"M5T2", {6, 10, 20, 30, 40, 60, 80, 100, 120}},
    {"M4T2", {10, 20, 30, 40, 60, 80, 100, 120}},
  };
  
  // C와 S 채널 별 에너지 범위 - 각 에너지별로 {min, max} 형태로 저장
  std::map<TString, int> map_pi_ReRange_C = {
    {"20",  0},
    {"40",  8},
    {"60",  10},
    {"80",  20},
    {"100", 20},
    {"120", 30}
  };
  
  std::map<TString, int> map_pi_ReRange_S = {
    {"20",  0},
    {"40",  10},
    {"60",  20},
    {"80",  30},
    {"100", 40},
    {"120", 60}
  };
  
  std::vector<double> vec_hadScanE = {20, 40, 60, 80, 100, 120};

  // normal(rot:x, it:x), rot(rot:o, it:x), it(rot:x, it:o)
  std::map<TString, std::vector<int>> map_hadronBeamRun_merged = {
    //center, GeV (merged files available)
    {"M5T2", {20, 40, 60, 80, 100, 120}},
  };
  std::map<TString, std::vector<int>> map_hadronBeamRun_merged_IT = {
    //center, GeV (merged files available)
    {"M5T2", {40, 60, 80, 100, 120}},
  };
}; // -- namespace TB2025