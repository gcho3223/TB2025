#include <chrono>
#include <numeric>
#include <vector>
#include <algorithm>
#include <iostream>
#include <filesystem>
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "TROOT.h"
#include "TStyle.h"
#include "TCanvas.h"
#include "TH1.h"
#include "TH2.h"
#include "TFile.h"
#include "TApplication.h"
#include "TRootCanvas.h"
#include "TPaveStats.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"

#include "TLegend.h"
#include "TLatex.h"
#include "TColor.h"

#include "./header/function.h"

namespace fs = std::filesystem;

// Function to convert channel name to branch name  
std::string channelToBranchName(const std::string& channel_name) {
    // M1-T1-S -> wave_M1_T1_S
    std::string branch_name = "wave_" + channel_name;
    std::replace(branch_name.begin(), branch_name.end(), '-', '_');
    return branch_name;
}

// This macro will draw single event waveform (without any PID cuts) from waveform ntuple
// This macro will store waveform plots of pre-shower, module1 tower1 C and S channels in .png format event by event

// How to execute
// On local or batch, run the following command :
// ./TBdrawWave.exe <run number> <max # of events to process>

int main(int argc, char **argv) {
    // Set batch mode to avoid GUI issues
    gROOT->SetBatch(kTRUE);
    
    gStyle->SetOptStat(0);
    gStyle->SetStatFormat("6.6g");
    
    int fRunNum = std::stoi(argv[1]);
    int fMaxEvent = std::stoi(argv[2]);
    int fMaxFile = -1;
    
    std::vector<std::string> channel_names;
    for (int plot_args = 3; plot_args < argc; plot_args++) {
        channel_names.push_back(argv[plot_args]);
    }
    
    fs::path dir("./output/Overlap");
    if (!(fs::exists(dir))) fs::create_directory(dir);
    
    fs::path dir2("./Overlap/Run_" + std::to_string(fRunNum));
    if (!(fs::exists(dir2))) fs::create_directory(dir2);
    
    std::vector<TH2F *> plots;
    for (int idx = 0; idx < channel_names.size(); idx++)
    {
        plots.push_back(new TH2F((TString)(channel_names.at(idx)), ";bin;ADC", 1024, 0., 1024., 4096, 0., 4096.));
        plots.at(idx)->GetYaxis()->SetRangeUser(0, 4096);
    }
    
    TCanvas *c = new TCanvas("c1", "c1");
    
    // Load ntuple file
    TFile* fNtuple = TFile::Open((TString)("/pnfs/knu.ac.kr/data/cms/store/user/sungwon/2025_DRC_TB_PromptAnalysis/Prompt_ntuple_Run_" + std::to_string(fRunNum) + ".root"), "READ");
    TTreeReader reader("evt", fNtuple);

    // Create TTreeReaderValue for each channel outside the loop
    std::vector<TTreeReaderValue<std::vector<short>>*> channel_readers;
    for (int idx = 0; idx < channel_names.size(); idx++) {
        std::string branch_name = channelToBranchName(channel_names.at(idx));
        std::cout << "Creating reader for channel: " << channel_names.at(idx) << " -> branch: " << branch_name << std::endl;
        channel_readers.push_back(new TTreeReaderValue<std::vector<short>>(reader, branch_name.c_str()));
    }
    
    // Set Maximum event
    Long64_t totalEntries = reader.GetEntries();
    if(fMaxEvent == -1 || fMaxEvent > totalEntries) fMaxEvent = totalEntries;
    
    TFile *outFile = new TFile(("./Overlap/Run_" + std::to_string(fRunNum) + "/Run_" + std::to_string(fRunNum) + ".root").c_str(), "RECREATE");
    outFile->cd();
    
    // Starting Evt Loop
    for (int iEvt = 0; iEvt < fMaxEvent; iEvt++) {
        printProgress(iEvt, fMaxEvent);
        reader.SetEntry(iEvt);
        
        for (int idx = 0; idx < channel_names.size(); idx++) {
            auto single_waveform = **channel_readers.at(idx);
            for (int bin = 1; bin < 1024; bin++) {
                plots.at(idx)->Fill(bin, single_waveform[bin-1]);
            }
        }
    }
    
    for (int idx = 0; idx < plots.size(); idx++) {
        outFile->cd();
        c->cd();
        c->RedrawAxis();
        plots.at(idx)->Draw("Hist");
        plots.at(idx)->Write();
        c->SaveAs((TString)("./Overlap/Run_" + std::to_string(fRunNum) + "/Run_" + std::to_string(fRunNum) + "_" + channel_names.at(idx) + ".pdf"));
        c->Clear();
    }
    
    outFile->Close();
    
    // Clean up dynamically allocated TTreeReaderValue objects
    for (auto* reader_ptr : channel_readers) {
        delete reader_ptr;
    }
}
