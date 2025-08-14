#include "Resolution.h"

///////////////////////////////////////////////////////////////////
// How to run                                                    //
// 1) run: root -l -b -q produce_emResol.cc'("beam_type")'       //
//    - beam_type: em, muon, pi, kaon, proton                    //
// 4) run: root -l -b -q produce_emResol.cc'("beam_type", "nnr")' //
//    - NoNoise + Reverse 조합 옵션                                  //
// 5) Available beam types for TB2025: em (center: M5-T2 or M4-T2)//
//    - For TB2025, currently only em beam data is available      //
// 6) To use M4-T2 as center: add "m4" option                     //
//    - Example: root -l -b -q produce_emResol.cc'("em", "m4")'   //
//    - Example: root -l -b -q produce_emResol.cc'("em", "nnm4")' //
////////////////////////////////////////////////////////////////////
void produce_Resol(string BeamType, string rotation, string interaction, string Option = "") {
  // 빔 타입에 따른 플롯 저장 경로 설정
  TString inputPath = "../output";
  if(rotation.find("on") != string::npos) {inputPath += TString::Format("/%s/Evtloop_Rot", BeamType.c_str());}
  else if(interaction.find("on") != string::npos) {inputPath += TString::Format("/%s/Evtloop_IT", BeamType.c_str());}
  else {inputPath += TString::Format("/%s", BeamType.c_str());}
  
  TString plotPath = TString::Format("./EResolution/%s", BeamType.c_str());
  if(rotation.find("on") != string::npos) {plotPath += TString::Format("_Rot");}
  else if(interaction.find("on") != string::npos) {plotPath += TString::Format("_IT");}
  else {plotPath += TString::Format("_%s", BeamType.c_str());}

  PlotTool::Make_Dir_IfNotExist(plotPath);

  Resolution* resolution = nullptr;

  if(BeamType == "em")
  {
    //EMResolution resolution("./data", "M8-T2");
    // TB2025에서는 M5-T2를 기본으로 사용, M4-T2도 가능
    TString centerTower = "M5-T2";
    if(Option.find("m4") != string::npos) {
      centerTower = "M4-T2";
      printf("[produce_Resol] Using M4-T2 as center tower.\n");
    } else {
      printf("[produce_Resol] Using M5-T2 as center tower (default).\n");
    }
    
    resolution = new Resolution(inputPath, centerTower, BeamType.c_str());
  } 
  else if(BeamType == "pi")     {
    TString centerTower = "M5-T2";
    resolution = new Resolution(inputPath, centerTower, BeamType.c_str());
  }
  else if(BeamType == "kaon")   {
    TString centerTower = "M5-T2";
    resolution = new Resolution(inputPath, centerTower, BeamType.c_str());
  }
  else if(BeamType == "proton") {
    TString centerTower = "M5-T2";
    resolution = new Resolution(inputPath, centerTower, BeamType.c_str());
  }
  else {throw std::invalid_argument("[produce_Resol] BeamType = " + BeamType + " is not recognized");}
  
  // 옵션 파싱 및 적용
  bool useNoNoiseReverse = false;
  
  if(Option.find("nnr") != string::npos)
  {
    useNoNoiseReverse = true;
    printf("[produce_Resol] NoNoise option is enabled. Fitting without noise term up to 110 GeV.\n");
  }
  
  if(useNoNoiseReverse) resolution->noNoiseReverse(kTRUE);
  
  resolution->Produce(plotPath);
  delete resolution;
}