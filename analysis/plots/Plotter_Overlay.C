//////////////////////////////////////////////////////////////////////////////////
//	? SinglePlotter.C Description												//
//	* root -l -b -q SinglePlotter.C'("version","channel")'						//
//	! version: version name														//
//	! channel: channel name														//
//////////////////////////////////////////////////////////////////////////////////
#include "DrawOverlay.cc"

void Plotter_Overlay(string run1, string run2, string run3 = "")
{
	string period = "UL2018";
	string version;

	vector<string> hist1D = {"Edep_M1_T1_S_0_noCut","Edep_M1_T1_S_1_DWC", "Edep_M1_T1_C_0_noCut","Edep_M1_T1_C_1_DWC",
							"intADC_M1_T1_S_0_noCut","intADC_M1_T1_S_1_DWC", "intADC_M1_T1_C_0_noCut","intADC_M1_T1_C_1_DWC",
							"MC_0_noCut","MC_1_DWC", "PS_0_noCut","PS_1_DWC",
	};
	vector<string> hist2D = {"DWC1_pos_0_noCut","DWC1_pos_1_DWC", "DWC2_pos_0_noCut","DWC2_pos_1_DWC",
							"DWC_x_corr_0_noCut","DWC_x_corr_1_DWC", "DWC_y_corr_0_noCut","DWC_y_corr_1_DWC",
	};
	
	// version3가 있으면 3개 버전 모드, 없으면 2개 버전 모드
	if(run3.empty()) {version = Form("%s_vs_%s",run1.c_str(),run2.c_str());}
	else				 {version = Form("%s_vs_%s_vs_%s",run1.c_str(),run2.c_str(),run3.c_str());}
	
	// load files
	TString path1	 = Form("../output/Calib/DRC_M7T3_full_Run%s.root",run1.c_str());
	TString path2	 = Form("../output/TB2024/Calib/DRC_M7T3_full_Run%s.root",run2.c_str());
	TString path3;
	if(!run3.empty())
	{path3 = Form("../output/Calib/DRC_M7T3_full_Run%s.root",run3.c_str());} // 3개 버전 모드

	TFile *f1	 = new TFile(path1,"READ");
	TFile *f2 	 = new TFile(path2,"READ");
	TFile *f3	 = nullptr;
	if(!run3.empty())
	{
		f3 = new TFile(path3,"READ");
		if(f1==NULL || f2==NULL || f3==NULL) {cout << "File is not found........." << endl;}
	}
	else {if(f1==NULL || f2==NULL) {cout << "File is not found........." << endl;}}
	
	string savepath = Form("./Overlay/%s",version.c_str());
	gSystem->mkdir(Form("%s",savepath.c_str()),kTRUE);
	
	/// draw histogram!! ///
	for(int ih1=0; ih1<hist1D.size(); ih1++)
	{
		TH1D *hist1 	= (TH1D*)f1->Get(Form("%s",hist1D[ih1].c_str()));
		TH1D *hist2 	= (TH1D*)f2->Get(Form("%s",hist1D[ih1].c_str()));
		TH1D *hist3 	= nullptr;
		if(!run3.empty())
		{
			hist3 = (TH1D*)f3->Get(Form("%s",hist1D[ih1].c_str()));
			if(hist1 == NULL || hist2 == NULL || hist3 == NULL)
			{cout << "hist1d: " << hist1D[ih1] << " is not found" << endl;}
		}
		else
		{
			if(hist1 == NULL || hist2 == NULL)
			{cout << "hist1d: " << hist1D[ih1] << " is not found" << endl;}
		}
		DrawOverlay(hist1,hist2,hist3,run1,run2,run3,savepath);
	}
	//TH1D *hist1 	= (TH1D*)f1->Get("h_nJets_beforeVeto");
	//TH1D *hist2 	= (TH1D*)f2->Get("h_nJets_afterVeto");
	//TH1D *hist3 	= nullptr;
	//DrawOverlayHistogram(hist1,hist2,hist3,run1,version2,savepath);
	delete f1;
	delete f2;
	if(f3) delete f3;
}