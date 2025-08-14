//////////////////////////////////////////////////////////////////////////////////
//	? Plotter_Kinematics.C Description											//
//	* root -l -b -q 'Plotter_Kinematics.C("runnumber", "beamtype", "center", "shape")'	//
//	! runnumber: specific run number (if empty, use beamtype)					//
//	! beamtype: beam type for runlist processing								//
//	! center: center configuration (e.g. "M5T3")								//
//	! shape: shape configuration (e.g. "full")									//
//////////////////////////////////////////////////////////////////////////////////
#include "DrawATS.cc"
#include <algorithm>
#include <cctype>

void Plotter_ATS(string runnumber = "", string center = "")
{
	string module = "";
	string tower = "";
	string beamtype = "Calib";
	if(!center.empty())
	{
		string centerUpper = center;
		// 대문자로 변환
		transform(centerUpper.begin(), centerUpper.end(), centerUpper.begin(), ::toupper);
		
		// M과 T의 위치 찾기
		size_t mPos = centerUpper.find('M');
		size_t tPos = centerUpper.find('T');
		
		if(mPos != string::npos && tPos != string::npos && mPos < tPos)
		{
			// M 다음부터 T 전까지 추출하여 module 생성
			string moduleNum = centerUpper.substr(mPos + 1, tPos - mPos - 1);
			module = "M" + moduleNum;
			
			// T 다음부터 끝까지 추출하여 tower 생성
			string towerNum = centerUpper.substr(tPos + 1);
			tower = "T" + towerNum;
		}
	}

	TString fin;
		
	/// file read ///
	fin = Form("../Avg/%s/Avg_Run_%s.root", beamtype.c_str(), runnumber.c_str());
	TFile *f = new TFile(fin,"READ");
	if(f==NULL || f->IsZombie()) {cout << "Error: File not found or corrupted: " << fin.Data() << endl;}

	string savepath;
		
	std::cout << "##################################################################################################################" << endl;
	std::cout << "                                     Processing Run: " << runnumber << "          " << endl;
	std::cout << "##################################################################################################################" << endl;

	/// draw histogram!! ///
	vector<string> aux = {"CC1", "CC2", "PS", "MC", "TC", "LC2", "LC4", "LC8", "LC10", "LC3", "LC5", "LC7", "LC9", "LC11", "LC12", "LC13", "LC19", "LC14", "LC15", "LC16", "LC20"};
	vector<string> ch = {"C", "S"};
	
	// Aux 히스토그램 저장 경로 설정
	string auxSavepath = Form("./ATS/%s/Run%s/Aux", beamtype.c_str(), runnumber.c_str());
	gSystem->mkdir(Form("%s",auxSavepath.c_str()),kTRUE);
	for(int ih1=0; ih1<aux.size(); ih1++)
	{
		TH1D *hist = (TH1D*)f->Get(Form("%s",aux[ih1].c_str()));
		if(hist == NULL) {cout << "hist: " << aux[ih1].c_str() << " is not found" << endl; continue;}
		DrawATS(hist, runnumber, auxSavepath);
	}
	
	// Module 히스토그램 저장 경로 설정
	string moduleSavepath = Form("./ATS/%s/Run%s/Module", beamtype.c_str(), runnumber.c_str());
	gSystem->mkdir(Form("%s",moduleSavepath.c_str()),kTRUE);
	for(int ih2=0; ih2<ch.size(); ih2++)
	{
		TH1D *hmodule = (TH1D*)f->Get(Form("%s_%s_%s",module.c_str(),tower.c_str(),ch[ih2].c_str()));
		if(hmodule == NULL) {cout << "hmodule: " << module.c_str() << "_" << tower.c_str() << "_" << ch[ih2].c_str() << " is not found" << endl; continue;}
		DrawATS(hmodule, runnumber, moduleSavepath);
	}
	f->Close();
	delete f;
	
	cout << "=== Plotter_ATS ===" << endl;
	cout << "- runnumber: " << runnumber << endl;
	cout << "- module   : " << module << endl;
	cout << "- tower    : " << tower << endl;
	cout << "=== Processing Complete ===" << endl;
}