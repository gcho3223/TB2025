#include "TBread.h"
#include "TButility.h"

#include <filesystem>
#include <chrono>
#include <numeric>
#include <vector>
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "TCanvas.h"
#include "TH1.h"
#include "TH2.h"
#include "TFile.h"
#include "TApplication.h"
#include "TRootCanvas.h"
#include "TPaveStats.h"

#include "TLegend.h"
#include "TLatex.h"
#include "TColor.h"

#include "function.h"

namespace fs = std::__fs::filesystem;

int main(int argc, char *argv[]) {
	gStyle->SetPalette(kVisibleSpectrum);
	gStyle->SetStatFormat("6.6g");
  
	// setup for prompt analysis
	int fRunNum = std::stoi(argv[1]);
	int fMaxEvent = std::stoi(argv[2]);
	int fStartBin = std::stoi(argv[3]);
	int fEndBin = std::stoi(argv[4]);
	int fMaxFile = -1;

    std::vector<std::string> channel_names;
    std::string full_channel_name = "";
    myColorPalette.clear();
	for (int plot_args = 5; plot_args < argc; plot_args++)
	{
		channel_names.push_back(argv[plot_args]);
		full_channel_name += std::string(argv[plot_args]) + "_";
        myColorPalette.push_back(gStyle->GetColorPalette( (plot_args - 10) * ((float)gStyle->GetNumberOfColors() / ((float)argc - 10.))));
	}
    full_channel_name = full_channel_name.substr(0, full_channel_name.size() - 1);

	fs::path dir("./PeakADC");   
	if (!(fs::exists(dir))) fs::create_directory(dir);
	fs::path dir2("./PeakADC/Run_" + std::to_string(fRunNum));
	if (!(fs::exists(dir2))) fs::create_directory(dir2);

	// Cut values
	float cut_CC1 = 200.;
	float cut_CC2 = 200.;
	float cut_DWC = 1.5;

	// Integral range
	// int start_bin = fStartBin;
	// int end_bin = fEndBin;
	// height for text legend
	TLatex* text = new TLatex();
	text->SetTextSize(0.025);  
	float height = 0.87;

	// initialize the utility class
	TButility util = TButility();
	util.LoadMapping("../../mapping/mapping_TB2025_v1.root");
	
	// TFile *f_DWC = TFile::Open((TString)("./DWC/DWC_Run_" + std::to_string(fRunNum) + ".root"), "READ");
	// TH2D *h_DWC1_pos = (TH2D *)f_DWC->Get("dwc1_pos");
	// TH2D *h_DWC2_pos = (TH2D *)f_DWC->Get("dwc2_pos");
	// std::vector<float> DWC1_offset = getDWCoffset(h_DWC1_pos); // DWC1_offset.at(0) == X, DWC1_offset.at(1) == Y
	// std::vector<float> DWC2_offset = getDWCoffset(h_DWC2_pos);

	// prepare CIDs that we want to use
	TBcid cid_CC1 = util.GetCID("CC1");
	TBcid cid_CC2 = util.GetCID("CC2");

	TBcid cid_DWC1_R = util.GetCID("DWC1R");
	TBcid cid_DWC1_L = util.GetCID("DWC1L");
	TBcid cid_DWC1_U = util.GetCID("DWC1U");
	TBcid cid_DWC1_D = util.GetCID("DWC1D");

	TBcid cid_DWC2_R = util.GetCID("DWC2R");
	TBcid cid_DWC2_L = util.GetCID("DWC2L");
	TBcid cid_DWC2_U = util.GetCID("DWC2U");
	TBcid cid_DWC2_D = util.GetCID("DWC2D");

	std::vector<TBcid> cids;
	std::vector<TH1F *> plots;
	for (int idx = 0; idx < channel_names.size(); idx++) {
		plots.push_back(new TH1F((TString)channel_names.at(idx), ";peakADC;nEvents", 4096, 0, 4096));
		cids.push_back(util.GetCID(channel_names.at(idx)));
	}

    // MID: 3-7: PMT modules, MID 9: LC, MID 10: Aux(CC1, CC2, PS, TC, MC), MID 12: Triggers (T1, T2, T1NIM, T2NIM, Coin), MID 14-17: MCP micro, MID 18: DWC
    TBread<TBwaveform> readerWave = TBread<TBwaveform>(fRunNum, fMaxEvent, fMaxFile, false, "/Volumes/Macintosh HD-1/Users/yhep/scratch/YUdaq", {3, 4, 5, 6, 7, 9, 10, 12, 14, 15, 16, 17, 18});
    // TBread<TBwaveform> readerWave = TBread<TBwaveform>(fRunNum, fMaxEvent, fMaxFile, false, "/Volumes/Macintosh HD-1/Users/yhep/scratch/YUdaq", {10});

	// Set Maximum event
    if (fMaxEvent == -1 || fMaxEvent > readerWave.GetMaxEvent())
    fMaxEvent = readerWave.GetMaxEvent();

	TCanvas *c = new TCanvas("c", "c", 1000, 800);
	c->cd();

	TLegend* leg = new TLegend(0.75, 0.2, 0.9, 0.4);

    TFile* outFile = new TFile( ( "./PeakADC/Run_" + std::to_string(fRunNum) + "/Run_" + std::to_string(fRunNum) + ".root" ).c_str(), "RECREATE");
    outFile->cd();

    // Starting Evt Loop
    for (int iEvt = 0; iEvt < fMaxEvent; iEvt++) {
		printProgress(iEvt, fMaxEvent);

		// Load event
		TBevt<TBwaveform> aEvent = readerWave.GetAnEvent();
		// Get waveform of certain channel we want to use

		// TBwaveform wave_CC1 = aEvent.GetData(cid_CC1);
		// TBwaveform wave_CC2 = aEvent.GetData(cid_CC2);

		// TBwaveform wave_DWC1_R = aEvent.GetData(cid_DWC1_R);
		// TBwaveform wave_DWC1_L = aEvent.GetData(cid_DWC1_L);
		// TBwaveform wave_DWC1_U = aEvent.GetData(cid_DWC1_U);
		// TBwaveform wave_DWC1_D = aEvent.GetData(cid_DWC1_D);
		// TBwaveform wave_DWC2_R = aEvent.GetData(cid_DWC2_R);
		// TBwaveform wave_DWC2_L = aEvent.GetData(cid_DWC2_L);
		// TBwaveform wave_DWC2_U = aEvent.GetData(cid_DWC2_U);
		// TBwaveform wave_DWC2_D = aEvent.GetData(cid_DWC2_D);

		// std::vector<float> DWC1_time;
		// DWC1_time.emplace_back(getLeadingEdgeTime_interpolated800(wave_DWC1_R.waveform(), 0.4, 1, 1000));
		// DWC1_time.emplace_back(getLeadingEdgeTime_interpolated800(wave_DWC1_L.waveform(), 0.4, 1, 1000));
		// DWC1_time.emplace_back(getLeadingEdgeTime_interpolated800(wave_DWC1_U.waveform(), 0.4, 1, 1000));
		// DWC1_time.emplace_back(getLeadingEdgeTime_interpolated800(wave_DWC1_D.waveform(), 0.4, 1, 1000));

		// std::vector<float> DWC2_time;
		// DWC2_time.emplace_back(getLeadingEdgeTime_interpolated800(wave_DWC2_R.waveform(), 0.4, 1, 1000));
		// DWC2_time.emplace_back(getLeadingEdgeTime_interpolated800(wave_DWC2_L.waveform(), 0.4, 1, 1000));
		// DWC2_time.emplace_back(getLeadingEdgeTime_interpolated800(wave_DWC2_U.waveform(), 0.4, 1, 1000));
		// DWC2_time.emplace_back(getLeadingEdgeTime_interpolated800(wave_DWC2_D.waveform(), 0.4, 1, 1000));

		// HERE: For event selection
		// std::vector<float> DWC1_corrected_pos = getDWC1position(DWC1_time, DWC1_offset);
		// std::vector<float> DWC2_corrected_pos = getDWC1position(DWC2_time, DWC2_offset);

		// Get pedestal corrected waveform
		// TBwaveform::pedcorrectedWaveform(float ped): when using external (e.g. run pedestal)
		// TBwaveform::pedcorrectedWaveform(): when using event-by-event pedestal (ped = average of first 100 bin except 0th bin)
		// std::vector<float> pedCorr_wave_C1 = wave_C1.pedcorrectedWaveform();
		// std::vector<float> pedCorr_wave_C2 = wave_C2.pedcorrectedWaveform();
		// std::vector<short> waveform_CC1 = wave_CC1.waveform();
		// std::vector<short> waveform_CC2 = wave_CC2.waveform();

		// integration range: should be set channel by channel with average time structure
		// float signal_C1 = std::accumulate(pedCorr_wave_C1.begin() + first, pedCorr_wave_C1.begin() + last, 0.);
		// float signal_C2 = std::accumulate(pedCorr_wave_C2.begin() + first, pedCorr_wave_C2.begin() + last, 0.);
		// float signal_CC1 = GetPeak(waveform_CC1, 300, 600);
		// float signal_CC2 = GetPeak(waveform_CC2, 300, 600);

		// Event selection here!!
		// Require C1, C2 to have intADC larger than cut
		// if (!((cut_C1 < signal_C1) && (cut_C2 < signal_C2)))
		// continue;
		// if (!(dwcCorrelationCut(DWC1_corrected_pos, DWC2_corrected_pos, 1.5)))
		//   continue;

		// filling plots
		for (int idx = 0; idx < plots.size(); idx++)
		{
			auto single_waveform = aEvent.GetData(cids.at(idx)).waveform();

			float PeakADC = GetPeak(single_waveform, fStartBin, fEndBin);
			plots.at(idx)->Fill(PeakADC);
		}
  	} //end of event loop
	
	outFile->cd();
	for (int idx = 0; idx < plots.size(); idx++) {
		plots.at(idx)->Draw("hist");
		plots.at(idx)->Write();
	}
	outFile->Close();

	return 0;
}