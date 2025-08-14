#ifndef HISTOLIST_H
#define HISTOLIST_H
///////////////////////////////////////////////////
// !              Histogram list               ! //
///////////////////////////////////////////////////
#include <vector>
#include <TString.h>

using namespace std;

// DRC Analysis Step Names
vector<TString> drcSteps = {"0_noCut", "1_DWC", "2_DWCsize", "3_CC1", "4_CC2", "5_PS", "6_MC"};

// DRC 1D Histograms - Total Energy & ADC
vector<TString> hist1D_drc_total = 
{
    "totalEdepScaled_C",    "totalEdepScaled_S",    "totalEdepScaled_Comb", //(S+C) 
    "totalEdepScaled_Comb_LC_compensated", // C+(S+LC)
    "totalEdepScaled_Comb_DRCorrection",   // (S+C) + DRCorrection
    "totalEdepScaled_Comb_DRCorrection_LC_compensated", // C+(S+LC) + DRCorrection
};

// DRC 1D Histograms - Auxiliary Detectors
vector<TString> hist1D_drc_aux = 
{
    "CC1",  "CC2",  "PS",   "MC",   "TC",
    "LC2",  "LC4",  "LC8",  "LC10",
    "LC3",  "LC5",  "LC7",  "LC9",
    "LC11", "LC12", "LC13", "LC19",
    "LC14", "LC15", "LC16", "LC20",
};

// DRC 2D Histograms - DWC Positions
vector<TString> hist2D_drc_dwc = {"DWC1_pos", "DWC2_pos", "DWC_x_corr", "DWC_y_corr"};

// Function to generate all module histograms dynamically
vector<TString> generateModuleHistograms() {
    vector<TString> histList;
    vector<TString> drcSteps = {"0_noCut", "1_DWC", "2_DWCsize", "3_CC1", "4_CC2", "5_PS", "6_MC"};
    vector<TString> channels = {"C", "S"};
    
    // Individual tower histograms: Module_Tower_Channel_Type_Step
    for(int m = 1; m <= 9; m++)
    {
        for(int t = 1; t <= 4; t++)
        {
            for(auto& channel : channels)
            {
                for(auto& step : drcSteps)
                {
                    TString histName = Form("%s_M%d_T%d_%s_%s", "Edep", m, t, channel.Data(), step.Data());
                    histList.push_back(histName);
                }
            }
        }
    }
    
    // intADC 
    for(int m = 1; m <= 9; m++)
    {
        for(int t = 1; t <= 4; t++)
        {
            for(auto& channel : channels)
            {
                for(auto& step : drcSteps)
                {
                    TString histName = Form("M%d_T%d_%s_%s", m, t, channel.Data(), step.Data());
                    histList.push_back(histName);
                }
            }
        }
    }
    
    return histList;
}

// Combined 1D histogram list (initialize as empty)
vector<TString> hist1D;

// Function to initialize all histogram lists
void initializeHistogramLists()
{    
    // Add all histogram types
    hist1D.insert(hist1D.end(), hist1D_drc_total.begin(), hist1D_drc_total.end());
    hist1D.insert(hist1D.end(), hist1D_drc_aux.begin()  , hist1D_drc_aux.end());
    
    // Add dynamically generated module histograms
    vector<TString> moduleHists = generateModuleHistograms();
    hist1D.insert(hist1D.end(), moduleHists.begin(), moduleHists.end());
}

// Combined 2D histogram list
vector<TString> hist2D = hist2D_drc_dwc;

#endif // HISTOLIST_H