#include "TBread.h"
#include "TButility.h"

#include <filesystem>
#include <iostream>
#include <chrono>
#include <numeric>
#include <vector>
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "TROOT.h"
#include "TStyle.h"
#include "TCanvas.h"
#include "TH1.h"
#include "TFile.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"

#include "header/function.h"

namespace fs = std::filesystem;

int main(int argc, char** argv) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <run_number> <max_event> <beam_type>" << std::endl;
        std::cerr << "- beam_type: em, pi, kp(kaon, proton)" << std::endl;
        return -1;
    }
    int fRunNum = std::stoi(argv[1]);
    int fMaxEvent = std::stoi(argv[2]);
    std::string fBeamType = argv[3];
    int fMaxFile = -1;

    // 하위 디렉토리(beam_type) 생성
    std::string savepath = Form("./output/%s/ATS", fBeamType.c_str());
    system(Form("mkdir -p %s", savepath.c_str()));
    
    TFile* fNtuple = TFile::Open((TString)("/pnfs/knu.ac.kr/data/cms/store/user/sungwon/2025_DRC_TB_PromptAnalysis/Prompt_ntuple_Run_" + std::to_string(fRunNum) + ".root"), "READ");
    TTreeReader reader("evt", fNtuple);

    // Create TTreeReaderValue for all waveform branches
    TTreeReaderValue<std::vector<short>> wave_M1_T1_C(reader, "wave_M1_T1_C"); TTreeReaderValue<std::vector<short>> wave_M1_T1_S(reader, "wave_M1_T1_S");
    TTreeReaderValue<std::vector<short>> wave_M1_T2_C(reader, "wave_M1_T2_C"); TTreeReaderValue<std::vector<short>> wave_M1_T2_S(reader, "wave_M1_T2_S");
    TTreeReaderValue<std::vector<short>> wave_M1_T3_C(reader, "wave_M1_T3_C"); TTreeReaderValue<std::vector<short>> wave_M1_T3_S(reader, "wave_M1_T3_S");
    TTreeReaderValue<std::vector<short>> wave_M1_T4_C(reader, "wave_M1_T4_C"); TTreeReaderValue<std::vector<short>> wave_M1_T4_S(reader, "wave_M1_T4_S");

    TTreeReaderValue<std::vector<short>> wave_M2_T1_C(reader, "wave_M2_T1_C"); TTreeReaderValue<std::vector<short>> wave_M2_T1_S(reader, "wave_M2_T1_S");
    TTreeReaderValue<std::vector<short>> wave_M2_T2_C(reader, "wave_M2_T2_C"); TTreeReaderValue<std::vector<short>> wave_M2_T2_S(reader, "wave_M2_T2_S");
    TTreeReaderValue<std::vector<short>> wave_M2_T3_C(reader, "wave_M2_T3_C"); TTreeReaderValue<std::vector<short>> wave_M2_T3_S(reader, "wave_M2_T3_S");
    TTreeReaderValue<std::vector<short>> wave_M2_T4_C(reader, "wave_M2_T4_C"); TTreeReaderValue<std::vector<short>> wave_M2_T4_S(reader, "wave_M2_T4_S");

    TTreeReaderValue<std::vector<short>> wave_M3_T1_C(reader, "wave_M3_T1_C"); TTreeReaderValue<std::vector<short>> wave_M3_T1_S(reader, "wave_M3_T1_S");
    TTreeReaderValue<std::vector<short>> wave_M3_T2_C(reader, "wave_M3_T2_C"); TTreeReaderValue<std::vector<short>> wave_M3_T2_S(reader, "wave_M3_T2_S");
    TTreeReaderValue<std::vector<short>> wave_M3_T3_C(reader, "wave_M3_T3_C"); TTreeReaderValue<std::vector<short>> wave_M3_T3_S(reader, "wave_M3_T3_S");
    TTreeReaderValue<std::vector<short>> wave_M3_T4_C(reader, "wave_M3_T4_C"); TTreeReaderValue<std::vector<short>> wave_M3_T4_S(reader, "wave_M3_T4_S");

    TTreeReaderValue<std::vector<short>> wave_M4_T1_C(reader, "wave_M4_T1_C"); TTreeReaderValue<std::vector<short>> wave_M4_T1_S(reader, "wave_M4_T1_S");
    TTreeReaderValue<std::vector<short>> wave_M4_T2_C(reader, "wave_M4_T2_C"); TTreeReaderValue<std::vector<short>> wave_M4_T2_S(reader, "wave_M4_T2_S");
    TTreeReaderValue<std::vector<short>> wave_M4_T3_C(reader, "wave_M4_T3_C"); TTreeReaderValue<std::vector<short>> wave_M4_T3_S(reader, "wave_M4_T3_S");
    TTreeReaderValue<std::vector<short>> wave_M4_T4_C(reader, "wave_M4_T4_C"); TTreeReaderValue<std::vector<short>> wave_M4_T4_S(reader, "wave_M4_T4_S");

    TTreeReaderValue<std::vector<short>> wave_M5_T1_C(reader, "wave_M5_T1_C"); TTreeReaderValue<std::vector<short>> wave_M5_T1_S(reader, "wave_M5_T1_S");
    TTreeReaderValue<std::vector<short>> wave_M5_T2_C(reader, "wave_M5_T2_C"); TTreeReaderValue<std::vector<short>> wave_M5_T2_S(reader, "wave_M5_T2_S");
    TTreeReaderValue<std::vector<short>> wave_M5_T3_C(reader, "wave_M5_T3_C"); TTreeReaderValue<std::vector<short>> wave_M5_T3_S(reader, "wave_M5_T3_S");
    TTreeReaderValue<std::vector<short>> wave_M5_T4_C(reader, "wave_M5_T4_C"); TTreeReaderValue<std::vector<short>> wave_M5_T4_S(reader, "wave_M5_T4_S");

    TTreeReaderValue<std::vector<short>> wave_M6_T1_C(reader, "wave_M6_T1_C"); TTreeReaderValue<std::vector<short>> wave_M6_T1_S(reader, "wave_M6_T1_S");
    TTreeReaderValue<std::vector<short>> wave_M6_T2_C(reader, "wave_M6_T2_C"); TTreeReaderValue<std::vector<short>> wave_M6_T2_S(reader, "wave_M6_T2_S");
    TTreeReaderValue<std::vector<short>> wave_M6_T3_C(reader, "wave_M6_T3_C"); TTreeReaderValue<std::vector<short>> wave_M6_T3_S(reader, "wave_M6_T3_S");
    TTreeReaderValue<std::vector<short>> wave_M6_T4_C(reader, "wave_M6_T4_C"); TTreeReaderValue<std::vector<short>> wave_M6_T4_S(reader, "wave_M6_T4_S");

    TTreeReaderValue<std::vector<short>> wave_M7_T1_C(reader, "wave_M7_T1_C"); TTreeReaderValue<std::vector<short>> wave_M7_T1_S(reader, "wave_M7_T1_S");
    TTreeReaderValue<std::vector<short>> wave_M7_T2_C(reader, "wave_M7_T2_C"); TTreeReaderValue<std::vector<short>> wave_M7_T2_S(reader, "wave_M7_T2_S");
    TTreeReaderValue<std::vector<short>> wave_M7_T3_C(reader, "wave_M7_T3_C"); TTreeReaderValue<std::vector<short>> wave_M7_T3_S(reader, "wave_M7_T3_S");
    TTreeReaderValue<std::vector<short>> wave_M7_T4_C(reader, "wave_M7_T4_C"); TTreeReaderValue<std::vector<short>> wave_M7_T4_S(reader, "wave_M7_T4_S");

    TTreeReaderValue<std::vector<short>> wave_M8_T1_C(reader, "wave_M8_T1_C"); TTreeReaderValue<std::vector<short>> wave_M8_T1_S(reader, "wave_M8_T1_S");
    TTreeReaderValue<std::vector<short>> wave_M8_T2_C(reader, "wave_M8_T2_C"); TTreeReaderValue<std::vector<short>> wave_M8_T2_S(reader, "wave_M8_T2_S");
    TTreeReaderValue<std::vector<short>> wave_M8_T3_C(reader, "wave_M8_T3_C"); TTreeReaderValue<std::vector<short>> wave_M8_T3_S(reader, "wave_M8_T3_S");
    TTreeReaderValue<std::vector<short>> wave_M8_T4_C(reader, "wave_M8_T4_C"); TTreeReaderValue<std::vector<short>> wave_M8_T4_S(reader, "wave_M8_T4_S");

    TTreeReaderValue<std::vector<short>> wave_M9_T1_C(reader, "wave_M9_T1_C"); TTreeReaderValue<std::vector<short>> wave_M9_T1_S(reader, "wave_M9_T1_S");
    TTreeReaderValue<std::vector<short>> wave_M9_T2_C(reader, "wave_M9_T2_C"); TTreeReaderValue<std::vector<short>> wave_M9_T2_S(reader, "wave_M9_T2_S");
    TTreeReaderValue<std::vector<short>> wave_M9_T3_C(reader, "wave_M9_T3_C"); TTreeReaderValue<std::vector<short>> wave_M9_T3_S(reader, "wave_M9_T3_S");
    TTreeReaderValue<std::vector<short>> wave_M9_T4_C(reader, "wave_M9_T4_C"); TTreeReaderValue<std::vector<short>> wave_M9_T4_S(reader, "wave_M9_T4_S");
    
    TTreeReaderValue<std::vector<short>> wave_CC1(reader, "wave_CC1"); TTreeReaderValue<std::vector<short>> wave_CC2(reader, "wave_CC2");
    TTreeReaderValue<std::vector<short>> wave_PS(reader, "wave_PS"); TTreeReaderValue<std::vector<short>> wave_MC(reader, "wave_MC"); TTreeReaderValue<std::vector<short>> wave_TC(reader, "wave_TC");

    TTreeReaderValue<std::vector<short>> wave_DWC1_R(reader, "wave_DWC1_R"); TTreeReaderValue<std::vector<short>> wave_DWC1_L(reader, "wave_DWC1_L"); TTreeReaderValue<std::vector<short>> wave_DWC1_U(reader, "wave_DWC1_U"); TTreeReaderValue<std::vector<short>> wave_DWC1_D(reader, "wave_DWC1_D");
    TTreeReaderValue<std::vector<short>> wave_DWC2_R(reader, "wave_DWC2_R"); TTreeReaderValue<std::vector<short>> wave_DWC2_L(reader, "wave_DWC2_L"); TTreeReaderValue<std::vector<short>> wave_DWC2_U(reader, "wave_DWC2_U"); TTreeReaderValue<std::vector<short>> wave_DWC2_D(reader, "wave_DWC2_D");

    TTreeReaderValue<std::vector<short>> wave_LC2(reader, "wave_LC2"); TTreeReaderValue<std::vector<short>> wave_LC3(reader, "wave_LC3"); TTreeReaderValue<std::vector<short>> wave_LC4(reader, "wave_LC4"); 
    TTreeReaderValue<std::vector<short>> wave_LC5(reader, "wave_LC5"); TTreeReaderValue<std::vector<short>> wave_LC7(reader, "wave_LC7"); TTreeReaderValue<std::vector<short>> wave_LC8(reader, "wave_LC8"); 
    TTreeReaderValue<std::vector<short>> wave_LC9(reader, "wave_LC9"); TTreeReaderValue<std::vector<short>> wave_LC10(reader, "wave_LC10"); TTreeReaderValue<std::vector<short>> wave_LC11(reader, "wave_LC11"); 
    TTreeReaderValue<std::vector<short>> wave_LC12(reader, "wave_LC12"); TTreeReaderValue<std::vector<short>> wave_LC13(reader, "wave_LC13"); TTreeReaderValue<std::vector<short>> wave_LC14(reader, "wave_LC14");
    TTreeReaderValue<std::vector<short>> wave_LC15(reader, "wave_LC15"); TTreeReaderValue<std::vector<short>> wave_LC16(reader, "wave_LC16"); TTreeReaderValue<std::vector<short>> wave_LC19(reader, "wave_LC19"); TTreeReaderValue<std::vector<short>> wave_LC20(reader, "wave_LC20");
    TTreeReaderValue<std::vector<short>> wave_T1(reader, "wave_T1"); TTreeReaderValue<std::vector<short>> wave_T2(reader, "wave_T2"); TTreeReaderValue<std::vector<short>> wave_T1NIM(reader, "wave_T1NIM"); TTreeReaderValue<std::vector<short>> wave_T2NIM(reader, "wave_T2NIM"); TTreeReaderValue<std::vector<short>> wave_Coin(reader, "wave_Coin");
    TTreeReaderValue<std::vector<short>> wave_Coin_ref_1(reader, "wave_Coin_ref_1"); TTreeReaderValue<std::vector<short>> wave_Coin_ref_2(reader, "wave_Coin_ref_2"); TTreeReaderValue<std::vector<short>> wave_Coin_ref_3(reader, "wave_Coin_ref_3"); TTreeReaderValue<std::vector<short>> wave_Coin_ref_4(reader, "wave_Coin_ref_4");

    // prepare the histograms wa want to draw
    TH1F* hist_M1_T1_C = new TH1F("M1_T1_C" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_M1_T2_C = new TH1F("M1_T2_C" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_M1_T3_C = new TH1F("M1_T3_C" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_M1_T4_C = new TH1F("M1_T4_C" , ";bin;nEvents", 1000, 0, 1000);

    TH1F* hist_M2_T1_C = new TH1F("M2_T1_C" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_M2_T2_C = new TH1F("M2_T2_C" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_M2_T3_C = new TH1F("M2_T3_C" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_M2_T4_C = new TH1F("M2_T4_C" , ";bin;nEvents", 1000, 0, 1000);

    TH1F* hist_M3_T1_C = new TH1F("M3_T1_C" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_M3_T2_C = new TH1F("M3_T2_C" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_M3_T3_C = new TH1F("M3_T3_C" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_M3_T4_C = new TH1F("M3_T4_C" , ";bin;nEvents", 1000, 0, 1000);

    TH1F* hist_M4_T1_C = new TH1F("M4_T1_C" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_M4_T2_C = new TH1F("M4_T2_C" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_M4_T3_C = new TH1F("M4_T3_C" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_M4_T4_C = new TH1F("M4_T4_C" , ";bin;nEvents", 1000, 0, 1000);

    TH1F* hist_M5_T1_C = new TH1F("M5_T1_C" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_M5_T2_C = new TH1F("M5_T2_C" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_M5_T3_C = new TH1F("M5_T3_C" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_M5_T4_C = new TH1F("M5_T4_C" , ";bin;nEvents", 1000, 0, 1000);

    TH1F* hist_M6_T1_C = new TH1F("M6_T1_C" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_M6_T2_C = new TH1F("M6_T2_C" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_M6_T3_C = new TH1F("M6_T3_C" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_M6_T4_C = new TH1F("M6_T4_C" , ";bin;nEvents", 1000, 0, 1000);

    TH1F* hist_M7_T1_C = new TH1F("M7_T1_C" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_M7_T2_C = new TH1F("M7_T2_C" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_M7_T3_C = new TH1F("M7_T3_C" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_M7_T4_C = new TH1F("M7_T4_C" , ";bin;nEvents", 1000, 0, 1000);

    TH1F* hist_M8_T1_C = new TH1F("M8_T1_C" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_M8_T2_C = new TH1F("M8_T2_C" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_M8_T3_C = new TH1F("M8_T3_C" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_M8_T4_C = new TH1F("M8_T4_C" , ";bin;nEvents", 1000, 0, 1000);

    TH1F* hist_M9_T1_C = new TH1F("M9_T1_C" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_M9_T2_C = new TH1F("M9_T2_C" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_M9_T3_C = new TH1F("M9_T3_C" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_M9_T4_C = new TH1F("M9_T4_C" , ";bin;nEvents", 1000, 0, 1000);

    TH1F* hist_M1_T1_S = new TH1F("M1_T1_S" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_M1_T2_S = new TH1F("M1_T2_S" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_M1_T3_S = new TH1F("M1_T3_S" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_M1_T4_S = new TH1F("M1_T4_S" , ";bin;nEvents", 1000, 0, 1000);

    TH1F* hist_M2_T1_S = new TH1F("M2_T1_S" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_M2_T2_S = new TH1F("M2_T2_S" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_M2_T3_S = new TH1F("M2_T3_S" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_M2_T4_S = new TH1F("M2_T4_S" , ";bin;nEvents", 1000, 0, 1000);

    TH1F* hist_M3_T1_S = new TH1F("M3_T1_S" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_M3_T2_S = new TH1F("M3_T2_S" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_M3_T3_S = new TH1F("M3_T3_S" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_M3_T4_S = new TH1F("M3_T4_S" , ";bin;nEvents", 1000, 0, 1000);

    TH1F* hist_M4_T1_S = new TH1F("M4_T1_S" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_M4_T2_S = new TH1F("M4_T2_S" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_M4_T3_S = new TH1F("M4_T3_S" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_M4_T4_S = new TH1F("M4_T4_S" , ";bin;nEvents", 1000, 0, 1000);

    TH1F* hist_M5_T1_S = new TH1F("M5_T1_S" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_M5_T2_S = new TH1F("M5_T2_S" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_M5_T3_S = new TH1F("M5_T3_S" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_M5_T4_S = new TH1F("M5_T4_S" , ";bin;nEvents", 1000, 0, 1000);

    TH1F* hist_M6_T1_S = new TH1F("M6_T1_S" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_M6_T2_S = new TH1F("M6_T2_S" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_M6_T3_S = new TH1F("M6_T3_S" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_M6_T4_S = new TH1F("M6_T4_S" , ";bin;nEvents", 1000, 0, 1000);

    TH1F* hist_M7_T1_S = new TH1F("M7_T1_S" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_M7_T2_S = new TH1F("M7_T2_S" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_M7_T3_S = new TH1F("M7_T3_S" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_M7_T4_S = new TH1F("M7_T4_S" , ";bin;nEvents", 1000, 0, 1000);

    TH1F* hist_M8_T1_S = new TH1F("M8_T1_S" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_M8_T2_S = new TH1F("M8_T2_S" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_M8_T3_S = new TH1F("M8_T3_S" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_M8_T4_S = new TH1F("M8_T4_S" , ";bin;nEvents", 1000, 0, 1000);

    TH1F* hist_M9_T1_S = new TH1F("M9_T1_S" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_M9_T2_S = new TH1F("M9_T2_S" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_M9_T3_S = new TH1F("M9_T3_S" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_M9_T4_S = new TH1F("M9_T4_S" , ";bin;nEvents", 1000, 0, 1000);

    TH1F* hist_CC1 = new TH1F("CC1", ";bin;Events", 1000, 0, 1000);
    TH1F* hist_CC2 = new TH1F("CC2", ";bin;Events", 1000, 0, 1000);

    TH1F* hist_PS = new TH1F("PS" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_MC = new TH1F("MC" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_TC = new TH1F("TC" , ";bin;nEvents", 1000, 0, 1000);

    TH1F* hist_LC2  = new TH1F("LC2 " , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_LC3  = new TH1F("LC3 " , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_LC4  = new TH1F("LC4 " , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_LC5  = new TH1F("LC5 " , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_LC7  = new TH1F("LC7 " , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_LC8  = new TH1F("LC8 " , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_LC9  = new TH1F("LC9 " , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_LC10 = new TH1F("LC10" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_LC11 = new TH1F("LC11" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_LC12 = new TH1F("LC12" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_LC13 = new TH1F("LC13" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_LC14 = new TH1F("LC14" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_LC15 = new TH1F("LC15" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_LC16 = new TH1F("LC16" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_LC19 = new TH1F("LC19" , ";bin;nEvents", 1000, 0, 1000);
    TH1F* hist_LC20 = new TH1F("LC20" , ";bin;nEvents", 1000, 0, 1000);

    TH1F* hist_DWC1R = new TH1F("DWC1R", ";bin;ADC", 1000, 0, 1000);
    TH1F* hist_DWC1L = new TH1F("DWC1L", ";bin;ADC", 1000, 0, 1000);
    TH1F* hist_DWC1U = new TH1F("DWC1U", ";bin;ADC", 1000, 0, 1000);
    TH1F* hist_DWC1D = new TH1F("DWC1D", ";bin;ADC", 1000, 0, 1000);

    TH1F* hist_DWC2R = new TH1F("DWC2R", ";bin;ADC", 1000, 0, 1000);
    TH1F* hist_DWC2L = new TH1F("DWC2L", ";bin;ADC", 1000, 0, 1000);
    TH1F* hist_DWC2U = new TH1F("DWC2U", ";bin;ADC", 1000, 0, 1000);
    TH1F* hist_DWC2D = new TH1F("DWC2D", ";bin;ADC", 1000, 0, 1000);

    // Set Maximum event
    Long64_t totalEntries = reader.GetEntries();
    if (fMaxEvent == -1 || fMaxEvent > totalEntries)
        fMaxEvent = totalEntries;

    // Evt Loop
    for (int iEvt = 0; iEvt < fMaxEvent; iEvt++) {
        printProgress(iEvt, fMaxEvent);
        reader.SetEntry(iEvt);

        for (int bin = 1; bin < 1001; bin++) {      

            hist_M1_T1_C->Fill(bin, (float) ((*wave_M1_T1_C).at(bin)) / (float) (fMaxEvent) );
            hist_M1_T2_C->Fill(bin, (float) ((*wave_M1_T2_C).at(bin)) / (float) (fMaxEvent) );
            hist_M1_T3_C->Fill(bin, (float) ((*wave_M1_T3_C).at(bin)) / (float) (fMaxEvent) );
            hist_M1_T4_C->Fill(bin, (float) ((*wave_M1_T4_C).at(bin)) / (float) (fMaxEvent) );

            hist_M2_T1_C->Fill(bin, (float) ((*wave_M2_T1_C).at(bin)) / (float) (fMaxEvent) );
            hist_M2_T2_C->Fill(bin, (float) ((*wave_M2_T2_C).at(bin)) / (float) (fMaxEvent) );
            hist_M2_T3_C->Fill(bin, (float) ((*wave_M2_T3_C).at(bin)) / (float) (fMaxEvent) );
            hist_M2_T4_C->Fill(bin, (float) ((*wave_M2_T4_C).at(bin)) / (float) (fMaxEvent) );

            hist_M3_T1_C->Fill(bin, (float) ((*wave_M3_T1_C).at(bin)) / (float) (fMaxEvent) );
            hist_M3_T2_C->Fill(bin, (float) ((*wave_M3_T2_C).at(bin)) / (float) (fMaxEvent) );
            hist_M3_T3_C->Fill(bin, (float) ((*wave_M3_T3_C).at(bin)) / (float) (fMaxEvent) );
            hist_M3_T4_C->Fill(bin, (float) ((*wave_M3_T4_C).at(bin)) / (float) (fMaxEvent) );

            hist_M4_T1_C->Fill(bin, (float) ((*wave_M4_T1_C).at(bin)) / (float) (fMaxEvent) );
            hist_M4_T2_C->Fill(bin, (float) ((*wave_M4_T2_C).at(bin)) / (float) (fMaxEvent) );
            hist_M4_T3_C->Fill(bin, (float) ((*wave_M4_T3_C).at(bin)) / (float) (fMaxEvent) );
            hist_M4_T4_C->Fill(bin, (float) ((*wave_M4_T4_C).at(bin)) / (float) (fMaxEvent) );

            hist_M5_T1_C->Fill(bin, (float) ((*wave_M5_T1_C).at(bin)) / (float) (fMaxEvent) );
            hist_M5_T2_C->Fill(bin, (float) ((*wave_M5_T2_C).at(bin)) / (float) (fMaxEvent) );
            hist_M5_T3_C->Fill(bin, (float) ((*wave_M5_T3_C).at(bin)) / (float) (fMaxEvent) );
            hist_M5_T4_C->Fill(bin, (float) ((*wave_M5_T4_C).at(bin)) / (float) (fMaxEvent) );

            hist_M6_T1_C->Fill(bin, (float) ((*wave_M6_T1_C).at(bin)) / (float) (fMaxEvent) );
            hist_M6_T2_C->Fill(bin, (float) ((*wave_M6_T2_C).at(bin)) / (float) (fMaxEvent) );
            hist_M6_T3_C->Fill(bin, (float) ((*wave_M6_T3_C).at(bin)) / (float) (fMaxEvent) );
            hist_M6_T4_C->Fill(bin, (float) ((*wave_M6_T4_C).at(bin)) / (float) (fMaxEvent) );

            hist_M7_T1_C->Fill(bin, (float) ((*wave_M7_T1_C).at(bin)) / (float) (fMaxEvent) );
            hist_M7_T2_C->Fill(bin, (float) ((*wave_M7_T2_C).at(bin)) / (float) (fMaxEvent) );
            hist_M7_T3_C->Fill(bin, (float) ((*wave_M7_T3_C).at(bin)) / (float) (fMaxEvent) );
            hist_M7_T4_C->Fill(bin, (float) ((*wave_M7_T4_C).at(bin)) / (float) (fMaxEvent) );

            hist_M8_T1_C->Fill(bin, (float) ((*wave_M8_T1_C).at(bin)) / (float) (fMaxEvent) );
            hist_M8_T2_C->Fill(bin, (float) ((*wave_M8_T2_C).at(bin)) / (float) (fMaxEvent) );
            hist_M8_T3_C->Fill(bin, (float) ((*wave_M8_T3_C).at(bin)) / (float) (fMaxEvent) );
            hist_M8_T4_C->Fill(bin, (float) ((*wave_M8_T4_C).at(bin)) / (float) (fMaxEvent) );

            hist_M9_T1_C->Fill(bin, (float) ((*wave_M9_T1_C).at(bin)) / (float) (fMaxEvent) );
            hist_M9_T2_C->Fill(bin, (float) ((*wave_M9_T2_C).at(bin)) / (float) (fMaxEvent) );
            hist_M9_T3_C->Fill(bin, (float) ((*wave_M9_T3_C).at(bin)) / (float) (fMaxEvent) );
            hist_M9_T4_C->Fill(bin, (float) ((*wave_M9_T4_C).at(bin)) / (float) (fMaxEvent) );

            hist_M1_T1_S->Fill(bin, (float) ((*wave_M1_T1_S).at(bin)) / (float) (fMaxEvent) );
            hist_M1_T2_S->Fill(bin, (float) ((*wave_M1_T2_S).at(bin)) / (float) (fMaxEvent) );
            hist_M1_T3_S->Fill(bin, (float) ((*wave_M1_T3_S).at(bin)) / (float) (fMaxEvent) );
            hist_M1_T4_S->Fill(bin, (float) ((*wave_M1_T4_S).at(bin)) / (float) (fMaxEvent) );

            hist_M2_T1_S->Fill(bin, (float) ((*wave_M2_T1_S).at(bin)) / (float) (fMaxEvent) );
            hist_M2_T2_S->Fill(bin, (float) ((*wave_M2_T2_S).at(bin)) / (float) (fMaxEvent) );
            hist_M2_T3_S->Fill(bin, (float) ((*wave_M2_T3_S).at(bin)) / (float) (fMaxEvent) );
            hist_M2_T4_S->Fill(bin, (float) ((*wave_M2_T4_S).at(bin)) / (float) (fMaxEvent) );

            hist_M3_T1_S->Fill(bin, (float) ((*wave_M3_T1_S).at(bin)) / (float) (fMaxEvent) );
            hist_M3_T2_S->Fill(bin, (float) ((*wave_M3_T2_S).at(bin)) / (float) (fMaxEvent) );
            hist_M3_T3_S->Fill(bin, (float) ((*wave_M3_T3_S).at(bin)) / (float) (fMaxEvent) );
            hist_M3_T4_S->Fill(bin, (float) ((*wave_M3_T4_S).at(bin)) / (float) (fMaxEvent) );

            hist_M4_T1_S->Fill(bin, (float) ((*wave_M4_T1_S).at(bin)) / (float) (fMaxEvent) );
            hist_M4_T2_S->Fill(bin, (float) ((*wave_M4_T2_S).at(bin)) / (float) (fMaxEvent) );
            hist_M4_T3_S->Fill(bin, (float) ((*wave_M4_T3_S).at(bin)) / (float) (fMaxEvent) );
            hist_M4_T4_S->Fill(bin, (float) ((*wave_M4_T4_S).at(bin)) / (float) (fMaxEvent) );

            hist_M5_T1_S->Fill(bin, (float) ((*wave_M5_T1_S).at(bin)) / (float) (fMaxEvent) );
            hist_M5_T2_S->Fill(bin, (float) ((*wave_M5_T2_S).at(bin)) / (float) (fMaxEvent) );
            hist_M5_T3_S->Fill(bin, (float) ((*wave_M5_T3_S).at(bin)) / (float) (fMaxEvent) );
            hist_M5_T4_S->Fill(bin, (float) ((*wave_M5_T4_S).at(bin)) / (float) (fMaxEvent) );

            hist_M6_T1_S->Fill(bin, (float) ((*wave_M6_T1_S).at(bin)) / (float) (fMaxEvent) );
            hist_M6_T2_S->Fill(bin, (float) ((*wave_M6_T2_S).at(bin)) / (float) (fMaxEvent) );
            hist_M6_T3_S->Fill(bin, (float) ((*wave_M6_T3_S).at(bin)) / (float) (fMaxEvent) );
            hist_M6_T4_S->Fill(bin, (float) ((*wave_M6_T4_S).at(bin)) / (float) (fMaxEvent) );

            hist_M7_T1_S->Fill(bin, (float) ((*wave_M7_T1_S).at(bin)) / (float) (fMaxEvent) );
            hist_M7_T2_S->Fill(bin, (float) ((*wave_M7_T2_S).at(bin)) / (float) (fMaxEvent) );
            hist_M7_T3_S->Fill(bin, (float) ((*wave_M7_T3_S).at(bin)) / (float) (fMaxEvent) );
            hist_M7_T4_S->Fill(bin, (float) ((*wave_M7_T4_S).at(bin)) / (float) (fMaxEvent) );

            hist_M8_T1_S->Fill(bin, (float) ((*wave_M8_T1_S).at(bin)) / (float) (fMaxEvent) );
            hist_M8_T2_S->Fill(bin, (float) ((*wave_M8_T2_S).at(bin)) / (float) (fMaxEvent) );
            hist_M8_T3_S->Fill(bin, (float) ((*wave_M8_T3_S).at(bin)) / (float) (fMaxEvent) );
            hist_M8_T4_S->Fill(bin, (float) ((*wave_M8_T4_S).at(bin)) / (float) (fMaxEvent) );

            hist_M9_T1_S->Fill(bin, (float) ((*wave_M9_T1_S).at(bin)) / (float) (fMaxEvent) );
            hist_M9_T2_S->Fill(bin, (float) ((*wave_M9_T2_S).at(bin)) / (float) (fMaxEvent) );
            hist_M9_T3_S->Fill(bin, (float) ((*wave_M9_T3_S).at(bin)) / (float) (fMaxEvent) );
            hist_M9_T4_S->Fill(bin, (float) ((*wave_M9_T4_S).at(bin)) / (float) (fMaxEvent) );

            hist_CC1->Fill(bin, (float) ((*wave_CC1).at(bin)) / (float) (fMaxEvent) );
            hist_CC2->Fill(bin, (float) ((*wave_CC2).at(bin)) / (float) (fMaxEvent) );

            hist_PS->Fill(bin, (float) ((*wave_PS).at(bin)) / (float) (fMaxEvent) );
            hist_MC->Fill(bin, (float) ((*wave_MC).at(bin)) / (float) (fMaxEvent) );
            hist_TC->Fill(bin, (float) ((*wave_TC).at(bin)) / (float) (fMaxEvent) );

            hist_LC2 ->Fill(bin, (float) ((*wave_LC2).at(bin)) / (float) (fMaxEvent) );
            hist_LC3 ->Fill(bin, (float) ((*wave_LC3).at(bin)) / (float) (fMaxEvent) );
            hist_LC4 ->Fill(bin, (float) ((*wave_LC4).at(bin)) / (float) (fMaxEvent) );
            hist_LC5 ->Fill(bin, (float) ((*wave_LC5).at(bin)) / (float) (fMaxEvent) );
            hist_LC7 ->Fill(bin, (float) ((*wave_LC7).at(bin)) / (float) (fMaxEvent) );
            hist_LC8 ->Fill(bin, (float) ((*wave_LC8).at(bin)) / (float) (fMaxEvent) );
            hist_LC9 ->Fill(bin, (float) ((*wave_LC9).at(bin)) / (float) (fMaxEvent) );
            hist_LC10->Fill(bin, (float) ((*wave_LC10).at(bin)) / (float) (fMaxEvent) );
            hist_LC11->Fill(bin, (float) ((*wave_LC11).at(bin)) / (float) (fMaxEvent) );
            hist_LC12->Fill(bin, (float) ((*wave_LC12).at(bin)) / (float) (fMaxEvent) );
            hist_LC13->Fill(bin, (float) ((*wave_LC13).at(bin)) / (float) (fMaxEvent) );
            hist_LC14->Fill(bin, (float) ((*wave_LC14).at(bin)) / (float) (fMaxEvent) );
            hist_LC15->Fill(bin, (float) ((*wave_LC15).at(bin)) / (float) (fMaxEvent) );
            hist_LC16->Fill(bin, (float) ((*wave_LC16).at(bin)) / (float) (fMaxEvent) );
            hist_LC19->Fill(bin, (float) ((*wave_LC19).at(bin)) / (float) (fMaxEvent) );
            hist_LC20->Fill(bin, (float) ((*wave_LC20).at(bin)) / (float) (fMaxEvent) );

            hist_DWC1R->Fill(bin, (float) ((*wave_DWC1_R).at(bin)) / (float) (fMaxEvent) );
            hist_DWC1L->Fill(bin, (float) ((*wave_DWC1_L).at(bin)) / (float) (fMaxEvent) );
            hist_DWC1U->Fill(bin, (float) ((*wave_DWC1_U).at(bin)) / (float) (fMaxEvent) );
            hist_DWC1D->Fill(bin, (float) ((*wave_DWC1_D).at(bin)) / (float) (fMaxEvent) );

            hist_DWC2R->Fill(bin, (float) ((*wave_DWC2_R).at(bin)) / (float) (fMaxEvent) );
            hist_DWC2L->Fill(bin, (float) ((*wave_DWC2_L).at(bin)) / (float) (fMaxEvent) );
            hist_DWC2U->Fill(bin, (float) ((*wave_DWC2_U).at(bin)) / (float) (fMaxEvent) );
            hist_DWC2D->Fill(bin, (float) ((*wave_DWC2_D).at(bin)) / (float) (fMaxEvent) );
        }
    }

    // Saving the DWC position & correlation plots in root file in ./dwc directory.
    std::string outFile = savepath + "/ATS_Run_" + std::to_string(fRunNum) + ".root";
    TFile* outputRoot = new TFile(outFile.c_str(), "RECREATE");
    outputRoot->cd();

    hist_M1_T1_C->Write();
    hist_M1_T2_C->Write();
    hist_M1_T3_C->Write();
    hist_M1_T4_C->Write();

    hist_M2_T1_C->Write();
    hist_M2_T2_C->Write();
    hist_M2_T3_C->Write();
    hist_M2_T4_C->Write();

    hist_M3_T1_C->Write();
    hist_M3_T2_C->Write();
    hist_M3_T3_C->Write();
    hist_M3_T4_C->Write();

    hist_M4_T1_C->Write();
    hist_M4_T2_C->Write();
    hist_M4_T3_C->Write();
    hist_M4_T4_C->Write();

    hist_M5_T1_C->Write();
    hist_M5_T2_C->Write();
    hist_M5_T3_C->Write();
    hist_M5_T4_C->Write();

    hist_M6_T1_C->Write();
    hist_M6_T2_C->Write();
    hist_M6_T3_C->Write();
    hist_M6_T4_C->Write();

    hist_M7_T1_C->Write();
    hist_M7_T2_C->Write();
    hist_M7_T3_C->Write();
    hist_M7_T4_C->Write();

    hist_M8_T1_C->Write();
    hist_M8_T2_C->Write();
    hist_M8_T3_C->Write();
    hist_M8_T4_C->Write();

    hist_M9_T1_C->Write();
    hist_M9_T2_C->Write();
    hist_M9_T3_C->Write();
    hist_M9_T4_C->Write();

    hist_M1_T1_S->Write();
    hist_M1_T2_S->Write();
    hist_M1_T3_S->Write();
    hist_M1_T4_S->Write();

    hist_M2_T1_S->Write();
    hist_M2_T2_S->Write();
    hist_M2_T3_S->Write();
    hist_M2_T4_S->Write();

    hist_M3_T1_S->Write();
    hist_M3_T2_S->Write();
    hist_M3_T3_S->Write();
    hist_M3_T4_S->Write();

    hist_M4_T1_S->Write();
    hist_M4_T2_S->Write();
    hist_M4_T3_S->Write();
    hist_M4_T4_S->Write();

    hist_M5_T1_S->Write();
    hist_M5_T2_S->Write();
    hist_M5_T3_S->Write();
    hist_M5_T4_S->Write();

    hist_M6_T1_S->Write();
    hist_M6_T2_S->Write();
    hist_M6_T3_S->Write();
    hist_M6_T4_S->Write();

    hist_M7_T1_S->Write();
    hist_M7_T2_S->Write();
    hist_M7_T3_S->Write();
    hist_M7_T4_S->Write();

    hist_M8_T1_S->Write();
    hist_M8_T2_S->Write();
    hist_M8_T3_S->Write();
    hist_M8_T4_S->Write();

    hist_M9_T1_S->Write();
    hist_M9_T2_S->Write();
    hist_M9_T3_S->Write();
    hist_M9_T4_S->Write();
    
    hist_CC1->Write();
    hist_CC2->Write();

    hist_PS->Write();
    hist_MC->Write();
    hist_TC->Write();

    hist_LC2 ->Write();
    hist_LC3 ->Write();
    hist_LC4 ->Write();
    hist_LC5 ->Write();
    hist_LC7 ->Write();
    hist_LC8 ->Write();
    hist_LC9 ->Write();
    hist_LC10->Write();
    hist_LC11->Write();
    hist_LC12->Write();
    hist_LC13->Write();
    hist_LC14->Write();
    hist_LC15->Write();
    hist_LC16->Write();
    hist_LC19->Write();
    hist_LC20->Write();

    hist_DWC1R->Write();
    hist_DWC1L->Write();
    hist_DWC1U->Write();
    hist_DWC1D->Write();

    hist_DWC2R->Write();
    hist_DWC2L->Write();
    hist_DWC2U->Write();
    hist_DWC2D->Write();

    outputRoot->Close();
}