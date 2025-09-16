#include "TBread.h"
#include "TButility.h"

#include <filesystem>
#include <chrono>
#include <numeric>
#include <vector>
#include <fstream>
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "TCanvas.h"
#include "TH1.h"
#include "TH2.h"
#include "TFile.h"
#include "TF1.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"
#include "TApplication.h"
#include "TROOT.h"

#include "./header/function.h"
#include "./header/preset.h"
#include "./header/HistogramManager.h"

namespace fs = std::filesystem;

int main(int argc, char *argv[])
{
    if (argc != 8) {
        std::cerr << "Usage: " << argv[0] << " <job version> <run_number> <max_event> <module> [beam_type] [rotation_on|rotation_off] [interaction_on|interaction_off]" << std::endl;
        std::cerr << "- module number is center module number: 5 (M5T2) or 4 (M4T2)" << std::endl;
        std::cerr << "- beam_type: em, pi, kaon, proton" << std::endl;
        return -1;
    }

    std::string fJobVersion = argv[1];
    int fRunNum = std::stoi(argv[2]);
    int fMaxEvent = std::stoi(argv[3]);
    int fModule = std::stoi(argv[4]);
    std::string beam_type = argv[5];
    std::string rotation_str = argv[6];
    std::string interaction_str = argv[7];
    int fMaxFile = -1;
    
    std::string version = "";
    if(rotation_str == "on")    {version += "_Rot";}
    if(interaction_str == "on") {version += "_IT";}
    
    // beam_type에 따른 컷 값 설정
    setCutsForBeamType(beam_type);
    std::cout << "Beam type: " << (beam_type.empty() ? "em (default)" : beam_type) << std::endl;
    std::cout << "Cut values set: CC1[" << CUT_CC1.first << ", " << CUT_CC1.second << "], ";
    std::cout << "CC2 cut [" << CUT_CC2.first << ", " << CUT_CC2.second << "], ";
    std::cout << "PS cut [" << CUT_PS.first << ", " << CUT_PS.second << "], ";
    std::cout << "MC cut [" << CUT_MC.first << ", " << CUT_MC.second << "]" << std::endl;
    
    std::string savepath = Form("./output/%s/%s/Evtloop%s", fJobVersion.c_str(), beam_type.c_str(), version.c_str());
    system(Form("mkdir -p %s", savepath.c_str()));
        
    // initialize the utility class
    TButility util = TButility();
    util.LoadMapping("../mapping/mapping_TB2025_v1.root");
    
    TFile* f_DWC = nullptr;
    TH2D* h_DWC1_pos = nullptr;
    TH2D* h_DWC2_pos = nullptr;
    std::vector<float> DWC1_offset;
    std::vector<float> DWC2_offset;
    
    // Load DWC data only for calibration runs (not for merged energy files)
    //std::cout << "[Loading] DWC data for " << beam_type << " beam type" << std::endl;
    if(beam_type.empty())
    {
        f_DWC = TFile::Open((TString)("./output/Calib/DWC/DWC_Run_" + std::to_string(fRunNum) + ".root"), "READ");
        h_DWC1_pos = (TH2D*) f_DWC->Get("dwc1_pos");
        h_DWC2_pos = (TH2D*) f_DWC->Get("dwc2_pos");
        DWC1_offset = getDWCoffset(h_DWC1_pos); // DWC1_offset.at(0) == X, DWC1_offset.at(1) == Y
        DWC2_offset = getDWCoffset(h_DWC2_pos);
    }
    else
    {
        if(beam_type == "pi")          {f_DWC = TFile::Open((TString)("./output/DWC/pi/DWC_Run_" + std::to_string(fRunNum) + ".root"), "READ");}
        else if(beam_type == "kaon")   {f_DWC = TFile::Open((TString)("./output/DWC/kp/DWC_Run_" + std::to_string(fRunNum) + ".root"), "READ");}
        else if(beam_type == "proton") {f_DWC = TFile::Open((TString)("./output/DWC/kp/DWC_Run_" + std::to_string(fRunNum) + ".root"), "READ");}
        else if(beam_type == "em")     {f_DWC = TFile::Open((TString)("./output/em/DWC/DWC_Run_" + std::to_string(fRunNum) + ".root"), "READ");}
        else
        {
            std::cerr << "Invalid beam type: " << beam_type << std::endl;
            std::cerr << "supported beam_type: em, pi, kaon, proton" << std::endl;
            return -1;
        }
        h_DWC1_pos = (TH2D*) f_DWC->Get("dwc1_pos");
        h_DWC2_pos = (TH2D*) f_DWC->Get("dwc2_pos");
        DWC1_offset = getDWCoffset(h_DWC1_pos); // DWC1_offset.at(0) == X, DWC1_offset.at(1) == Y
        DWC2_offset = getDWCoffset(h_DWC2_pos);
    }
    
    // Initialize histogram manager and create all histograms
    //std::cout << "[Initializing] Histogram manager" << std::endl;
    HistogramManager histManager;
    histManager.DeclareHistograms();

    int nEvent_count_0 = 0;
    int nEvent_count_1 = 0;
    int nEvent_count_2 = 0;
    int nEvent_count_3 = 0;
    int nEvent_count_4 = 0;
    int nEvent_count_5 = 0;
    int nEvent_count_6 = 0;
    
    //std::cout << "[Loading] Ntuple file for " << beam_type << " beam type" << std::endl;
    TChain* chain = new TChain("evt");
    TFile* fNtuple = TFile::Open((TString)("/pnfs/knu.ac.kr/data/cms/store/user/sungwon/2025_DRC_TB_PromptAnalysis/Prompt_ntuple_Run_" + std::to_string(fRunNum) + ".root"), "READ");
    // Create TTreeReader
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

    // Set Maximum event
    //std::cout << "[Setting] Maximum event" << std::endl;
    Long64_t totalEntries = reader.GetEntries();
    if (fMaxEvent == -1 || fMaxEvent > totalEntries)
        fMaxEvent = totalEntries;
    
    for (int iEvt = 0; iEvt < fMaxEvent; iEvt++)
    {
        printProgress(iEvt, fMaxEvent);
        // Load event using TTreeReader
        reader.SetEntry(iEvt);
        
        std::vector<float> intADC_M1_C; std::vector<float> intADC_M1_S; std::vector<float> intADC_M2_C; std::vector<float> intADC_M2_S; std::vector<float> intADC_M3_C; std::vector<float> intADC_M3_S;
        std::vector<float> intADC_M4_C; std::vector<float> intADC_M4_S; std::vector<float> intADC_M5_C; std::vector<float> intADC_M5_S; std::vector<float> intADC_M6_C; std::vector<float> intADC_M6_S;
        std::vector<float> intADC_M7_C; std::vector<float> intADC_M7_S; std::vector<float> intADC_M8_C; std::vector<float> intADC_M8_S; std::vector<float> intADC_M9_C; std::vector<float> intADC_M9_S;

        // Get IntADC for M1, M2, M3, M4, M5, M6, M7, M8, M9
        // Using std::pair ranges instead of individual FIRST/LAST variables
        intADC_M1_C.emplace_back( GetInt(*wave_M1_T1_C, C_RANGE.first, C_RANGE.second) ); intADC_M1_C.emplace_back( GetInt(*wave_M1_T2_C, C_RANGE.first, C_RANGE.second) ); intADC_M1_C.emplace_back( GetInt(*wave_M1_T3_C, C_RANGE.first, C_RANGE.second) ); intADC_M1_C.emplace_back( GetInt(*wave_M1_T4_C, C_RANGE.first, C_RANGE.second) );
        intADC_M2_C.emplace_back( GetInt(*wave_M2_T1_C, C_RANGE.first, C_RANGE.second) ); intADC_M2_C.emplace_back( GetInt(*wave_M2_T2_C, C_RANGE.first, C_RANGE.second) ); intADC_M2_C.emplace_back( GetInt(*wave_M2_T3_C, C_RANGE.first, C_RANGE.second) ); intADC_M2_C.emplace_back( GetInt(*wave_M2_T4_C, C_RANGE.first, C_RANGE.second) );
        intADC_M3_C.emplace_back( GetInt(*wave_M3_T1_C, C_RANGE.first, C_RANGE.second) ); intADC_M3_C.emplace_back( GetInt(*wave_M3_T2_C, C_RANGE.first, C_RANGE.second) ); intADC_M3_C.emplace_back( GetInt(*wave_M3_T3_C, C_RANGE.first, C_RANGE.second) ); intADC_M3_C.emplace_back( GetInt(*wave_M3_T4_C, C_RANGE.first, C_RANGE.second) );
        intADC_M4_C.emplace_back( GetInt(*wave_M4_T1_C, C_RANGE.first, C_RANGE.second) ); intADC_M4_C.emplace_back( GetInt(*wave_M4_T2_C, C_RANGE.first, C_RANGE.second) ); intADC_M4_C.emplace_back( GetInt(*wave_M4_T3_C, C_RANGE.first, C_RANGE.second) ); intADC_M4_C.emplace_back( GetInt(*wave_M4_T4_C, C_RANGE.first, C_RANGE.second) );
        intADC_M5_C.emplace_back( GetInt(*wave_M5_T1_C, C_RANGE.first, C_RANGE.second) ); intADC_M5_C.emplace_back( GetInt(*wave_M5_T2_C, C_RANGE.first, C_RANGE.second) ); intADC_M5_C.emplace_back( GetInt(*wave_M5_T3_C, M5T3C_RANGE.first, M5T3C_RANGE.second) ); intADC_M5_C.emplace_back( GetInt(*wave_M5_T4_C, C_RANGE.first, C_RANGE.second) );
        intADC_M6_C.emplace_back( GetInt(*wave_M6_T1_C, C_RANGE.first, C_RANGE.second) ); intADC_M6_C.emplace_back( GetInt(*wave_M6_T2_C, C_RANGE.first, C_RANGE.second) ); intADC_M6_C.emplace_back( GetInt(*wave_M6_T3_C, C_RANGE.first, C_RANGE.second) ); intADC_M6_C.emplace_back( GetInt(*wave_M6_T4_C, C_RANGE.first, C_RANGE.second) );
        intADC_M7_C.emplace_back( GetInt(*wave_M7_T1_C, C_RANGE.first, C_RANGE.second) ); intADC_M7_C.emplace_back( GetInt(*wave_M7_T2_C, C_RANGE.first, C_RANGE.second) ); intADC_M7_C.emplace_back( GetInt(*wave_M7_T3_C, C_RANGE.first, C_RANGE.second) ); intADC_M7_C.emplace_back( GetInt(*wave_M7_T4_C, C_RANGE.first, C_RANGE.second) );
        intADC_M8_C.emplace_back( GetInt(*wave_M8_T1_C, C_RANGE.first, C_RANGE.second) ); intADC_M8_C.emplace_back( GetInt(*wave_M8_T2_C, C_RANGE.first, C_RANGE.second) ); intADC_M8_C.emplace_back( GetInt(*wave_M8_T3_C, C_RANGE.first, C_RANGE.second) ); intADC_M8_C.emplace_back( GetInt(*wave_M8_T4_C, C_RANGE.first, C_RANGE.second) );
        intADC_M9_C.emplace_back( GetInt(*wave_M9_T1_C, C_RANGE.first, C_RANGE.second) ); intADC_M9_C.emplace_back( GetInt(*wave_M9_T2_C, C_RANGE.first, C_RANGE.second) ); intADC_M9_C.emplace_back( GetInt(*wave_M9_T3_C, C_RANGE.first, C_RANGE.second) ); intADC_M9_C.emplace_back( GetInt(*wave_M9_T4_C, C_RANGE.first, C_RANGE.second) );

        intADC_M1_S.emplace_back( GetInt(*wave_M1_T1_S, S_RANGE.first, S_RANGE.second) ); intADC_M1_S.emplace_back( GetInt(*wave_M1_T2_S, S_RANGE.first, S_RANGE.second) ); intADC_M1_S.emplace_back( GetInt(*wave_M1_T3_S, S_RANGE.first, S_RANGE.second) ); intADC_M1_S.emplace_back( GetInt(*wave_M1_T4_S, S_RANGE.first, S_RANGE.second) );
        intADC_M2_S.emplace_back( GetInt(*wave_M2_T1_S, S_RANGE.first, S_RANGE.second) ); intADC_M2_S.emplace_back( GetInt(*wave_M2_T2_S, S_RANGE.first, S_RANGE.second) ); intADC_M2_S.emplace_back( GetInt(*wave_M2_T3_S, S_RANGE.first, S_RANGE.second) ); intADC_M2_S.emplace_back( GetInt(*wave_M2_T4_S, S_RANGE.first, S_RANGE.second) );
        intADC_M3_S.emplace_back( GetInt(*wave_M3_T1_S, S_RANGE.first, S_RANGE.second) ); intADC_M3_S.emplace_back( GetInt(*wave_M3_T2_S, S_RANGE.first, S_RANGE.second) ); intADC_M3_S.emplace_back( GetInt(*wave_M3_T3_S, S_RANGE.first, S_RANGE.second) ); intADC_M3_S.emplace_back( GetInt(*wave_M3_T4_S, S_RANGE.first, S_RANGE.second) );
        intADC_M4_S.emplace_back( GetInt(*wave_M4_T1_S, S_RANGE.first, S_RANGE.second) ); intADC_M4_S.emplace_back( GetInt(*wave_M4_T2_S, S_RANGE.first, S_RANGE.second) ); intADC_M4_S.emplace_back( GetInt(*wave_M4_T3_S, S_RANGE.first, S_RANGE.second) ); intADC_M4_S.emplace_back( GetInt(*wave_M4_T4_S, S_RANGE.first, S_RANGE.second) );
        intADC_M5_S.emplace_back( GetInt(*wave_M5_T1_S, S_RANGE.first, S_RANGE.second) ); intADC_M5_S.emplace_back( GetInt(*wave_M5_T2_S, S_RANGE.first, S_RANGE.second) ); intADC_M5_S.emplace_back( GetInt(*wave_M5_T3_S, M5T3S_RANGE.first, M5T3S_RANGE.second) ); intADC_M5_S.emplace_back( GetInt(*wave_M5_T4_S, S_RANGE.first, S_RANGE.second) );
        intADC_M6_S.emplace_back( GetInt(*wave_M6_T1_S, S_RANGE.first, S_RANGE.second) ); intADC_M6_S.emplace_back( GetInt(*wave_M6_T2_S, S_RANGE.first, S_RANGE.second) ); intADC_M6_S.emplace_back( GetInt(*wave_M6_T3_S, S_RANGE.first, S_RANGE.second) ); intADC_M6_S.emplace_back( GetInt(*wave_M6_T4_S, S_RANGE.first, S_RANGE.second) );
        intADC_M7_S.emplace_back( GetInt(*wave_M7_T1_S, S_RANGE.first, S_RANGE.second) ); intADC_M7_S.emplace_back( GetInt(*wave_M7_T2_S, S_RANGE.first, S_RANGE.second) ); intADC_M7_S.emplace_back( GetInt(*wave_M7_T3_S, S_RANGE.first, S_RANGE.second) ); intADC_M7_S.emplace_back( GetInt(*wave_M7_T4_S, S_RANGE.first, S_RANGE.second) );
        intADC_M8_S.emplace_back( GetInt(*wave_M8_T1_S, S_RANGE.first, S_RANGE.second) ); intADC_M8_S.emplace_back( GetInt(*wave_M8_T2_S, S_RANGE.first, S_RANGE.second) ); intADC_M8_S.emplace_back( GetInt(*wave_M8_T3_S, S_RANGE.first, S_RANGE.second) ); intADC_M8_S.emplace_back( GetInt(*wave_M8_T4_S, S_RANGE.first, S_RANGE.second) );
        intADC_M9_S.emplace_back( GetInt(*wave_M9_T1_S, S_RANGE.first, S_RANGE.second) ); intADC_M9_S.emplace_back( GetInt(*wave_M9_T2_S, S_RANGE.first, S_RANGE.second) ); intADC_M9_S.emplace_back( GetInt(*wave_M9_T3_S, S_RANGE.first, S_RANGE.second) ); intADC_M9_S.emplace_back( GetInt(*wave_M9_T4_S, S_RANGE.first, S_RANGE.second) );

        // Aux detector, PeakADC - using std::pair ranges
        float signal_CC1 = GetPeak(*wave_CC1, CC1_PEAK_RANGE.first, CC1_PEAK_RANGE.second);
        float signal_CC2 = GetPeak(*wave_CC2, CC2_PEAK_RANGE.first, CC2_PEAK_RANGE.second);
        float signal_PS = GetPeak(*wave_PS, PS_RANGE.first, PS_RANGE.second);
        float signal_MC = GetPeak(*wave_MC, MC_RANGE.first, MC_RANGE.second);
        float signal_TC = GetPeak(*wave_TC, TC_RANGE.first, TC_RANGE.second);

        float T2_timing = getLeadingEdgeTime_interpolated200(*wave_T2, 0.4, 450, 700);
        float M5T2_timing = getLeadingEdgeTime_interpolated200(*wave_M5_T2_S, 0.35, 250, 500);

        float timing_difference = T2_timing - M5T2_timing;

        float depth_S = (1/eff_photon_speed) * (timing_difference - time_ele_showermax) + depth_ele_showermax;
        
        std::vector<float> DWC1_time;
        DWC1_time.emplace_back(getLeadingEdgeTime_interpolated800(*wave_DWC1_R, 0.4, 1, 1000)); // Get 40% leading Edge Time for DWC 1
        DWC1_time.emplace_back(getLeadingEdgeTime_interpolated800(*wave_DWC1_L, 0.4, 1, 1000)); // Get 40% leading Edge Time for DWC 1
        DWC1_time.emplace_back(getLeadingEdgeTime_interpolated800(*wave_DWC1_U, 0.4, 1, 1000)); // Get 40% leading Edge Time for DWC 1
        DWC1_time.emplace_back(getLeadingEdgeTime_interpolated800(*wave_DWC1_D, 0.4, 1, 1000)); // Get 40% leading Edge Time for DWC 1
        
        std::vector<float> DWC2_time;
        DWC2_time.emplace_back(getLeadingEdgeTime_interpolated800(*wave_DWC2_R, 0.4, 1, 1000)); // Get 40% leading Edge Time for DWC 2
        DWC2_time.emplace_back(getLeadingEdgeTime_interpolated800(*wave_DWC2_L, 0.4, 1, 1000)); // Get 40% leading Edge Time for DWC 2
        DWC2_time.emplace_back(getLeadingEdgeTime_interpolated800(*wave_DWC2_U, 0.4, 1, 1000)); // Get 40% leading Edge Time for DWC 2
        DWC2_time.emplace_back(getLeadingEdgeTime_interpolated800(*wave_DWC2_D, 0.4, 1, 1000)); // Get 40% leading Edge Time for DWC 2
        
        std::vector<float> DWC1_corrected_pos = getDWC1position(DWC1_time, DWC1_offset); // DWC1 X, Y
        std::vector<float> DWC2_corrected_pos = getDWC2position(DWC2_time, DWC2_offset); // DWC2 X, Y
                
        float intADC_LC2  = GetInt(*wave_LC2 , LC_RANGE.first, LC_RANGE.second); float intADC_LC4  = GetInt(*wave_LC4 , LC_RANGE.first, LC_RANGE.second); float intADC_LC8  = GetInt(*wave_LC8 , LC_RANGE.first, LC_RANGE.second); float intADC_LC10 = GetInt(*wave_LC10, LC_RANGE.first, LC_RANGE.second);
        float intADC_LC3  = GetInt(*wave_LC3 , LC_RANGE.first, LC_RANGE.second); float intADC_LC5  = GetInt(*wave_LC5 , LC_RANGE.first, LC_RANGE.second); float intADC_LC7  = GetInt(*wave_LC7 , LC_RANGE.first, LC_RANGE.second); float intADC_LC9  = GetInt(*wave_LC9 , LC_RANGE.first, LC_RANGE.second);
        float intADC_LC11 = GetInt(*wave_LC11, LC_RANGE.first, LC_RANGE.second); float intADC_LC12 = GetInt(*wave_LC12, LC_RANGE.first, LC_RANGE.second); float intADC_LC13 = GetInt(*wave_LC13, LC_RANGE.first, LC_RANGE.second); float intADC_LC19 = GetInt(*wave_LC19, LC_RANGE.first, LC_RANGE.second);
        float intADC_LC14 = GetInt(*wave_LC14, LC_RANGE.first, LC_RANGE.second); float intADC_LC15 = GetInt(*wave_LC15, LC_RANGE.first, LC_RANGE.second); float intADC_LC16 = GetInt(*wave_LC16, LC_RANGE.first, LC_RANGE.second); float intADC_LC20 = GetInt(*wave_LC20, LC_RANGE.first, LC_RANGE.second);

        float Edep_LC2  = (intADC_LC2  * MAP_LC_CALIB_CONST["LC2" ] * LC_scalefactor); //if(Edep_LC2 < 0)  Edep_LC2 = 0;
        float Edep_LC3  = (intADC_LC3  * MAP_LC_CALIB_CONST["LC3" ] * LC_scalefactor); //if(Edep_LC3 < 0)  Edep_LC3 = 0;
        float Edep_LC4  = (intADC_LC4  * MAP_LC_CALIB_CONST["LC4" ] * LC_scalefactor); //if(Edep_LC4 < 0)  Edep_LC4 = 0;
        float Edep_LC5  = (intADC_LC5  * MAP_LC_CALIB_CONST["LC5" ] * LC_scalefactor); //if(Edep_LC5 < 0)  Edep_LC5 = 0;
        float Edep_LC7  = (intADC_LC7  * MAP_LC_CALIB_CONST["LC7" ] * LC_scalefactor); //if(Edep_LC7 < 0)  Edep_LC7 = 0;
        float Edep_LC8  = (intADC_LC8  * MAP_LC_CALIB_CONST["LC8" ] * LC_scalefactor); //if(Edep_LC8 < 0)  Edep_LC8 = 0;
        float Edep_LC9  = (intADC_LC9  * MAP_LC_CALIB_CONST["LC9" ] * LC_scalefactor); //if(Edep_LC9 < 0)  Edep_LC9 = 0;
        float Edep_LC10 = (intADC_LC10 * MAP_LC_CALIB_CONST["LC10"] * LC_scalefactor); //if(Edep_LC10 < 0) Edep_LC10 = 0;
        float Edep_LC11 = (intADC_LC11 * MAP_LC_CALIB_CONST["LC11"] * LC_scalefactor); //if(Edep_LC11 < 0) Edep_LC11 = 0;
        float Edep_LC12 = (intADC_LC12 * MAP_LC_CALIB_CONST["LC12"] * LC_scalefactor); //if(Edep_LC12 < 0) Edep_LC12 = 0;
        float Edep_LC13 = (intADC_LC13 * MAP_LC_CALIB_CONST["LC13"] * LC_scalefactor); //if(Edep_LC13 < 0) Edep_LC13 = 0;
        float Edep_LC14 = (intADC_LC14 * MAP_LC_CALIB_CONST["LC14"] * LC_scalefactor); //if(Edep_LC14 < 0) Edep_LC14 = 0;
        float Edep_LC15 = (intADC_LC15 * MAP_LC_CALIB_CONST["LC15"] * LC_scalefactor); //if(Edep_LC15 < 0) Edep_LC15 = 0;
        float Edep_LC16 = (intADC_LC16 * MAP_LC_CALIB_CONST["LC16"] * LC_scalefactor); //if(Edep_LC16 < 0) Edep_LC16 = 0;
        float Edep_LC19 = (intADC_LC19 * MAP_LC_CALIB_CONST["LC19"] * LC_scalefactor); //if(Edep_LC19 < 0) Edep_LC19 = 0;
        float Edep_LC20 = (intADC_LC20 * MAP_LC_CALIB_CONST["LC20"] * LC_scalefactor); //if(Edep_LC20 < 0) Edep_LC20 = 0;
        float Edep_LC_Jura   = Edep_LC3 + Edep_LC5 + Edep_LC7 + Edep_LC9;
        float Edep_LC_Saleve = Edep_LC2 + Edep_LC4 + Edep_LC8 + Edep_LC10;
        float Edep_LC_Top    = Edep_LC11 + Edep_LC12 + Edep_LC13 + Edep_LC19;
        float Edep_LC_Bottom = Edep_LC14 + Edep_LC15 + Edep_LC16 + Edep_LC20;
        float Edep_LC        = Edep_LC_Jura + Edep_LC_Saleve + Edep_LC_Top + Edep_LC_Bottom;

        // Module - C Channel
        float Edep_M1T1_C = (intADC_M1_C.at(0) * MAP_CALIB_CONST["M1T1_C"]); float Edep_M1T2_C = (intADC_M1_C.at(1) * MAP_CALIB_CONST["M1T2_C"]); float Edep_M1T3_C = (intADC_M1_C.at(2) * MAP_CALIB_CONST["M1T3_C"]); float Edep_M1T4_C = (intADC_M1_C.at(3) * MAP_CALIB_CONST["M1T4_C"]);
        //if(Edep_M1T1_C < 0) Edep_M1T1_C = 0; if(Edep_M1T2_C < 0) Edep_M1T2_C = 0; if(Edep_M1T3_C < 0) Edep_M1T3_C = 0; if(Edep_M1T4_C < 0) Edep_M1T4_C = 0;
        float Edep_M2T1_C = (intADC_M2_C.at(0) * MAP_CALIB_CONST["M2T1_C"]); float Edep_M2T2_C = (intADC_M2_C.at(1) * MAP_CALIB_CONST["M2T2_C"]); float Edep_M2T3_C = (intADC_M2_C.at(2) * MAP_CALIB_CONST["M2T3_C"]); float Edep_M2T4_C = (intADC_M2_C.at(3) * MAP_CALIB_CONST["M2T4_C"]);
        //if(Edep_M2T1_C < 0) Edep_M2T1_C = 0; if(Edep_M2T2_C < 0) Edep_M2T2_C = 0; if(Edep_M2T3_C < 0) Edep_M2T3_C = 0; if(Edep_M2T4_C < 0) Edep_M2T4_C = 0;
        float Edep_M3T1_C = (intADC_M3_C.at(0) * MAP_CALIB_CONST["M3T1_C"]); float Edep_M3T2_C = (intADC_M3_C.at(1) * MAP_CALIB_CONST["M3T2_C"]); float Edep_M3T3_C = (intADC_M3_C.at(2) * MAP_CALIB_CONST["M3T3_C"]); float Edep_M3T4_C = (intADC_M3_C.at(3) * MAP_CALIB_CONST["M3T4_C"]);
        //if(Edep_M3T1_C < 0) Edep_M3T1_C = 0; if(Edep_M3T2_C < 0) Edep_M3T2_C = 0; if(Edep_M3T3_C < 0) Edep_M3T3_C = 0; if(Edep_M3T4_C < 0) Edep_M3T4_C = 0;
        float Edep_M4T1_C = (intADC_M4_C.at(0) * MAP_CALIB_CONST["M4T1_C"]); float Edep_M4T2_C = (intADC_M4_C.at(1) * MAP_CALIB_CONST["M4T2_C"]); float Edep_M4T3_C = (intADC_M4_C.at(2) * MAP_CALIB_CONST["M4T3_C"]); float Edep_M4T4_C = (intADC_M4_C.at(3) * MAP_CALIB_CONST["M4T4_C"]);
        //if(Edep_M4T1_C < 0) Edep_M4T1_C = 0; if(Edep_M4T2_C < 0) Edep_M4T2_C = 0; if(Edep_M4T3_C < 0) Edep_M4T3_C = 0; if(Edep_M4T4_C < 0) Edep_M4T4_C = 0;
        float Edep_M5T1_C = (intADC_M5_C.at(0) * MAP_CALIB_CONST["M5T1_C"]); float Edep_M5T2_C = (intADC_M5_C.at(1) * MAP_CALIB_CONST["M5T2_C"]); float Edep_M5T3_C = (intADC_M5_C.at(2) * MAP_CALIB_CONST["M5T3_C"]); float Edep_M5T4_C = (intADC_M5_C.at(3) * MAP_CALIB_CONST["M5T4_C"]);
        //if(Edep_M5T1_C < 0) Edep_M5T1_C = 0; if(Edep_M5T2_C < 0) Edep_M5T2_C = 0; if(Edep_M5T3_C < 0) Edep_M5T3_C = 0; if(Edep_M5T4_C < 0) Edep_M5T4_C = 0;
        float Edep_M6T1_C = (intADC_M6_C.at(0) * MAP_CALIB_CONST["M6T1_C"]); float Edep_M6T2_C = (intADC_M6_C.at(1) * MAP_CALIB_CONST["M6T2_C"]); float Edep_M6T3_C = (intADC_M6_C.at(2) * MAP_CALIB_CONST["M6T3_C"]); float Edep_M6T4_C = (intADC_M6_C.at(3) * MAP_CALIB_CONST["M6T4_C"]);
        //if(Edep_M6T1_C < 0) Edep_M6T1_C = 0; if(Edep_M6T2_C < 0) Edep_M6T2_C = 0; if(Edep_M6T3_C < 0) Edep_M6T3_C = 0; if(Edep_M6T4_C < 0) Edep_M6T4_C = 0;
        float Edep_M7T1_C = (intADC_M7_C.at(0) * MAP_CALIB_CONST["M7T1_C"]); float Edep_M7T2_C = (intADC_M7_C.at(1) * MAP_CALIB_CONST["M7T2_C"]); float Edep_M7T3_C = (intADC_M7_C.at(2) * MAP_CALIB_CONST["M7T3_C"]); float Edep_M7T4_C = (intADC_M7_C.at(3) * MAP_CALIB_CONST["M7T4_C"]);
        //if(Edep_M7T1_C < 0) Edep_M7T1_C = 0; if(Edep_M7T2_C < 0) Edep_M7T2_C = 0; if(Edep_M7T3_C < 0) Edep_M7T3_C = 0; if(Edep_M7T4_C < 0) Edep_M7T4_C = 0;
        float Edep_M8T1_C = (intADC_M8_C.at(0) * MAP_CALIB_CONST["M8T1_C"]); float Edep_M8T2_C = (intADC_M8_C.at(1) * MAP_CALIB_CONST["M8T2_C"]); float Edep_M8T3_C = (intADC_M8_C.at(2) * MAP_CALIB_CONST["M8T3_C"]); float Edep_M8T4_C = (intADC_M8_C.at(3) * MAP_CALIB_CONST["M8T4_C"]);
        //if(Edep_M8T1_C < 0) Edep_M8T1_C = 0; if(Edep_M8T2_C < 0) Edep_M8T2_C = 0; if(Edep_M8T3_C < 0) Edep_M8T3_C = 0; if(Edep_M8T4_C < 0) Edep_M8T4_C = 0;
        float Edep_M9T1_C = (intADC_M9_C.at(0) * MAP_CALIB_CONST["M9T1_C"]); float Edep_M9T2_C = (intADC_M9_C.at(1) * MAP_CALIB_CONST["M9T2_C"]); float Edep_M9T3_C = (intADC_M9_C.at(2) * MAP_CALIB_CONST["M9T3_C"]); float Edep_M9T4_C = (intADC_M9_C.at(3) * MAP_CALIB_CONST["M9T4_C"]);
        //if(Edep_M9T1_C < 0) Edep_M9T1_C = 0; if(Edep_M9T2_C < 0) Edep_M9T2_C = 0; if(Edep_M9T3_C < 0) Edep_M9T3_C = 0; if(Edep_M9T4_C < 0) Edep_M9T4_C = 0;
        
        float Edep_M1_C = Edep_M1T1_C + Edep_M1T2_C + Edep_M1T3_C + Edep_M1T4_C;
        float Edep_M2_C = Edep_M2T1_C + Edep_M2T2_C + Edep_M2T3_C + Edep_M2T4_C;
        float Edep_M3_C = Edep_M3T1_C + Edep_M3T2_C + Edep_M3T3_C + Edep_M3T4_C;
        float Edep_M4_C = Edep_M4T1_C + Edep_M4T2_C + Edep_M4T3_C + Edep_M4T4_C;
        float Edep_M5_C = Edep_M5T1_C + Edep_M5T2_C + Edep_M5T3_C + Edep_M5T4_C;
        float Edep_M6_C = Edep_M6T1_C + Edep_M6T2_C + Edep_M6T3_C + Edep_M6T4_C;
        float Edep_M7_C = Edep_M7T1_C + Edep_M7T2_C + Edep_M7T3_C + Edep_M7T4_C;
        float Edep_M8_C = Edep_M8T1_C + Edep_M8T2_C + Edep_M8T3_C + Edep_M8T4_C;
        float Edep_M9_C = Edep_M9T1_C + Edep_M9T2_C + Edep_M9T3_C + Edep_M9T4_C;
        
        float Total_Edep_C = Edep_M1_C + Edep_M2_C + Edep_M3_C + 
                            Edep_M4_C + Edep_M5_C + Edep_M6_C + 
                            Edep_M7_C + Edep_M8_C + Edep_M9_C;
        
        // Module - S channel
        float Edep_M1T1_S = (intADC_M1_S.at(0) * MAP_CALIB_CONST["M1T1_S"]); float Edep_M1T2_S = (intADC_M1_S.at(1) * MAP_CALIB_CONST["M1T2_S"]); float Edep_M1T3_S = (intADC_M1_S.at(2) * MAP_CALIB_CONST["M1T3_S"]); float Edep_M1T4_S = (intADC_M1_S.at(3) * MAP_CALIB_CONST["M1T4_S"]);
        //if(Edep_M1T1_S < 0) Edep_M1T1_S = 0; if(Edep_M1T2_S < 0) Edep_M1T2_S = 0; if(Edep_M1T3_S < 0) Edep_M1T3_S = 0; if(Edep_M1T4_S < 0) Edep_M1T4_S = 0;
        float Edep_M2T1_S = (intADC_M2_S.at(0) * MAP_CALIB_CONST["M2T1_S"]); float Edep_M2T2_S = (intADC_M2_S.at(1) * MAP_CALIB_CONST["M2T2_S"]); float Edep_M2T3_S = (intADC_M2_S.at(2) * MAP_CALIB_CONST["M2T3_S"]); float Edep_M2T4_S = (intADC_M2_S.at(3) * MAP_CALIB_CONST["M2T4_S"]);
        //if(Edep_M2T1_S < 0) Edep_M2T1_S = 0; if(Edep_M2T2_S < 0) Edep_M2T2_S = 0; if(Edep_M2T3_S < 0) Edep_M2T3_S = 0; if(Edep_M2T4_S < 0) Edep_M2T4_S = 0;
        float Edep_M3T1_S = (intADC_M3_S.at(0) * MAP_CALIB_CONST["M3T1_S"]); float Edep_M3T2_S = (intADC_M3_S.at(1) * MAP_CALIB_CONST["M3T2_S"]); float Edep_M3T3_S = (intADC_M3_S.at(2) * MAP_CALIB_CONST["M3T3_S"]); float Edep_M3T4_S = (intADC_M3_S.at(3) * MAP_CALIB_CONST["M3T4_S"]);
        //if(Edep_M3T1_S < 0) Edep_M3T1_S = 0; if(Edep_M3T2_S < 0) Edep_M3T2_S = 0; if(Edep_M3T3_S < 0) Edep_M3T3_S = 0; if(Edep_M3T4_S < 0) Edep_M3T4_S = 0;
        float Edep_M4T1_S = (intADC_M4_S.at(0) * MAP_CALIB_CONST["M4T1_S"]); float Edep_M4T2_S = (intADC_M4_S.at(1) * MAP_CALIB_CONST["M4T2_S"]); float Edep_M4T3_S = (intADC_M4_S.at(2) * MAP_CALIB_CONST["M4T3_S"]); float Edep_M4T4_S = (intADC_M4_S.at(3) * MAP_CALIB_CONST["M4T4_S"]);
        //if(Edep_M4T1_S < 0) Edep_M4T1_S = 0; if(Edep_M4T2_S < 0) Edep_M4T2_S = 0; if(Edep_M4T3_S < 0) Edep_M4T3_S = 0; if(Edep_M4T4_S < 0) Edep_M4T4_S = 0;
        float Edep_M5T1_S = (intADC_M5_S.at(0) * MAP_CALIB_CONST["M5T1_S"]); float Edep_M5T2_S = (intADC_M5_S.at(1) * MAP_CALIB_CONST["M5T2_S"]); float Edep_M5T3_S = (intADC_M5_S.at(2) * MAP_CALIB_CONST["M5T3_S"]); float Edep_M5T4_S = (intADC_M5_S.at(3) * MAP_CALIB_CONST["M5T4_S"]);
        //if(Edep_M5T1_S < 0) Edep_M5T1_S = 0; if(Edep_M5T2_S < 0) Edep_M5T2_S = 0; if(Edep_M5T3_S < 0) Edep_M5T3_S = 0; if(Edep_M5T4_S < 0) Edep_M5T4_S = 0;
        float Edep_M6T1_S = (intADC_M6_S.at(0) * MAP_CALIB_CONST["M6T1_S"]); float Edep_M6T2_S = (intADC_M6_S.at(1) * MAP_CALIB_CONST["M6T2_S"]); float Edep_M6T3_S = (intADC_M6_S.at(2) * MAP_CALIB_CONST["M6T3_S"]); float Edep_M6T4_S = (intADC_M6_S.at(3) * MAP_CALIB_CONST["M6T4_S"]);
        //if(Edep_M6T1_S < 0) Edep_M6T1_S = 0; if(Edep_M6T2_S < 0) Edep_M6T2_S = 0; if(Edep_M6T3_S < 0) Edep_M6T3_S = 0; if(Edep_M6T4_S < 0) Edep_M6T4_S = 0;
        float Edep_M7T1_S = (intADC_M7_S.at(0) * MAP_CALIB_CONST["M7T1_S"]); float Edep_M7T2_S = (intADC_M7_S.at(1) * MAP_CALIB_CONST["M7T2_S"]); float Edep_M7T3_S = (intADC_M7_S.at(2) * MAP_CALIB_CONST["M7T3_S"]); float Edep_M7T4_S = (intADC_M7_S.at(3) * MAP_CALIB_CONST["M7T4_S"]);
        //if(Edep_M7T1_S < 0) Edep_M7T1_S = 0; if(Edep_M7T2_S < 0) Edep_M7T2_S = 0; if(Edep_M7T3_S < 0) Edep_M7T3_S = 0; if(Edep_M7T4_S < 0) Edep_M7T4_S = 0;
        float Edep_M8T1_S = (intADC_M8_S.at(0) * MAP_CALIB_CONST["M8T1_S"]); float Edep_M8T2_S = (intADC_M8_S.at(1) * MAP_CALIB_CONST["M8T2_S"]); float Edep_M8T3_S = (intADC_M8_S.at(2) * MAP_CALIB_CONST["M8T3_S"]); float Edep_M8T4_S = (intADC_M8_S.at(3) * MAP_CALIB_CONST["M8T4_S"]);
        //if(Edep_M8T1_S < 0) Edep_M8T1_S = 0; if(Edep_M8T2_S < 0) Edep_M8T2_S = 0; if(Edep_M8T3_S < 0) Edep_M8T3_S = 0; if(Edep_M8T4_S < 0) Edep_M8T4_S = 0;
        float Edep_M9T1_S = (intADC_M9_S.at(0) * MAP_CALIB_CONST["M9T1_S"]); float Edep_M9T2_S = (intADC_M9_S.at(1) * MAP_CALIB_CONST["M9T2_S"]); float Edep_M9T3_S = (intADC_M9_S.at(2) * MAP_CALIB_CONST["M9T3_S"]); float Edep_M9T4_S = (intADC_M9_S.at(3) * MAP_CALIB_CONST["M9T4_S"]);
        //if(Edep_M9T1_S < 0) Edep_M9T1_S = 0; if(Edep_M9T2_S < 0) Edep_M9T2_S = 0; if(Edep_M9T3_S < 0) Edep_M9T3_S = 0; if(Edep_M9T4_S < 0) Edep_M9T4_S = 0;
        
        float Edep_M1_S = Edep_M1T1_S + Edep_M1T2_S + Edep_M1T3_S + Edep_M1T4_S;
        float Edep_M2_S = Edep_M2T1_S + Edep_M2T2_S + Edep_M2T3_S + Edep_M2T4_S;
        float Edep_M3_S = Edep_M3T1_S + Edep_M3T2_S + Edep_M3T3_S + Edep_M3T4_S;
        float Edep_M4_S = Edep_M4T1_S + Edep_M4T2_S + Edep_M4T3_S + Edep_M4T4_S;
        float Edep_M5_S = Edep_M5T1_S + Edep_M5T2_S + Edep_M5T3_S + Edep_M5T4_S;
        float Edep_M6_S = Edep_M6T1_S + Edep_M6T2_S + Edep_M6T3_S + Edep_M6T4_S;
        float Edep_M7_S = Edep_M7T1_S + Edep_M7T2_S + Edep_M7T3_S + Edep_M7T4_S;
        float Edep_M8_S = Edep_M8T1_S + Edep_M8T2_S + Edep_M8T3_S + Edep_M8T4_S;
        float Edep_M9_S = Edep_M9T1_S + Edep_M9T2_S + Edep_M9T3_S + Edep_M9T4_S;
        
        float Total_Edep_S = Edep_M1_S + Edep_M2_S + Edep_M3_S + 
                            Edep_M4_S + Edep_M5_S + Edep_M6_S + 
                            Edep_M7_S + Edep_M8_S + Edep_M9_S;
        // Channel Combine
        float Total_Edep_Comb = Total_Edep_C + Total_Edep_S;
        /////////////////////////////////
        // After applying scale factor //
        /////////////////////////////////
        float Total_Edep_SF_C = 0;
        float Total_Edep_SF_S = 0;
        if(fModule == 5)
        {
            Total_Edep_SF_C = Total_Edep_C * MAP_SCALE_FACTOR["M5T2_C"];
            Total_Edep_SF_S = Total_Edep_S * MAP_SCALE_FACTOR["M5T2_S"];
        }
        else if(fModule == 4)
        {
            Total_Edep_SF_C = Total_Edep_C * MAP_SCALE_FACTOR["M4T2_C"];
            Total_Edep_SF_S = Total_Edep_S * MAP_SCALE_FACTOR["M4T2_S"];
        }
        else {std::cout << "Module " << fModule << " is not supported" << std::endl; exit(1);}
        // Add LC correction
        float Total_Edep_SF_S_LCcor = Total_Edep_SF_S + Edep_LC;
        // Add attenuation correction
        float Total_Edep_SF_S_ATTcor = Total_Edep_SF_S * std::exp((depth_ele_showermax - depth_S)/attenlen_S);
        // Add attenuation correction + Add LC correction
        float Total_Edep_SF_S_LCATTcor = Total_Edep_SF_S_ATTcor + Edep_LC;
        // combine channel
        float Total_Edep_SF_Comb = Total_Edep_SF_C + Total_Edep_SF_S;
        // Apply DR correction
        float Total_Edep_SF_DRcor = DRCorrection(Total_Edep_SF_S, Total_Edep_SF_C);
        // Apply DR correction + Add LC correction
        float Total_Edep_SF_DRcor_LCcor = DRCorrection(Total_Edep_SF_S_LCcor, Total_Edep_SF_C);
        // Apply DR correction + Add attenuation correction
        float Total_Edep_SF_DRcor_ATTcor = DRCorrection(Total_Edep_SF_S_ATTcor, Total_Edep_SF_C);
        // Apply DR correction + Add attenuation correction + Add LC correction
        float Total_Edep_SF_DRcor_LCATTcor = DRCorrection(Total_Edep_SF_S_LCATTcor, Total_Edep_SF_C);
        
        // Prepare data for histograms
        std::vector<float> M1_intADC_C = {intADC_M1_C.at(0), intADC_M1_C.at(1), intADC_M1_C.at(2), intADC_M1_C.at(3)};
        std::vector<float> M1_intADC_S = {intADC_M1_S.at(0), intADC_M1_S.at(1), intADC_M1_S.at(2), intADC_M1_S.at(3)};
        std::vector<float> M1_Edep_C = {Edep_M1T1_C, Edep_M1T2_C, Edep_M1T3_C, Edep_M1T4_C};
        std::vector<float> M1_Edep_S = {Edep_M1T1_S, Edep_M1T2_S, Edep_M1T3_S, Edep_M1T4_S};
        
        std::vector<float> M2_intADC_C = {intADC_M2_C.at(0), intADC_M2_C.at(1), intADC_M2_C.at(2), intADC_M2_C.at(3)};
        std::vector<float> M2_intADC_S = {intADC_M2_S.at(0), intADC_M2_S.at(1), intADC_M2_S.at(2), intADC_M2_S.at(3)};
        std::vector<float> M2_Edep_C = {Edep_M2T1_C, Edep_M2T2_C, Edep_M2T3_C, Edep_M2T4_C};
        std::vector<float> M2_Edep_S = {Edep_M2T1_S, Edep_M2T2_S, Edep_M2T3_S, Edep_M2T4_S};
        
        std::vector<float> M3_intADC_C = {intADC_M3_C.at(0), intADC_M3_C.at(1), intADC_M3_C.at(2), intADC_M3_C.at(3)};
        std::vector<float> M3_intADC_S = {intADC_M3_S.at(0), intADC_M3_S.at(1), intADC_M3_S.at(2), intADC_M3_S.at(3)};
        std::vector<float> M3_Edep_C = {Edep_M3T1_C, Edep_M3T2_C, Edep_M3T3_C, Edep_M3T4_C};
        std::vector<float> M3_Edep_S = {Edep_M3T1_S, Edep_M3T2_S, Edep_M3T3_S, Edep_M3T4_S};
        
        std::vector<float> M4_intADC_C = {intADC_M4_C.at(0), intADC_M4_C.at(1), intADC_M4_C.at(2), intADC_M4_C.at(3)};
        std::vector<float> M4_intADC_S = {intADC_M4_S.at(0), intADC_M4_S.at(1), intADC_M4_S.at(2), intADC_M4_S.at(3)};
        std::vector<float> M4_Edep_C = {Edep_M4T1_C, Edep_M4T2_C, Edep_M4T3_C, Edep_M4T4_C};
        std::vector<float> M4_Edep_S = {Edep_M4T1_S, Edep_M4T2_S, Edep_M4T3_S, Edep_M4T4_S};
        
        std::vector<float> M5_intADC_C = {intADC_M5_C.at(0), intADC_M5_C.at(1), intADC_M5_C.at(2), intADC_M5_C.at(3)};
        std::vector<float> M5_intADC_S = {intADC_M5_S.at(0), intADC_M5_S.at(1), intADC_M5_S.at(2), intADC_M5_S.at(3)};
        std::vector<float> M5_Edep_C = {Edep_M5T1_C, Edep_M5T2_C, Edep_M5T3_C, Edep_M5T4_C};
        std::vector<float> M5_Edep_S = {Edep_M5T1_S, Edep_M5T2_S, Edep_M5T3_S, Edep_M5T4_S};
        
        std::vector<float> M6_intADC_C = {intADC_M6_C.at(0), intADC_M6_C.at(1), intADC_M6_C.at(2), intADC_M6_C.at(3)};
        std::vector<float> M6_intADC_S = {intADC_M6_S.at(0), intADC_M6_S.at(1), intADC_M6_S.at(2), intADC_M6_S.at(3)};
        std::vector<float> M6_Edep_C = {Edep_M6T1_C, Edep_M6T2_C, Edep_M6T3_C, Edep_M6T4_C};
        std::vector<float> M6_Edep_S = {Edep_M6T1_S, Edep_M6T2_S, Edep_M6T3_S, Edep_M6T4_S};
        
        std::vector<float> M7_intADC_C = {intADC_M7_C.at(0), intADC_M7_C.at(1), intADC_M7_C.at(2), intADC_M7_C.at(3)};
        std::vector<float> M7_intADC_S = {intADC_M7_S.at(0), intADC_M7_S.at(1), intADC_M7_S.at(2), intADC_M7_S.at(3)};
        std::vector<float> M7_Edep_C = {Edep_M7T1_C, Edep_M7T2_C, Edep_M7T3_C, Edep_M7T4_C};
        std::vector<float> M7_Edep_S = {Edep_M7T1_S, Edep_M7T2_S, Edep_M7T3_S, Edep_M7T4_S};
        
        std::vector<float> M8_intADC_C = {intADC_M8_C.at(0), intADC_M8_C.at(1), intADC_M8_C.at(2), intADC_M8_C.at(3)};
        std::vector<float> M8_intADC_S = {intADC_M8_S.at(0), intADC_M8_S.at(1), intADC_M8_S.at(2), intADC_M8_S.at(3)};
        std::vector<float> M8_Edep_C = {Edep_M8T1_C, Edep_M8T2_C, Edep_M8T3_C, Edep_M8T4_C};
        std::vector<float> M8_Edep_S = {Edep_M8T1_S, Edep_M8T2_S, Edep_M8T3_S, Edep_M8T4_S};
        
        std::vector<float> M9_intADC_C = {intADC_M9_C.at(0), intADC_M9_C.at(1), intADC_M9_C.at(2), intADC_M9_C.at(3)};
        std::vector<float> M9_intADC_S = {intADC_M9_S.at(0), intADC_M9_S.at(1), intADC_M9_S.at(2), intADC_M9_S.at(3)};
        std::vector<float> M9_Edep_C = {Edep_M9T1_C, Edep_M9T2_C, Edep_M9T3_C, Edep_M9T4_C};
        std::vector<float> M9_Edep_S = {Edep_M9T1_S, Edep_M9T2_S, Edep_M9T3_S, Edep_M9T4_S};
        
        // Prepare data for fillAllStepHistograms function
        std::vector<std::vector<float>> All_intADC_C = {M1_intADC_C, M2_intADC_C, M3_intADC_C, M4_intADC_C, M5_intADC_C, M6_intADC_C, M7_intADC_C, M8_intADC_C, M9_intADC_C};
        std::vector<std::vector<float>> All_intADC_S = {M1_intADC_S, M2_intADC_S, M3_intADC_S, M4_intADC_S, M5_intADC_S, M6_intADC_S, M7_intADC_S, M8_intADC_S, M9_intADC_S};
        std::vector<std::vector<float>> All_Edep_C = {M1_Edep_C, M2_Edep_C, M3_Edep_C, M4_Edep_C, M5_Edep_C, M6_Edep_C, M7_Edep_C, M8_Edep_C, M9_Edep_C};
        std::vector<std::vector<float>> All_Edep_S = {M1_Edep_S, M2_Edep_S, M3_Edep_S, M4_Edep_S, M5_Edep_S, M6_Edep_S, M7_Edep_S, M8_Edep_S, M9_Edep_S};
        std::vector<float> All_Module_Edep_C = {Edep_M1_C, Edep_M2_C, Edep_M3_C, Edep_M4_C, Edep_M5_C, Edep_M6_C, Edep_M7_C, Edep_M8_C, Edep_M9_C};
        std::vector<float> All_Module_Edep_S = {Edep_M1_S, Edep_M2_S, Edep_M3_S, Edep_M4_S, Edep_M5_S, Edep_M6_S, Edep_M7_S, Edep_M8_S, Edep_M9_S};

        std::vector<float> DWC1_pos = {DWC1_corrected_pos.at(0), DWC1_corrected_pos.at(1)};
        std::vector<float> DWC2_pos = {DWC2_corrected_pos.at(0), DWC2_corrected_pos.at(1)};
        //////////////////////////////////////////////////////////////////////////////////////
        //                              Event Selection                                     //
        //////////////////////////////////////////////////////////////////////////////////////
        bool passDwcCorrCut = (dwcCorrelationCut(DWC1_corrected_pos, DWC2_corrected_pos, CUT_DWC));
        bool passDwcBeamSpotCut = ( (std::abs(DWC1_corrected_pos.at(0)) < 5.) && (std::abs(DWC1_corrected_pos.at(1)) < 5.) && (std::abs(DWC2_corrected_pos.at(0)) < 5.) && (std::abs(DWC2_corrected_pos.at(1)) < 5.));
        
        // std::pair 범위 컷 적용 - 헬퍼 함수 사용 (더 깔끔!)
        bool passCC1Cut = (signal_CC1 > CUT_CC1.first) && (signal_CC1 < CUT_CC1.second);
        bool passCC2Cut = (signal_CC2 > CUT_CC2.first) && (signal_CC2 < CUT_CC2.second);
        bool passPSCut  = (signal_PS > CUT_PS.first) && (signal_PS < CUT_PS.second);
        bool passMCCut  = (signal_MC > CUT_MC.first) && (signal_MC < CUT_MC.second);
        
        
        nEvent_count_0++;        
        if(passDwcCorrCut) nEvent_count_1++;
        if(passDwcCorrCut && passDwcBeamSpotCut ) nEvent_count_2++;
        if(passDwcCorrCut && passDwcBeamSpotCut && passCC1Cut) nEvent_count_3++;
        if(passDwcCorrCut && passDwcBeamSpotCut && passCC1Cut && passCC2Cut) nEvent_count_4++;
        if(passDwcCorrCut && passDwcBeamSpotCut && passCC1Cut && passCC2Cut && passPSCut) nEvent_count_5++;
        if(passDwcCorrCut && passDwcBeamSpotCut && passCC1Cut && passCC2Cut && passPSCut && passMCCut) nEvent_count_6++;

        // Step 0: No cuts (all events)
        histManager.fillAllStepHistograms(
            All_intADC_C, All_intADC_S, // S,C channel intADC
            All_Edep_C, All_Edep_S, // S, C channel Edep
            All_Module_Edep_C, All_Module_Edep_S, // M1-9 module Edep for C, S channel
            Total_Edep_C, Total_Edep_S, Total_Edep_Comb, // Total Edep for C, S channel, S channel + LC correction, C + S channel
            // Total Edep for C, S channel, S+LC, S+LC+ATT, S+LC+ATT+LC
            Total_Edep_SF_C, Total_Edep_SF_S, Total_Edep_SF_S_LCcor, Total_Edep_SF_S_ATTcor, Total_Edep_SF_S_LCATTcor, 
            Total_Edep_SF_Comb, // Total Edep for C + S channel
            // apply DR correction, DRcorrection + LC correction, DRcorrection + ATT correction, DRcorrection + LC correction + ATT correction
            Total_Edep_SF_DRcor, Total_Edep_SF_DRcor_LCcor, Total_Edep_SF_DRcor_ATTcor, Total_Edep_SF_DRcor_LCATTcor,
            signal_CC1, signal_CC2, signal_PS, signal_MC, signal_TC, Edep_LC, DWC1_pos, DWC2_pos,
            "0_noCut"
        );
        // Step 1: DWC cut
        if(!(passDwcCorrCut)) continue;
        histManager.fillAllStepHistograms(
            All_intADC_C, All_intADC_S,
            All_Edep_C, All_Edep_S,
            All_Module_Edep_C, All_Module_Edep_S,
            Total_Edep_C, Total_Edep_S, Total_Edep_Comb,
            Total_Edep_SF_C, Total_Edep_SF_S, Total_Edep_SF_S_LCcor, Total_Edep_SF_S_ATTcor, Total_Edep_SF_S_LCATTcor, 
            Total_Edep_SF_Comb,
            Total_Edep_SF_DRcor, Total_Edep_SF_DRcor_LCcor, Total_Edep_SF_DRcor_ATTcor, Total_Edep_SF_DRcor_LCATTcor,
            signal_CC1, signal_CC2, signal_PS, signal_MC, signal_TC, Edep_LC, DWC1_pos, DWC2_pos,
            "1_DWC"
        );
        // Step 2: DWC size cut
        if(!passDwcBeamSpotCut) continue;
        histManager.fillAllStepHistograms(
            All_intADC_C, All_intADC_S,
            All_Edep_C, All_Edep_S,
            All_Module_Edep_C, All_Module_Edep_S,
            Total_Edep_C, Total_Edep_S, Total_Edep_Comb,
            Total_Edep_SF_C, Total_Edep_SF_S, Total_Edep_SF_S_LCcor, Total_Edep_SF_S_ATTcor, Total_Edep_SF_S_LCATTcor, 
            Total_Edep_SF_Comb,
            Total_Edep_SF_DRcor, Total_Edep_SF_DRcor_LCcor, Total_Edep_SF_DRcor_ATTcor, Total_Edep_SF_DRcor_LCATTcor,
            signal_CC1, signal_CC2, signal_PS, signal_MC, signal_TC, Edep_LC, DWC1_pos, DWC2_pos,
            "2_DWCsize"
        );
        
        // Step 3: DWC + CC1 cuts
        if(!passCC1Cut) continue;
        histManager.fillAllStepHistograms(
            All_intADC_C, All_intADC_S,
            All_Edep_C, All_Edep_S,
            All_Module_Edep_C, All_Module_Edep_S,
            Total_Edep_C, Total_Edep_S, Total_Edep_Comb,
            Total_Edep_SF_C, Total_Edep_SF_S, Total_Edep_SF_S_LCcor, Total_Edep_SF_S_ATTcor, Total_Edep_SF_S_LCATTcor, 
            Total_Edep_SF_Comb,
            Total_Edep_SF_DRcor, Total_Edep_SF_DRcor_LCcor, Total_Edep_SF_DRcor_ATTcor, Total_Edep_SF_DRcor_LCATTcor,
            signal_CC1, signal_CC2, signal_PS, signal_MC, signal_TC, Edep_LC, DWC1_pos, DWC2_pos,
            "3_CC1"
        );
                
        // Step 4: DWC + CC1 + CC2 cuts
        if(!passCC2Cut) continue;
        histManager.fillAllStepHistograms(
            All_intADC_C, All_intADC_S,
            All_Edep_C, All_Edep_S,
            All_Module_Edep_C, All_Module_Edep_S,
            Total_Edep_C, Total_Edep_S, Total_Edep_Comb,
            Total_Edep_SF_C, Total_Edep_SF_S, Total_Edep_SF_S_LCcor, Total_Edep_SF_S_ATTcor, Total_Edep_SF_S_LCATTcor, 
            Total_Edep_SF_Comb,
            Total_Edep_SF_DRcor, Total_Edep_SF_DRcor_LCcor, Total_Edep_SF_DRcor_ATTcor, Total_Edep_SF_DRcor_LCATTcor,
            signal_CC1, signal_CC2, signal_PS, signal_MC, signal_TC, Edep_LC, DWC1_pos, DWC2_pos,
            "4_CC2"
        );
                    
        // Step 5: DWC + CC1 + CC2 + PS cuts
        if(!passPSCut) continue;
        histManager.fillAllStepHistograms(
            All_intADC_C, All_intADC_S,
            All_Edep_C, All_Edep_S,
            All_Module_Edep_C, All_Module_Edep_S,
            Total_Edep_C, Total_Edep_S, Total_Edep_Comb,
            Total_Edep_SF_C, Total_Edep_SF_S, Total_Edep_SF_S_LCcor, Total_Edep_SF_S_ATTcor, Total_Edep_SF_S_LCATTcor, 
            Total_Edep_SF_Comb,
            Total_Edep_SF_DRcor, Total_Edep_SF_DRcor_LCcor, Total_Edep_SF_DRcor_ATTcor, Total_Edep_SF_DRcor_LCATTcor,
            signal_CC1, signal_CC2, signal_PS, signal_MC, signal_TC, Edep_LC, DWC1_pos, DWC2_pos,
            "5_PS"
        );
                        
        // Step 6: All cuts (DWC + CC1 + CC2 + PS + MC)
        if(!passMCCut) continue;
        histManager.fillAllStepHistograms(
            All_intADC_C, All_intADC_S,
            All_Edep_C, All_Edep_S,
            All_Module_Edep_C, All_Module_Edep_S,
            Total_Edep_C, Total_Edep_S, Total_Edep_Comb,
            Total_Edep_SF_C, Total_Edep_SF_S, Total_Edep_SF_S_LCcor, Total_Edep_SF_S_ATTcor, Total_Edep_SF_S_LCATTcor, 
            Total_Edep_SF_Comb,
            Total_Edep_SF_DRcor, Total_Edep_SF_DRcor_LCcor, Total_Edep_SF_DRcor_ATTcor, Total_Edep_SF_DRcor_LCATTcor,
            signal_CC1, signal_CC2, signal_PS, signal_MC, signal_TC, Edep_LC, DWC1_pos, DWC2_pos,
            "6_MC"
        );
        
    }

    TF1 *fit_scale_C = new TF1("fit_scale_C","gaus",0,200);
    fit_scale_C->SetParameters(1,0,1);
    histManager.getHist1D("Total_Edep_C_6_MC")->Fit("fit_scale_C");
    
    TF1 *fit_scale_S = new TF1("fit_scale_S","gaus",0,200);
    fit_scale_S->SetParameters(1,0,1);
    histManager.getHist1D("Total_Edep_S_6_MC")->Fit("fit_scale_S");
    
    TF1 *fit_reso_C = new TF1("fit_reso_C","gaus",0,200);
    fit_reso_C->SetParameters(1,0,1);
    histManager.getHist1D("Total_Edep_SF_C_6_MC")->Fit("fit_reso_C");
    
    TF1 *fit_reso_S = new TF1("fit_reso_S","gaus",0,200);
    fit_reso_S->SetParameters(1,0,1);
    histManager.getHist1D("Total_Edep_SF_S_6_MC")->Fit("fit_reso_S");
    
    TF1 *fit_reso_Comb = new TF1("fit_reso_Comb","gaus",0,400);
    fit_reso_Comb->SetParameters(1,0,1);
    histManager.getHist1D("Total_Edep_SF_Comb_6_MC")->Fit("fit_reso_Comb");
    
    // Write histograms and create output ROOT file
    std::string output_file;
    // For energy scan runs, create Energy_ntuple directory and save with energy info
    std::string module_str = (fModule == 5) ? "M5T2" : "M4T2";
    output_file = savepath + "/" + beam_type + "_" + module_str + "_" + std::to_string(fRunNum) + ".root";
    
    TFile* outputRoot = TFile::Open((TString)(output_file), "RECREATE");
    histManager.writeAllHistograms(outputRoot);

    fit_scale_C->Write();
    fit_scale_S->Write();
    fit_reso_C->Write();
    fit_reso_S->Write();
    fit_reso_Comb->Write();
    
    outputRoot->Close();
    histManager.closeAllHistograms();
    if(f_DWC) {f_DWC->Close();}

    std::cout << "Program completed successfully!" << std::endl;

    return 0;
}