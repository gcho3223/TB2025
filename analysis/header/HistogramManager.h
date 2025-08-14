#ifndef HISTOGRAM_MANAGER_H
#define HISTOGRAM_MANAGER_H

#include <map>
#include <vector>
#include <string>
#include "TH1F.h"
#include "TH2D.h"
#include "TFile.h"

class HistogramManager {
public:
    HistogramManager();
    ~HistogramManager();
    
    void DeclareHistograms();
    
    void fillHistogram1D(const std::string& name, double x, double weight = 1.0);
    void fillHistogram2D(const std::string& name, double x, double y, double weight = 1.0);
    
    // Write and close
    void writeAllHistograms(TFile* outputFile);
    void closeAllHistograms();
    
    // Get histogram by name (need to fitting)
    TH1F* getHist1D(const std::string& name);
    TH2D* getHist2D(const std::string& name);
    
    void fillModuleTowerHistograms(int module,
                                   const std::vector<float>& intADC_C, const std::vector<float>& intADC_S,
                                   const std::vector<float>& Edep_C, const std::vector<float>& Edep_S, const std::string& step = "");
    void fillModuleHistograms(int module, float Edep_C_sum, float Edep_S_sum, const std::string& step = "");
    void fillAuxiliaryHistograms(float CC1, float CC2, float PS, float MC, float TC, const std::string& step = "");
    void fillLCTotalEnergyHistograms(float Edep_LC, const std::string& step = "");
    void fillTotalEnergyHistograms(float Total_Edep_C, float Total_Edep_S, 
                                   float Total_Edep_Comb, const std::string& step = "");
    void fillScaledEnergyHistograms(float Total_Edep_SF_C, float Total_Edep_SF_S, float Total_Edep_SF_S_LCcor, float Total_Edep_SF_S_ATTcor, float Total_Edep_SF_S_LCATTcor,
                                   float Total_Edep_SF_Comb, 
                                   float Total_Edep_SF_DRcor, float Total_Edep_SF_DRcor_LCcor, float Total_Edep_SF_DRcor_ATTcor, float Total_Edep_SF_DRcor_LCATTcor,
                                   const std::string& step = "");
    void fillDWCHistograms(const std::vector<float>& DWC1_pos, const std::vector<float>& DWC2_pos, float Total_Edep_SF_S_LCATTcor, float Total_Edep_SF_C, const std::string& step = "");
    
    // Convenience function to fill all histograms for a given step
    void fillAllStepHistograms(
        const std::vector<std::vector<float>>& All_intADC_C,const std::vector<std::vector<float>>& All_intADC_S, // S,C channel intADC
        const std::vector<std::vector<float>>& All_Edep_C,  const std::vector<std::vector<float>>& All_Edep_S,   // S,C channel Edep
        const std::vector<float>& All_Module_Edep_C,        const std::vector<float>& All_Module_Edep_S,         // M1-9 module Edep for C, S channel
        float Total_Edep_C, float Total_Edep_S, float Total_Edep_Comb,                                           // Total Edep for C, S channel, S channel + LC correction, C + S channel
        float Total_Edep_SF_C, float Total_Edep_SF_S, float Total_Edep_SF_S_LCcor, float Total_Edep_SF_S_ATTcor, float Total_Edep_SF_S_LCATTcor, // Total Edep for C, S channel, S+LC, S+LC+ATT, S+LC+ATT+LC
        float Total_Edep_SF_Comb,                                                                                // Total Edep for C + S channel
        float Total_Edep_SF_DRcor, float Total_Edep_SF_DRcor_LCcor, float Total_Edep_SF_DRcor_ATTcor, float Total_Edep_SF_DRcor_LCATTcor, // apply DR correction, DRcorrection + LC correction, DRcorrection + ATT correction, DRcorrection + LC correction + ATT correction
        float signal_CC1, float signal_CC2, float signal_PS, float signal_MC, float signal_TC, float Edep_LC, const std::vector<float>& DWC1_pos, const std::vector<float>& DWC2_pos,
        const std::string& step = ""
    );

private:
    std::map<std::string, TH1F*> histograms1D;
    std::map<std::string, TH2D*> histograms2D;
};

// Implementation
HistogramManager::HistogramManager() {}

HistogramManager::~HistogramManager() {closeAllHistograms();}

void HistogramManager::DeclareHistograms()
{
    std::vector<std::string> steps = {"0_noCut", "1_DWC", "2_DWCsize", "3_CC1", "4_CC2", "5_PS", "6_MC"};
    
    for(const std::string& step : steps) {
        // Module-Tower intADC histograms (M1-M9, T1-T4, C/S)
        std::vector<int> modules = {1, 2, 3, 4, 5, 6, 7, 8, 9};
        std::vector<int> towers  = {1, 2, 3, 4};
        std::vector<std::string> channels = {"C", "S"};
        
        for(int module : modules) {
            for(int tower : towers) {
                for(const std::string& ch : channels) {
                    // intADC histograms
                    std::string nameADC = "M" + std::to_string(module) + "_T" + std::to_string(tower) + "_" + ch + "_" + step;
                    histograms1D[nameADC] = new TH1F(nameADC.c_str(), ";intADC;nEvents", 320, -20000, 300000);
                    
                    // Energy deposit histograms
                    std::string nameEdep = "Edep_M" + std::to_string(module) + "_T" + std::to_string(tower) + "_" + ch + "_" + step;
                    histograms1D[nameEdep] = new TH1F(nameEdep.c_str(), ";GeV;Event", 110, -20, 200);
                }
            }
        }
        
        // Module sum histograms (M1-M9, C/S)
        for(int module : modules) {
            for(const std::string& ch : channels) {
                std::string nameEdep_module = "Edep_M" + std::to_string(module) + "_" + ch + "_" + step;
                histograms1D[nameEdep_module] = new TH1F(nameEdep_module.c_str(), ";GeV;Event", 110, -20, 200);
            }
        }
        
        // Auxiliary detector histograms
        histograms1D["CC1_" + step] = new TH1F(("CC1_" + step).c_str(), ";peakADC;Events", 1024, 0, 4096);
        histograms1D["CC2_" + step] = new TH1F(("CC2_" + step).c_str(), ";peakADC;Events", 1024, 0, 4096);
        histograms1D["PS_" + step]  = new TH1F(("PS_" + step).c_str(), ";peakADC;nEvents", 1024, 0, 4096);
        histograms1D["MC_" + step]  = new TH1F(("MC_" + step).c_str(), ";peakADC;nEvents", 1024, 0, 4096);
        histograms1D["TC_" + step]  = new TH1F(("TC_" + step).c_str(), ";peakADC;nEvents", 1024, 0, 4096);
        
        // Leakage counter histograms
        std::vector<int> lcNumbers = {2, 3, 4, 5, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 19, 20};
        for(int lc : lcNumbers) {
            std::string nameLc = "LC" + std::to_string(lc) + "_" + step;
            histograms1D[nameLc] = new TH1F(nameLc.c_str(), ";intADC;nEvents", 320, -20000, 300000);
        }
        
        // Total energy histograms
        // combined channel only for em analysis
        histograms1D["Total_Edep_C_" + step]                 = new TH1F(("Total_Edep_C_" + step).c_str(), ";GeV;Event", 110, -20, 200);
        histograms1D["Total_Edep_S_" + step]                 = new TH1F(("Total_Edep_S_" + step).c_str(), ";GeV;Event", 110, -20, 200);
        histograms1D["Total_Edep_Comb_" + step]              = new TH1F(("Total_Edep_Comb_" + step).c_str(), ";GeV;Event", 110, -40, 400);
        
        // Scaled energy histograms
        // S_LCcor, S_ATTcor, S_LCATTcor, DRCor, DRcor_LCcor, DRcor_ATTcor, DRcor_LCATTcor for hadronic analysis
        histograms1D["Total_Edep_SF_C_" + step]              = new TH1F(("Total_Edep_SF_C_" + step).c_str(), ";GeV;Event", 110, -20, 200);
        histograms1D["Total_Edep_SF_S_" + step]              = new TH1F(("Total_Edep_SF_S_" + step).c_str(), ";GeV;Event", 110, -20, 200);
        histograms1D["Total_Edep_SF_S_LCcor_" + step]        = new TH1F(("Total_Edep_SF_S_LCcor_" + step).c_str(), ";GeV;Event", 110, -20, 200);
        histograms1D["Total_Edep_SF_S_ATTcor_" + step]       = new TH1F(("Total_Edep_SF_S_ATTcor_" + step).c_str(), ";GeV;Event", 110, -20, 200);
        histograms1D["Total_Edep_SF_S_LCATTcor_" + step]     = new TH1F(("Total_Edep_SF_S_LCATTcor_" + step).c_str(), ";GeV;Event", 110, -20, 200);
        histograms1D["Total_Edep_SF_Comb_" + step]           = new TH1F(("Total_Edep_SF_Comb_" + step).c_str(), ";GeV;Event", 110, -40, 400);
        histograms1D["Total_Edep_SF_DRcor_" + step]          = new TH1F(("Total_Edep_SF_DRcor_" + step).c_str(), ";GeV;Event", 110, -40, 400);
        histograms1D["Total_Edep_SF_DRcor_LCcor_" + step]    = new TH1F(("Total_Edep_SF_DRcor_LCcor_" + step).c_str(), ";GeV;Event", 110, -40, 400);
        histograms1D["Total_Edep_SF_DRcor_ATTcor_" + step]   = new TH1F(("Total_Edep_SF_DRcor_ATTcor_" + step).c_str(), ";GeV;Event", 110, -40, 400);
        histograms1D["Total_Edep_SF_DRcor_LCATTcor_" + step] = new TH1F(("Total_Edep_SF_DRcor_LCATTcor_" + step).c_str(), ";GeV;Event", 110, -40, 400);

        histograms2D["Total_Edep_S_vs_Total_Edep_C_" + step] = new TH2D(("Total_Edep_S_vs_Total_Edep_C_" + step).c_str(), ";S Energy [GeV];C Energy [GeV];Event", 110, -20, 200, 110, -20, 200);

        
        // LC total energy histograms
        histograms1D["Edep_LC_" + step] = new TH1F(("Edep_LC_" + step).c_str(), ";GeV;Event", 110, -10, 100);

        // DWC position histograms
        histograms2D["DWC1_pos_" + step]   = new TH2D(("DWC1_pos_"   + step).c_str(), "dwc1_pos;mm;mm;events", 480, -120., 120., 480, -120., 120.);
        histograms2D["DWC2_pos_" + step]   = new TH2D(("DWC2_pos_"   + step).c_str(), "dwc2_pos;mm;mm;events", 480, -120., 120., 480, -120., 120.);
        histograms2D["DWC_x_corr_" + step] = new TH2D(("DWC_x_corr_" + step).c_str(), "dwc_x_corr;DWC1_X_mm;DWC2_X_mm;events", 480, -120., 120., 480, -120., 120.);
        histograms2D["DWC_y_corr_" + step] = new TH2D(("DWC_y_corr_" + step).c_str(), "dwc_y_corr;DWC1_Y_mm;DWC2_Y_mm;events", 480, -120., 120., 480, -120., 120.);
    }
}

void HistogramManager::fillHistogram1D(const std::string& name, double x, double weight)
{
    auto it = histograms1D.find(name);
    if(it != histograms1D.end()) {it->second->Fill(x, weight);}
}

void HistogramManager::fillHistogram2D(const std::string& name, double x, double y, double weight)
{
    auto it = histograms2D.find(name);
    if(it != histograms2D.end()) {it->second->Fill(x, y, weight);}
}

TH1F* HistogramManager::getHist1D(const std::string& name)
{
    auto it = histograms1D.find(name);
    return (it != histograms1D.end()) ? it->second : nullptr;
}

TH2D* HistogramManager::getHist2D(const std::string& name)
{
    auto it = histograms2D.find(name);
    return (it != histograms2D.end()) ? it->second : nullptr;
}

void HistogramManager::writeAllHistograms(TFile* outputFile)
{
    outputFile->cd();
    for(auto& pair : histograms1D) {pair.second->Write();}
    for(auto& pair : histograms2D) {pair.second->Write();}
}

void HistogramManager::closeAllHistograms()
{
    for(auto& pair : histograms1D) {delete pair.second;} histograms1D.clear();
    for(auto& pair : histograms2D) {delete pair.second;} histograms2D.clear();
}

// Implementation of high-level helper functions for evtloop with step suffix
void HistogramManager::fillModuleTowerHistograms(int module, const std::vector<float>& intADC_C, const std::vector<float>& intADC_S,
                                                 const std::vector<float>& Edep_C, const std::vector<float>& Edep_S, const std::string& step)
{
    std::string suffix = step.empty() ? "" : "_" + step;
    
    for(int tower = 1; tower <= 4; tower++) {
        std::string nameADC_C  = "M"      + std::to_string(module) + "_T" + std::to_string(tower) + "_C" + suffix;
        std::string nameADC_S  = "M"      + std::to_string(module) + "_T" + std::to_string(tower) + "_S" + suffix;
        std::string nameEdep_C = "Edep_M" + std::to_string(module) + "_T" + std::to_string(tower) + "_C" + suffix;
        std::string nameEdep_S = "Edep_M" + std::to_string(module) + "_T" + std::to_string(tower) + "_S" + suffix;
        fillHistogram1D(nameADC_C,  intADC_C[tower-1]);
        fillHistogram1D(nameADC_S,  intADC_S[tower-1]);
        fillHistogram1D(nameEdep_C, Edep_C[tower-1]);
        fillHistogram1D(nameEdep_S, Edep_S[tower-1]);
    }
}

void HistogramManager::fillModuleHistograms(int module, float Edep_C_sum, float Edep_S_sum, const std::string& step)
{
    std::string suffix = step.empty() ? "" : "_" + step;
    std::string nameC = "Edep_M" + std::to_string(module) + "_C" + suffix;
    std::string nameS = "Edep_M" + std::to_string(module) + "_S" + suffix;
    
    fillHistogram1D(nameC, Edep_C_sum);
    fillHistogram1D(nameS, Edep_S_sum);
}

void HistogramManager::fillAuxiliaryHistograms(float CC1, float CC2, float PS, float MC, float TC, const std::string& step)
{
    std::string suffix = step.empty() ? "" : "_" + step;
    fillHistogram1D("CC1" + suffix, CC1);
    fillHistogram1D("CC2" + suffix, CC2);
    fillHistogram1D("PS"  + suffix, PS);
    fillHistogram1D("MC"  + suffix, MC);
    fillHistogram1D("TC"  + suffix, TC);
}

void HistogramManager::fillLCTotalEnergyHistograms(float Edep_LC, const std::string& step)
{
    std::string suffix = step.empty() ? "" : "_" + step;
    fillHistogram1D("Edep_LC" + suffix, Edep_LC);
}

void HistogramManager::fillTotalEnergyHistograms(float Total_Edep_C, float Total_Edep_S, float Total_Edep_Comb, const std::string& step)
{
    std::string suffix = step.empty() ? "" : "_" + step;
    fillHistogram1D("Total_Edep_C"       + suffix, Total_Edep_C);
    fillHistogram1D("Total_Edep_S"       + suffix, Total_Edep_S);
    fillHistogram1D("Total_Edep_Comb"    + suffix, Total_Edep_Comb);
}

void HistogramManager::fillScaledEnergyHistograms(float Total_Edep_SF_C, float Total_Edep_SF_S, float Total_Edep_SF_S_LCcor, float Total_Edep_SF_S_ATTcor, float Total_Edep_SF_S_LCATTcor,
                                                  float Total_Edep_SF_Comb, 
                                                  float Total_Edep_SF_DRcor, float Total_Edep_SF_DRcor_LCcor, float Total_Edep_SF_DRcor_ATTcor, float Total_Edep_SF_DRcor_LCATTcor,
                                                  const std::string& step)
{
    std::string suffix = step.empty() ? "" : "_" + step;
    fillHistogram1D("Total_Edep_SF_C"               + suffix, Total_Edep_SF_C);
    fillHistogram1D("Total_Edep_SF_S"               + suffix, Total_Edep_SF_S);
    fillHistogram1D("Total_Edep_SF_S_LCcor"         + suffix, Total_Edep_SF_S_LCcor);
    fillHistogram1D("Total_Edep_SF_S_ATTcor"        + suffix, Total_Edep_SF_S_ATTcor);
    fillHistogram1D("Total_Edep_SF_S_LCATTcor"      + suffix, Total_Edep_SF_S_LCATTcor);
    fillHistogram1D("Total_Edep_SF_Comb"            + suffix, Total_Edep_SF_Comb);
    fillHistogram1D("Total_Edep_SF_DRcor"           + suffix, Total_Edep_SF_DRcor);
    fillHistogram1D("Total_Edep_SF_DRcor_LCcor"     + suffix, Total_Edep_SF_DRcor_LCcor);
    fillHistogram1D("Total_Edep_SF_DRcor_ATTcor"    + suffix, Total_Edep_SF_DRcor_ATTcor);
    fillHistogram1D("Total_Edep_SF_DRcor_LCATTcor"  + suffix, Total_Edep_SF_DRcor_LCATTcor);
}

void HistogramManager::fillDWCHistograms(const std::vector<float>& DWC1_pos, const std::vector<float>& DWC2_pos, float Total_Edep_SF_S_LCATTcor, float Total_Edep_SF_C, const std::string& step)
{
    std::string suffix = "_" + step;
    fillHistogram2D("DWC1_pos"   + suffix, DWC1_pos[0], DWC1_pos[1]);
    fillHistogram2D("DWC2_pos"   + suffix, DWC2_pos[0], DWC2_pos[1]);
    fillHistogram2D("DWC_x_corr" + suffix, DWC1_pos[0], DWC2_pos[0]);
    fillHistogram2D("DWC_y_corr" + suffix, DWC1_pos[1], DWC2_pos[1]);
    fillHistogram2D("Total_Edep_S_vs_Total_Edep_C" + suffix, Total_Edep_SF_S_LCATTcor, Total_Edep_SF_C); // 이름 수정
}

void HistogramManager::fillAllStepHistograms(
    const std::vector<std::vector<float>>& All_intADC_C, const std::vector<std::vector<float>>& All_intADC_S,
    const std::vector<std::vector<float>>& All_Edep_C, const std::vector<std::vector<float>>& All_Edep_S,
    const std::vector<float>& All_Module_Edep_C, const std::vector<float>& All_Module_Edep_S,
    float Total_Edep_C, float Total_Edep_S, float Total_Edep_Comb,
    float Total_Edep_SF_C, float Total_Edep_SF_S, float Total_Edep_SF_S_LCcor, float Total_Edep_SF_S_ATTcor, float Total_Edep_SF_S_LCATTcor,
    float Total_Edep_SF_Comb,
    float Total_Edep_SF_DRcor, float Total_Edep_SF_DRcor_LCcor, float Total_Edep_SF_DRcor_ATTcor, float Total_Edep_SF_DRcor_LCATTcor,
    float signal_CC1, float signal_CC2, float signal_PS, float signal_MC, float signal_TC, float Edep_LC,
    const std::vector<float>& DWC1_pos, const std::vector<float>& DWC2_pos,
    const std::string& step
)
{
    // Fill all module tower histograms (M1-M9)
    for(int module = 1; module <= 9; module++) {
        fillModuleTowerHistograms(module, All_intADC_C[module-1], All_intADC_S[module-1], All_Edep_C[module-1], All_Edep_S[module-1], step);
        fillModuleHistograms(module, All_Module_Edep_C[module-1], All_Module_Edep_S[module-1], step);
    }
    
    // Fill energy histograms
    fillTotalEnergyHistograms(Total_Edep_C, Total_Edep_S, Total_Edep_Comb, step);
    fillScaledEnergyHistograms(Total_Edep_SF_C, Total_Edep_SF_S, Total_Edep_SF_S_LCcor, Total_Edep_SF_S_ATTcor, Total_Edep_SF_S_LCATTcor, Total_Edep_SF_Comb, Total_Edep_SF_DRcor, Total_Edep_SF_DRcor_LCcor, Total_Edep_SF_DRcor_ATTcor, Total_Edep_SF_DRcor_LCATTcor, step);
    
    // Fill auxiliary histograms
    fillAuxiliaryHistograms(signal_CC1, signal_CC2, signal_PS, signal_MC, signal_TC, step);
    fillLCTotalEnergyHistograms(Edep_LC, step);
    fillDWCHistograms(DWC1_pos, DWC2_pos, Total_Edep_SF_S_LCATTcor, Total_Edep_SF_C, step);
}

#endif // HISTOGRAM_MANAGER_H 