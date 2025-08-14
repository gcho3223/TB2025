#ifndef DRAWPLOTS_H
#define DRAWPLOTS_H

#include <string>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <map>
#include <set>
#include "TSystem.h"
#include "TH1.h"
#include "TH2.h"
#include "TLegend.h"
#include "TPad.h"
#include "TLatex.h"

using std::string;

// 1D histogram
void DrawKinematics(TH1D *hist, string beamtype, string runnumber, string savepath);
void DrawKinematics(TH1D *hist, string runnumber, string savepath);
// 2D histogram
void DrawKinematics2D(TH2D *hist, string sample, string version, string savepath, string type);
// New main drawing function
void DrawAllHistograms(TFile* f, const string& savepath, const string& beamtype, const string& runinfo);
// Overlay histogram
void DrawOverlayHistogram(TH1D *hist1, TH1D *hist2, TH1D *hist3, string version1, string version2, string version3, string period, string channel, string savepath);
// Get max value
double GetMaxValue(TH1D* hist1, TH1D* hist2)
{
    double maxVal = std::max(hist1->GetMaximum(), hist2->GetMaximum());
    return maxVal*1.2;
}
double GetMaxValue(TH1D* hist1, TH1D* hist2, TH1D* hist3)
{
    double maxVal = std::max({hist1->GetMaximum(), hist2->GetMaximum(), hist3->GetMaximum()});
    return maxVal*1.2;
}

#endif // DRAWPLOTS_H