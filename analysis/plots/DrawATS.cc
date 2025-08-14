#include <iostream>
using namespace std;

void DrawATS(TH1D *hist, string runnumber, string savepath)
{
	int module_s_range[] = {260, 460};
	int module_c_range[] = {260, 420};
	int ps_range[] = {230, 370};
	int mc_range[] = {720, 880};
	int tc_range[] = {300, 450};
	int lc_range[] = {400, 600};
	int cc1_range[] = {700, 755};
	int cc2_range[] = {660, 745};
	
	TLine *line_v1;
	TLine *line_v2;
	TLatex *text1;
	TLatex *text2;

	TString hname = hist->GetName();
	
	/// canvas ///
	TCanvas *c = new TCanvas("c",hname,800,800);
	c->cd();
	c->SetMargin(0.12,0.12,0.12,0.12);
	hist->Draw("hist");
	hist->SetTitle("");
	hist->GetYaxis()->SetTitle("Entries");
	hist->GetYaxis()->SetTitleOffset(1.5);
	double ymax = hist->GetMaximum();
	
	// 조건에 따라 범위 설정 및 라인/텍스트 생성
	int start_range = 0;
	int end_range = 0;
	
	/// Cerenkov channel ///
	if(hname.Contains("_C"))		{start_range = module_c_range[0]; end_range = module_c_range[1];}
	else if(hname.Contains("_S")) 	{start_range = module_s_range[0]; end_range = module_s_range[1];}
	else if(hname.Contains("PS")) 	{hist->GetYaxis()->SetRangeUser(3300, 4000); start_range = ps_range[0]; end_range = ps_range[1];}
	else if(hname.Contains("MC")) 	{hist->GetYaxis()->SetRangeUser(3550, 3650); start_range = mc_range[0]; end_range = mc_range[1];}
	else if(hname.Contains("TC")) 	{hist->GetYaxis()->SetRangeUser(3500, 4000); start_range = tc_range[0]; end_range = tc_range[1];}
	else if(hname.Contains("LC")) 	{hist->GetYaxis()->SetRangeUser(3500, 4000); start_range = lc_range[0]; end_range = lc_range[1];}
	else if(hname.Contains("CC1")) 	{hist->GetYaxis()->SetRangeUser(3620, 3660); start_range = cc1_range[0]; end_range = cc1_range[1];}
	else if(hname.Contains("CC2")) 	{hist->GetYaxis()->SetRangeUser(3300, 4000); start_range = cc2_range[0]; end_range = cc2_range[1];}
	
	//if(start_range != 0 && end_range != 0)
	//{
	//	line_v1 = new TLine(start_range, 0, start_range, ymax);
	//	line_v2 = new TLine(end_range, 0, end_range, ymax);
	//	text1 = new TLatex(start_range-150, ymax-200, Form("Start %d", start_range));
	//	text2 = new TLatex(end_range+50, ymax-200, Form("End %d", end_range));
	//	
	//	line_v1->SetLineColor(kRed);
	//	line_v1->SetLineWidth(2);
	//	line_v1->Draw();
	//	line_v2->SetLineColor(kRed);
	//	line_v2->SetLineWidth(2);
	//	line_v2->Draw();
	//	text1->SetTextSize(0.02);
	//	text1->Draw();
	//	text2->SetTextSize(0.02);
	//	text2->Draw();
	//}

	/// save canvas ///
	c->SaveAs(Form("%s/%s.pdf",savepath.c_str(),hname.Data()));

	delete c;
}