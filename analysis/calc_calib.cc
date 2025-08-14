#include "TBread.h"
#include "TButility.h"

#include <iostream>
#include <fstream>
#include <chrono>
#include <numeric>
#include <vector>
#include <cstdlib>
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "TROOT.h"
#include "TStyle.h"
#include "TCanvas.h"
#include "TH1.h"
#include "TFile.h"
#include "TH2.h"

#include "./header/function.h"

void saveCalibConfigFormat(const std::string& filename, const std::string& fRunNum, 
                          const std::string& fModuleNum, const std::string& fTowerNum, 
                          float Equ_const_C, float Equ_const_S) {
    std::cout << "Creating config file: " << filename << std::endl;
    std::ofstream outFile(filename);
    
    if(!outFile.is_open()) {std::cerr << "ERROR: Cannot create file: " << filename << std::endl; return;}
    
    outFile << "# Calibration Constants for Module " << fModuleNum << " Tower " << fTowerNum << std::endl;
    outFile << "# Generated from Run " << fRunNum << std::endl;
    outFile << "# Format: {\"key\", value}" << std::endl;
    outFile << "# Normalized to 60 GeV beam energy" << std::endl;
    outFile << std::endl;
    
    outFile << "{\"M" << fModuleNum << "-T" << fTowerNum << "-C\", " << Equ_const_C << "}," << std::endl;
    outFile << "{\"M" << fModuleNum << "-T" << fTowerNum << "-S\", " << Equ_const_S << "}," << std::endl;
    
    outFile.close();
    if (outFile.fail()) {
        std::cerr << "ERROR: Failed to write to file: " << filename << std::endl;
        return;
    }
    std::cout << "Calibration constants saved in config format: " << filename << std::endl;
}

void saveCalibJSONFormat(const std::string& filename, const std::string& fRunNum, 
                        const std::string& fModuleNum, const std::string& fTowerNum, 
                        float Equ_const_C, float Equ_const_S)
{
    std::cout << "Creating JSON file: " << filename << std::endl;
    std::ofstream outFile(filename);
    
    if(!outFile.is_open()) {std::cerr << "ERROR: Cannot create file: " << filename << std::endl; return;}
    
    outFile << "{" << std::endl;
    outFile << "  \"metadata\": {" << std::endl;
    outFile << "  \"run_number\": \"" << fRunNum << "\"," << std::endl;
    outFile << "  \"module_number\": \"" << fModuleNum << "\"," << std::endl;
    outFile << "  \"tower_number\": \"" << fTowerNum << "\"," << std::endl;
    outFile << "  \"beam_energy\": \"60 GeV\"," << std::endl;
    outFile << "  \"generated_date\": \"" << __DATE__ << " " << __TIME__ << "\"" << std::endl;
    outFile << "  }," << std::endl;
    outFile << "  \"calibration_constants\": {" << std::endl;
    outFile << "    \"M" << fModuleNum << "-T" << fTowerNum << "-C\": " << Equ_const_C << "," << std::endl;
    outFile << "    \"M" << fModuleNum << "-T" << fTowerNum << "-S\": " << Equ_const_S << std::endl;
    outFile << "  }" << std::endl;
    outFile << "}" << std::endl;
    
    outFile.close();
    if(outFile.fail()) {std::cerr << "ERROR: Failed to write to file: " << filename << std::endl; return;}
    std::cout << "Calibration constants saved in JSON format: " << filename << std::endl;
}

int main(int argc, char** argv)
{
    if(argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <run_number> <module_number> <tower_number>" << std::endl;
        return -1;
    }

    std::string fRunNum    = argv[1];
    std::string fModuleNum = argv[2];
    std::string fTowerNum  = argv[3]; // Added tower number argument

    std::string dir_path = "./CalibConst";
    int status = system(("mkdir -p " + dir_path).c_str());
    if(status == 0) {std::cout << "Created directory: " << dir_path << std::endl;}
    else {std::cerr << "ERROR: Failed to create directory: " << dir_path << std::endl; return -1;}

    std::string calib_file = "./Calib_ntuple/Calib_Run_" + fRunNum + ".root";
    std::cout << "Attempting to open file: " << calib_file << std::endl;
    
    TFile* f = new TFile(calib_file.c_str(), "READ");
    if(!f || f->IsZombie())
    {
        std::cerr << "ERROR: Cannot open input file: " << calib_file << std::endl;
        std::cerr << "Please check if the file exists and is readable." << std::endl;
        return -1;
    }
    std::cout << "Successfully opened file: " << calib_file << std::endl;
    f->cd();

    // Calculate equalization constant for specific tower based on 60GeV e- beam
    TH1F* hist_Tower_C;
    TH1F* hist_Tower_S;
    float Equ_const_C, Equ_const_S;
    
    // Load histograms for the specific tower only
    std::string histName_C = "M" + fModuleNum + "_T" + fTowerNum + "_C_5_MC";
    std::string histName_S = "M" + fModuleNum + "_T" + fTowerNum + "_S_5_MC";
    
    std::cout << "Looking for histogram: " << histName_C << std::endl;
    hist_Tower_C = (TH1F*) f->Get(histName_C.c_str());
    if(!hist_Tower_C)
    {
        std::cerr << "ERROR: Cannot find histogram: " << histName_C << std::endl;
        f->Close();
        return -1;
    }
    
    std::cout << "Looking for histogram: " << histName_S << std::endl;
    hist_Tower_S = (TH1F*) f->Get(histName_S.c_str());
    if(!hist_Tower_S)
    {
        std::cerr << "ERROR: Cannot find histogram: " << histName_S << std::endl;
        f->Close();
        return -1;
    }
    
    // Calculate equalization constants (normalized to 60 GeV)
    Equ_const_C = (60.0) / (float)(hist_Tower_C->GetMean());
    Equ_const_S = (60.0) / (float)(hist_Tower_S->GetMean());
    
    std::cout << "Module " << fModuleNum << " Tower " << fTowerNum << " C ch. Equ. const. = " << Equ_const_C << std::endl;
    std::cout << "Module " << fModuleNum << " Tower " << fTowerNum << " S ch. Equ. const. = " << Equ_const_S << std::endl;

    // Save calibration constants in different formats
    std::string configFile = "./CalibConst/Run_" + fRunNum + "_Module_" + fModuleNum + "_Tower_" + fTowerNum + "_calib.txt";
    std::string jsonFile = "./CalibConst/Run_" + fRunNum + "_Module_" + fModuleNum + "_Tower_" + fTowerNum + "_calib.json";
    std::string csvFile = "./CalibConst/Run_" + fRunNum + "_Module_" + fModuleNum + "_Tower_" + fTowerNum + "_calib.csv";
    
    // Save in config format (main format)
    saveCalibConfigFormat(configFile, fRunNum, fModuleNum, fTowerNum, Equ_const_C, Equ_const_S);
    
    // Save in JSON format (for advanced usage)
    saveCalibJSONFormat(jsonFile, fRunNum, fModuleNum, fTowerNum, Equ_const_C, Equ_const_S);
    
    // Save in original CSV format (for compatibility)
    std::cout << "Creating CSV file: " << csvFile << std::endl;
    std::ofstream csvOutFile(csvFile);
    if (!csvOutFile.is_open()) {
        std::cerr << "ERROR: Cannot create file: " << csvFile << std::endl;
        f->Close();
        return -1;
    }
    
    csvOutFile << "{\"M" + fModuleNum + "-T" + fTowerNum + "-C\", " << Equ_const_C << "}, {\"M" + fModuleNum + "-T" + fTowerNum + "-S\", " << Equ_const_S << "}," << std::endl;
    
    csvOutFile.close();
    if (csvOutFile.fail()) {
        std::cerr << "ERROR: Failed to write to file: " << csvFile << std::endl;
        f->Close();
        return -1;
    }
    std::cout << "Calibration constants saved in CSV format: " << csvFile << std::endl;
    
    std::cout << std::endl;
    std::cout << "Calibration constants saved in three formats:" << std::endl;
    std::cout << "Config format: " << configFile << " (recommended for use with analysis codes)" << std::endl;
    std::cout << "JSON format:   " << jsonFile << " (for advanced processing)" << std::endl;
    std::cout << "CSV format:    " << csvFile << " (for compatibility)" << std::endl;
    
    f->Close();
    
    return 0;
} 