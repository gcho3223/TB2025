#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <cmath>

using namespace std;

#include "TStyle.h"
#include "TFile.h"
#include "TH1.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TLegendEntry.h"
#include "TPaveStats.h"
#include "TString.h"
#include "TLatex.h"

// PlotTool 헤더 추가
#include "plots/header/SimplePlotTools.h"

int main(int argc, char* argv[]) {
    gStyle->SetOptFit(0); // 피팅 정보 표시 제거
    gStyle->SetOptStat(0); // 통계 정보 박스 제거

    if(argc < 3) {
        printf("Usage: %s <energy> <beamType>\n", argv[0]);
        printf("energy: beam energy in GeV (e.g., 120)\n");
        printf("beamType: em, pi, kaon, proton\n");
        return 1;
    }

    double energy = std::stod(argv[1]);
    std::string beamType = argv[2];

    // 센터 타워를 M5-T2로 고정
    std::string centerTowerTag = "M5-T2";
    std::string towerName = "M5T2";

    // beam type 유효성 검사
    if(beamType != "em" && beamType != "pi" && beamType != "kaon" && beamType != "proton") {
        printf("Error: Invalid beam type '%s'. Must be one of: em, pi, kaon, proton\n", beamType.c_str());
        return 1;
    }

    // 파일 경로 설정
    //TString fileName = TString::Format("./output/%s/merged/%s_%s_%.0lfGeV.root", beamType.c_str(), beamType.c_str(), towerName.c_str(), energy);
    TString fileName = TString::Format("./output/pi/pi_M5T2_12352.root");
    
    TFile* f = TFile::Open(fileName, "READ");
    if(!f || f->IsZombie()) {
        printf("Error: Cannot open file %s\n", fileName.Data());
        return 1;
    }
    f->cd();

    // 히스토그램 이름들
    TH1D* hist_C = nullptr;
    TH1D* hist_S = nullptr;
    TH1D* hist_Comb = nullptr;
    TH1D* hist_SLC = nullptr;
    TH1D* hist_CombLC = nullptr;
    TH1D* hist_CombDRC = nullptr;
    TH1D* hist_CombDRCLC = nullptr;

    if(beamType == "em") {
        hist_C    = (TH1D*) f->Get("totalEdepScaled_C_5_MC");     // C channel, scaled, final cuts
        hist_S    = (TH1D*) f->Get("totalEdepScaled_S_5_MC");     // S channel, scaled, final cuts
        hist_Comb = (TH1D*) f->Get("totalEdepScaled_Comb_5_MC");  // Combined channel, scaled, final cuts
    } else {
        // 하드론 빔의 경우 (pi, kaon, proton)
        hist_C    = (TH1D*) f->Get("totalEdepScaled_C_6_MC");           // C channel
        hist_S    = (TH1D*) f->Get("totalEdepScaled_S_6_MC");           // S channel
        hist_SLC  = (TH1D*) f->Get("totalEdepScaled_S_LC_compensated_6_MC");           // S channel
        hist_Comb = (TH1D*) f->Get("totalEdepScaled_Comb_6_MC");             // Combined channel
        hist_CombLC = (TH1D*) f->Get("totalEdepScaled_Comb_LC_compensated_6_MC");             // Combined channel
        hist_CombDRC = (TH1D*) f->Get("totalEdepScaled_Comb_DRCorrection_6_MC");             // Combined channel
        hist_CombDRCLC = (TH1D*) f->Get("totalEdepScaled_Comb_DRCorrection_LC_compensated_6_MC");             // Combined channel
    }

    if(!hist_C || !hist_S || !hist_Comb) {
        printf("Error: Cannot find required histograms in file %s\n", fileName.Data());
        printf("Looking for histograms for beam type: %s\n", beamType.c_str());
        f->Close();
        return 1;
    }

    // 플롯 디렉토리 생성 (EMResolution 스타일)
    TString plotPath = TString::Format("./plots/EResolution/%s", beamType.c_str());
    std::filesystem::create_directories(plotPath.Data());

    // 빔 정보 설정
    TString beamInfo;
    if(beamType == "em") {
        beamInfo = TString::Format("e+ beam (%.0lf GeV)", energy);
    } else if(beamType == "pi") {
        beamInfo = TString::Format("#pi^{+} beam (%.0lf GeV)", energy);
    } else if(beamType == "kaon") {
        beamInfo = TString::Format("#K^{+} beam (%.0lf GeV)", energy);
    } else if(beamType == "proton") {
        beamInfo = TString::Format("proton beam (%.0lf GeV)", energy);
    }

    // C 채널 플롯 (PlotTool::HistCanvas 사용)
    TString canvasName_C = TString::Format("eDep_C_energy%.0lf_%s", energy, towerName.c_str());
    PlotTool::HistCanvas* canvas_C = new PlotTool::HistCanvas(canvasName_C, 0, 0);
    canvas_C->SetTitle("Energy deposit [GeV]", "# events");
    
    canvas_C->Register(hist_C, "", kBlue);
    canvas_C->SetLegendPosition(0, 0, 0, 0); // 범례 숨김
    canvas_C->SetAutoRangeY();
    
    // 텍스트 라벨 추가
    canvas_C->RegisterLatex(0.14, 0.96, 62, 0.8, "TB2025");
    canvas_C->RegisterLatex(0.65, 0.96, 62, 0.6, beamInfo);
    
    TString centerInfo = "Center: " + (TString)centerTowerTag;
    canvas_C->RegisterLatex(0.75, 0.91, 42, 0.6, centerInfo);
    
    TString runInfo_C = TString::Format("C channel, %.0lf GeV", energy);
    canvas_C->RegisterLatex(0.16, 0.91, 42, 0.6, runInfo_C);
    
    // C 채널 통계 정보 계산 및 표시
    double mean_C = hist_C->GetMean();
    double stdDev_C = hist_C->GetStdDev();
    double resol_C = stdDev_C / mean_C;
    double absUnc_mean_C = hist_C->GetMeanError();
    double absUnc_stdDev_C = hist_C->GetStdDevError();
    double absUnc_resol_C = sqrt(pow(absUnc_stdDev_C/mean_C, 2) + pow(stdDev_C * absUnc_mean_C / (mean_C*mean_C), 2));
    
    TString histInfo_C = TString::Format("Hist. statistics (# events = %.0lf):", hist_C->Integral(0, hist_C->GetNbinsX()+1));
    canvas_C->RegisterLatex(0.16, 0.87, 62, 0.6, histInfo_C);
    TString muInfo_C = TString::Format("mean = %.2lf #pm %.2lf GeV", mean_C, absUnc_mean_C);
    TString sigmaInfo_C = TString::Format("std.dev. = %.2lf #pm %.2lf GeV", stdDev_C, absUnc_stdDev_C);
    TString resolInfo_C = TString::Format("std.dev/mean = %.2lf #pm %.2lf", resol_C, absUnc_resol_C);
    canvas_C->RegisterLatex(0.16, 0.83, 42, 0.6, muInfo_C);
    canvas_C->RegisterLatex(0.16, 0.79, 42, 0.6, sigmaInfo_C);
    canvas_C->RegisterLatex(0.16, 0.75, 42, 0.6, resolInfo_C);
    
    canvas_C->SetSavePath(plotPath);
    canvas_C->Draw("hist");

    // S 채널 플롯
    TString canvasName_S = TString::Format("eDep_S_energy%.0lf_%s", energy, towerName.c_str());
    PlotTool::HistCanvas* canvas_S = new PlotTool::HistCanvas(canvasName_S, 0, 0);
    canvas_S->SetTitle("Energy deposit [GeV]", "# events");
    
    canvas_S->Register(hist_S, "", kRed);
    canvas_S->SetLegendPosition(0, 0, 0, 0);
    canvas_S->SetAutoRangeY();
    
    canvas_S->RegisterLatex(0.14, 0.96, 62, 0.8, "TB2025");
    canvas_S->RegisterLatex(0.65, 0.96, 62, 0.6, beamInfo);
    canvas_S->RegisterLatex(0.75, 0.91, 42, 0.6, centerInfo);
    
    TString runInfo_S = TString::Format("S channel, %.0lf GeV", energy);
    canvas_S->RegisterLatex(0.16, 0.91, 42, 0.6, runInfo_S);
    
    // S 채널 통계 정보 계산 및 표시
    double mean_S = hist_S->GetMean();
    double stdDev_S = hist_S->GetStdDev();
    double resol_S = stdDev_S / mean_S;
    double absUnc_mean_S = hist_S->GetMeanError();
    double absUnc_stdDev_S = hist_S->GetStdDevError();
    double absUnc_resol_S = sqrt(pow(absUnc_stdDev_S/mean_S, 2) + pow(stdDev_S * absUnc_mean_S / (mean_S*mean_S), 2));
    
    TString histInfo_S = TString::Format("Hist. statistics (# events = %.0lf):", hist_S->Integral(0, hist_S->GetNbinsX()+1));
    canvas_S->RegisterLatex(0.16, 0.87, 62, 0.6, histInfo_S);
    TString muInfo_S = TString::Format("mean = %.2lf #pm %.2lf GeV", mean_S, absUnc_mean_S);
    TString sigmaInfo_S = TString::Format("std.dev. = %.2lf #pm %.2lf GeV", stdDev_S, absUnc_stdDev_S);
    TString resolInfo_S = TString::Format("std.dev/mean = %.2lf #pm %.2lf", resol_S, absUnc_resol_S);
    canvas_S->RegisterLatex(0.16, 0.83, 42, 0.6, muInfo_S);
    canvas_S->RegisterLatex(0.16, 0.79, 42, 0.6, sigmaInfo_S);
    canvas_S->RegisterLatex(0.16, 0.75, 42, 0.6, resolInfo_S);
    
    canvas_S->SetSavePath(plotPath);
    canvas_S->Draw("hist");

    // Combined 채널 플롯
    TString canvasName_Comb = TString::Format("eDep_Comb_energy%.0lf_%s", energy, towerName.c_str());
    PlotTool::HistCanvas* canvas_Comb = new PlotTool::HistCanvas(canvasName_Comb, 0, 0);
    canvas_Comb->SetTitle("Energy deposit [GeV]", "# events");
    
    canvas_Comb->Register(hist_Comb, "", kBlack);
    canvas_Comb->SetLegendPosition(0, 0, 0, 0);
    canvas_Comb->SetAutoRangeY();
    
    canvas_Comb->RegisterLatex(0.14, 0.96, 62, 0.8, "TB2025");
    canvas_Comb->RegisterLatex(0.65, 0.96, 62, 0.6, beamInfo);
    canvas_Comb->RegisterLatex(0.75, 0.91, 42, 0.6, centerInfo);
    
    TString runInfo_Comb = TString::Format("Combined channel, %.0lf GeV", energy);
    canvas_Comb->RegisterLatex(0.16, 0.91, 42, 0.6, runInfo_Comb);
    
    // Combined 채널 통계 정보 계산 및 표시
    double mean_Comb = hist_Comb->GetMean();
    double stdDev_Comb = hist_Comb->GetStdDev();
    double resol_Comb = stdDev_Comb / mean_Comb;
    double absUnc_mean_Comb = hist_Comb->GetMeanError();
    double absUnc_stdDev_Comb = hist_Comb->GetStdDevError();
    double absUnc_resol_Comb = sqrt(pow(absUnc_stdDev_Comb/mean_Comb, 2) + pow(stdDev_Comb * absUnc_mean_Comb / (mean_Comb*mean_Comb), 2));
    
    TString histInfo_Comb = TString::Format("Hist. statistics (# events = %.0lf):", hist_Comb->Integral(0, hist_Comb->GetNbinsX()+1));
    canvas_Comb->RegisterLatex(0.16, 0.87, 62, 0.6, histInfo_Comb);
    TString muInfo_Comb = TString::Format("mean = %.2lf #pm %.2lf GeV", mean_Comb, absUnc_mean_Comb);
    TString sigmaInfo_Comb = TString::Format("std.dev. = %.2lf #pm %.2lf GeV", stdDev_Comb, absUnc_stdDev_Comb);
    TString resolInfo_Comb = TString::Format("std.dev/mean = %.2lf #pm %.2lf", resol_Comb, absUnc_resol_Comb);
    canvas_Comb->RegisterLatex(0.16, 0.83, 42, 0.6, muInfo_Comb);
    canvas_Comb->RegisterLatex(0.16, 0.79, 42, 0.6, sigmaInfo_Comb);
    canvas_Comb->RegisterLatex(0.16, 0.75, 42, 0.6, resolInfo_Comb);
    
    canvas_Comb->SetSavePath(plotPath);
    canvas_Comb->Draw("hist");

    // 하드론 빔의 경우 추가 채널들 플롯
    if(beamType != "em") {
        // S LC 채널 플롯
        if(hist_SLC) {
            TString canvasName_SLC = TString::Format("eDep_S_LC_energy%.0lf_%s", energy, towerName.c_str());
            PlotTool::HistCanvas* canvas_SLC = new PlotTool::HistCanvas(canvasName_SLC, 0, 0);
            canvas_SLC->SetTitle("Energy deposit [GeV]", "# events");
            
            canvas_SLC->Register(hist_SLC, "", kGreen+2);
            canvas_SLC->SetLegendPosition(0, 0, 0, 0);
            canvas_SLC->SetAutoRangeY();
            
            canvas_SLC->RegisterLatex(0.14, 0.96, 62, 0.8, "TB2025");
            canvas_SLC->RegisterLatex(0.65, 0.96, 62, 0.6, beamInfo);
            canvas_SLC->RegisterLatex(0.75, 0.91, 42, 0.6, centerInfo);
            
            TString runInfo_SLC = TString::Format("S LC compensated, %.0lf GeV", energy);
            canvas_SLC->RegisterLatex(0.16, 0.91, 42, 0.6, runInfo_SLC);
            
            // S LC 채널 통계 정보 계산 및 표시
            double mean_SLC = hist_SLC->GetMean();
            double stdDev_SLC = hist_SLC->GetStdDev();
            double resol_SLC = stdDev_SLC / mean_SLC;
            double absUnc_mean_SLC = hist_SLC->GetMeanError();
            double absUnc_stdDev_SLC = hist_SLC->GetStdDevError();
            double absUnc_resol_SLC = sqrt(pow(absUnc_stdDev_SLC/mean_SLC, 2) + pow(stdDev_SLC * absUnc_mean_SLC / (mean_SLC*mean_SLC), 2));
            
            TString histInfo_SLC = TString::Format("Hist. statistics (# events = %.0lf):", hist_SLC->Integral(0, hist_SLC->GetNbinsX()+1));
            canvas_SLC->RegisterLatex(0.16, 0.87, 62, 0.6, histInfo_SLC);
            TString muInfo_SLC = TString::Format("mean = %.2lf #pm %.2lf GeV", mean_SLC, absUnc_mean_SLC);
            TString sigmaInfo_SLC = TString::Format("std.dev. = %.2lf #pm %.2lf GeV", stdDev_SLC, absUnc_stdDev_SLC);
            TString resolInfo_SLC = TString::Format("std.dev/mean = %.2lf #pm %.2lf", resol_SLC, absUnc_resol_SLC);
            canvas_SLC->RegisterLatex(0.16, 0.83, 42, 0.6, muInfo_SLC);
            canvas_SLC->RegisterLatex(0.16, 0.79, 42, 0.6, sigmaInfo_SLC);
            canvas_SLC->RegisterLatex(0.16, 0.75, 42, 0.6, resolInfo_SLC);
            
            canvas_SLC->SetSavePath(plotPath);
            canvas_SLC->Draw("hist");
        }

        // Comb LC 채널 플롯
        if(hist_CombLC) {
            TString canvasName_CombLC = TString::Format("eDep_Comb_LC_energy%.0lf_%s", energy, towerName.c_str());
            PlotTool::HistCanvas* canvas_CombLC = new PlotTool::HistCanvas(canvasName_CombLC, 0, 0);
            canvas_CombLC->SetTitle("Energy deposit [GeV]", "# events");
            
            canvas_CombLC->Register(hist_CombLC, "", kOrange);
            canvas_CombLC->SetLegendPosition(0, 0, 0, 0);
            canvas_CombLC->SetAutoRangeY();
            
            canvas_CombLC->RegisterLatex(0.14, 0.96, 62, 0.8, "TB2025");
            canvas_CombLC->RegisterLatex(0.65, 0.96, 62, 0.6, beamInfo);
            canvas_CombLC->RegisterLatex(0.75, 0.91, 42, 0.6, centerInfo);
            
            TString runInfo_CombLC = TString::Format("Combined LC compensated, %.0lf GeV", energy);
            canvas_CombLC->RegisterLatex(0.16, 0.91, 42, 0.6, runInfo_CombLC);
            
            // Comb LC 채널 통계 정보 계산 및 표시
            double mean_CombLC = hist_CombLC->GetMean();
            double stdDev_CombLC = hist_CombLC->GetStdDev();
            double resol_CombLC = stdDev_CombLC / mean_CombLC;
            double absUnc_mean_CombLC = hist_CombLC->GetMeanError();
            double absUnc_stdDev_CombLC = hist_CombLC->GetStdDevError();
            double absUnc_resol_CombLC = sqrt(pow(absUnc_stdDev_CombLC/mean_CombLC, 2) + pow(stdDev_CombLC * absUnc_mean_CombLC / (mean_CombLC*mean_CombLC), 2));
            
            TString histInfo_CombLC = TString::Format("Hist. statistics (# events = %.0lf):", hist_CombLC->Integral(0, hist_CombLC->GetNbinsX()+1));
            canvas_CombLC->RegisterLatex(0.16, 0.87, 62, 0.6, histInfo_CombLC);
            TString muInfo_CombLC = TString::Format("mean = %.2lf #pm %.2lf GeV", mean_CombLC, absUnc_mean_CombLC);
            TString sigmaInfo_CombLC = TString::Format("std.dev. = %.2lf #pm %.2lf GeV", stdDev_CombLC, absUnc_stdDev_CombLC);
            TString resolInfo_CombLC = TString::Format("std.dev/mean = %.2lf #pm %.2lf", resol_CombLC, absUnc_resol_CombLC);
            canvas_CombLC->RegisterLatex(0.16, 0.83, 42, 0.6, muInfo_CombLC);
            canvas_CombLC->RegisterLatex(0.16, 0.79, 42, 0.6, sigmaInfo_CombLC);
            canvas_CombLC->RegisterLatex(0.16, 0.75, 42, 0.6, resolInfo_CombLC);
            
            canvas_CombLC->SetSavePath(plotPath);
            canvas_CombLC->Draw("hist");
        }

        // Comb DRC 채널 플롯
        if(hist_CombDRC) {
            TString canvasName_CombDRC = TString::Format("eDep_Comb_DRC_energy%.0lf_%s", energy, towerName.c_str());
            PlotTool::HistCanvas* canvas_CombDRC = new PlotTool::HistCanvas(canvasName_CombDRC, 0, 0);
            canvas_CombDRC->SetTitle("Energy deposit [GeV]", "# events");
            
            canvas_CombDRC->Register(hist_CombDRC, "", kMagenta);
            canvas_CombDRC->SetLegendPosition(0, 0, 0, 0);
            canvas_CombDRC->SetAutoRangeY();
            
            canvas_CombDRC->RegisterLatex(0.14, 0.96, 62, 0.8, "TB2025");
            canvas_CombDRC->RegisterLatex(0.65, 0.96, 62, 0.6, beamInfo);
            canvas_CombDRC->RegisterLatex(0.75, 0.91, 42, 0.6, centerInfo);
            
            TString runInfo_CombDRC = TString::Format("Combined DR correction, %.0lf GeV", energy);
            canvas_CombDRC->RegisterLatex(0.16, 0.91, 42, 0.6, runInfo_CombDRC);
            
            // Comb DRC 채널 통계 정보 계산 및 표시
            double mean_CombDRC = hist_CombDRC->GetMean();
            double stdDev_CombDRC = hist_CombDRC->GetStdDev();
            double resol_CombDRC = stdDev_CombDRC / mean_CombDRC;
            double absUnc_mean_CombDRC = hist_CombDRC->GetMeanError();
            double absUnc_stdDev_CombDRC = hist_CombDRC->GetStdDevError();
            double absUnc_resol_CombDRC = sqrt(pow(absUnc_stdDev_CombDRC/mean_CombDRC, 2) + pow(stdDev_CombDRC * absUnc_mean_CombDRC / (mean_CombDRC*mean_CombDRC), 2));
            
            TString histInfo_CombDRC = TString::Format("Hist. statistics (# events = %.0lf):", hist_CombDRC->Integral(0, hist_CombDRC->GetNbinsX()+1));
            canvas_CombDRC->RegisterLatex(0.16, 0.87, 62, 0.6, histInfo_CombDRC);
            TString muInfo_CombDRC = TString::Format("mean = %.2lf #pm %.2lf GeV", mean_CombDRC, absUnc_mean_CombDRC);
            TString sigmaInfo_CombDRC = TString::Format("std.dev. = %.2lf #pm %.2lf GeV", stdDev_CombDRC, absUnc_stdDev_CombDRC);
            TString resolInfo_CombDRC = TString::Format("std.dev/mean = %.2lf #pm %.2lf", resol_CombDRC, absUnc_resol_CombDRC);
            canvas_CombDRC->RegisterLatex(0.16, 0.83, 42, 0.6, muInfo_CombDRC);
            canvas_CombDRC->RegisterLatex(0.16, 0.79, 42, 0.6, sigmaInfo_CombDRC);
            canvas_CombDRC->RegisterLatex(0.16, 0.75, 42, 0.6, resolInfo_CombDRC);
            
            canvas_CombDRC->SetSavePath(plotPath);
            canvas_CombDRC->Draw("hist");
        }

        // Comb DRC LC 채널 플롯
        if(hist_CombDRCLC) {
            TString canvasName_CombDRCLC = TString::Format("eDep_Comb_DRC_LC_energy%.0lf_%s", energy, towerName.c_str());
            PlotTool::HistCanvas* canvas_CombDRCLC = new PlotTool::HistCanvas(canvasName_CombDRCLC, 0, 0);
            canvas_CombDRCLC->SetTitle("Energy deposit [GeV]", "# events");
            
            canvas_CombDRCLC->Register(hist_CombDRCLC, "", kCyan);
            canvas_CombDRCLC->SetLegendPosition(0, 0, 0, 0);
            canvas_CombDRCLC->SetAutoRangeY();
            
            canvas_CombDRCLC->RegisterLatex(0.14, 0.96, 62, 0.8, "TB2025");
            canvas_CombDRCLC->RegisterLatex(0.65, 0.96, 62, 0.6, beamInfo);
            canvas_CombDRCLC->RegisterLatex(0.75, 0.91, 42, 0.6, centerInfo);
            
            TString runInfo_CombDRCLC = TString::Format("Combined DR correction + LC, %.0lf GeV", energy);
            canvas_CombDRCLC->RegisterLatex(0.16, 0.91, 42, 0.6, runInfo_CombDRCLC);
            
            // Comb DRC LC 채널 통계 정보 계산 및 표시
            double mean_CombDRCLC = hist_CombDRCLC->GetMean();
            double stdDev_CombDRCLC = hist_CombDRCLC->GetStdDev();
            double resol_CombDRCLC = stdDev_CombDRCLC / mean_CombDRCLC;
            double absUnc_mean_CombDRCLC = hist_CombDRCLC->GetMeanError();
            double absUnc_stdDev_CombDRCLC = hist_CombDRCLC->GetStdDevError();
            double absUnc_resol_CombDRCLC = sqrt(pow(absUnc_stdDev_CombDRCLC/mean_CombDRCLC, 2) + pow(stdDev_CombDRCLC * absUnc_mean_CombDRCLC / (mean_CombDRCLC*mean_CombDRCLC), 2));
            
            TString histInfo_CombDRCLC = TString::Format("Hist. statistics (# events = %.0lf):", hist_CombDRCLC->Integral(0, hist_CombDRCLC->GetNbinsX()+1));
            canvas_CombDRCLC->RegisterLatex(0.16, 0.87, 62, 0.6, histInfo_CombDRCLC);
            TString muInfo_CombDRCLC = TString::Format("mean = %.2lf #pm %.2lf GeV", mean_CombDRCLC, absUnc_mean_CombDRCLC);
            TString sigmaInfo_CombDRCLC = TString::Format("std.dev. = %.2lf #pm %.2lf GeV", stdDev_CombDRCLC, absUnc_stdDev_CombDRCLC);
            TString resolInfo_CombDRCLC = TString::Format("std.dev/mean = %.2lf #pm %.2lf", resol_CombDRCLC, absUnc_resol_CombDRCLC);
            canvas_CombDRCLC->RegisterLatex(0.16, 0.83, 42, 0.6, muInfo_CombDRCLC);
            canvas_CombDRCLC->RegisterLatex(0.16, 0.79, 42, 0.6, sigmaInfo_CombDRCLC);
            canvas_CombDRCLC->RegisterLatex(0.16, 0.75, 42, 0.6, resolInfo_CombDRCLC);
            
            canvas_CombDRCLC->SetSavePath(plotPath);
            canvas_CombDRCLC->Draw("hist");
        }
    }

    printf("Plots saved to: %s\n", plotPath.Data());
    printf("Beam type: %s, Energy: %.0lf GeV, Center: %s\n", beamType.c_str(), energy, centerTowerTag.c_str());
    
    f->Close();
    return 0;
}