#include <iostream>
using namespace std;

void DrawDWC(TH2D *hist, string runnumber, string savepath)
{
	TString hname = hist->GetName();
	
	hist->SetStats(0);
	/// canvas ///
	TCanvas *c = new TCanvas("c",hname,800,800);
	c->cd();
	c->SetMargin(0.12,0.12,0.12,0.12);
	hist->Draw("colz");
	hist->SetTitle("");
	
	/// save canvas ///
	c->SaveAs(Form("%s/%s.pdf",savepath.c_str(),hname.Data()));

	delete c;
}