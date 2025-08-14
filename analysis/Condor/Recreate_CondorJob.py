import os
from platform import machine
import sys

# 명령줄 인자 확인
if len(sys.argv) < 2:
    print("Usage: python Recreate_CondorJob.py [data|mc|16]")
    sys.exit(1)

# 첫 번째 인자를 Type으로 설정
Type = sys.argv[1].lower()  # 소문자로 변환하여 대소문자 구분 없애기

if Type == "data":
    samplelist = [
        ## 2018 Data ##
        ["Data_DoubleMuon_Run2018A", ["1","2","3","11",]],
        ["Data_DoubleMuon_Run2018B", ["12","13","19",]],
        ["Data_DoubleMuon_Run2018C", []],
        ["Data_DoubleMuon_Run2018D", ["23","86",]],
        ["Data_SingleMuon_Run2018A", ["23",]],
        ["Data_SingleMuon_Run2018B", []],
        ["Data_SingleMuon_Run2018C", []],
        ["Data_SingleMuon_Run2018D", []],
    ]
    runPeriod = "UL2018/Data"
elif Type == "mc":
    samplelist = [
        ## 2018 MC ##
        # currently madgraph is not used
        #["DYJetsToLL_M_10To50_madgraphMLM", []],
        #["DYJetsToLL_M_50_madgraphMLM", []],
        #["TTJets_TuneCP5_amcatnloFXFX", []],
        #["TTJets_TuneCP5_madgraphMLM", []],
        #["WJetsToLNu_madgraphMLM", []],
        ##----------------------------------##
        ["TTbar_Signal", ["0"]],
        ["DYJetsToLL_M_10To50", []],
        ["DYJetsToLL_M_50", []],
        ["ST_s-channel_4f_leptonDecays", []],
        ["ST_t-channel_antitop_4f_InclusiveDecays", []],
        ["ST_t-channel_top_4f_InclusiveDecays", []],
        ["ST_tW_antitop_5f_NoFullyHadronicDecays", []],
        ["ST_tW_top_5f_NoFullyHadronicDecays", []],
        ["TTbar_AllHadronic", []],
        ["TTbar_SemiLeptonic", []],
        ["TTWJetsToLNu", []],
        ["TTWJetsToQQ", []],
        ["TTZToLLNuNu", []],
        ["TTZToQQ", []],
        ["WJetsToLNu", []],
        ["WW", []],
        ["WZ", []],
        ["ZZ", []],
    ]
    runPeriod = "UL2018/MC"
elif Type == "16":
    samplelist = [
        ## 2016PreVFP MC ##
        ["TTbar_Signal", []],
    ]
    runPeriod = "UL2016PreVFP/MC"
else:
    print(f"Invalid Type: {Type}")
    print("Usage: python Recreate_CondorJob.py [data|mc|16] [re]")
    sys.exit(1)

Channels = "MuMu"
NumEvt = -1
Branchlist = "UL2018/branch_list.txt"
version = "2_JetVeto_JetLv"

if Channels == "MuMu":
    confch = "ULSummer20/" + runPeriod.split('/')[0] + "/dimuon.config"
elif Channels == "ElEl":
    confch = "ULSummer20/" + runPeriod.split('/')[0] + "/dielec.config"
elif Channels == "MuEl":
    confch = "ULSummer20/" + runPeriod.split('/')[0] + "/muelec.config"

for sample in range(len(samplelist)):
    ### skip if there are no failed jobs ###
    if not samplelist[sample][1]:
        #print(f"skipping {samplelist[sample][0]} because there are no failed jobs")
        continue
    ### set up jobversion ###
    if "dtG" in samplelist[sample][0]:
        jobversion = f"Job_Version/{version}/CPV_Sample"
    else:
        jobversion = f"Job_Version/{version}"
    ### set up directory ###
    sample_dir = f"{jobversion}/{Channels}/{runPeriod}/{samplelist[sample][0]}"
    ### delete log, err, out files for the failed jobs in log_condor directory ###
    for f in os.listdir(f"{sample_dir}/log_condor/err"):
        os.remove(os.path.join(f"{sample_dir}/log_condor/err", f))
    for f in os.listdir(f"{sample_dir}/log_condor/log"):
        os.remove(os.path.join(f"{sample_dir}/log_condor/log", f))
    for f in os.listdir(f"{sample_dir}/log_condor/out"):
        os.remove(os.path.join(f"{sample_dir}/log_condor/out", f))
    ### created executive condor job: recondor_run.sh ###
    with open(f"{sample_dir}/recondor_run.sh", "w") as file:
        file.write("#!/bin/bash \n\n")
        ### CMSSW environment setting ###
        file.write("export SCRAM_ARCH=el9_amd64_gcc12\n")
        file.write("source /cvmfs/cms.cern.ch/cmsset_default.sh \n\n")
        ### ROOT environment setting ###
        file.write("ROOTSYS=/cvmfs/cms.cern.ch/el9_amd64_gcc12/lcg/root/6.26.11-09a813662aa0f8eece54cb6d94bfed85\n")
        file.write("export PATH=$ROOTSYS/bin:$PATH\n")
        file.write("export ROOT_INCLUDE_PATH=$ROOTSYS/include\n")
        file.write("export LD_LIBRARY_PATH=$ROOTSYS/lib:$LD_LIBRARY_PATH\n\n")
        ### ROOT related special environment variables ###
        file.write("export ROOT_CLING_SYMLINK_BOOL=1\n")
        file.write("export CLING_STANDARD_PCH=none\n")
        file.write("export ROOT_HIST=0\n\n")
        file.write("fileListNum=$((${1}+1)) \n")
        ### CMSSW basic directory and working directory variables ###
        file.write("cd /u/user/gcho/TopPhysics/CPV/NanoAOD/CMSSW_13_3_0/src/SSBNanoAODANCode/ \n")
        file.write("cmsenv \n\n")
        ### output directory creation ###
        file.write("mkdir -p ./output/%s/\n" %(sample_dir))
        if "dtG" in samplelist[sample][0]:
            file.write("./ssb_analysis CEDM_Sample/%s/%s/%s_${fileListNum}.list /%s/%s_${fileListNum}.root 0 %s \n" % (runPeriod,samplelist[sample][0],samplelist[sample][0],sample_dir,samplelist[sample][0],samplelist[sample][0]))
        else:
            file.write("./ssb_analysis %s/%s/%s_${fileListNum}.list %s/%s_${fileListNum}.root %s None %s %s %s \n" % (runPeriod,samplelist[sample][0],samplelist[sample][0],sample_dir,samplelist[sample][0],confch,runPeriod.split('/')[0],NumEvt,Branchlist))
    ### created submit condor job: recondor_sub.sub ###
    with open(f"{sample_dir}/recondor_sub.sub", "w") as file:
        file.write("universe = vanilla \n")
        file.write("executable = recondor_run.sh \n\n")
        ## set up memory request: data: 6144, MC: 3072, CPV: 2048
        if "Data" in samplelist[sample][0]:
            file.write("request_memory = 1024 MB \n\n") ## data samples
        elif "dtG" in samplelist[sample][0]:
            file.write("request_memory = 1024 MB \n\n") ## CPV samples
        else:
            file.write("request_memory = 1024 MB \n\n") ## MC samples
        file.write("should_transfer_files = YES \n")
        file.write("when_to_transfer_output = ON_EXIT \n\n")
        file.write('Requirements = (machine =!= "cluster288.knu.ac.kr") && (machine =!= "cluster275.knu.ac.kr") && (machine =!= "cluster244.knu.ac.kr") && (machine =!= "cluster290.knu.ac.kr") && (machine =!= "dm01.knu.ac.kr") && (machine =!= "dm02.knu.ac.kr") && (TARGET.Arch == "X86_64") && (TARGET.OpSys == "LINUX") && (TARGET.HasFileTransfer)\n')
        file.write("JobBatchName = Re_%s \n\n" %(samplelist[sample][0]) )
        file.write("output = /u/user/gcho/TopPhysics/CPV/NanoAOD/CMSSW_13_3_0/src/SSBNanoAODANCode/Condor/%s/log_condor/out/out_$(Process).out \n" %(sample_dir) )
        file.write("error  = /u/user/gcho/TopPhysics/CPV/NanoAOD/CMSSW_13_3_0/src/SSBNanoAODANCode/Condor/%s/log_condor/err/err_$(Process).err \n" %(sample_dir) )
        file.write("log    = /u/user/gcho/TopPhysics/CPV/NanoAOD/CMSSW_13_3_0/src/SSBNanoAODANCode/Condor/%s/log_condor/log/log_$(Process).log \n\n" %(sample_dir) )
        
        # 최신 HTCondor 방식으로 단일 큐 명령 사용
        file.write("arguments = $(Item)\n")
        
        # 각 작업 번호를 공백으로 구분된 목록으로 변환
        job_list = " ".join(samplelist[sample][1])
        file.write(f"queue Item in ({job_list})\n")
        
    print(f"{samplelist[sample][0]} submit file recreated!!!")
