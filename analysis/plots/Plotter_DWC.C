//////////////////////////////////////////////////////////////////////////////////
//	? Plotter_Kinematics.C Description											//
//	* root -l -b -q 'Plotter_Kinematics.C("runnumber", "beamtype", "center", "shape")'	//
//	! runnumber: specific run number (if empty, use beamtype)					//
//	! beamtype: beam type for runlist processing								//
//	! center: center configuration (e.g. "M5T3")								//
//	! shape: shape configuration (e.g. "full")									//
//////////////////////////////////////////////////////////////////////////////////
#include "DrawDWC.cc"
#include <algorithm>
#include <cctype>

void Plotter_DWC(string runnumber = "")
{
	TString fin;
		
	/// file read ///
	fin = Form("../DWC/TB2024/DWC_Run_%s.root", runnumber.c_str());
	TFile *f = new TFile(fin,"READ");
	if(f==NULL || f->IsZombie()) {cout << "Error: File not found or corrupted: " << fin.Data() << endl;}

	string savepath;
		
	std::cout << "##################################################################################################################" << endl;
	std::cout << "                                     Processing Run: " << runnumber << "          " << endl;
	std::cout << "##################################################################################################################" << endl;

	/// draw histogram!! ///
	vector<string> dwc = {"dwc1_pos", "dwc2_pos", "dwc_x_corr", "dwc_y_corr"};
	
	string dwcSavepath = Form("./DWC/Run%s", runnumber.c_str());
	gSystem->mkdir(Form("%s",dwcSavepath.c_str()),kTRUE);
	for(int ih1=0; ih1<dwc.size(); ih1++)
	{
		TH2D *hist = (TH2D*)f->Get(Form("%s",dwc[ih1].c_str()));
		if(hist == NULL) {cout << "hist: " << dwc[ih1].c_str() << " is not found" << endl; continue;}
		DrawDWC(hist, runnumber, dwcSavepath);
	}
	f->Close();
	delete f;
	
	cout << "=== Plotter_DWC ===" << endl;
	cout << "- runnumber: " << runnumber << endl;
	cout << "=== Processing Complete ===" << endl;
}