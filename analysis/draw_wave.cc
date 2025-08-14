#include "TBread.h"
#include "TButility.h"

#include <chrono>
#include <numeric>
#include <vector>
#include <algorithm>
#include <filesystem>
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

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

namespace fs = std::__fs::filesystem;

// Function to convert channel name to branch name  
std::string channelToBranchName(const std::string& channel_name) {
    // M1-T1-S -> wave_M1_T1_S
    std::string branch_name = "wave_" + channel_name;
    std::replace(branch_name.begin(), branch_name.end(), '-', '_');
    return branch_name;
}


// How to execute
// On local or batch, run the following command :
// ./draw_wave <run number> <max # of events to process> <channel name> <channel name> ...
// Channel name = M1-T3-C, S63, PS, CC1, T1 etc...

int main(int argc, char** argv) {
    gStyle->SetOptStat(0);

    int fRunNum = std::stoi(argv[1]);
    int fMaxEvent = std::stoi(argv[2]);
    int fMaxFile = -1;
    gStyle->SetPalette(kVisibleSpectrum);

    std::vector<std::string> channel_names;
    myColorPalette.clear();
    for (int plot_args = 3; plot_args < argc; plot_args++)
    {
        channel_names.push_back(argv[plot_args]);
        // myColorPalette.push_back(gStyle->GetColorPalette((plot_args - 10) * ((float)gStyle->GetNumberOfColors() / ((float)argc - 10.))));
        myColorPalette.push_back(gStyle->GetColorPalette((plot_args) * ((float)gStyle->GetNumberOfColors() / ((float)argc))));
    }

    fs::path dir("./Waveform");   
    if (!(fs::exists(dir))) fs::create_directory(dir);

    fs::path dir2("./Waveform/Run_" + std::to_string(fRunNum));   
    if (!(fs::exists(dir2))) fs::create_directory(dir2);

    TCanvas* c = new TCanvas("c1", "c1");

    // Here we load mapping information to get cid (channel ID)
    TButility util = TButility();
    util.LoadMapping("../../mapping/mapping_TB2025_v1.root");
    
    std::vector<TBcid> cids;
    for (int idx = 0; idx < channel_names.size(); idx++) {
        cids.push_back(util.GetCID(channel_names.at(idx)));

        fs::path dir("./Waveform/Run_" + std::to_string(fRunNum) + "/" + channel_names.at(idx));   
        if (!(fs::exists(dir))) fs::create_directory(dir);
    }

    // MID: 3-7: PMT modules, MID 9: LC, MID 10: Aux(CC1, CC2, PS, TC, MC), MID 12: Triggers (T1, T2, T1NIM, T2NIM, Coin), MID 14-17: MCP micro, MID 18: DWC
    //TBread<TBwaveform> readerWave = TBread<TBwaveform>(fRunNum, fMaxEvent, fMaxFile, false, "/Volumes/Macintosh HD-1/Users/yhep/scratch/YUdaq", {3, 4, 5, 6, 7, 9, 10, 12, 14, 15, 16, 17, 18});
    //TBread<TBwaveform> readerWave = TBread<TBwaveform>(fRunNum, fMaxEvent, fMaxFile, false, "/Volumes/Macintosh HD-1/Volumes/HDD_16TB_3", {3, 4, 5, 6, 7, 9, 10, 12, 14, 15, 16, 17, 18});
    TFile* fNtuple = TFile::Open((TString)("/Users/yhep/DRC/TB2025/analysis/SW/Prompt_ntuple/Prompt_ntuple_Run_" + std::to_string(fRunNum) + ".root"), "READ");
    TTreeReader reader("evt", fNtuple);

    // Create TTreeReaderValue for each channel outside the loop
    std::vector<TTreeReaderValue<std::vector<short>>*> channel_readers;
    for (int idx = 0; idx < channel_names.size(); idx++) {
        std::string branch_name = channelToBranchName(channel_names.at(idx));
        channel_readers.push_back(new TTreeReaderValue<std::vector<short>>(reader, branch_name.c_str()));
    }

    // Set Maximum event
    //if (fMaxEvent == -1 || fMaxEvent > readerWave.GetMaxEvent())
    //    fMaxEvent = readerWave.GetMaxEvent();
    Long64_t totalEntries = reader.GetEntries();
    if(fMaxEvent == -1 || fMaxEvent > totalEntries) fMaxEvent = totalEntries;

    // Exercise 1 : Define cid of both Module 1 Tower 1 Cerenkov channel and Scintillation channel (generic PMT)
    TFile* outFile = new TFile( ( "./Waveform/Run_" + std::to_string(fRunNum) + "/Run_" + std::to_string(fRunNum) + ".root" ).c_str(), "RECREATE");
    outFile->cd();
    
    // Starting Evt Loop
    for (int iEvt = 0; iEvt < fMaxEvent; iEvt++) {
        // Get entry from ntuple TChain
        // Event data can be accessed from TBevt<TBwaveform>* anEvt
        printProgress(iEvt, fMaxEvent);
        //TBevt<TBwaveform> aEvent = readerWave.GetAnEvent();
        reader.SetEntry(iEvt);

        TLegend* leg = new TLegend(0.75, 0.2, 0.9, 0.4);
        std::vector<TH1F *> plots;
        for (int idx = 0; idx < channel_names.size(); idx++) {
            plots.push_back(new TH1F((TString) (channel_names.at(idx) + "_Evt_" + std::to_string(iEvt) + channel_names.at(idx)), ";bin;ADC", 1000, 0, 1000));
            plots.at(idx)->SetLineColor(myColorPalette.at(idx));
            plots.at(idx)->GetYaxis()->SetRangeUser(3000, 4096);
            // leg->AddEntry(plots.at(idx), (TString)channel_names.at(idx), "l");

        //auto single_waveform = aEvent.GetData(cids.at(idx)).waveform();
        auto single_waveform = **channel_readers.at(idx);

            for (int bin = 1; bin < 1001; bin++) {
                plots.at(idx)->SetBinContent(bin, single_waveform[bin]);
                plots.at(idx)->SetBinError(bin, 0);
            }
        }

        for (int idx = 0; idx < plots.size(); idx++) {
            outFile->cd();
            c->cd();
            c->RedrawAxis();
            plots.at(idx)->Draw("Hist");
            leg->AddEntry(plots.at(idx), (TString)channel_names.at(idx), "l");
            // if (idx == 0) plots.at(idx)->Draw("Hist");
            // else plots.at(idx)->Draw("Hist&sames");
            leg->Draw("sames");
            plots.at(idx)->Write();
            c->SaveAs( (TString) ( "./Waveform/Run_" + std::to_string(fRunNum) + "/" + channel_names.at(idx) + "/Evt_" + std::to_string(iEvt) + ".png") );
            c->Clear();
            leg->Clear();
        }
    }
    outFile->Close();
    
    // Clean up dynamically allocated TTreeReaderValue objects
    for (auto* reader_ptr : channel_readers) {
        delete reader_ptr;
    }
}
