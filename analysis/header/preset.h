#ifndef PRESET_H
#define PRESET_H

#include "TString.h"
#include <map>
#include <utility>  // for std::pair
#include <string>   // for std::string
#include <iostream> // for std::cout

//========================================================================================
// TB2025 분석 설정값들
// 이 헤더파일은 evtloop_emDRC.cc에서 사용하는 모든 설정값들을 포함합니다.
//========================================================================================

// Module integration ranges using std::pair<first, last>
const std::pair<int, int> C_RANGE = {260, 420};      // Module integration range
const std::pair<int, int> S_RANGE = {260, 460};      // Module integration range
const std::pair<int, int> M5T3C_RANGE = {260, 420};  // M5T3 integration range, for hadronic beam  
const std::pair<int, int> M5T3S_RANGE = {260, 460};  // M5T3 integration range, for hadronic beam

const std::pair<int, int> CC1_PEAK_RANGE = {700, 755};  // Peak search range
const std::pair<int, int> CC2_PEAK_RANGE = {660, 745};  // Peak search range
const std::pair<int, int> PS_RANGE = {230, 370};        // PS integration range
const std::pair<int, int> MC_RANGE = {710, 770};        // MC integration range
const std::pair<int, int> TC_RANGE = {300, 450};        // TC peak search range
const std::pair<int, int> LC_RANGE = {400, 600};        // LC integration range

// PID Cut values - 기본값 설정 (calibration run 기준)
std::pair<float, float> CUT_CC1 = {80, 200};
std::pair<float, float> CUT_CC2 = {140, 1e5};
std::pair<float, float> CUT_PS = {400, 1e5};
std::pair<float, float> CUT_MC = {0, 56};
constexpr float CUT_DWC = 4;      // Beam geometry cut for DWC

// Beam type별 컷 값 설정 함수 (evtloop_DRC.cc에서 사용)
inline void setCutsForBeamType(const std::string& beam_type) {
    if(beam_type == "pi") {
        CUT_CC1 = {0, 50};      // select pedestal
        CUT_CC2 = {150, 1e5};   // select signal
        CUT_PS = {80, 300};     // select 1mip peak
        CUT_MC = {0, 30};       // select pedestal (reject muon)
    } else if(beam_type == "kaon") {
        CUT_CC1 = {0, 80};      // select pedestal
        CUT_CC2 = {200, 1e5};   // select signal
        CUT_PS = {80, 300};     // select 1mip peak
        CUT_MC = {0, 50};       // select pedestal (reject muon)
    } else if(beam_type == "proton") {
        CUT_CC1 = {0, 80};      // select pedestal
        CUT_CC2 = {0, 200};     // select pedestal
        CUT_PS = {80, 300};     // select 1mip peak
        CUT_MC = {0, 50};       // select pedestal (reject muon)
    } else if(beam_type == "em" || beam_type.empty()) {
        // 전자 빔 - 기본값 사용
        CUT_CC1 = {80, 200};
        CUT_CC2 = {140, 1e5};
        CUT_PS = {400, 1e5};
        CUT_MC = {0, 56};
    } else {
        std::cout << "ERROR: Invalid beam type: " << beam_type << std::endl;
    }
}

// module calibration constants(calibration constant)
std::map<TString, double> MAP_CALIB_CONST = {
    {"M1T1_C", 0.00150708}, {"M1T1_S", 0.000789338}, {"M1T2_C", 0.00151428}, {"M1T2_S", 0.000780193}, {"M1T3_C", 0.001556},   {"M1T3_S", 0.000819258}, {"M1T4_C", 0.00150676}, {"M1T4_S", 0.000827371},
    {"M2T1_C", 0.00140138}, {"M2T1_S", 0.000783668}, {"M2T2_C", 0.00151431}, {"M2T2_S", 0.000782582}, {"M2T3_C", 0.00142252}, {"M2T3_S", 0.000815883}, {"M2T4_C", 0.00153853}, {"M2T4_S", 0.000850718},
    {"M3T1_C", 0.00147114}, {"M3T1_S", 0.000787217}, {"M3T2_C", 0.00162378}, {"M3T2_S", 0.000813549}, {"M3T3_C", 0.0015684},  {"M3T3_S", 0.00080892},  {"M3T4_C", 0.00155923}, {"M3T4_S", 0.000841111},
    {"M4T1_C", 0.0015008},  {"M4T1_S", 0.00082428},  {"M4T2_C", 0.00125955}, {"M4T2_S", 0.000776664}, {"M4T3_C", 0.00150114}, {"M4T3_S", 0.000807619}, {"M4T4_C", 0.00147611}, {"M4T4_S", 0.000829999},
    {"M5T1_C", 0.00165787}, {"M5T1_S", 0.000812581}, {"M5T2_C", 0.00151347}, {"M5T2_S", 0.000747246}, {"M5T3_C", 0.00191922}, {"M5T3_S", 0.000967332}, {"M5T4_C", 0.00144368}, {"M5T4_S", 0.000851485},
    {"M6T1_C", 0.00155174}, {"M6T1_S", 0.000829883}, {"M6T2_C", 0.00143409}, {"M6T2_S", 0.000825618}, {"M6T3_C", 0.001602},   {"M6T3_S", 0.000840675}, {"M6T4_C", 0.00167525}, {"M6T4_S", 0.000859839},
    {"M7T1_C", 0.00156551}, {"M7T1_S", 0.000823712}, {"M7T2_C", 0.00153424}, {"M7T2_S", 0.000797242}, {"M7T3_C", 0.00156439}, {"M7T3_S", 0.000769436}, {"M7T4_C", 0.00154212}, {"M7T4_S", 0.000832135},
    {"M8T1_C", 0.00159879}, {"M8T1_S", 0.000815434}, {"M8T2_C", 0.00153042}, {"M8T2_S", 0.000779217}, {"M8T3_C", 0.00151916}, {"M8T3_S", 0.00079686},  {"M8T4_C", 0.0015338},  {"M8T4_S", 0.000830145},
    {"M9T1_C", 0.00155364}, {"M9T1_S", 0.000822387}, {"M9T2_C", 0.00155126}, {"M9T2_S", 0.000799692}, {"M9T3_C", 0.00154087}, {"M9T3_S", 0.000828242}, {"M9T4_C", 0.00147478}, {"M9T4_S", 0.000853169}
};
// module scale factor
std::map<TString, double> MAP_SCALE_FACTOR = {
    {"M5T2_C", 0.76195988}, {"M5T2_S", 0.78445651},
    //{"M4T2_C", 0.76738764}, {"M4T2_S", 0.79116037}
};
// LC calibration constants(equalization constant)
std::map<TString, double> MAP_LC_CALIB_CONST = {
    {"LC2", 0.0038424*0.001},  {"LC4", 0.0035562*0.001},  {"LC8", 0.0058593*0.001},  {"LC10", 0.0063329*0.001},
    {"LC3", 0.0050539*0.001},  {"LC5", 0.0055991*0.001},  {"LC7", 0.0041235*0.001},  {"LC9", 0.0054286*0.001},
    {"LC11", 0.0084997*0.001}, {"LC12", 0.0045217*0.001}, {"LC13", 0.0074176*0.001}, {"LC19", 0.0110179*0.001},
    {"LC14", 0.0068763*0.001}, {"LC15", 0.0066162*0.001}, {"LC16", 0.0065418*0.001}, {"LC20", 0.0136235*0.001},
};
float LC_scalefactor = 7.25689405;

//depth calcuation
float time_ele_showermax = 36.65; // 42.65 (calculated from electron events) - 6ns delay
float depth_ele_showermax = 0.219;
float eff_photon_speed = 2.55;

float attenlen_S = 7.893; // Attenuation length of S fiber, measured with 60 GeV pion events.

#endif // PRESET_H 