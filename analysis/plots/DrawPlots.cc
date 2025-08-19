#include "header/histlist.h"
#include "DrawPlots.h"
#include <fstream>
#include <iomanip>
#include <map>
#include <string>
#include <set>
#include <TLatex.h>

// 전역 변수들의 실제 정의
std::map<std::string, std::map<std::string, double>> pvDataMap;
std::map<std::string, std::map<std::string, double>> pvMCDataMap;
std::string currentVersion = "";
std::string currentType = "";
std::string currentMCVersion = "";
std::string currentMCType = "";

// 히스토그램 이름에서 디렉토리 분류 함수
string GetHistogramCategory(const TString& histName) 
{
    // 모듈 Edep 히스토그램들: Edep/M{module}/ 형태로 저장
    if(histName.Contains("Edep_M")) {
        for(int m = 1; m <= 9; m++) {
            if(histName.Contains(Form("Edep_M%d_", m))) {
                return Form("Edep/M%d", m);
            }
        }
    }
    
    // intADC 모듈 히스토그램들: intADC/M{module}/ 형태로 저장  
    if(histName.Contains("M") && histName.Contains("T") && !histName.Contains("Edep")) {
        for(int m = 1; m <= 9; m++) {
            if(histName.Contains(Form("M%d_", m))) {
                return Form("intADC/M%d", m);
            }
        }
    }
    
    // totalEdepScaled 계열
    if(histName.Contains("totalEdepScaled_C")) return "totalEdep_C";
    if(histName.Contains("totalEdepScaled_S")) return "totalEdep_S";
    if(histName.Contains("totalEdepScaled_Comb")) return "totalEdep_Comb";
    
    // 보조 검출기들 - 검출기 이름별로 분류 (스텝별 아님)
    if(histName.Contains("CC1_")) return "CC1";
    if(histName.Contains("CC2_")) return "CC2";
    if(histName.Contains("PS_")) return "PS";
    if(histName.Contains("MC_")) return "MC";
    if(histName.Contains("TC_")) return "TC";
    if(histName.Contains("LC")) return "LC";
    if(histName.Contains("DWC")) return "DWC";
    
    return "General";
}

// 채널 정보 추출 함수
TString GetChannelInfo(const TString& histName) 
{
    if(histName.Contains("_C_") || histName.EndsWith("_C")) return "C channel";
    if(histName.Contains("_S_") || histName.EndsWith("_S")) return "S channel";
    if(histName.Contains("Comb_LC_compensated")) return "C+(S+LC) compensated";
    if(histName.Contains("Comb_DRCorrection_LC_compensated")) return "C+(S+LC) + DR correction";
    if(histName.Contains("Comb_DRCorrection")) return "(S+C) + DR correction";
    if(histName.Contains("Comb")) return "Combined (S+C)";
    if(histName.Contains("CC1")) return "Cerenkov Counter 1";
    if(histName.Contains("CC2")) return "Cerenkov Counter 2";
    if(histName.Contains("PS")) return "PreShower";
    if(histName.Contains("MC")) return "Muon Counter";
    if(histName.Contains("TC")) return "Tail Catcher";
    if(histName.Contains("LC")) {
        TString lcNum = "";
        if(histName.Contains("LC2")) lcNum = "2";
        else if(histName.Contains("LC3")) lcNum = "3";
        else if(histName.Contains("LC4")) lcNum = "4";
        else if(histName.Contains("LC5")) lcNum = "5";
        else if(histName.Contains("LC7")) lcNum = "7";
        else if(histName.Contains("LC8")) lcNum = "8";
        else if(histName.Contains("LC9")) lcNum = "9";
        else if(histName.Contains("LC10")) lcNum = "10";
        else if(histName.Contains("LC11")) lcNum = "11";
        else if(histName.Contains("LC12")) lcNum = "12";
        else if(histName.Contains("LC13")) lcNum = "13";
        else if(histName.Contains("LC14")) lcNum = "14";
        else if(histName.Contains("LC15")) lcNum = "15";
        else if(histName.Contains("LC16")) lcNum = "16";
        else if(histName.Contains("LC19")) lcNum = "19";
        else if(histName.Contains("LC20")) lcNum = "20";
        return "Leakage Counter " + lcNum;
    }
    if(histName.Contains("DWC1_pos")) return "DWC1 Position";
    if(histName.Contains("DWC2_pos")) return "DWC2 Position";
    if(histName.Contains("DWC_x_corr")) return "DWC X Correlation";
    if(histName.Contains("DWC_y_corr")) return "DWC Y Correlation";
    
    // 모듈 히스토그램 처리
    if(histName.Contains("_M") && histName.Contains("_T")) {
        // Edep_M1_T1_C_0_noCut 또는 M1_T1_C_0_noCut 형태
        TString moduleInfo = "";
        for(int m = 1; m <= 9; m++) {
            for(int t = 1; t <= 4; t++) {
                if(histName.Contains(Form("M%d_T%d", m, t))) {
                    moduleInfo = Form("Module %d, Tower %d", m, t);
                    if(histName.Contains(Form("M%d_T%d_C", m, t))) moduleInfo += ", C channel";
                    else if(histName.Contains(Form("M%d_T%d_S", m, t))) moduleInfo += ", S channel";
                    return moduleInfo;
                }
            }
        }
    }
    
    return "General";
}

// 빔 정보 생성 함수
TString GetBeamInfo(const string& beamtype, const string& runinfo) 
{
    TString beamInfo = "";
    
    if(beamtype == "em") beamInfo = "e^{-} beam";
    else if(beamtype == "pi") beamInfo = "#pi^{+} beam";
    else if(beamtype == "kaon") beamInfo = "K^{+} beam";
    else if(beamtype == "proton") beamInfo = "proton beam";
    else if(beamtype == "calib") beamInfo = "calibration";
    else beamInfo = beamtype.c_str();
    
    // 에너지 정보 추가
    if(runinfo.find("GeV") != string::npos) {
        string energy = runinfo.substr(0, runinfo.find("GeV"));
        beamInfo += Form(" (%s GeV)", energy.c_str());
    }
    else if(runinfo.find("Run") != string::npos) {
        beamInfo += Form(" (%s)", runinfo.c_str());
    }
    
    return beamInfo;
}

// 1D 히스토그램 그리기 함수
void DrawSingleHistogram1D(TFile* f, const TString& histBaseName, const TString& step, 
                           const string& savepath, const string& beamtype, const string& runinfo)
{
    TString fullHistName = histBaseName + "_" + step;
    TH1D* hist = (TH1D*)f->Get(fullHistName);
    if(!hist) {
        cout << "Histogram not found: " << fullHistName << endl;
        return;
    }
    
    // 카테고리 분류 (fullHistName으로 분류)
    string category = GetHistogramCategory(fullHistName);
    string categoryPath = Form("%s/%s", savepath.c_str(), category.c_str());
    gSystem->mkdir(categoryPath.c_str(), kTRUE);
    
    // 캔버스 생성
    TString canvasName = Form("c_%s_%s", histBaseName.Data(), step.Data());
    TCanvas* c = new TCanvas(canvasName, canvasName, 800, 800);
    c->cd();
    c->SetMargin(0.12, 0.12, 0.12, 0.12);
    
    // 히스토그램 그리기
    hist->Draw("hist");
    hist->SetTitle("");
    hist->GetYaxis()->SetTitle("Entries");
    hist->GetYaxis()->SetTitleOffset(1.5);
    
    // 히스토그램별 특별 설정 (리빈 제거)
    if(histBaseName.Contains("Edep") || histBaseName.Contains("totalEdepScaled")) {
    }
    else if(histBaseName.Contains("TC")) {
        hist->GetXaxis()->SetRangeUser(0, 1000);
        c->SetLogy(1);  // 로그 스케일
    }
    else if(histBaseName.Contains("MC")) {
        hist->GetXaxis()->SetRangeUser(0, 500);
        c->SetLogy(1);  // 로그 스케일
    }
    else if(histBaseName.Contains("CC1")) {
        hist->GetXaxis()->SetRangeUser(0, 1000);
        c->SetLogy(1);  // 로그 스케일
    }
    else if(histBaseName.Contains("CC2") || histBaseName.Contains("PS") || histBaseName.Contains("LC")) {
    }
    
    // 색상 설정
    hist->SetLineColor(kBlack);
    hist->SetFillColor(0);
    
    // TLatex를 사용해서 정보 추가
    TLatex* latex = new TLatex();
    latex->SetNDC();
    latex->SetTextAlign(12);
    
    // TB2025 라벨
    latex->SetTextFont(62);
    latex->SetTextSize(0.8 * 0.05);  // 0.05는 기본 텍스트 크기
    latex->DrawLatex(0.14, 0.96, "TB2025");
    
    // 빔 정보
    TString beamInfo = GetBeamInfo(beamtype, runinfo);
    latex->SetTextFont(62);
    latex->SetTextSize(0.7 * 0.05);
    latex->DrawLatex(0.28, 0.9, beamInfo);
    
    // 채널 정보
    TString channelInfo = GetChannelInfo(fullHistName);
    latex->SetTextFont(42);
    latex->SetTextSize(0.8 * 0.05);
    latex->DrawLatex(0.55, 0.9, channelInfo);
    
    // 저장
    TString saveFileName = Form("%s/%s_%s.pdf", categoryPath.c_str(), histBaseName.Data(), step.Data());
    c->SaveAs(saveFileName);
    
    cout << "Saved: " << saveFileName << endl;
    delete c;
}

// 2D 히스토그램 그리기 함수
void DrawSingleHistogram2D(TFile* f, const TString& histBaseName, const TString& step, 
                           const string& savepath, const string& beamtype, const string& runinfo)
{
    TString fullHistName = histBaseName + "_" + step;
    TH2D* hist = (TH2D*)f->Get(fullHistName);
    if(!hist) {
        cout << "2D Histogram not found: " << fullHistName << endl;
        return;
    }
    
    // 카테고리 분류 (fullHistName으로 분류)
    string category = GetHistogramCategory(fullHistName);
    string categoryPath = Form("%s/%s", savepath.c_str(), category.c_str());
    gSystem->mkdir(categoryPath.c_str(), kTRUE);
    
    // 캔버스 생성
    TString canvasName = Form("c2D_%s_%s", histBaseName.Data(), step.Data());
    TCanvas* c = new TCanvas(canvasName, canvasName, 800, 800);
    c->cd();
    c->SetMargin(0.12, 0.15, 0.12, 0.12);
    
    // 히스토그램 그리기
    hist->Draw("colz");
    hist->SetTitle("");
    
    // 히스토그램별 특별 설정
    if(histBaseName.Contains("DWC")) {
        if(histBaseName.Contains("pos")) {
            hist->GetXaxis()->SetTitle("X (mm)");
            hist->GetYaxis()->SetTitle("Y (mm)");
            hist->GetXaxis()->SetRangeUser(-50, 50);
            hist->GetYaxis()->SetRangeUser(-50, 50);
        }
        else if(histBaseName.Contains("corr")) {
            if(histBaseName.Contains("x_corr")) {
                hist->GetXaxis()->SetTitle("DWC1 X (mm)");
                hist->GetYaxis()->SetTitle("DWC2 X (mm)");
            }
            else if(histBaseName.Contains("y_corr")) {
                hist->GetXaxis()->SetTitle("DWC1 Y (mm)");
                hist->GetYaxis()->SetTitle("DWC2 Y (mm)");
            }
        }
        c->SetLogz(0);  // DWC는 로그 스케일 사용 안함
    }
    else {
        c->SetLogz(1);  // 기본적으로 Z축 로그 스케일
        hist->SetMinimum(1e-3);
    }
    
    // TLatex를 사용해서 정보 추가
    TLatex* latex = new TLatex();
    latex->SetNDC();
    latex->SetTextAlign(12);
    
    // TB2025 라벨
    latex->SetTextFont(62);
    latex->SetTextSize(0.8 * 0.05);
    latex->DrawLatex(0.14, 0.9, "TB2025");
    
    // 빔 정보
    TString beamInfo = GetBeamInfo(beamtype, runinfo);
    latex->SetTextFont(62);
    latex->SetTextSize(0.7 * 0.05);
    latex->DrawLatex(0.28, 0.9, beamInfo);
    
    // 채널 정보
    TString channelInfo = GetChannelInfo(fullHistName);
    latex->SetTextFont(42);
    latex->SetTextSize(0.8 * 0.05);
    latex->DrawLatex(0.55, 0.9, channelInfo);
    
    // 저장
    TString saveFileName = Form("%s/%s_%s.pdf", categoryPath.c_str(), histBaseName.Data(), step.Data());
    c->SaveAs(saveFileName);
    
    cout << "Saved: " << saveFileName << endl;
    delete c;
}

void DrawKinematics(TH1D *hist, string beamtype, string runnumber, string savepath)
{
    // 기존 함수는 단순히 호출만 - 실제로는 아래의 새로운 함수들이 사용됨
    cout << "DrawKinematics called for: " << hist->GetName() << endl;
}

void DrawKinematics(TH1D *hist, string runnumber, string savepath)
{
    DrawKinematics(hist, "unknown", runnumber, savepath);
}

void DrawKinematics2D(TH2D *hist, string sample, string version, string savepath, string type)
{
    cout << "DrawKinematics2D called for: " << hist->GetName() << endl;
}

// 새로운 메인 그리기 함수
void DrawAllHistograms(TFile* f, const string& savepath, const string& beamtype, const string& runinfo)
{
    cout << "=== Drawing All Histograms ===" << endl;
    
    // 1. hist1D_drc_total 그리기
    cout << "Drawing hist1D_drc_total..." << endl;
    for(const auto& histBase : hist1D_drc_total) {
        for(const auto& step : drcSteps) {
            DrawSingleHistogram1D(f, histBase, step, savepath, beamtype, runinfo);
        }
    }
    
    // 2. hist1D_drc_aux 그리기  
    cout << "Drawing hist1D_drc_aux..." << endl;
    for(const auto& histBase : hist1D_drc_aux) {
        for(const auto& step : drcSteps) {
            DrawSingleHistogram1D(f, histBase, step, savepath, beamtype, runinfo);
        }
    }
    
    // 3. hist2D_drc_dwc 그리기
    cout << "Drawing hist2D_drc_dwc..." << endl;
    for(const auto& histBase : hist2D_drc_dwc) {
        for(const auto& step : drcSteps) {
            DrawSingleHistogram2D(f, histBase, step, savepath, beamtype, runinfo);
        }
    }
    
    // 4. generateModuleHistograms 그리기
    cout << "Drawing module histograms..." << endl;
    vector<TString> moduleHists = generateModuleHistograms();
    for(const auto& fullHistName : moduleHists) {
        TH1D* hist = (TH1D*)f->Get(fullHistName);
        if(!hist) {
            cout << "Module histogram not found: " << fullHistName << endl;
            continue;
        }
        
        // 모듈 히스토그램은 이미 step이 포함되어 있으므로 직접 처리
        string category = GetHistogramCategory(fullHistName);
        string categoryPath = Form("%s/%s", savepath.c_str(), category.c_str());
        gSystem->mkdir(categoryPath.c_str(), kTRUE);
        
        TString canvasName = Form("c_%s", fullHistName.Data());
        TCanvas* c = new TCanvas(canvasName, canvasName, 800, 800);
        c->cd();
        c->SetMargin(0.12, 0.12, 0.12, 0.12);
        
        hist->Draw("hist");
        hist->SetTitle("");
        hist->GetYaxis()->SetTitle("Entries");
        hist->GetYaxis()->SetTitleOffset(1.5);
        
        // 에너지 히스토그램 설정 (리빈 제거)
        if(fullHistName.Contains("Edep")) {
            hist->GetXaxis()->SetTitle("Energy (GeV)");
            // 리빈 제거: hist->Rebin(5);
        }
        else if(fullHistName.Contains("ADC")) {
            hist->GetXaxis()->SetTitle("ADC Count");
            // 리빈 제거: hist->Rebin(2);
        }
        
        // 색상 설정
        Int_t color = kBlue;
        if(beamtype == "em") color = kRed;
        else if(beamtype == "pi") color = kGreen+2;
        else if(beamtype == "kaon") color = kMagenta;
        else if(beamtype == "proton") color = kOrange;
        
        hist->SetLineColor(kBlack);
        hist->SetFillColor(0);
        
        // TLatex를 사용해서 정보 추가
        TLatex* latex = new TLatex();
        latex->SetNDC();
        latex->SetTextAlign(12);
        
        // TB2025 라벨
        latex->SetTextFont(62);
        latex->SetTextSize(0.8 * 0.05);
        latex->DrawLatex(0.14, 0.9, "TB2025");
        
        // 빔 정보
        TString beamInfo = GetBeamInfo(beamtype, runinfo);
        latex->SetTextFont(62);
        latex->SetTextSize(0.7 * 0.05);
        latex->DrawLatex(0.28, 0.9, beamInfo);
        
        // 채널 정보
        TString channelInfo = GetChannelInfo(fullHistName);
        latex->SetTextFont(42);
        latex->SetTextSize(0.8 * 0.05);
        latex->DrawLatex(0.55, 0.9, channelInfo);
        
        TString saveFileName = Form("%s/%s.pdf", categoryPath.c_str(), fullHistName.Data());
        c->SaveAs(saveFileName);
        
        cout << "Saved: " << saveFileName << endl;
        delete c;
    }
    
    cout << "=== All Histograms Drawn ===" << endl;
}