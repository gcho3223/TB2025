#include "TFile.h"
#include "TH1D.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TROOT.h"
#include "TPad.h"
#include "TSystem.h"
#include "TColor.h"
#include "TPaveStats.h"

void DrawOverlay(TH1D *hist1, TH1D *hist2, TH1D *hist3, string run1, string run2, string run3, string savepath)
{
    // version3가 비어있으면 2개 버전 모드, 있으면 3개 버전 모드
    bool isThreeVersionMode = !run3.empty() && hist3 != nullptr;
    
    // color setting
    vector<int> colors = {
        kBlack,        // hist1
        kP10Red,       // hist2
        kP10Blue,      // hist3
    };
    
    TString hname = hist1->GetName();
    
    // 실제 표시 범위를 추적하기 위한 변수들
    double xmin = 0, xmax = 0;
    bool hasDrawnOverflowBin = false;

    // color setting
    //hist1->SetMarkerColor(colors[0]);
    //hist1->SetMarkerSize(0.75);
    hist1->SetLineColor(colors[0]);
    hist1->SetLineWidth(1);

    //hist2->SetMarkerColor(colors[1]);
    //hist2->SetMarkerStyle(20);
    //hist2->SetMarkerSize(0.75);
    hist2->SetLineColor(colors[1]);
    hist2->SetLineWidth(1);

    if(isThreeVersionMode)
    {
        //hist3->SetMarkerColor(colors[2]);
        //hist3->SetMarkerStyle(22);
        //hist3->SetMarkerSize(0.75);
        hist3->SetLineColor(colors[2]);
        hist3->SetLineWidth(1);
    }
    
    // create canvas
    TCanvas *c = new TCanvas("c", hname, 800, 800);
    c->cd();
	c->SetMargin(0.12,0.12,0.12,0.12);
    
    // top pad (histogram)
    //TPad *pad1 = new TPad("pad1", "pad1", 0, 0.3, 1, 1.0);
    //pad1->SetBottomMargin(0.02);
    //pad1->SetLeftMargin(0.12);
    //pad1->SetRightMargin(0.04);
    //pad1->SetTopMargin(0.1);
    //pad1->Draw();
    //pad1->cd();

    hist1->SetStats(0);
    hist2->SetStats(0);
    if(isThreeVersionMode) hist3->SetStats(0);

    hist1->SetTitle("");
    if(hist1->GetMaximum() > hist2->GetMaximum()) {hist1->Draw("hist"); hist2->Draw("hist same");}
    else {hist2->Draw("hist"); hist1->Draw("hist same");}
    if(isThreeVersionMode) hist3->Draw("hist same");

    if(hname.Contains("MC"))
    {
        c->SetLogy();
        hist1->GetXaxis()->SetRangeUser(0, 500);
        hist2->GetXaxis()->SetRangeUser(0, 500);
        if(isThreeVersionMode) hist3->GetXaxis()->SetRangeUser(0, 500);
    }

    // 수동으로 stat box 생성
    TPaveStats *h1stat = new TPaveStats(0.7, 0.7, 0.96, 0.9, "NDC");
    h1stat->SetBorderSize(1);
    h1stat->SetLineColor(colors[0]);
    h1stat->AddText(Form("Run %s (TB2025)",run1.c_str()));
    h1stat->SetTextColor(colors[0]);
    h1stat->SetTextSize(0.02);
    h1stat->SetFillColor(0);
    h1stat->AddText(Form("Entries: %.0f", hist1->GetEntries()));
    h1stat->AddText(Form("Mean: %.3f", hist1->GetMean()));
    h1stat->AddText(Form("Std dev: %.3f", hist1->GetRMS()));
    h1stat->Draw();

    TPaveStats *h2stat = new TPaveStats(0.7, 0.5, 0.96, 0.7, "NDC");
    h2stat->SetBorderSize(1);
    h2stat->SetLineColor(colors[1]);
    h2stat->AddText(Form("Run%s (TB2024)",run2.c_str()));
    h2stat->SetTextColor(colors[1]);
    h2stat->SetTextSize(0.02);
    h2stat->SetFillColor(0);
    h2stat->AddText(Form("Entries: %.0f", hist2->GetEntries()));
    h2stat->AddText(Form("Mean: %.3f", hist2->GetMean()));
    h2stat->AddText(Form("Std dev: %.3f", hist2->GetRMS()));
    h2stat->Draw();
    
    /*
    //------------------------------------ ratio plot ------------------------------------
    c->cd();
    // bottom pad (ratio plot)
    TPad *pad2 = new TPad("pad2", "pad2", 0, 0.0, 1, 0.3);
    pad2->SetTopMargin(0.03);
    pad2->SetBottomMargin(0.3);
    pad2->SetLeftMargin(0.12);
    pad2->SetRightMargin(0.04);
    pad2->Draw();
    pad2->cd();
    
    // create ratio histogram (if data exists)
    TH1D *h_ratio1 = nullptr;
    TH1D *h_ratio2 = nullptr;
    
    if(hist1 && hist2)
    {
        h_ratio1 = (TH1D*)hist2->Clone("h_ratio1");
        if(isThreeVersionMode) h_ratio2 = (TH1D*)hist3->Clone("h_ratio2");
        //----------------------------------------------------
        h_ratio1->SetDirectory(0);
        h_ratio1->Divide(hist1); // hist2/hist1
        
        h_ratio1->SetTitle("");
        h_ratio1->SetStats(0);
        h_ratio1->SetMarkerStyle(20);
        h_ratio1->SetMarkerSize(0.5);
        h_ratio1->SetLineWidth(1);
        h_ratio1->SetLineColor(colors[1]);
        h_ratio1->SetMarkerColor(colors[1]);
        
        h_ratio1->GetXaxis()->SetTitle(xlabel.c_str());
        h_ratio1->GetXaxis()->SetTitleSize(0.12);
        h_ratio1->GetXaxis()->SetLabelSize(0.10);
        h_ratio1->GetXaxis()->SetTickLength(0.07);
        
        //h_ratio1->GetYaxis()->SetTitle(Form("%s/%s",run2.c_str(),run1.c_str()));
        h_ratio1->GetYaxis()->SetTitle(Form("Ratio"));
        h_ratio1->GetYaxis()->SetTitleSize(0.10);
        h_ratio1->GetYaxis()->SetLabelSize(0.08);
        h_ratio1->GetYaxis()->SetTitleOffset(0.5);
        h_ratio1->GetYaxis()->SetNdivisions(505);
        h_ratio1->GetYaxis()->SetRangeUser(0.8, 1.1); // set ratio range
        //----------------------------------------------------
        
        if(isThreeVersionMode)
        {
            h_ratio2->SetDirectory(0);
            h_ratio2->Divide(hist1); // hist3/hist1
            
            h_ratio2->SetTitle("");
            h_ratio2->SetStats(0);
            h_ratio2->SetMarkerStyle(20);
            h_ratio2->SetMarkerSize(0.5);
            h_ratio2->SetLineWidth(1);
            h_ratio2->SetLineColor(colors[2]);
            h_ratio2->SetMarkerColor(colors[2]);
            
            h_ratio2->GetYaxis()->SetNdivisions(505);
            h_ratio2->GetYaxis()->SetRangeUser(0.8, 1.1); // set ratio range
        }
        
        h_ratio1->Draw("ep");
        if(isThreeVersionMode) h_ratio2->Draw("ep same");

        c->Update();
        
        // draw line = 1 with correct range
        double lineXmin, lineXmax;
        if(hasDrawnOverflowBin) {lineXmin = xmin; lineXmax = xmax;}
        else {lineXmin = h_ratio1->GetXaxis()->GetXmin(); lineXmax = h_ratio1->GetXaxis()->GetXmax();}
        
        line = new TLine(lineXmin, 1.0, lineXmax, 1.0);
        line->SetLineStyle(2);
        line->SetLineColor(kRed);
        line->Draw();
    }
    */
    // save canvas
    gSystem->mkdir(Form("%s",savepath.c_str()),kTRUE);
    c->SaveAs(Form("%s/%s.pdf",savepath.c_str(),hname.Data()));
    
    // clean up memory
    delete c;
    delete hist1;
    delete hist2;
    if(isThreeVersionMode) delete hist3;
}