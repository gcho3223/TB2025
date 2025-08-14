#!/bin/bash

if [ "$#" -lt 4 ]; then
    echo "Usage: ./run_summary.sh particle:[calib|em|pi|kp] program:[ats|dwc|evt] rotation:[on|off] interaction target:[on|off]"
    exit 1
fi

particle=$1
program_arg=$2
rotation_str=$3
interaction_str=$4

rotation=false
if [ "$rotation_str" == "on" ]; then
    rotation=true
fi

interaction=false
if [ "$interaction_str" == "on" ]; then
    interaction=true
fi

program="ATS"
if [ "$program_arg" == "ats" ]; then
    program="ATS"
elif [ "$program_arg" == "dwc" ]; then
    program="DWC"
elif [ "$program_arg" == "evt" ]; then
    program="Evtloop"
else
    echo "Invalid program: $program_arg"
    echo "Usage: ./submit_condor.sh particle:[calib|em|pi|kp] program:[ats|dwc|evt] rotation:[on|off] interaction target:[on|off]"
    exit 1
fi

version="PA"
if $rotation; then
    version+="_Rot"
fi
if $interaction; then
    version+="_IT"
fi

if [ "$particle" == "calib" ]; then
    runlist=(
        12171 12172 12183 12182  # -- M1, T1-4
        12174 12175 12181 12180  # -- M2, T1-4
        12176 12177 12179 12178  # -- M3, T1-4
        12184 12185 12195 12194  # -- M4, T1-4
        12186 12187 12193 12192  # -- M5, T1-4
        12188 12189 12191 12190  # -- M6, T1-4
        12196 12197 12207 12206  # -- M7, T1-4
        12198 12199 12205 12204  # -- M8, T1-4
        12200 12201 12203 12202  # -- M9, T1-4
    )
elif [ "$particle" == "em" ]; then
    runlist=(
        12263 12264 12267 12268 12279 12280 # 120 GeV
        12260 12261 # 100 GeV
        12265 12266 # 80 GeV
        12269 12270 # 60 GeV
        12271 12272 # 40 GeV
        12273 12274 # 30 GeV
        12275 12276 # 20 GeV
        12277 # 10 GeV
        12278 # 6 GeV
    )
elif [ "$particle" == "pi" ]; then
    if $rotation && ! $interaction; then
        runlist=(
            12438 12439 12440 12441 12443 12444 12445 # 120GeV
            12432 12433 12434 12435 12436 12437 # 100GeV
            12446 12447 12448 12449 12450 12451 # 80GeV
            12452 12453 12454 12455 12456 # 60GeV
            12457 12458 12459 # 40GeV
            12460 12461 # 20GeV
        )
    elif ! $rotation && $interaction; then
        runlist=(
            12502 12503 12504 12505 12506 12507 12508 12509 # 120GeV
            12510 12511 12512 12513 12514 12515 # 100GeV
            12496 12497 12498 12499 12500 12501 # 80GeV
            12516 12517 12518 12519 12520 # 60GeV
            12522 12523 12524 12525 # 40GeV
        )
    elif ! $rotation && ! $interaction; then
        runlist=(
            12352 12353 12354 12355 12356 12357 12358 12359 12360 12361 12363 # 120GeV
            12364 12365 12366 12367 12368 12369 12370 12371 12372 12373 # 100GeV
            12374 12375 12376 12377 12378 12379 12380 12381 12382 12383 # 80GeV
            12384 12385 12386 12387 12388 # 60GeV
            12389 12390 12391 # 40GeV
            12392 12393 # 20GeV
        )
    else
        echo "Invalid combination of rotation and interaction for pi particle."
        exit 1
    fi
elif [ "$particle" == "kaon" ] || [ "$particle" == "proton" ]; then
    if ! $rotation && ! $interaction; then
        runlist=(
            12401 12402 12403 12404 12405 12406 12407 12408 # 120GeV
            12410 12411 12412 12413 12414 12415 # 100GeV
            12416 12417 12418 12419 12420 12421 # 80GeV
            12422 12423 12424 12425 12426 # 60GeV
            12427 12428 12429 # 40GeV
            12430 12431 # 20GeV
        )
    elif $rotation && $interaction; then
        runlist=(
            12462 12463 12465 12465 12466 12467 12468 12469 # 120GeV
            12470 12471 12472 12473 12474 12475 # 100GeV
            12476 12477 12478 12479 12480 12481 # 80GeV
            12482 12483 12484 12485 12486 # 60GeV
            12487 12488 12489 # 40GeV
            12490 12491 12492 # 20GeV
        )
    elif ! $rotation && $interaction; then
        runlist=(
            12502 12503 12504 12505 12506 12507 12508 12509 # 120GeV
            12510 12511 12512 12513 12514 12515 # 100GeV
            12496 12497 12498 12499 12500 12501 # 80GeV
            12516 12517 12518 12519 12520 # 60GeV
            12522 12523 12524 12525 # 40GeV
        )
    else
        echo "Invalid combination of rotation and interaction for kp particle."
        exit 1
    fi
else
    echo "Invalid particle: $particle"
    echo "Usage: ./submit_condor.sh particle:[calib|em|pi|kp] program:[ats|dwc|evt] rotation:[on|off] interaction target:[on|off]"
    exit 1
fi
######################################################################
### before running, should change samples you want to make summary ###
### you can use CPV samples or Dataset samples at the same time    ###
######################################################################
#############################
### ! version information ###
#############################
# keep earlier values; version already computed from rotation/interaction
particle="$particle"
program="$program"
# version is preserved as "$version"

#########################################################
### set up sample_path                                ###
### calculate total # of log files                    ###
### set up log file path & variables for progress bar ###
#########################################################
### progress bar variables ###
total_sample=0
completed=0

# patterns indicating errors in ROOT/condor outputs
ERROR_REGEX='Error in <|Fatal in <|Segmentation violation|Bus error|core dumped|No TTree called|no keys recovered|made a Zombie|is a Zombie|std::bad_alloc|terminate called|Aborted|Killed|Exception|stack trace'
# treat some TFile warnings as errors (zombie/no keys)
WARN_AS_ERROR_REGEX='Warning in <TFile::Init>.*(Zombie|no keys recovered)'

### set up sample_path & calculate total # of log files ###
for i in "${runlist[@]}"
do
    sample_path="./${version}/${particle}/${program}"
    # 로그 파일 개수 계산
    log_dir="${sample_path}/${i}/log_condor/out"
    file_count=$(ls "$log_dir"/*.out 2>/dev/null | wc -l)
    total_sample=$((total_sample + file_count))  #sum of total log files
done

# total_sample가 0인지 확인
if (( total_sample == 0 )); then
    echo "No log files found. Exiting."
    echo "Please check the log files in ${sample_path} directory."
    exit 1
fi
#########################################
###        start summary !!           ###
#########################################
for i in "${runlist[@]}"
do
    ################################################
    ###   check sample name & set up make dir    ###
    ################################################
    # CPV 샘플인지 확인
    if [[ $i == TTJets_Signal_dtG_* ]]; then
        load_path="./${version}/${particle}/${program}"
        sample_path="./${version}/${particle}/${program}"
    else
        load_path="./${version}/${particle}/${program}"
        sample_path="./${version}/${particle}/${program}"
    fi

    # 디렉토리 및 파일 생성
    mkdir -p ./Run_Summary/${sample_path}/${i}
    # 파일 경로 및 로그 디렉토리 설정
    summary_file="./Run_Summary/${sample_path}/${i}/Run_Summary_${i}.txt"
    memory_file="./Run_Summary/${sample_path}/${i}/Memory_Summary_${i}.txt"
    > "$summary_file"
    > "$memory_file"
    # define log, out, err path
    log_path="${load_path}/${i}/log_condor/log"
    out_path="${load_path}/${i}/log_condor/out"
    err_path="${load_path}/${i}/log_condor/err"
    
    ##########################################  
    ###       calc # of log files         ###
    ##########################################
    file_count=$(ls "$out_path"/*.out 2>/dev/null | wc -l)
    # skip if there is no logfile -> to obtail the number of queue
    if (( file_count == 0 )); then
        echo "No log files found for ${i}" >> "$summary_file"
        echo "sample path: ${sample_path}" >> "$summary_file"
        continue
    fi
    failed_evt=()
    ##########################################
    ###      write summary to txt files    ###
    ##########################################
    ## indicate Sample name ##
    echo "[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[ ${i} ]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]" >> "$summary_file"
    echo "[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[ ${i} ]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]" >> "$memory_file"
    
    for ((j=0; j<file_count; j++))
    do 
        ##########################################
        ###       memory summary              ###
        ##########################################
        echo ">> $j th-----------------------------------------------------------------------------------" >> "$memory_file"
        echo "                              	Usage    Requested   Allocated" >> "$memory_file"
        echo "`grep " Memory (MB)" "$log_path/log_$j.log"`" >> "$memory_file"
        
        ##########################################
        ###       err, out ,log summary        ###
        ##########################################
        if ! grep -q "exit-code 0" "$log_path/log_$j.log" \
           || ! grep -q "Program completed successfully!" "$out_path/out_$j.out" \
           || grep -Eq "$ERROR_REGEX" "$out_path/out_$j.out" \
           || grep -Eq "$ERROR_REGEX" "$err_path/err_$j.err" \
           || grep -Eq "$WARN_AS_ERROR_REGEX" "$out_path/out_$j.out"; then
            echo ">> $j th-----------------------------------------------------------------------------------" >> "$summary_file"
            # exit-code 확인 : exit-code 0(정상종료)이 아니면 기록
            if ! grep -q "exit-code 0" "$log_path/log_$j.log"; then
                echo "Log: " >> "$summary_file"
                echo "`grep "SlotName" "$log_path/log_$j.log"`" >> "$summary_file"
                echo "`grep "exit-code" "$log_path/log_$j.log"`" >> "$summary_file"
                echo "`grep " Memory (MB)" "$log_path/log_$j.log"`" >> "$summary_file"
            fi
            # err 파일 확인 : killed가 있으면 기록
            if grep -q "killed" "$err_path/err_$j.err"; then
                echo "Err: " >> "$summary_file"
                echo "`grep "killed" "$err_path/err_$j.err"`" >> "$summary_file"
            fi
            # out 파일 확인 : End processing(정상종료)이 없으면 기록
            if ! grep -q "End processing" "$out_path/out_$j.out"; then
                echo "Out: " >> "$summary_file"
                echo "`tail -n 7 "$out_path/out_$j.out"`" >> "$summary_file"
            fi
            # Detected error/warning patterns for quick triage
            echo "Detected patterns (out):" >> "$summary_file"
            grep -E "$ERROR_REGEX" "$out_path/out_$j.out" >> "$summary_file" 2>/dev/null
            grep -E "$WARN_AS_ERROR_REGEX" "$out_path/out_$j.out" >> "$summary_file" 2>/dev/null
            echo "Detected patterns (err):" >> "$summary_file"
            grep -E "$ERROR_REGEX" "$err_path/err_$j.err" >> "$summary_file" 2>/dev/null
            failed_evt[$j]=$j 
        fi
        ################################
        ###       progress bar       ###
        ################################
        ((completed++))
        echo -ne "Progress in $i: ["
        progress=$(( (completed * 100) / total_sample )) 
        for ((k=0; k<=100; k+=2))
        do
            if [ $k -le $progress ]
            then
                echo -ne ">"
            else
                echo -ne " "
            fi
        done
        echo -ne "] $progress% \r"
    done
    
    echo "------------------------------------ Summary -----------------------------------------------" >> "$summary_file"
    echo "Number of failed inputlist number: ${#failed_evt[@]}" >> "$summary_file"
    echo -n "Failed inputlist number: " >> "$summary_file"
    for j in "${failed_evt[@]}"
    do
        echo -n "\"$j\"," >> "$summary_file"
    done
    echo "" >> "$summary_file"
done