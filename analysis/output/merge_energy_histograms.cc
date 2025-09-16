#include <iostream>
#include <vector>
#include <map>
#include <TFile.h>
#include <TH1D.h>
#include <TH1F.h>
#include <TH2D.h>
#include <TString.h>
#include <TKey.h>
#include <TClass.h>
#include <TSystem.h>

void merge_energy_histograms(TString beamType = "em", TString rotation = "", TString interaction = "", TString centerTower = "M5T2")
{
    if(!(rotation == "on" || rotation == "off")) {std::cout << "Invalid rotation: " << rotation << std::endl; return;}
    if(!(interaction == "on" || interaction == "off")) {std::cout << "Invalid interaction: " << interaction << std::endl; return;}

    TString version = "";
    if(rotation == "on")    {version += "_Rot";}
    if(interaction == "on") {version += "_IT";}
    
    std::map<TString, std::vector<int>> energyRunMap;
    
    if(beamType == "em")
    {
        // M5-T2 에너지별 런 맵
        std::map<TString, std::vector<int>> energyRunMap_M5;
        energyRunMap_M5["6GeV"] = {12278};
        energyRunMap_M5["10GeV"] = {12277};
        energyRunMap_M5["20GeV"] = {12275, 12276};
        energyRunMap_M5["30GeV"] = {12273, 12274};
        energyRunMap_M5["40GeV"] = {12271, 12272};
        energyRunMap_M5["60GeV"] = {12269, 12270};
        energyRunMap_M5["80GeV"] = {12265, 12266};
        energyRunMap_M5["100GeV"] = {12260, 12261};
        energyRunMap_M5["120GeV"] = {12263, 12264, 12267, 12268, 12279, 12280};
        
        // M4-T2 에너지별 런 맵
        std::map<TString, std::vector<int>> energyRunMap_M4;
        energyRunMap_M4["10GeV"] = {12286, 12292, 12319};
        energyRunMap_M4["20GeV"] = {12316, 12317, 12318};
        energyRunMap_M4["30GeV"] = {12313, 12314, 12315};
        energyRunMap_M4["40GeV"] = {12310, 12311, 12312};
        energyRunMap_M4["60GeV"] = {12307, 12308, 12309};
        energyRunMap_M4["80GeV"] = {12304, 12305, 12306};
        energyRunMap_M4["100GeV"] = {12301, 12302, 12303};
        energyRunMap_M4["120GeV"] = {12285, 12293, 12294, 12295, 12296, 12297, 12298, 12299, 12300};
        
        // 사용할 맵 선택
        if(centerTower == "M5T2") {
            energyRunMap = energyRunMap_M5;
        } else if(centerTower == "M4T2") {
            energyRunMap = energyRunMap_M4;
        } else {
            std::cout << "Unknown center tower: " << centerTower << std::endl;
            return;
        }
    }
    else if(beamType == "pi")
    {
        std::map<TString, std::vector<int>> energyRunMap_M5;
        energyRunMap_M5["20GeV"]  = {12392, 12393};
        energyRunMap_M5["40GeV"]  = {12389, 12390, 12391};
        energyRunMap_M5["60GeV"]  = {12384, 12385, 12386, 12387, 12388};
        energyRunMap_M5["80GeV"]  = {12374, 12375, 12376, 12377, 12378, 12379, 12380, 12381, 12382, 12383};
        energyRunMap_M5["100GeV"] = {12364, 12365, 12366, 12367, 12368, 12369, 12370, 12371, 12372, 12373};
        energyRunMap_M5["120GeV"] = {12352, 12353, 12354, 12355, 12356, 12357, 12358, 12359, 12360, 12361, 12362, 12363};
        std::map<TString, std::vector<int>> energyRunMap_Rot;
        energyRunMap_Rot["20GeV"]  = {12460, 12461};
        energyRunMap_Rot["40GeV"]  = {12457, 12458, 12459};
        energyRunMap_Rot["60GeV"]  = {12452, 12453, 12454, 12455, 12456};
        energyRunMap_Rot["80GeV"]  = {12446, 12447, 12448, 12449, 12450, 12451};
        energyRunMap_Rot["100GeV"] = {12432, 12433, 12434, 12435, 12436, 12437};
        energyRunMap_Rot["120GeV"] = {12439, 12440, 12441, 12442, 12443, 12444, 12445};
        std::map<TString, std::vector<int>> energyRunMap_IT;
        energyRunMap_IT["40GeV"]  = {12522, 12523, 12524, 12525, 12526, 12527, 12528, 12529, 12530, 12531, 12532, 12533, 12534, 12535, 12536};
        energyRunMap_IT["60GeV"]  = {12516, 12517, 12518, 12519, 12520, 12537, 12538, 12539, 12540, 12541, 12542, 12543, 12544, 12545, 12546, 12547, 12548, 12549, 12550, 12551, 12552, 12553, 12554, 12555, 12556, 12557, 12558, 12559, 12560, 12561, 12562, 12563};
        energyRunMap_IT["80GeV"]  = {12496, 12497, 12498, 12499, 12500, 12501};
        energyRunMap_IT["100GeV"] = {12510, 12511, 12512, 12513, 12514, 12515, 12564, 12565, 12566, 12567, 12568, 12569, 12570, 12571, 12572, 12573, 12574, 12575, 12576};
        energyRunMap_IT["120GeV"] = {12503, 12504, 12505, 12506, 12507, 12508, 12509};
        if(rotation == "on")         {energyRunMap = energyRunMap_Rot;}
        else if(interaction == "on") {energyRunMap = energyRunMap_IT;}
        else {std::cout << "Unknown rotation/interaction target for pi beam: " << rotation << std::endl; return;}
    }
    else if(beamType == "proton" || beamType == "kaon")
    {
        std::map<TString, std::vector<int>> energyRunMap_M5;
        energyRunMap_M5["20GeV"]  = {12430, 12431};
        energyRunMap_M5["40GeV"]  = {12427, 12428, 12429};
        energyRunMap_M5["60GeV"]  = {12422, 12423, 12424, 12425, 12426};
        energyRunMap_M5["80GeV"]  = {12416, 12417, 12418, 12419, 12420, 12421};
        energyRunMap_M5["100GeV"] = {12410, 12411, 12412, 12413, 12414, 12415};
        energyRunMap_M5["120GeV"] = {12401, 12402, 12403, 12404, 12405, 12406, 12407, 12408};
        std::map<TString, std::vector<int>> energyRunMap_Rot;
        energyRunMap_Rot["20GeV"]  = {12490, 12491, 12492};
        energyRunMap_Rot["40GeV"]  = {12487, 12488, 12489};
        energyRunMap_Rot["60GeV"]  = {12482, 12483, 12484, 12485, 12486};
        energyRunMap_Rot["80GeV"]  = {12476, 12477, 12478, 12479, 12480, 12481};
        energyRunMap_Rot["100GeV"] = {12470, 12471, 12472, 12473, 12474, 12475};
        energyRunMap_Rot["120GeV"] = {12462, 12463, 12464, 12465, 12466, 12467, 12468, 12469};
        if(rotation == "on")       {energyRunMap = energyRunMap_Rot;}
        else if(rotation == "off") {energyRunMap = energyRunMap_M5;}
        else {std::cout << "Unknown rotation/interaction target for proton/kaon beam: " << rotation << std::endl; return;}
    }
    
    // 출력 디렉토리 생성
    TString outputDir = "./" + beamType + "/Evtloop" + version + "/merged";
    gSystem->mkdir(outputDir, kTRUE);
    
    std::cout << "Starting histogram merging for " << beamType << " beam, center: " << centerTower << std::endl;
    
    // 각 에너지별로 처리
    for(const auto& energyRuns : energyRunMap) {
        TString energy = energyRuns.first;
        const std::vector<int>& runs = energyRuns.second;
        
        std::cout << "\nProcessing " << energy << " with " << runs.size() << " runs: ";
        for(int r : runs) std::cout << r << " ";
        std::cout << std::endl;
        
        // 출력 파일 생성
        TString outputFileName = outputDir + "/" + beamType + "_" + centerTower + "_" + energy + ".root";
        TFile* outputFile = TFile::Open(outputFileName, "RECREATE");
        
        std::map<TString, TH1*> mergedHistograms;
        
        // 각 런의 파일을 처리
        for(int i = 0; i < runs.size(); ++i) {
            int run = runs[i];
            TString inputFileName = TString::Format("./%s/Evtloop%s/%s_%s_%d.root", 
                                                  beamType.Data(), version.Data(), beamType.Data(), centerTower.Data(), run);
            
            std::cout << "  Loading run " << run << ": " << inputFileName << std::endl;
            
            TFile* inputFile = TFile::Open(inputFileName);
            if(!inputFile || inputFile->IsZombie()) {
                std::cout << "  ERROR: Cannot open " << inputFileName << std::endl;
                continue;
            }
            
            // 파일의 모든 히스토그램을 순회
            TIter next(inputFile->GetListOfKeys());
            TKey* key;
            while((key = (TKey*)next())) {
                if(strcmp(key->GetClassName(), "TH1F") != 0 && 
                   strcmp(key->GetClassName(), "TH1D") != 0 &&
                   strcmp(key->GetClassName(), "TH2D") != 0) continue;
                
                TString histName = key->GetName();
                TH1* hist = (TH1*)key->ReadObj();
                
                if(i == 0) {
                    // 첫 번째 런: 히스토그램 복사
                    mergedHistograms[histName] = (TH1*)hist->Clone();
                    mergedHistograms[histName]->SetDirectory(outputFile);
                } else {
                    // 나머지 런들: 히스토그램 합치기
                    if(mergedHistograms.find(histName) != mergedHistograms.end()) {
                        mergedHistograms[histName]->Add(hist);
                    }
                }
            }
            
            inputFile->Close();
            delete inputFile;
        }
        
        std::cout << "  Created merged file: " << outputFileName << std::endl;
        std::cout << "  Total histograms: " << mergedHistograms.size() << std::endl;
        
        // 출력 파일에 모든 히스토그램 저장
        outputFile->cd();
        for(auto& pair : mergedHistograms) {
            pair.second->Write();
        }
        
        outputFile->Close();
        delete outputFile;
    }
    
    std::cout << "\nHistogram merging completed!" << std::endl;
} 