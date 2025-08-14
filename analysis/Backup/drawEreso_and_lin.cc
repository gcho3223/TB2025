#include "TROOT.h"
#include "TStyle.h"
#include "TH1.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TPaveStats.h"
#include "TString.h"
#include "TGraphErrors.h"
#include "TChain.h"
#include "TFile.h"
#include "TLegend.h"
#include "TLegendEntry.h"
#include "TLatex.h"
#include "TSystem.h"

#include <utility>
#include <vector>
#include <iostream>
#include <string>
#include <filesystem>

// Run this macro by `root drawEreso.cc` in your command line

// cf) Fitting function parameters of gaussian fit function
// GetParameter(0) : Constant, no need for this code
// GetParameter(1) : Mean
// GetParameter(2) : StdDev

// cf) Errors related with gaussian fit function
// GetParError(0) : Error of constant ( parameter(0) )
// GetParError(1) : Error of mean     ( parameter(1) )
// GetParError(2) : Error of StdDev   ( parameter(2) )

// Receive fitting function, and calculate propagated error of (sig / Mean)
float CalculateError(TF1* fit_func) {
    float mean = fit_func->GetParameter(1); // Get mean of fit function
    float sig  = fit_func->GetParameter(2); // Get StdDev of fit function

    float meanErr = fit_func->GetParError(1); // Get mean error of fit function
    float sigErr  = fit_func->GetParError(2); // Get StdDev error of fit function

    float err = std::sqrt( std::pow( (sigErr / mean) , 2) + std::pow( ( - (sig / std::pow(mean, 2)) * (meanErr) ) , 2) ); // Error propagation

    return err;
}

// Main function to run
void drawEreso_and_lin()
{
    gStyle->SetOptFit(0); // No fitting info
    gStyle->SetOptStat(0); // No stat box

    // Canvas for drawing E reso. and E lin. plot (EMResolution 스타일: 800x600)
    TCanvas* c = new TCanvas("c", "c", 800, 600);
    c->SetGrid(1, 1); // 그리드 활성화

    // EMResolution 스타일 디렉토리 구조 생성
    TString plotPath = "./EResolution/em";
    std::filesystem::create_directories(plotPath.Data());

    // Loading root file, change to your root file
    // Note: 이 파일들은 피팅 결과 파일이므로 경로를 그대로 유지
    TFile *f0p5 = new TFile(Form("./Fit/Fit_Run_0p5GeV.root"), "read");
    TFile *f0p75 = new TFile(Form("./Fit/Fit_Run_0p75GeV.root"), "read");
    TFile *f1 = new TFile(Form("./Fit/Fit_Run_4238.root"), "read");
    TFile *f2 = new TFile(Form("./Fit/Fit_Run_4237.root"), "read");
    TFile *f3 = new TFile(Form("./Fit/Fit_Run_4236.root"), "read");
    TFile *f4 = new TFile(Form("./Fit/Fit_Run_4235.root"), "read");
    TFile *f5 = new TFile(Form("./Fit/Fit_Run_4234.root"), "read");

    // 파일들이 존재하는지 확인
    if(!f0p5 || f0p5->IsZombie()) { printf("Error: Cannot open ./Fit/Fit_Run_0p5GeV.root\n"); return; }
    if(!f0p75 || f0p75->IsZombie()) { printf("Error: Cannot open ./Fit/Fit_Run_0p75GeV.root\n"); return; }
    if(!f1 || f1->IsZombie()) { printf("Error: Cannot open ./Fit/Fit_Run_4238.root\n"); return; }
    if(!f2 || f2->IsZombie()) { printf("Error: Cannot open ./Fit/Fit_Run_4237.root\n"); return; }
    if(!f3 || f3->IsZombie()) { printf("Error: Cannot open ./Fit/Fit_Run_4236.root\n"); return; }
    if(!f4 || f4->IsZombie()) { printf("Error: Cannot open ./Fit/Fit_Run_4235.root\n"); return; }
    if(!f5 || f5->IsZombie()) { printf("Error: Cannot open ./Fit/Fit_Run_4234.root\n"); return; }

    // Load Calib mean fit func
    TF1* f_C_0p5 = (TF1*) f0p5->Get("gFit_C");
    TF1* f_C_0p75 = (TF1*) f0p75->Get("gFit_C");
    TF1* f_C_1 = (TF1*) f1->Get("gFit_C");
    TF1* f_C_2 = (TF1*) f2->Get("gFit_C");
    TF1* f_C_3 = (TF1*) f3->Get("gFit_C");
    TF1* f_C_4 = (TF1*) f4->Get("gFit_C");
    TF1* f_C_5 = (TF1*) f5->Get("gFit_C");

    TF1* f_S_0p5 = (TF1*) f0p5->Get("gFit_S");
    TF1* f_S_0p75 = (TF1*) f0p75->Get("gFit_S");
    TF1* f_S_1 = (TF1*) f1->Get("gFit_S");
    TF1* f_S_2 = (TF1*) f2->Get("gFit_S");
    TF1* f_S_3 = (TF1*) f3->Get("gFit_S");
    TF1* f_S_4 = (TF1*) f4->Get("gFit_S");
    TF1* f_S_5 = (TF1*) f5->Get("gFit_S");

    TF1* f_Sum_0p5 = (TF1*) f0p5->Get("gFit_Sum");
    TF1* f_Sum_0p75 = (TF1*) f0p75->Get("gFit_Sum");
    TF1* f_Sum_1 = (TF1*) f1->Get("gFit_Sum");
    TF1* f_Sum_2 = (TF1*) f2->Get("gFit_Sum");
    TF1* f_Sum_3 = (TF1*) f3->Get("gFit_Sum");
    TF1* f_Sum_4 = (TF1*) f4->Get("gFit_Sum");
    TF1* f_Sum_5 = (TF1*) f5->Get("gFit_Sum");

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // We first draw E-linearity plot using TGraph
    // To do so, we create arrays of X and Y coordinates

    // How to fill TGraph, and draw them (graph with no associated error bars)
    // TGraph* graph = new TGraph( # of points, array of X coordinate, array of Y coordinate);

    // # of energy points, in our case its 7 (0.5, 0.75, 1, 2, 3, 4, 5 GeV)
    const Int_t n = 7;

    // Linear_C
    // C ch. X coordinate for E linearity plot
    Double_t Lin_C_x[n] = { 0.5 ,
                            0.75 ,
                            1. ,
                            2. ,
                            3. ,
                            4. ,
                            5. };
    // C ch. Y coordinate for E linearity plot
    Double_t Lin_C_y[n] = { f_C_0p5->GetParameter(1) / 0.5, // parameter(1) == mean value of the fit function
                            f_C_0p75->GetParameter(1) / 0.75,
                            f_C_1->GetParameter(1) / 1.,
                            f_C_2->GetParameter(1) / 2.,
                            f_C_3->GetParameter(1) / 3.,
                            f_C_4->GetParameter(1) / 4.,
                            f_C_5->GetParameter(1) / 5. };

    // Linear_S
    Double_t Lin_S_x[n] = { 0.5 ,
                            0.75 ,
                            1. ,
                            2. ,
                            3. ,
                            4. ,
                            5. };

    Double_t Lin_S_y[n] = { f_S_0p5->GetParameter(1) / 0.5,
                            f_S_0p75->GetParameter(1) / 0.75,
                            f_S_1->GetParameter(1) / 1.,
                            f_S_2->GetParameter(1) / 2.,
                            f_S_3->GetParameter(1) / 3.,
                            f_S_4->GetParameter(1) / 4.,
                            f_S_5->GetParameter(1) / 5. };

    // Linear_Sum
    Double_t Lin_SC_x[n] = { 0.5 ,
                            0.75 ,
                            1. ,
                            2. ,
                            3. ,
                            4. ,
                            5. };

    Double_t Lin_SC_y[n] = { f_Sum_0p5->GetParameter(1) / 0.5,
                             f_Sum_0p75->GetParameter(1) / 0.75,
                             f_Sum_1->GetParameter(1) / 1.,
                             f_Sum_2->GetParameter(1) / 2.,
                             f_Sum_3->GetParameter(1) / 3.,
                             f_Sum_4->GetParameter(1) / 4.,
                             f_Sum_5->GetParameter(1) / 5. };

    // Next we declare the TGraph to draw C ch. E linearity
    TGraph* lin_g_C = new TGraph(n, Lin_C_x, Lin_C_y);
    lin_g_C->SetName("lin_g_C");
    lin_g_C->GetYaxis()->SetRangeUser(0.5, 1.5); // Set Y axis range
    lin_g_C->SetMarkerColor(kBlue);             // Draw blue marker
    lin_g_C->SetMarkerStyle(21);                // Draw with square marker
    lin_g_C->SetTitle(";E [GeV];E_{measured}/E_{beam}"); // Title formatting matching EMResolution style
    lin_g_C->SetMarkerSize(1);
    lin_g_C->SetLineWidth(2);
    lin_g_C->Draw("AP"); // Draw with marker and no line

    TGraph* lin_g_S = new TGraph(n, Lin_S_x, Lin_S_y);
    lin_g_S->SetName("lin_g_S");
    lin_g_S->SetMarkerColor(kRed);
    lin_g_S->SetMarkerStyle(21);
    lin_g_S->SetMarkerSize(1);
    lin_g_S->SetLineWidth(2);
    lin_g_S->Draw("P & sames"); // sames option will draw S ch. on same canvas with C ch

    TGraph* lin_g_SC = new TGraph(n, Lin_SC_x, Lin_SC_y);
    lin_g_SC->SetName("lin_g_SC");
    lin_g_SC->SetMarkerColor(kBlack);
    lin_g_SC->SetMarkerStyle(21);
    lin_g_SC->SetMarkerSize(1);
    lin_g_SC->SetLineWidth(2);
    lin_g_SC->Draw("P & sames");

    // EMResolution 스타일 레전드
    TLegend* legend_linear = new TLegend(0.50, 0.70, 0.95, 0.95);
    legend_linear->SetTextFont(42);
    legend_linear->SetTextSize(0.04);
    legend_linear->SetBorderSize(0);
    legend_linear->SetFillStyle(0);
    
    TLegendEntry* le_lin_C = legend_linear->AddEntry("lin_g_C", "C ", "lep");
    le_lin_C->SetTextColor(kBlue);
    TLegendEntry* le_lin_S = legend_linear->AddEntry("lin_g_S", "S ", "lep");
    le_lin_S->SetTextColor(kRed);
    TLegendEntry* le_lin_SC = legend_linear->AddEntry("lin_g_SC", "C+S ", "lep");
    le_lin_SC->SetTextColor(kBlack);
    legend_linear->Draw();

    // EMResolution 스타일 텍스트 추가
    TLatex* latex = new TLatex();
    latex->SetNDC();
    latex->SetTextFont(62);
    latex->SetTextSize(0.8*0.05);
    latex->DrawLatex(0.14, 0.96, "TB2025");
    
    latex->SetTextFont(62);
    latex->SetTextSize(0.6*0.05);
    latex->DrawLatex(0.65, 0.96, "e+ beam (0.5 - 5 GeV)");
    
    latex->SetTextFont(42);
    latex->SetTextSize(0.6*0.05);
    latex->DrawLatex(0.16, 0.91, "EM Linearity");

    // EMResolution 스타일 저장 경로 사용
    TString linearitySavePath = TString::Format("%s/c_emLinearity_set3.pdf", plotPath.Data());
    c->SaveAs(linearitySavePath);

    ////////////////////////////////////////////////////////////////////////////////////////////
    // Next, we draw E resolution plot using TGraphErrors (피팅 제거)

    // part for E_C
    // E_C x coordinate
    Double_t E_C_x[n] = {   1. / std::sqrt(0.5) ,
                            1. / std::sqrt(0.75) ,
                            1. / std::sqrt(1.) ,
                            1. / std::sqrt(2.) ,
                            1. / std::sqrt(3.) ,
                            1. / std::sqrt(4.) ,
                            1. / std::sqrt(5.) };
    // E_C y coordinate
    Double_t E_C_y[n] = {   f_C_0p5->GetParameter(2) / f_C_0p5->GetParameter(1), // parameter(2) == sigma, parameter(1) == mean
                            f_C_0p75->GetParameter(2) / f_C_0p75->GetParameter(1),
                            f_C_1->GetParameter(2) / f_C_1->GetParameter(1),
                            f_C_2->GetParameter(2) / f_C_2->GetParameter(1),
                            f_C_3->GetParameter(2) / f_C_3->GetParameter(1),
                            f_C_4->GetParameter(2) / f_C_4->GetParameter(1),
                            f_C_5->GetParameter(2) / f_C_5->GetParameter(1) };

    Double_t E_C_xe[n] = { 0, 0, 0, 0, 0, 0 ,0}; // No error with x axis

    Double_t E_C_ye[n] = {  CalculateError(f_C_0p5), // Calculate error with y axis
                            CalculateError(f_C_0p75),
                            CalculateError(f_C_1),
                            CalculateError(f_C_2),
                            CalculateError(f_C_3),
                            CalculateError(f_C_4),
                            CalculateError(f_C_5) };

    // same goes for E_S
    Double_t E_S_x[n] = {   1. / std::sqrt(0.5) ,
                            1. / std::sqrt(0.75) ,
                            1. / std::sqrt(1.) ,
                            1. / std::sqrt(2.) ,
                            1. / std::sqrt(3.) ,
                            1. / std::sqrt(4.) ,
                            1. / std::sqrt(5.) };

    Double_t E_S_y[n] = {   f_S_0p5->GetParameter(2) / f_S_0p5->GetParameter(1),
                            f_S_0p75->GetParameter(2) / f_S_0p75->GetParameter(1),
                            f_S_1->GetParameter(2) / f_S_1->GetParameter(1),
                            f_S_2->GetParameter(2) / f_S_2->GetParameter(1),
                            f_S_3->GetParameter(2) / f_S_3->GetParameter(1),
                            f_S_4->GetParameter(2) / f_S_4->GetParameter(1),
                            f_S_5->GetParameter(2) / f_S_5->GetParameter(1) };

    Double_t E_S_xe[n] = { 0, 0, 0, 0, 0, 0 ,0};

    Double_t E_S_ye[n] = {  CalculateError(f_S_0p5),
                            CalculateError(f_S_0p75),
                            CalculateError(f_S_1),
                            CalculateError(f_S_2),
                            CalculateError(f_S_3),
                            CalculateError(f_S_4),
                            CalculateError(f_S_5) };

    // E_SC
    Double_t E_SC_x[n] = {  1. / std::sqrt(0.5) ,
                            1. / std::sqrt(0.75) ,
                            1. / std::sqrt(1.) ,
                            1. / std::sqrt(2.) ,
                            1. / std::sqrt(3.) ,
                            1. / std::sqrt(4.) ,
                            1. / std::sqrt(5.) };

    Double_t E_SC_y[n] = {  f_Sum_0p5->GetParameter(2) / f_Sum_0p5->GetParameter(1),
                            f_Sum_0p75->GetParameter(2) / f_Sum_0p75->GetParameter(1),
                            f_Sum_1->GetParameter(2) / f_Sum_1->GetParameter(1),
                            f_Sum_2->GetParameter(2) / f_Sum_2->GetParameter(1),
                            f_Sum_3->GetParameter(2) / f_Sum_3->GetParameter(1),
                            f_Sum_4->GetParameter(2) / f_Sum_4->GetParameter(1),
                            f_Sum_5->GetParameter(2) / f_Sum_5->GetParameter(1) };

    Double_t E_SC_xe[n] = { 0, 0, 0, 0, 0, 0 ,0};

    Double_t E_SC_ye[n] = { CalculateError(f_Sum_0p5),
                            CalculateError(f_Sum_0p75),
                            CalculateError(f_Sum_1),
                            CalculateError(f_Sum_2),
                            CalculateError(f_Sum_3),
                            CalculateError(f_Sum_4),
                            CalculateError(f_Sum_5) };

    c->Clear(); // Clear canvas (E lin. plot already drawn on canvas, so we remove it)

    TGraphErrors* gr_C = new TGraphErrors(n, E_C_x, E_C_y, E_C_xe, E_C_ye); // Declare C ch E resolution plot with TGraphErrors
    gr_C->SetName("gr_C");
    gr_C->GetYaxis()->SetRangeUser(0., .5);
    gr_C->SetLineColor(kBlue);
    gr_C->SetMarkerColor(kBlue);
    gr_C->SetMarkerStyle(21);
    gr_C->SetTitle(";1/#sqrt{E};#sigma/E"); // EMResolution 스타일 타이틀
    gr_C->SetMarkerSize(1);
    gr_C->SetLineWidth(2);
    gr_C->Draw("AP RX"); // RX means reversed X-axis

    TGraphErrors* gr_S = new TGraphErrors(n, E_S_x, E_S_y, E_S_xe, E_S_ye);
    gr_S->SetName("gr_S");
    gr_S->SetLineColor(kRed);
    gr_S->SetMarkerColor(kRed);
    gr_S->SetMarkerStyle(21);
    gr_S->SetMarkerSize(1);
    gr_S->SetLineWidth(2);
    gr_S->Draw("P RX & sames");

    TGraphErrors* gr_SC = new TGraphErrors(n, E_SC_x, E_SC_y, E_SC_xe, E_SC_ye);
    gr_SC->SetName("gr_SC");
    gr_SC->SetLineColor(kBlack);
    gr_SC->SetMarkerColor(kBlack);
    gr_SC->SetMarkerStyle(21);
    gr_SC->SetMarkerSize(1);
    gr_SC->SetLineWidth(2);
    gr_SC->Draw("P RX & sames");

    // This part is for drawing legend (EMResolution 스타일)
    TLegend* legend = new TLegend(0.35, 0.70, 0.65, 0.90);
    legend->SetTextFont(42);
    legend->SetTextSize(0.04);
    legend->SetBorderSize(0);
    legend->SetFillStyle(0);

    TLegendEntry* le_C = legend->AddEntry("gr_C", "C ", "lep");
    le_C->SetTextColor(kBlue);
    TLegendEntry* le_S = legend->AddEntry("gr_S", "S ", "lep");
    le_S->SetTextColor(kRed);
    TLegendEntry* le_SC = legend->AddEntry("gr_SC", "C+S ", "lep");
    le_SC->SetTextColor(kBlack);
    legend->Draw();

    // EMResolution 스타일 텍스트
    latex->DrawLatex(0.14, 0.96, "TB2025");
    latex->DrawLatex(0.65, 0.96, "e+ beam (0.5 - 5 GeV)");
    latex->DrawLatex(0.16, 0.91, "EM Energy Resolution");

    gPad->SetGrid(1, 1);

    // EMResolution 스타일 저장 경로 사용
    TString resolSavePath = TString::Format("%s/c_emResol_set3.pdf", plotPath.Data());
    c->SaveAs(resolSavePath);
    
    printf("Plots saved to: %s\n", plotPath.Data());
    printf("  - Linearity plot: %s\n", linearitySavePath.Data());
    printf("  - Resolution plot: %s\n", resolSavePath.Data());
    
    delete latex;
    
    // 파일들 닫기
    f0p5->Close();
    f0p75->Close();
    f1->Close();
    f2->Close();
    f3->Close();
    f4->Close();
    f5->Close();
}
