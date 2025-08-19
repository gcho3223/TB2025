//////////////////////////////////////////////////////////////////////////////////
//	? Plotter_Kinematics.C Description											//
//	* root -l -b -q 'Plotter_Kinematics.C("runnumber", "beamtype", "center")'		//
//	! runnumber: specific run number (if empty, use beamtype)					//
//	! beamtype: beam type for runlist processing								//
//	! center: center configuration (e.g. "M5T2")								//
//////////////////////////////////////////////////////////////////////////////////
#include "DrawPlots.cc"
#include "header/histlist.h"
#include "header/RunInfo.h"

void Plotter_Plots(string runnumber = "", string beamtype = "", string center = "M5T2")
{
	// 히스토그램 리스트 초기화
	initializeHistogramLists();
	
	vector<string> runlist;
	vector<string> processRuns;
	vector<string> energyList;  // 에너지 리스트 (merged files용)
	
	cout << "=== Plotter_Plots ===" << endl;
	cout << "Input parameters:" << endl;
	cout << "  - runnumber: " << runnumber << endl;
	cout << "  - beamtype: " << beamtype << endl;
	cout << "  - center: " << center << endl;
	
	// runnumber가 주어진 경우: 해당 런만 처리
	if(!runnumber.empty() && runnumber != "")
	{
		processRuns.push_back(runnumber);
		cout << "Processing single run: " << runnumber << endl;
		
		// beamtype이 없으면 기본값 설정
		if(beamtype.empty()) beamtype = "calib";
	}
	// runnumber가 없는 경우: beamtype에 따른 런리스트 처리
	else if(!beamtype.empty() && beamtype != "")
	{
		if(beamtype == "calib")
		{
			// Calibration runs 사용
			for(int run : TB2025::vec_calibRun) {
				runlist.push_back(to_string(run));
			}
			cout << "Using calibration runs from RunInfo.h: " << runlist.size() << " runs" << endl;
		}
		else if(beamtype == "em") 
		{
			// EM beam merged files 사용
			TString centerTag = center;
			if(TB2025::map_emScanRun_merged.find(centerTag) != TB2025::map_emScanRun_merged.end()) {
				for(int energy : TB2025::map_emScanRun_merged.at(centerTag)) {
					energyList.push_back(to_string(energy));
				}
				cout << "Using EM beam merged files for " << centerTag << ": " << energyList.size() << " energy points" << endl;
			} else {
				cout << "No EM beam data available for center: " << centerTag << endl;
				return;
			}
		}
		else if(beamtype == "pi") 
		{
			// Pion beam merged files 사용
			TString centerTag = center;
			if(TB2025::map_hadronBeamRun_merged.find(centerTag) != TB2025::map_hadronBeamRun_merged.end()) {
				for(int energy : TB2025::map_hadronBeamRun_merged.at(centerTag)) {
					energyList.push_back(to_string(energy));
				}
				cout << "Using Pion beam merged files for " << centerTag << ": " << energyList.size() << " energy points" << endl;
			} else {
				cout << "No Pion beam data available for center: " << centerTag << endl;
				return;
			}
		}
		else if(beamtype == "kaon" || beamtype == "proton") 
		{
			// Kaon/Proton beam runs (개별 런 번호 사용)
			for(auto& pair : TB2025::map_hadronBeamRun_merged) {
				for(int run : pair.second) {
					runlist.push_back(to_string(run));
				}
			}
			cout << "Using kaon/proton beam runs from RunInfo.h: " << runlist.size() << " runs" << endl;
		}
		else {
			cout << "Invalid beam type: " << beamtype << ". Available types: calib, em, pi, kaon, proton" << endl; 
			return;
		}
		
		if(runlist.empty() && energyList.empty())
		{
			cout << "No runs or energy points available for beam type: " << beamtype << endl;
			return;
		}
		
		// merged files (em, pi)인 경우 energyList를 processRuns로 사용
		if(!energyList.empty()) {
			processRuns = energyList;
		} else {
			processRuns = runlist;
		}
		
		cout << "Processing " << beamtype << " beam type with " << processRuns.size() << " files/runs" << endl;
	}
	else
	{
		// 파라미터가 둘 다 없으면 기본 실행
		cout << "No specific run or beamtype provided. Using default: Run 11883, calib" << endl;
		processRuns.push_back("11883");
		beamtype = "calib";
	}

	for(int isn=0; isn<processRuns.size(); isn++)
	{
		string currentItem = processRuns[isn];
		TString fin;
		
		/// file read ///
		if(beamtype == "em" || beamtype == "pi")
		{
			// Merged files: output/<beamtype>/merged/<beamtype>_M5T2_<energy>GeV.root
			fin = Form("../output/%s/merged/%s_%s_%sGeV.root", beamtype.c_str(), beamtype.c_str(), center.c_str(), currentItem.c_str());
		}
		else if(beamtype == "kaon" || beamtype == "proton")
		{
			// Individual run files for kaon/proton
			fin = Form("../output/%s/DRC_%s_Run%s.root", beamtype.c_str(), center.c_str(), currentItem.c_str());
		}
		else // calib
		{
			// Calibration files
			fin = Form("../output/calib/DRC_%s_Run%s.root", center.c_str(), currentItem.c_str());
		}
		
		cout << "Trying to open: " << fin.Data() << endl;
		
		TFile *f = new TFile(fin,"READ");
		if(f==NULL || f->IsZombie())  
		{
			cout << "File not found: " << fin.Data() << endl;
			// 대안 경로들 시도
			vector<TString> altPaths;
			
			if(beamtype == "em" || beamtype == "pi") {
				altPaths = {
					Form("./output/%s/merged/%s_%s_%sGeV.root", beamtype.c_str(), beamtype.c_str(), center.c_str(), currentItem.c_str()),
					Form("../output/%s/merged/%s_M5T2_%sGeV.root", beamtype.c_str(), beamtype.c_str(), currentItem.c_str()),
					Form("./output/%s/merged/%s_M5T2_%sGeV.root", beamtype.c_str(), beamtype.c_str(), currentItem.c_str())
				};
			} else {
				altPaths = {
					Form("./output/%s/DRC_%s_Run%s.root", beamtype.c_str(), center.c_str(), currentItem.c_str()),
					Form("../output/calib/DRC_%s_Run%s.root", center.c_str(), currentItem.c_str()),
					Form("./output/calib/DRC_%s_Run%s.root", center.c_str(), currentItem.c_str())
				};
			}
			
			bool found = false;
			for(auto& altPath : altPaths)
			{
				TFile *altFile = new TFile(altPath, "READ");
				if(altFile && !altFile->IsZombie())
				{
					fin = altPath;
					f = altFile;
					found = true;
					cout << "Alternative file found: " << fin.Data() << endl;
					break;
				}
				if(altFile) delete altFile;
			}
			
			if(!found)
			{
				cout << "No valid file found for " << currentItem << endl;
				continue;
			}
		}
		else 
		{
			cout << "Successfully opened: " << fin.Data() << endl;
		}
		
		string savepath;
		if(beamtype == "em" || beamtype == "pi") {
			savepath = Form("./%s/%s/%s_%s_%sGeV", beamtype.c_str(), center.c_str(), beamtype.c_str(), center.c_str(), currentItem.c_str());
		} else {
			savepath = Form("./plots/%s/DRC_%s_Run%s", beamtype.c_str(), center.c_str(), currentItem.c_str());
		}
		gSystem->mkdir(Form("%s",savepath.c_str()),kTRUE);
		
		cout << "##################################################################################################################" << endl;
		if(beamtype == "em" || beamtype == "pi") {
			cout << "                                     Processing " << beamtype << " " << currentItem << " GeV          " << endl;
		} else {
			cout << "                                     Processing Run: " << currentItem << "          " << endl;
		}
		cout << "##################################################################################################################" << endl;

		/// draw all histograms using new function ///
		string runinfo;
		if(beamtype == "em" || beamtype == "pi") {
			runinfo = currentItem + "GeV";
		} else {
			runinfo = "Run" + currentItem;
		}
		
		// 새로운 통합 그리기 함수 호출
		DrawAllHistograms(f, savepath, beamtype, runinfo);
		
		f->Close();
		delete f;
	}
	
	cout << "=== Processing Complete ===" << endl;
}