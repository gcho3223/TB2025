#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <filesystem>

#include "TStyle.h"
#include "TChain.h"
#include "TFile.h"
#include "TH1.h"
#include "TH2.h"
#include "TCanvas.h"
#include "TGraph.h"

namespace fs = std::filesystem;

int main(){
    
    gStyle->SetPalette(kRainbow);

    TFile* f_pos_1 = TFile::Open("./Calib/Calib_Run_10578_PSMCDWC.root", "READ");
    TFile* f_pos_2 = TFile::Open("./Calib/Calib_Run_10579_PSMCDWC.root", "READ");
    TFile* f_pos_3 = TFile::Open("./Calib/Calib_Run_10580_PSMCDWC.root", "READ");
    TFile* f_pos_4 = TFile::Open("./Calib/Calib_Run_10577_PSMCDWC.root", "READ");
    TFile* f_pos_5 = TFile::Open("./Calib/Calib_Run_10576_PSMCDWC.root", "READ");
    TFile* f_pos_6 = TFile::Open("./Calib/Calib_Run_10581_PSMCDWC.root", "READ");
    TFile* f_pos_7 = TFile::Open("./Calib/Calib_Run_10584_PSMCDWC.root", "READ");
    TFile* f_pos_8 = TFile::Open("./Calib/Calib_Run_10583_PSMCDWC.root", "READ");
    TFile* f_pos_9 = TFile::Open("./Calib/Calib_Run_10582_PSMCDWC.root", "READ");

    TCanvas* c = new TCanvas("c","c", 900, 1000);
    c->cd();
    c->SetGrid(1, 1);
    c->SetRightMargin(0.2);
    // c->SetMinimum(0.);
    // c->SetMaximum(5.);

    TH2D* hist_map_C = new TH2D("C_energy", "M5T3 Tower uniformity  (C. ch.);;", 3, 0, 3, 3, 0, 3);
    hist_map_C->SetStats(0);
    hist_map_C->SetContour(15);

    hist_map_C->SetBinContent(1 ,3 , ((TH1D*) f_pos_1->Get("totalEdepScaled_C_after"))->GetMean() );
    hist_map_C->SetBinContent(2 ,3 , ((TH1D*) f_pos_2->Get("totalEdepScaled_C_after"))->GetMean() );
    hist_map_C->SetBinContent(3 ,3 , ((TH1D*) f_pos_3->Get("totalEdepScaled_C_after"))->GetMean() );
    hist_map_C->SetBinContent(1 ,2 , ((TH1D*) f_pos_4->Get("totalEdepScaled_C_after"))->GetMean() );
    hist_map_C->SetBinContent(2 ,2 , ((TH1D*) f_pos_5->Get("totalEdepScaled_C_after"))->GetMean() );
    hist_map_C->SetBinContent(3 ,2 , ((TH1D*) f_pos_6->Get("totalEdepScaled_C_after"))->GetMean() );
    hist_map_C->SetBinContent(1 ,1 , ((TH1D*) f_pos_7->Get("totalEdepScaled_C_after"))->GetMean() );
    hist_map_C->SetBinContent(2 ,1 , ((TH1D*) f_pos_8->Get("totalEdepScaled_C_after"))->GetMean() );
    hist_map_C->SetBinContent(3 ,1 , ((TH1D*) f_pos_9->Get("totalEdepScaled_C_after"))->GetMean() );

    hist_map_C->GetXaxis()->ChangeLabel(1, -1, -1, -1, -1, -1, " ");
    hist_map_C->GetXaxis()->ChangeLabel(2, -1, -1, -1, -1, -1, " ");
    hist_map_C->GetXaxis()->ChangeLabel(3, -1, -1, -1, -1, -1, " ");
    hist_map_C->GetXaxis()->ChangeLabel(4, -1, -1, -1, -1, -1, " ");

    hist_map_C->GetYaxis()->ChangeLabel(1, -1, -1, -1, -1, -1, " ");
    hist_map_C->GetYaxis()->ChangeLabel(2, -1, -1, -1, -1, -1, " ");
    hist_map_C->GetYaxis()->ChangeLabel(3, -1, -1, -1, -1, -1, " ");
    hist_map_C->GetYaxis()->ChangeLabel(4, -1, -1, -1, -1, -1, " ");

    hist_map_C->GetZaxis()->SetRangeUser(70, 130);
    hist_map_C->GetZaxis()->SetTitle("Measured energy (GeV)");
    hist_map_C->SetNdivisions(3, "X");
    hist_map_C->SetNdivisions(3, "Y");

    hist_map_C->Draw("colz & text");
    
    c->SaveAs("./Uniformity_scan/map_M5T3_C.png");
    // c->SaveAs("./Uniformity_scan/map_M5T3_C.pdf");


    TH2D* hist_map_S = new TH2D("S_energy", "M5T3 Tower uniformity  (S. ch.);;", 3, 0, 3, 3, 0, 3);
    hist_map_S->SetStats(0);
    hist_map_S->SetContour(15);

    hist_map_S->SetBinContent(1 ,3 , ((TH1D*) f_pos_1->Get("totalEdepScaled_S_after"))->GetMean() );
    hist_map_S->SetBinContent(2 ,3 , ((TH1D*) f_pos_2->Get("totalEdepScaled_S_after"))->GetMean() );
    hist_map_S->SetBinContent(3 ,3 , ((TH1D*) f_pos_3->Get("totalEdepScaled_S_after"))->GetMean() );
    hist_map_S->SetBinContent(1 ,2 , ((TH1D*) f_pos_4->Get("totalEdepScaled_S_after"))->GetMean() );
    hist_map_S->SetBinContent(2 ,2 , ((TH1D*) f_pos_5->Get("totalEdepScaled_S_after"))->GetMean() );
    hist_map_S->SetBinContent(3 ,2 , ((TH1D*) f_pos_6->Get("totalEdepScaled_S_after"))->GetMean() );
    hist_map_S->SetBinContent(1 ,1 , ((TH1D*) f_pos_7->Get("totalEdepScaled_S_after"))->GetMean() );
    hist_map_S->SetBinContent(2 ,1 , ((TH1D*) f_pos_8->Get("totalEdepScaled_S_after"))->GetMean() );
    hist_map_S->SetBinContent(3 ,1 , ((TH1D*) f_pos_9->Get("totalEdepScaled_S_after"))->GetMean() );

    hist_map_S->GetXaxis()->ChangeLabel(1, -1, -1, -1, -1, -1, " ");
    hist_map_S->GetXaxis()->ChangeLabel(2, -1, -1, -1, -1, -1, " ");
    hist_map_S->GetXaxis()->ChangeLabel(3, -1, -1, -1, -1, -1, " ");
    hist_map_S->GetXaxis()->ChangeLabel(4, -1, -1, -1, -1, -1, " ");

    hist_map_S->GetYaxis()->ChangeLabel(1, -1, -1, -1, -1, -1, " ");
    hist_map_S->GetYaxis()->ChangeLabel(2, -1, -1, -1, -1, -1, " ");
    hist_map_S->GetYaxis()->ChangeLabel(3, -1, -1, -1, -1, -1, " ");
    hist_map_S->GetYaxis()->ChangeLabel(4, -1, -1, -1, -1, -1, " ");

    hist_map_S->GetZaxis()->SetRangeUser(70, 130);
    hist_map_S->GetZaxis()->SetTitle("Measured energy (GeV)");
    hist_map_S->SetNdivisions(3, "X");
    hist_map_S->SetNdivisions(3, "Y");

    hist_map_S->Draw("colz & text");
    
    c->SaveAs("./Uniformity_scan/map_M5T3_S.png");
    // c->SaveAs("./Uniformity_scan/map_M5T3_S.pdf");

    return 0;   
}
