#include "header/SimplePlotTools.h"
#include "header/RunInfo.h"
#include "header/GausFitter.h"

class EMResolution {
public:
  EMResolution(TString inputPath, TString centerTowerTag, TString beamType): 
  inputPath_(inputPath), centerTowerTag_(centerTowerTag), beamType_(beamType) { }

  void Set_PlotPath(TString path) { plotPath_ = path; }

  void NoFit(bool noFit=kTRUE) { noFit_ = noFit; }

  void noNoiseReverse(bool noNoiseReverse=kTRUE) { noNoiseReverse_ = noNoiseReverse; }

  void Use_PartialTower(TString tag_partialTower) {
    use_partialTower_ = kTRUE;
    tag_partialTower_ = tag_partialTower;
    if( (tag_partialTower_ == "nearM8T2" && centerTowerTag_ != "M8-T2") || 
        (tag_partialTower_ == "nearM9T1" && centerTowerTag_ != "M9-T1") ||
        (tag_partialTower_ == "nearM5T1" && centerTowerTag_ != "M5-T1") ||
        (tag_partialTower_ == "nearM5T3" && centerTowerTag_ != "M5-T3") ) {
      printf("[EMResolution::Use_PartialTower] tag for partial tower is %s, but reference tower is %s!\n", tag_partialTower_.Data(), centerTowerTag_.Data());
      throw std::invalid_argument("tower inconsistency is found");
    }
  }

  void Produce() {
    Init();

    ProducePlot_PerPoint();
    ProducePlot_Resol_vs_E();
    
    // NoNoise + reverse 옵션인 경우 리니어리티 플롯 생략
    if(!noNoiseReverse_) {ProducePlot_Linearity_vs_E();}
  }

  void Produce(TString plotPath) {
    Set_PlotPath(plotPath);
    Produce();
  }

private:
  TString inputPath_;
  TString plotPath_ = "./";
  TString beamType_ = "em";  // 기본값으로 "em" 설정

  TString centerTowerTag_;
  std::vector<int> vec_run_;
  std::vector<double> vec_energy_;

  bool noFit_ = kFALSE; // -- default: do gaussian fit
  bool noNoiseReverse_ = kFALSE; // -- default: include noise term in fitting
  bool reverse_ = kFALSE; // -- default: use E vs σ/E, if true use 1/√E vs σ/E

  // -- when partial # towers are used, not full 3x3 modules
  bool use_partialTower_ = kFALSE;
  TString tag_partialTower_ = "undefined"; // -- tag for hist. name (e.g. "nearM8T2" if the hist. = h_eDep_nearM8T2_DWCPSMC)
  
  // 지정된 범위에서의 평균 계산 함수
  double CalculateRangedMean(TH1D* hist, double minRange, double maxRange)
  {
    int binLow = hist->FindBin(minRange);
    int binHigh = hist->FindBin(maxRange);
    double integral = hist->Integral(binLow, binHigh);
    double weightedSum = 0;
    
    for(int bin = binLow; bin <= binHigh; bin++)
    {
      double binContent = hist->GetBinContent(bin);
      double binCenter = hist->GetBinCenter(bin);
      weightedSum += binContent * binCenter;
    }
    return (integral > 0) ? weightedSum / integral : hist->GetMean();
  }
  
  // 지정된 범위에서의 표준편차 계산 함수
  double CalculateRangedStdDev(TH1D* hist, double minRange, double maxRange, double rangedMean)
  {
    int binLow = hist->FindBin(minRange);
    int binHigh = hist->FindBin(maxRange);
    double integral = hist->Integral(binLow, binHigh);
    double sumSquaredDiff = 0;
    
    for(int bin = binLow; bin <= binHigh; bin++)
    {
      double binContent = hist->GetBinContent(bin);
      double binCenter = hist->GetBinCenter(bin);
      sumSquaredDiff += binContent * (binCenter - rangedMean) * (binCenter - rangedMean);
    }
    return (integral > 1) ? sqrt(sumSquaredDiff / integral) : hist->GetStdDev();
  }

  void Init()
  {
    if( beamType_ == "em" )
    {
      // 센터 타워에 맞는 에너지 벡터와 런 정보 사용
      TString centerKey = centerTowerTag_;
      centerKey.ReplaceAll("-", "");  // "M5-T2" -> "M5T2"
      
      if(TB2025::map_emScanRun_merged.find(centerKey) != TB2025::map_emScanRun_merged.end()) {
        const auto& energies = TB2025::map_emScanRun_merged[centerKey];
        
        vec_energy_.clear();
        vec_run_.clear();
        
        for(int energy : energies) {
          vec_energy_.push_back((double)energy);
          vec_run_.push_back(energy);  // 런 번호 대신 에너지를 사용 (merged 파일명 생성용)
        }
      }
      else {
        throw std::invalid_argument("[EMResolution::Init] centerTowerTag_ = " + centerTowerTag_ + " is not found in merged map");
      }
    }
    else if( beamType_ == "pi" || beamType_ == "kaon" || beamType_ == "proton" )
    {
      if( beamType_ == "pi" )
      {
        // pi beam도 em beam과 같은 방식으로 머지된 파일 사용
        TString centerKey = centerTowerTag_;
        centerKey.ReplaceAll("-", "");  // "M5-T2" -> "M5T2"
        
        if(TB2025::map_piBeamRun_merged.find(centerKey) != TB2025::map_piBeamRun_merged.end()) {
          const auto& energies = TB2025::map_piBeamRun_merged[centerKey];
          
          vec_energy_.clear();
          vec_run_.clear();
          
          for(int energy : energies) {
            vec_energy_.push_back((double)energy);
            vec_run_.push_back(energy);  // 런 번호 대신 에너지를 사용 (merged 파일명 생성용)
          }
        }
        else {
          throw std::invalid_argument("[EMResolution::Init] centerTowerTag_ = " + centerTowerTag_ + " is not found in pi merged map");
        }
      }
      else if( beamType_ == "kaon" || beamType_ == "proton" )
      {
        // kaon/proton은 기존 방식 유지
        vec_energy_ = TB2025::vec_hadScanE;
        vec_run_ = TB2025::map_kpBeamRun[centerTowerTag_];
      }
      
      // 하드로닉 입자의 경우 기본적으로 가우시안 피팅을 하지 않도록 설정
      // 개별 채널별로 피팅 여부는 ProducePlot_PerPoint에서 처리
      if(!noFit_) NoFit(kTRUE);
    }
    else
      throw std::invalid_argument("[EMResolution::Init] beamType_ = " + beamType_ + " is not recognized");

    int nPoint = (int)vec_energy_.size();
    vector<TString> vec_fiberType;
    if( beamType_ == "em" ) {vec_fiberType = {"C", "S", "Comb"};}
    else {vec_fiberType = {"C", "S", "Comb", "S_LC_compensated", "Comb_LC_compensated", "DRcorrection", "DRcorrection_LC_compensated"};}
    
    for(auto& fiberType : vec_fiberType )
    {
      map_resol_.insert( std::make_pair(fiberType, new TGraphAsymmErrors(nPoint)) );
      map_linearity_.insert( std::make_pair(fiberType, new TGraphAsymmErrors(nPoint)) );
    }
  }

  std::map<TString, TGraphAsymmErrors*> map_resol_;
  std::map<TString, TGraphAsymmErrors*> map_linearity_;

  void ProducePlot_PerPoint() {
    int nPoint = (int)vec_energy_.size();
    for(int i=0; i<nPoint; ++i) {
      ProducePlot_PerPoint(i, vec_run_[i], vec_energy_[i], "C");
      ProducePlot_PerPoint(i, vec_run_[i], vec_energy_[i], "S");
      ProducePlot_PerPoint(i, vec_run_[i], vec_energy_[i], "Comb");
      
      // 하드론 빔인 경우에만 LC 채널 처리
      if( beamType_ == "pi" || beamType_ == "kaon" || beamType_ == "proton" )
      {
        ProducePlot_PerPoint(i, vec_run_[i], vec_energy_[i], "S_LC_compensated");
        ProducePlot_PerPoint(i, vec_run_[i], vec_energy_[i], "Comb_LC_compensated");
        ProducePlot_PerPoint(i, vec_run_[i], vec_energy_[i], "DRcorrection");
        ProducePlot_PerPoint(i, vec_run_[i], vec_energy_[i], "DRcorrection_LC_compensated");
      }
    }
  }

  void ProducePlot_Resol_vs_E()
  {
    // 피팅 함수 정의 - NoNoise 옵션에 따라 다른 함수 사용
    TF1* resolFunc_C = nullptr;
    TF1* resolFunc_S = nullptr;
    TF1* resolFunc_sum = nullptr;
    TF1* resolFunc_drc = nullptr;
    TF1* resolFunc_lc = nullptr;
    
    double fitRangeMax = noNoiseReverse_ ? 130.0 : 130.0;
    
    // 캔버스 이름 설정
    TString canvasName = TString::Format("c_%sResol_%s", beamType_.Data(), centerTowerTag_.Data());
    if(noNoiseReverse_) canvasName += "_NNR";
    
    if(noNoiseReverse_)
    {
      // Reverse 모드: 1/√E vs σ/E 그래프 + pol1 피팅
      TGraphAsymmErrors* resolGraph_C_transformed = Transform_to_InvSqrtE_GraphAsymm(map_resol_["C"]);
      TGraphAsymmErrors* resolGraph_S_transformed = Transform_to_InvSqrtE_GraphAsymm(map_resol_["S"]);
      TGraphAsymmErrors* resolGraph_sum_transformed = nullptr;
      TGraphAsymmErrors* resolGraph_drc_transformed = nullptr;
      TGraphAsymmErrors* resolGraph_lc_transformed = nullptr;

      // pol1 피팅 함수 (20-120 GeV 범위, 1/√E: 0.091-0.223)
      resolFunc_C = new TF1("resolFunc_C", "pol1", 0.087, 0.224);
      resolFunc_C->SetLineColor(kBlue);
      resolFunc_C->SetBit(TF1::kNotDraw);

      resolFunc_S = new TF1("resolFunc_S", "pol1", 0.087, 0.224);
      resolFunc_S->SetLineColor(kRed);
      resolFunc_S->SetBit(TF1::kNotDraw);

      if( beamType_ == "em" )
      {
        resolGraph_sum_transformed = Transform_to_InvSqrtE_GraphAsymm(map_resol_["Sum"]);
        resolFunc_sum = new TF1("resolFunc_sum", "pol1", 0.087, 0.224);
        resolFunc_sum->SetLineColor(kBlack);
        resolFunc_sum->SetBit(TF1::kNotDraw);
      }
      else if( beamType_ == "pi" || beamType_ == "kaon" || beamType_ == "proton" )
      {
        resolGraph_drc_transformed = Transform_to_InvSqrtE_GraphAsymm(map_resol_["DRcorrection"]);
        resolGraph_lc_transformed = Transform_to_InvSqrtE_GraphAsymm(map_resol_["DRcorrection_LC_compensated"]);
        
        resolFunc_drc = new TF1("resolFunc_drc", "pol1", 0.087, 0.224);
        resolFunc_drc->SetLineColor(kMagenta);
        resolFunc_drc->SetBit(TF1::kNotDraw);

        resolFunc_lc = new TF1("resolFunc_lc", "pol1", 0.087, 0.224);
        resolFunc_lc->SetLineColor(kGreen+2);
        resolFunc_lc->SetBit(TF1::kNotDraw);
      }

      // pol1 피팅 수행
      resolGraph_C_transformed->Fit("resolFunc_C");
      resolGraph_S_transformed->Fit("resolFunc_S");
      if( beamType_ == "em" && resolGraph_sum_transformed )
      {
        resolGraph_sum_transformed->Fit("resolFunc_sum");
      }
      if( beamType_ == "pi" || beamType_ == "kaon" || beamType_ == "proton" )
      {
        if(resolGraph_drc_transformed) resolGraph_drc_transformed->Fit("resolFunc_drc");
        if(resolGraph_lc_transformed) resolGraph_lc_transformed->Fit("resolFunc_lc");
      }

      // GraphCanvas 사용
      PlotTool::GraphCanvas* canvas = new PlotTool::GraphCanvas(canvasName, 0, 0);
      canvas->SetTitle("1/#sqrt{E}", "#sigma/E");

      canvas->Register(resolGraph_C_transformed, MakeLegend_Resol_nNR("C", resolFunc_C), kBlue);
      canvas->Register(resolGraph_S_transformed, MakeLegend_Resol_nNR("S", resolFunc_S), kRed);
      if( beamType_ == "em" && resolGraph_sum_transformed )
      {
        canvas->Register(resolGraph_sum_transformed, MakeLegend_Resol_nNR("Sum", resolFunc_sum), kBlack);
      }
      else if( beamType_ == "pi" || beamType_ == "kaon" || beamType_ == "proton" )
      {
        if(resolGraph_drc_transformed) canvas->Register(resolGraph_drc_transformed, MakeLegend_Resol_nNR("DR correcttion", resolFunc_drc), kMagenta);
        //if(resolGraph_lc_transformed) canvas->Register(resolGraph_lc_transformed, MakeLegend_Resol_nNR("DR correction + LC", resolFunc_lc), kGreen+2);
      }

      // 축 범위 설정 (reverse 범위)
      canvas->SetRangeX(0.087, 0.24);  // 1/√130 ~ 1/√20 (전체 데이터 범위)
      canvas->SetRangeY(0.05, 0.45);
      
      canvas->SetLegendPosition(0.24, 0.65, 0.95, 0.91);

      canvas->RegisterLatex(0.14, 0.96, 62, 0.8, "TB2025");
      TString beamInfo;
      if(beamType_ == "em") {beamInfo = "e+ beam (6 - 120 GeV)";}
      else if(beamType_ == "pi") {beamInfo = "#pi^{+} beam (20 - 120 GeV)";}
      else if(beamType_ == "kaon") {beamInfo = "#K^{+} beam (20 - 120 GeV)";}
      else if(beamType_ == "proton") {beamInfo = "proton beam (20 - 120 GeV)";}
      canvas->RegisterLatex(0.65, 0.96, 62, 0.6, beamInfo);
      
      TString centerInfo = "Center: "+centerTowerTag_;
      canvas->RegisterLatex(0.75, 0.91, 42, 0.6, centerInfo);
      
      canvas->SetSavePath(plotPath_);
      canvas->Draw("P RX");
      
      // 피팅 함수들을 캔버스에 추가로 그리기
      canvas->c_->cd();
      TGraph* reversed_fitFunc_C = DrawFitReverse_TGraph(resolFunc_C);
      reversed_fitFunc_C->Draw("L RX & sames");
      TGraph* reversed_fitFunc_S = DrawFitReverse_TGraph(resolFunc_S);
      reversed_fitFunc_S->Draw("L RX & sames");
      if( beamType_ == "em" && resolFunc_sum )
      {
        TGraph* reversed_fitFunc_sum = DrawFitReverse_TGraph(resolFunc_sum);
        reversed_fitFunc_sum->Draw("L RX & sames");
      }
      if( beamType_ == "pi" || beamType_ == "kaon" || beamType_ == "proton" )
      {
        if(resolFunc_drc) {
          TGraph* reversed_fitFunc_drc = DrawFitReverse_TGraph(resolFunc_drc);
          reversed_fitFunc_drc->Draw("L RX & sames");
        }
        if(resolFunc_lc) {
          TGraph* reversed_fitFunc_lc = DrawFitReverse_TGraph(resolFunc_lc);
          //reversed_fitFunc_lc->Draw("L RX & sames");
        }
      }
      canvas->c_->Update();
      
      // 피팅 함수가 포함된 상태로 저장
      TString savePath = plotPath_ + "/" + canvasName + ".pdf";
      canvas->c_->SaveAs(savePath);
      
      return; // NoNoise + Reverse 모드에서는 여기서 함수 종료
    }
    else
    {
      // 기본 옵션: 3-term 함수 사용
      TString fit_noiseterm = "TMath::Sqrt( ([0]/x)*([0]/x) + ([1]/TMath::Sqrt(x))*([1]/TMath::Sqrt(x)) + [2]*[2] )";
      //2-term 함수 사용 (constant term 제거)
      //TString fit_noiseterm = "TMath::Sqrt( ([0]/x)*([0]/x) + ([1]/TMath::Sqrt(x))*([1]/TMath::Sqrt(x)) )";
      
      resolFunc_C = new TF1("resolFunc_C", fit_noiseterm, 10, fitRangeMax);
      resolFunc_C->SetParameter(0, 1.0);   // noise term
      resolFunc_C->SetParameter(1, 0.2);   // stochastic term  
      resolFunc_C->SetParameter(2, 0.01);  // constant term (removed)
      resolFunc_C->SetParLimits(0, 0., 15.);
      resolFunc_C->SetParLimits(1, 0., 15.);
      resolFunc_C->SetParLimits(2, 0., 15.);  // constant term (removed)
      resolFunc_C->SetLineColor(kBlue);

      resolFunc_S = new TF1("resolFunc_S", fit_noiseterm, 10, fitRangeMax);
      resolFunc_S->SetParameter(0, 1.0);
      resolFunc_S->SetParameter(1, 0.2);
      resolFunc_S->SetParameter(2, 0.01);  // constant term (removed)
      resolFunc_S->SetParLimits(0, 0., 15.);
      resolFunc_S->SetParLimits(1, 0., 15.);
      resolFunc_S->SetParLimits(2, 0., 15.);  // constant term (removed)
      resolFunc_S->SetLineColor(kRed);
      
      resolFunc_sum = new TF1("resolFunc_sum", fit_noiseterm, 10, fitRangeMax);
      resolFunc_sum->SetParameter(0, 1.0);     //noise term
      resolFunc_sum->SetParameter(1, 0.2);     //stochastic term
      resolFunc_sum->SetParameter(2, 0.01);    //constant term
      resolFunc_sum->SetParLimits(0, 0., 15.); //noise term limit
      resolFunc_sum->SetParLimits(1, 0., 15.); //stochastic term limit
      resolFunc_sum->SetParLimits(2, 0., 15.); //constant term limit
      resolFunc_sum->SetLineColor(kBlack);

      if( beamType_ == "pi" || beamType_ == "kaon" || beamType_ == "proton" )
      {
        resolFunc_drc = new TF1("resolFunc_drc", fit_noiseterm, 10, fitRangeMax);
        resolFunc_drc->SetParameter(0, 1.0);        //noise term
        resolFunc_drc->SetParameter(1, 0.2);        //stochastic term
        resolFunc_drc->SetParameter(2, 0.01);    //constant term
        resolFunc_drc->SetParLimits(0, 0., 15.);    //noise term limit
        resolFunc_drc->SetParLimits(1, 0., 15.);    //stochastic term limit
        resolFunc_drc->SetParLimits(2, 0., 15.); // constant term limit
        resolFunc_drc->SetLineColor(kMagenta);

        resolFunc_lc = new TF1("resolFunc_lc", fit_noiseterm, 10, fitRangeMax);
        resolFunc_lc->SetParameter(0, 1.0);         //noise term
        resolFunc_lc->SetParameter(1, 0.2);         //stochastic term
        resolFunc_lc->SetParameter(2, 0.01);     //constant term
        resolFunc_lc->SetParLimits(0, 0., 15.);     //noise term limit
        resolFunc_lc->SetParLimits(1, 0., 15.);     //stochastic term limit
        resolFunc_lc->SetParLimits(2, 0., 15.);  // constant term limit
        resolFunc_lc->SetLineColor(kGreen+2);
      }

      // 모든 채널에 대해 피팅 수행
      map_resol_["C"]->Fit("resolFunc_C", "", "", 10, fitRangeMax);
      map_resol_["S"]->Fit("resolFunc_S", "", "", 10, fitRangeMax);
      map_resol_["Comb"]->Fit("resolFunc_sum", "", "", 10, fitRangeMax);
      // 하드론 빔인 경우에만 LC 채널 피팅
      if( beamType_ == "pi" || beamType_ == "kaon" || beamType_ == "proton" )
      {
        if(resolFunc_drc) map_resol_["DRcorrection"]->Fit("resolFunc_drc");
        if(resolFunc_lc) map_resol_["DRcorrection_LC_compensated"]->Fit("resolFunc_lc");
      }

      PlotTool::GraphCanvas* canvas = new PlotTool::GraphCanvas(canvasName, 0, 0);
      canvas->SetTitle("E [GeV]", "#sigma/E");

      canvas->Register(map_resol_["C"],   MakeLegend_Resol("C",   resolFunc_C),   kBlue);
      canvas->Register(map_resol_["S"],   MakeLegend_Resol("S",   resolFunc_S),   kRed);
      
      if( beamType_ == "pi" || beamType_ == "kaon" || beamType_ == "proton" )
      {
        if(resolFunc_drc) canvas->Register(map_resol_["DRcorrection"], MakeLegend_Resol("DR correction", resolFunc_drc), kMagenta);
        //if(resolFunc_lc) canvas->Register(map_resol_["DRcorrection_LC_compensated"], MakeLegend_Resol("DR correction + LC", resolFunc_lc), kGreen+2);
      }
      else {canvas->Register(map_resol_["Comb"], MakeLegend_Resol("Comb", resolFunc_sum), kBlack);}

      // 기본 모드 축 범위 설정
      if( beamType_ == "em" ) canvas->SetRangeY(0, 0.4);
      else if( beamType_ == "pi" || beamType_ == "kaon" || beamType_ == "proton" ) canvas->SetRangeY(0, 0.5);

      // 기본 모드 설정
      if( noFit_ ) canvas->SetLegendPosition(0.16, 0.14, 0.80, 0.35);
      else         canvas->SetLegendPosition(0.24, 0.65, 0.95, 0.91);

      canvas->RegisterLatex(0.14, 0.96, 62, 0.8, "TB2025");
      TString beamInfo;
      if(beamType_ == "em") {beamInfo = "e+ beam (6 - 120 GeV)";}
      else if(beamType_ == "pi") {beamInfo = "#pi^{+} beam (20 - 120 GeV)";}
      else if(beamType_ == "kaon") {beamInfo = "#K^{+} beam (20 - 120 GeV)";}
      else if(beamType_ == "proton") {beamInfo = "proton beam (20 - 120 GeV)";}
      canvas->RegisterLatex(0.65, 0.96, 62, 0.6, beamInfo);

      TString centerInfo = "Center: "+centerTowerTag_;
      canvas->RegisterLatex(0.75, 0.91, 42, 0.6, centerInfo);

      canvas->SetSavePath(plotPath_);
      canvas->Draw();
      
      // 피팅 함수들을 캔버스에 추가로 그리기
      canvas->c_->cd();
      if(resolFunc_C) resolFunc_C->Draw("SAME");
      if(resolFunc_S) resolFunc_S->Draw("SAME");
      if(resolFunc_sum) resolFunc_sum->Draw("SAME");
      if( beamType_ == "pi" || beamType_ == "kaon" || beamType_ == "proton" )
      {
        if(resolFunc_drc) resolFunc_drc->Draw("SAME");
        if(resolFunc_lc) resolFunc_lc->Draw("SAME");
      }
      canvas->c_->Update();
      
      // 피팅 함수가 포함된 상태로 저장
      //TString savePath = plotPath_ + "/" + canvasName + ".pdf";
      //canvas->c_->SaveAs(savePath);
    }
  }

  TString MakeLegend_Resol(TString fiberType, TF1* fitFunc)
  {
    TString legend;
    
    if(noNoiseReverse_)
    {
      // noNoiseReverse 모드: stochastic + constant 텀만
      legend = TString::Format("%s: %.3lf/#sqrt{E} #oplus %.3lf (#chi^{2}/ndf = %.1lf/%d)", 
                fiberType.Data(), 
                fitFunc->GetParameter(0),  // stochastic term
                fitFunc->GetParameter(1),  // constant term
                fitFunc->GetChisquare(), 
                fitFunc->GetNDF());
    }
    else
    {
      //기본 모드: noise + stochastic + constant 텀
      legend = TString::Format("%s: %.3lf/E #oplus %.3lf/#sqrt{E} #oplus %.3lf (#chi^{2}/ndf = %.1lf/%d)", 
                fiberType.Data(), 
                fitFunc->GetParameter(0),  // noise term
                fitFunc->GetParameter(1),  // stochastic term
                fitFunc->GetParameter(2),  // constant term
                fitFunc->GetChisquare(), 
                fitFunc->GetNDF());
      
      // 2-term 모드: noise + stochastic 텀만 (constant term 제거)
      //legend = TString::Format("%s: %.3lf/E #oplus %.3lf/#sqrt{E} (#chi^{2}/ndf = %.1lf/%d)", 
      //          fiberType.Data(), 
      //          fitFunc->GetParameter(0),  // noise term
      //          fitFunc->GetParameter(1),  // stochastic term
      //          fitFunc->GetChisquare(), 
      //          fitFunc->GetNDF());
    }
    return legend;
  }

  // TB2022 스타일 레전드 함수 (pol1 피팅용)
  TString MakeLegend_Resol_nNR(TString fiberType, TF1* fitFunc)
  {
    // TB2022 스타일: pol1 피팅 결과를 "stoch/#sqrt{E} + const" 형식으로 표시
    TString legend = TString::Format("%s: %.3lf/#sqrt{E} + %.3lf (#chi^{2}/ndf = %.1lf/%d)", 
              fiberType.Data(), 
              fitFunc->GetParameter(1),  // stochastic term (slope)
              fitFunc->GetParameter(0),  // constant term (intercept)
              fitFunc->GetChisquare(), 
              fitFunc->GetNDF());

    return legend;
  }

  // E vs σ/E 그래프를 1/√E vs σ/E 그래프로 변환하는 함수 (TGraphErrors 버전)
  TGraphErrors* Transform_to_InvSqrtE_GraphErrors(TGraphAsymmErrors* originalGraph)
  {
    Int_t nPoints = originalGraph->GetN();
    TGraphErrors* transformedGraph = new TGraphErrors(nPoints);
    
    for(Int_t i = 0; i < nPoints; i++) {
      Double_t energy, resolution;
      originalGraph->GetPoint(i, energy, resolution);
      
      // X축 변환: E → 1/√E
      Double_t invSqrtE = 1.0 / TMath::Sqrt(energy);
      
      // Y축은 그대로 (σ/E)
      transformedGraph->SetPoint(i, invSqrtE, resolution);
      
      // 에러도 변환 (Y 에러만 복사)
      Double_t errorY = originalGraph->GetErrorYlow(i);
      transformedGraph->SetPointError(i, 0, errorY);
    }
    
    return transformedGraph;
  }

  // TB2022 스타일 DrawFitReverse 함수
  TGraph* DrawFitReverse_TGraph(TF1 *f)
  {
    f->SetBit(TF1::kNotDraw);
    auto color = f->GetLineColor();

    auto gf = new TGraph();
    gf->SetLineColor(color);
    gf->SetLineWidth(1);

    double x1 = 0.087;   // 1/√120 (120 GeV)
    double x2 = 0.316;   // 1/√20 (20 GeV)

    gf->SetPoint(0, x1, f->Eval(x1));
    gf->SetPoint(1, x2, f->Eval(x2));

    return gf;
  }

  // TGraphErrors를 TGraphAsymmErrors로 변환하는 함수
  TGraphAsymmErrors* Convert_TGraphErrors_to_TGraphAsymmErrors(TGraphErrors* g_errors)
  {
    Int_t nPoints = g_errors->GetN();
    TGraphAsymmErrors* g_asymm = new TGraphAsymmErrors(nPoints);
    
    for(Int_t i = 0; i < nPoints; i++)
    {
      Double_t x, y;
      g_errors->GetPoint(i, x, y);
      
      Double_t errorX = g_errors->GetErrorX(i);
      Double_t errorY = g_errors->GetErrorY(i);
      
      g_asymm->SetPoint(i, x, y);
      g_asymm->SetPointError(i, errorX, errorX, errorY, errorY);
    }
    return g_asymm;
  }

  void ProducePlot_Linearity_vs_E() {
    TString canvasName = TString::Format("c_%sLinearity_%s", beamType_.Data(), centerTowerTag_.Data());
    if(noNoiseReverse_) canvasName += "_NN";
    
    PlotTool::GraphCanvas* canvas = new PlotTool::GraphCanvas(canvasName, 0, 0);
    canvas->SetTitle("E [GeV]", "E_{measured}/E_{beam}");

    canvas->Register(map_linearity_["C"],   "C ",   kBlue);
    canvas->Register(map_linearity_["S"],   "S ",   kRed);
    if( beamType_ == "em" ) 
    {
      canvas->Register(map_linearity_["Comb"], "C+S ", kBlack);
    }
    else if( beamType_ == "pi" || beamType_ == "kaon" || beamType_ == "proton" )
    {
      canvas->Register(map_linearity_["DRcorrection"], "DR correction", kMagenta);
      canvas->Register(map_linearity_["DRcorrection_LC_compensated"], "DR correction + LC", kGreen+2);
    }
    
    // canvas->SetMarkerSize(2.0);

    canvas->SetLegendPosition(0.50, 0.70, 0.95, 0.95);

    // canvas->SetRangeX(minX, maxX);
    if( beamType_ == "em" ) canvas->SetRangeY(0.7, 1.5);
    else if( beamType_ == "pi" || beamType_ == "kaon" || beamType_ == "proton" ) canvas->SetRangeY(0.0, 1.5);
    // canvas->SetAutoRangeY();

    // canvas->Latex_CMSInternal();
    canvas->RegisterLatex(0.14, 0.96, 62, 0.8, "TB2025");
    TString beamInfo;
    if(beamType_ == "em") {beamInfo = "e+ beam (6 - 120 GeV)";}
    else if(beamType_ == "pi") {beamInfo = "#pi^{+} beam (40 - 120 GeV)";}
    else if(beamType_ == "kaon") {beamInfo = "#K^{+} beam (40 - 100 GeV)";}
    else if(beamType_ == "proton") {beamInfo = "proton beam (40 - 100 GeV)";}
    canvas->RegisterLatex(0.65, 0.96, 62, 0.6, beamInfo);

    TString centerInfo = "Center: "+centerTowerTag_;

    canvas->RegisterLatex(0.16, 0.91, 42, 0.6, centerInfo);

    // NoNoise 옵션에 따른 파일명 설정 - 리니어리티 플롯
    canvas->SetSavePath(plotPath_);
    canvas->Draw();
    
  }

  void ProducePlot_PerPoint(int i_point, int run, double energy, TString fiberType) {
    // 히스토그램 이름 설정 (evtloop_emDRC.cc와 HistogramManager.h 참고)
    TString histName = "";
    if (beamType_ == "em") {
      if (fiberType == "Comb") histName = "totalEdepScaled_Comb_5_MC";      // Combined (C+S) channel, scaled, final cuts
      else if (fiberType == "C") histName = "totalEdepScaled_C_5_MC";      // C channel, scaled, final cuts
      else if (fiberType == "S") histName = "totalEdepScaled_S_5_MC";      // S channel, scaled, final cuts
    } 
    else if(beamType_ == "pi" || beamType_ == "kaon" || beamType_ == "proton")
    {
      // 하드론 빔의 경우 다른 히스토그램 이름들 사용 (추후 구현)
      if (fiberType == "Comb") histName = "totalEdepScaled_Comb_6_MC";      // Combined (C+S) channel, scaled, final cuts
      else if (fiberType == "C") histName = "totalEdepScaled_C_6_MC";      // C channel, scaled, final cuts
      else if (fiberType == "S") histName = "totalEdepScaled_S_6_MC";      // S channel, scaled, final cuts
      else if(fiberType == "DRcorrection")          histName = "totalEdepScaled_Comb_DRCorrection_6_MC";
      else if(fiberType == "DRcorrection_LC_compensated") histName = "totalEdepScaled_Comb_DRCorrection_LC_compensated_6_MC";
      else if(fiberType == "S_LC_compensated")            histName = "totalEdepScaled_S_LC_compensated_6_MC";
      else if(fiberType == "Comb_LC_compensated")              histName = "totalEdepScaled_Comb_LC_compensated_6_MC";
      else                                     histName = TString::Format("totalEdepScaled_%s_6_MC", fiberType.Data());
    }

    // Partial tower 기능은 현재 evtloop_emDRC.cc에서 지원하지 않음
    // if( use_partialTower_ )
    //   histName.ReplaceAll("eDep_all", "eDep_"+tag_partialTower_);

    // Merged 파일에서 히스토그램 가져오기
    TString fileName = Get_FileName(energy);
    TH1D* h_eDep_combined = PlotTool::Get_Hist(fileName, histName);
    double totalEvents = h_eDep_combined->Integral();
    
    cout << "(energy, fiber) = (" << energy << ", " << fiberType << "): # events = " << totalEvents << " (from merged file: " << fileName << ")" << endl;

    // -- from histogram values (no fit)
    double mean   = h_eDep_combined->GetMean();
    double stdDev = h_eDep_combined->GetStdDev();
    double resol;
    double minRange, maxRange, rangedMean, rangedStdDev;
    if(beamType_ == "pi")
    {
      if(energy == 20)
      {
        if(fiberType == "C")
        {
          minRange = TB2025::map_pi_ReRange_C[TString::Format("%d", (int)energy)].first; maxRange = TB2025::map_pi_ReRange_C[TString::Format("%d", (int)energy)].second;
          rangedMean = CalculateRangedMean(h_eDep_combined, minRange, maxRange); rangedStdDev = CalculateRangedStdDev(h_eDep_combined, minRange, maxRange, rangedMean);
          mean = rangedMean; stdDev = rangedStdDev; resol = rangedStdDev / rangedMean;
        }
        else if(fiberType == "S")
        {
          minRange = TB2025::map_pi_ReRange_S[TString::Format("%d", (int)energy)].first; maxRange = TB2025::map_pi_ReRange_S[TString::Format("%d", (int)energy)].second;
          rangedMean = CalculateRangedMean(h_eDep_combined, minRange, maxRange); rangedStdDev = CalculateRangedStdDev(h_eDep_combined, minRange, maxRange, rangedMean);
          mean = rangedMean; stdDev = rangedStdDev; resol = rangedStdDev / rangedMean;
        }
      }
      else if(energy == 40)
      {
        if(fiberType == "C")
        {
          minRange = TB2025::map_pi_ReRange_C[TString::Format("%d", (int)energy)].first; maxRange = TB2025::map_pi_ReRange_C[TString::Format("%d", (int)energy)].second;
          rangedMean = CalculateRangedMean(h_eDep_combined, minRange, maxRange); rangedStdDev = CalculateRangedStdDev(h_eDep_combined, minRange, maxRange, rangedMean);
          mean = rangedMean; stdDev = rangedStdDev; resol = rangedStdDev / rangedMean;
        }
        else if(fiberType == "S")
        {
          minRange = TB2025::map_pi_ReRange_S[TString::Format("%d", (int)energy)].first; maxRange = TB2025::map_pi_ReRange_S[TString::Format("%d", (int)energy)].second;
          rangedMean = CalculateRangedMean(h_eDep_combined, minRange, maxRange); rangedStdDev = CalculateRangedStdDev(h_eDep_combined, minRange, maxRange, rangedMean);
          mean = rangedMean; stdDev = rangedStdDev; resol = rangedStdDev / rangedMean;
        }
      }
      else if(energy == 60)
      {
        if(fiberType == "C")
        {
          minRange = TB2025::map_pi_ReRange_C[TString::Format("%d", (int)energy)].first; maxRange = TB2025::map_pi_ReRange_C[TString::Format("%d", (int)energy)].second;
          rangedMean = CalculateRangedMean(h_eDep_combined, minRange, maxRange); rangedStdDev = CalculateRangedStdDev(h_eDep_combined, minRange, maxRange, rangedMean);
          mean = rangedMean; stdDev = rangedStdDev; resol = rangedStdDev / rangedMean;
        }
        else if(fiberType == "S")
        {
          minRange = TB2025::map_pi_ReRange_S[TString::Format("%d", (int)energy)].first; maxRange = TB2025::map_pi_ReRange_S[TString::Format("%d", (int)energy)].second;
          rangedMean = CalculateRangedMean(h_eDep_combined, minRange, maxRange); rangedStdDev = CalculateRangedStdDev(h_eDep_combined, minRange, maxRange, rangedMean);
          mean = rangedMean; stdDev = rangedStdDev; resol = rangedStdDev / rangedMean;
        }
      }
      else if(energy == 80)
      {
        if(fiberType == "C")
        {
          minRange = TB2025::map_pi_ReRange_C[TString::Format("%d", (int)energy)].first; maxRange = TB2025::map_pi_ReRange_C[TString::Format("%d", (int)energy)].second;
          rangedMean = CalculateRangedMean(h_eDep_combined, minRange, maxRange); rangedStdDev = CalculateRangedStdDev(h_eDep_combined, minRange, maxRange, rangedMean);
          mean = rangedMean; stdDev = rangedStdDev; resol = rangedStdDev / rangedMean;
        }
        else if(fiberType == "S")
        {
          minRange = TB2025::map_pi_ReRange_S[TString::Format("%d", (int)energy)].first; maxRange = TB2025::map_pi_ReRange_S[TString::Format("%d", (int)energy)].second;
          rangedMean = CalculateRangedMean(h_eDep_combined, minRange, maxRange); rangedStdDev = CalculateRangedStdDev(h_eDep_combined, minRange, maxRange, rangedMean);
          mean = rangedMean; stdDev = rangedStdDev; resol = rangedStdDev / rangedMean;
        }
      }
      else if(energy == 100)
      {
        if(fiberType == "C")
        {
          minRange = TB2025::map_pi_ReRange_C[TString::Format("%d", (int)energy)].first; maxRange = TB2025::map_pi_ReRange_C[TString::Format("%d", (int)energy)].second;
          rangedMean = CalculateRangedMean(h_eDep_combined, minRange, maxRange); rangedStdDev = CalculateRangedStdDev(h_eDep_combined, minRange, maxRange, rangedMean);
          mean = rangedMean; stdDev = rangedStdDev; resol = rangedStdDev / rangedMean;
        }
        else if(fiberType == "S")
        {
          minRange = TB2025::map_pi_ReRange_S[TString::Format("%d", (int)energy)].first; maxRange = TB2025::map_pi_ReRange_S[TString::Format("%d", (int)energy)].second;
          rangedMean = CalculateRangedMean(h_eDep_combined, minRange, maxRange); rangedStdDev = CalculateRangedStdDev(h_eDep_combined, minRange, maxRange, rangedMean);
          mean = rangedMean; stdDev = rangedStdDev; resol = rangedStdDev / rangedMean;
        }
      }
      else if(energy == 120)
      {
        if(fiberType == "C")
        {
          minRange = TB2025::map_pi_ReRange_C[TString::Format("%d", (int)energy)].first; maxRange = TB2025::map_pi_ReRange_C[TString::Format("%d", (int)energy)].second;
          rangedMean = CalculateRangedMean(h_eDep_combined, minRange, maxRange); rangedStdDev = CalculateRangedStdDev(h_eDep_combined, minRange, maxRange, rangedMean);
          mean = rangedMean; stdDev = rangedStdDev; resol = rangedStdDev / rangedMean;
        }
        else if(fiberType == "S")
        {
          minRange = TB2025::map_pi_ReRange_S[TString::Format("%d", (int)energy)].first; maxRange = TB2025::map_pi_ReRange_S[TString::Format("%d", (int)energy)].second;
          rangedMean = CalculateRangedMean(h_eDep_combined, minRange, maxRange); rangedStdDev = CalculateRangedStdDev(h_eDep_combined, minRange, maxRange, rangedMean);
          mean = rangedMean; stdDev = rangedStdDev; resol = rangedStdDev / rangedMean;
        }
      }
    }
    else {resol = stdDev / mean;}
    double absUnc_mean   = h_eDep_combined->GetMeanError();
    double absUnc_stdDev = h_eDep_combined->GetStdDevError();
    double absUnc_resol  = AbsUnc_AoverB(stdDev, absUnc_stdDev, mean, absUnc_mean);

    // -- fit
    GausFitter fitter(h_eDep_combined);
    bool doFit = false;
    // EM 입자인 경우와 하드로닉 입자에서 DR Corrected, LC 채널인 경우에만 피팅 수행
    if(beamType_ == "em" && !noFit_) {doFit = true;}
    else if((beamType_ == "pi" || beamType_ == "kaon" || beamType_ == "proton") && (fiberType == "DRcorrection" || fiberType == "DRcorrection_LC_compensated"))
    {doFit = true;}
    
    // 히스토그램이 비어있으면 피팅 건너뛰기
    if(doFit && h_eDep_combined->GetEntries() == 0) {
      doFit = false;
    }
    
    if(doFit)
    {
      if(fiberType == "Comb") fitter.SetColor(kBlue);
      fitter.Fit();
    }

    // NoNoise 옵션인 경우 플롯 저장 생략, 데이터만 처리
    if(noNoiseReverse_) {
      // 데이터 처리만 수행 (그래프 채우기)
      if(!doFit) Fill_TGraph(fiberType, i_point, energy, mean, absUnc_mean, resol, absUnc_resol);
      else       Fill_TGraph(fiberType, i_point, energy, fitter.Get("mu"), fitter.GetUnc("mu"), fitter.Get("resol"), fitter.GetUnc("resol"));
      
      return; // 플롯 저장 없이 함수 종료
    }

    // 기본 모드에서만 플롯 생성 및 저장
    TString canvasName = TString::Format("eDep_%s_energy%.0lf_run%d", fiberType.Data(), energy, run);
    // if( use_partialTower_ )
    //   canvasName.ReplaceAll("eDep", "eDep_"+tag_partialTower_);

    PlotTool::HistCanvas* canvas = new PlotTool::HistCanvas(canvasName, 0, 0);
    canvas->SetTitle("Energy deposit [GeV]", "# events");

    int color;
    if( fiberType == "C" ) color = kBlue;
    if( fiberType == "S" ) color = kRed;
    if( fiberType == "Comb" ) color = kBlack;
    if( fiberType == "DRcorrection" ) color = kMagenta;
    if( fiberType == "DRcorrection_LC_compensated" ) color = kGreen+2;

    canvas->Register(h_eDep_combined, "", color);

    canvas->SetLegendPosition(0, 0, 0, 0);

    // canvas->SetRangeX(minX, maxX);
    // canvas->SetRangeY(0, 200);
    // canvas->SetRangeRatio(0.7, 1.3);
    canvas->SetAutoRangeY();
    // canvas->SetAutoRangeRatio();
    
    canvas->RegisterLatex(0.14, 0.96, 62, 0.8, "TB2025");
    TString beamInfo;
    if(beamType_ == "em") {beamInfo = TString::Format("e+ beam (%.0lf GeV)", energy);}
    else if(beamType_ == "pi") {beamInfo = TString::Format("#pi^{+} beam (%.0lf GeV)", energy);}
    else if(beamType_ == "kaon") {beamInfo = TString::Format("#K^{+} beam (%.0lf GeV)", energy);}
    else if(beamType_ == "proton") {beamInfo = TString::Format("proton beam (%.0lf GeV)", energy);}
    canvas->RegisterLatex(0.73, 0.96, 62, 0.6, beamInfo);

    TString runInfo = TString::Format("%s channel(%s scan @ %s, %.0lf GeV)", fiberType.Data(), beamType_.Data(), centerTowerTag_.Data(), energy);

    canvas->RegisterLatex(0.16, 0.91, 42, 0.6, runInfo);

    if(!doFit) {
      TString histInfo = TString::Format("Hist. statistics (# events = %.0lf):", h_eDep_combined->Integral(0, h_eDep_combined->GetNbinsX()+1));
      canvas->RegisterLatex(0.16, 0.87, 62, 0.6, histInfo);
      TString muInfo    = TString::Format("mean = %.2lf #pm %.2lf GeV", mean, absUnc_mean);
      TString sigmaInfo = TString::Format("std.dev. = %.2lf #pm %.2lf GeV", stdDev, absUnc_stdDev);
      TString resolInfo = TString::Format("std.dev/mean = %.2lf #pm %.2lf", resol, absUnc_resol);
      canvas->RegisterLatex(0.16, 0.83, 42, 0.6, muInfo);
      canvas->RegisterLatex(0.16, 0.79, 42, 0.6, sigmaInfo);
      canvas->RegisterLatex(0.16, 0.75, 42, 0.6, resolInfo);
    }
    else {
      TString fitInfo = TString::Format("Fit results (#chi2/nDOF = %.2lf/%.0lf):", fitter.Get("chi2"), fitter.Get("nDOF"));
      canvas->RegisterLatex(0.16, 0.87, 62, 0.6, fitInfo);

      TString muInfo    = TString::Format("#mu = %.2lf #pm %.2lf GeV", fitter.Get("mu"), fitter.GetUnc("mu"));
      TString sigmaInfo = TString::Format("#sigma = %.2lf #pm %.2lf GeV", fitter.Get("sigma"), fitter.GetUnc("sigma"));
      TString resolInfo = TString::Format("#sigma/#mu = %.2lf #pm %.2lf", fitter.Get("resol"), fitter.GetUnc("resol"));
      canvas->RegisterLatex(0.16, 0.83, 42, 0.6, muInfo);
      canvas->RegisterLatex(0.16, 0.79, 42, 0.6, sigmaInfo);
      canvas->RegisterLatex(0.16, 0.75, 42, 0.6, resolInfo);
    }

    // Partial tower 관련 정보 표시 (현재 미사용)
    // if( use_partialTower_ ) {
    //   if( tag_partialTower_.Contains("near") ) {
    //     TString towerName = tag_partialTower_;
    //     towerName.ReplaceAll("near", "");
    //     TString sumInfo = TString::Format("Sum of the towers near %s only (9 towers)", towerName.Data());
    //     canvas->RegisterLatex(0.16, 0.71, 42, 0.5, sumInfo);
    //   }
    // }

    // canvas->RemoveRatioError(); // remove error in the ratio (useful when the error is meaningless)

    // 기본 모드에서만 저장 경로 설정 및 플롯 저장
    TString path = TString::Format("%s/%s", plotPath_.Data(), fiberType.Data());
    PlotTool::Make_Dir_IfNotExist(path);
    canvas->SetSavePath(path);
    
    canvas->Draw("hist");

    // DRCorrection 관련 채널에서 피팅이 수행된 경우 피팅 함수를 히스토그램 위에 그리기
    if(doFit && (fiberType == "DRcorrection" || fiberType == "DRcorrection_LC_compensated")) {
      canvas->c_->cd();
      
      TF1* fitFunc = fitter.GetFunction();
      if(!fitFunc) {
        fitFunc = h_eDep_combined->GetFunction("g");
      }
      
      if(fitFunc) {
        fitFunc->SetLineColor(kBlack);
        fitFunc->SetLineWidth(2);
        fitFunc->SetLineStyle(1);
        fitFunc->Draw("SAME");
        
        // 피팅 함수가 포함된 상태로 다시 저장
        canvas->c_->Update();
        TString savePath = path + "/" + canvasName + ".pdf";
        canvas->c_->SaveAs(savePath);
      }
    }

    if(!doFit)
      Fill_TGraph(fiberType, i_point, energy, mean, absUnc_mean, resol, absUnc_resol);
    else
      Fill_TGraph(fiberType, i_point, energy, fitter.Get("mu"), fitter.GetUnc("mu"), fitter.Get("resol"), fitter.GetUnc("resol"));
  }

  void Fill_TGraph(TString fiberType, int i_point, double energy, 
                   double mu, double absUnc_mu, double resol, double absUnc_resol) {

    map_resol_[fiberType]->SetPoint(i_point, energy, resol);
    map_resol_[fiberType]->SetPointError(i_point, 0, 0, absUnc_resol, absUnc_resol);

    double linearity     = mu / energy;
    double unc_linearity = absUnc_mu / energy;
    if( fiberType == "Comb" ) { // -- divide one more
      linearity /= 2.0;
      unc_linearity /= 2.0;
      printf("linearity = %lf\n", linearity);
    }
    map_linearity_[fiberType]->SetPoint(i_point, energy, linearity);
    map_linearity_[fiberType]->SetPointError(i_point, 0, 0, unc_linearity, unc_linearity);
  }

  TString Get_FileName(double energy) { 
    // Merged 디렉토리의 파일 경로 구성
    // ../../Guk/output/em/merged/em_M5T2_120GeV.root 형식
    
    // 센터 타워명을 형식에 맞게 변환 (M5-T2 -> M5T2)
    TString centerStr = centerTowerTag_;
    centerStr.ReplaceAll("-", "");
    
    return TString::Format("../output/%s/merged/%s_%s_%.0lfGeV.root", beamType_.Data(), beamType_.Data(), centerStr.Data(), energy);
    //return TString::Format("../output/pi/pi_M5T2_12352.root");
  }

  double AbsUnc_AoverB(double A, double unc_A, double B, double unc_B) {
    if( B == 0 ) return 0; // -- A/B is not defined

    double relUnc_A = (A == 0) ? 0.0 : unc_A/A;
    double relUnc_B = (B == 0) ? 0.0 : unc_B/B;
    double relUnc = std::sqrt(relUnc_A*relUnc_A + relUnc_B*relUnc_B);

    return (A/B)*relUnc;
  }

  // 변환 함수 수정 (TGraphAsymmErrors 유지)
  TGraphAsymmErrors* Transform_to_InvSqrtE_GraphAsymm(TGraphAsymmErrors* originalGraph)
  {
    Int_t nPoints = originalGraph->GetN();
    TGraphAsymmErrors* transformedGraph = new TGraphAsymmErrors(nPoints);
    
    for(Int_t i = 0; i < nPoints; i++) {
      Double_t energy, resolution;
      originalGraph->GetPoint(i, energy, resolution);
      
      // X축 변환: E → 1/√E
      Double_t invSqrtE = 1.0 / TMath::Sqrt(energy);
      
      transformedGraph->SetPoint(i, invSqrtE, resolution);
      
      // 에러 변환
      transformedGraph->SetPointEXlow(i, 0);
      transformedGraph->SetPointEXhigh(i, 0);
      transformedGraph->SetPointEYlow(i, originalGraph->GetErrorYlow(i));
      transformedGraph->SetPointEYhigh(i, originalGraph->GetErrorYhigh(i));
    }
    
    return transformedGraph;
  }
};