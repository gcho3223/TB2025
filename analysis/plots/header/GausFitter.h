class GausFitter {
public:
  GausFitter(TH1D* h): h_(h) { }

  void SetColor(int color) { color_ = color; }

  void Set_FitRange(double fitRangeConst) { fitRangeConst_ = fitRangeConst; }

  void Fit()
  {
    isFit_ = kTRUE;

    double lowerEdge = h_->GetMean() - fitRangeConst_*h_->GetStdDev();
    double upperEdge = h_->GetMean() + fitRangeConst_*h_->GetStdDev();

    // double energy_maxEvt = h_->GetBinCenter( h_->GetMaximumBin() );
    // double lowerEdge = energy_maxEvt - fitRangeConst_*h_->GetStdDev();
    // double upperEdge = energy_maxEvt + fitRangeConst_*h_->GetStdDev();
    // printf("[GausFitter::Fit] (e(maxEvt), stdDev) = (%.1lf, %.1lf) -> fit range: %.1lf < x < %.1lf\n", energy_maxEvt, h_->GetStdDev(), lowerEdge, upperEdge);

    TF1 *g = new TF1("g", "gaus", lowerEdge, upperEdge);
    
    g->SetLineColor(kBlack);
    if(color_ != 0) g->SetLineColor(color_); // -- custom color

    // double energy_maxEvt = h_->GetBinCenter( h_->GetMaximumBin() );
    // g->SetParameters(1, energy_maxEvt, h_->GetStdDev()); // -- initialize

    g->SetParameters(1, h_->GetMean(), h_->GetStdDev()); // -- initialize

    // h_->Fit("g", "", "", lowerEdge, upperEdge);
    h_->Fit("g");
    
    // 피팅 함수 저장
    fitFunc_ = h_->GetFunction("g");
    
    if(fitFunc_)
    {
      chi2_ = fitFunc_->GetChisquare();
      nDOF_ = fitFunc_->GetNDF();

      mu_     = fitFunc_->GetParameter(1);
      unc_mu_ = fitFunc_->GetParError(1);
      sigma_     = fitFunc_->GetParameter(2);
      unc_sigma_ = fitFunc_->GetParError(2);
      resol_     = (mu_ == 0) ? 0 : sigma_/mu_;
      unc_resol_ = AbsUnc_AoverB(sigma_, unc_sigma_, mu_, unc_mu_);
    }
  }

  double Get(TString var)
  {
    if(!isFit_) Fit();

    if(var == "mu")    return mu_;
    if(var == "sigma") return sigma_;
    if(var == "resol") return resol_;
    // -- fit quality
    if(var == "chi2")     return chi2_;
    if(var == "nDOF")     return nDOF_;
    if(var == "normChi2") return normChi2_;

    throw std::invalid_argument("var = " + var + " is not supported");
  }

  double GetUnc(TString var)
  {
    if(!isFit_) Fit();

    if(var == "mu")    return unc_mu_;
    if(var == "sigma") return unc_sigma_;
    if(var == "resol") return unc_resol_;

    throw std::invalid_argument("var = " + var + " is not supported");
  }

  TF1* GetFunction()
  {
    if(!isFit_) Fit();
    return fitFunc_;
  }

private:
  TH1D* h_;
  int color_ = 0;

  double fitRangeConst_ = 3.0; // -- +- 3*sigma range from the mean

  bool isFit_ = kFALSE;
  TF1* fitFunc_ = nullptr;  // 피팅 함수를 저장할 멤버 변수
  double mu_; // -- mean
  double unc_mu_;
  double sigma_;
  double unc_sigma_;
  double resol_; // -- sigma/mu
  double unc_resol_; // -- quad. sum of unc_mu & unc_sigma (actually not correct...)
  double chi2_;
  double nDOF_;
  double normChi2_;

  // -- abs. unc. on A/B
  double AbsUnc_AoverB(double A, double unc_A, double B, double unc_B)
  {
    if(B == 0) return 0; // -- A/B is not defined

    double relUnc_A = (A == 0) ? 0.0 : unc_A/A;
    double relUnc_B = (B == 0) ? 0.0 : unc_B/B;
    double relUnc = std::sqrt(relUnc_A*relUnc_A + relUnc_B*relUnc_B);

    return (A/B)*relUnc;
  }

};