#include "TBread.h"
#include "TButility.h"

#include <filesystem>
#include <iostream>
#include <fstream>
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
#include "TH2.h"
#include "TPaveStats.h"
#include "function.h"

namespace fs = std::__fs::filesystem;

int main(int argc, char** argv) {

    std::string fRunNum    = argv[1];
    std::string fModuleNum = argv[2];
    std::string fTowerNum  = argv[3];

    fs::path dir("./Calc_calib");   
    if (!(fs::exists(dir))) fs::create_directory(dir);

    // std::string calib_file = "./Single_Module_Calib/PS_cut_1000/M" + (fModuleNum) + "/Calib_Run_" + fRunNum + "_PSMC.root";
    std::string calib_file = "./Single_Module_Calib/M" + (fModuleNum) + "/Calib_Run_" + fRunNum + ".root";
    TFile* f = new TFile(calib_file.c_str(), "READ");
    f->cd();

    // Calculate equalization constant based on 20GeV e- beam

    TH1F* hist_Tower_1_C = (TH1F*) f->Get( (TString) ("M" + fModuleNum + "_T" + fTowerNum + "_C_after") );
    TH1F* hist_Tower_1_S = (TH1F*) f->Get( (TString) ("M" + fModuleNum + "_T" + fTowerNum + "_S_after") );

    TCanvas* c = new TCanvas("c", "c", 1000, 800);
    c->cd();

    std::cout << "here" << std::endl;
    hist_Tower_1_C->SetLineColor(kBlue);
    hist_Tower_1_S->SetLineColor(kRed);

    hist_Tower_1_C->Draw("Hist"); c->Update();
    TPaveStats* stats_T1_C = (TPaveStats*) hist_Tower_1_C->FindObject("stats");
    stats_T1_C->SetTextColor(kBlue);
    stats_T1_C->SetX1NDC(0.7);
    stats_T1_C->SetX2NDC(0.9);
    stats_T1_C->SetY1NDC(0.7);
    stats_T1_C->SetY2NDC(0.9);

    hist_Tower_1_S->Draw("Hist&sames"); c->Update();
    TPaveStats* stats_T1_S = (TPaveStats*) hist_Tower_1_S->FindObject("stats");
    stats_T1_S->SetTextColor(kRed);
    stats_T1_S->SetX1NDC(0.7);
    stats_T1_S->SetX2NDC(0.9);
    stats_T1_S->SetY1NDC(0.5);
    stats_T1_S->SetY2NDC(0.7);
    c->SaveAs((TString) ("./Single_Module_Calib/M" + (fModuleNum) + "/Plot_Run_" + fRunNum + "_T" + fTowerNum + ".png"));

    return 0;
}
