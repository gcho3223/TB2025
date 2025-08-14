#!/bin/bash
###########################################################
# How to run                                              #
# run: ./macro.sh <beam type> <program> [module]          #
#    - beam type: beam type (Calib, em, pi, kaon, proton) #
#    - program: program name (ats, dwc, em, const)       #
#    - module: module number (4 for M4T2, 5 for M5T2)     #
# Examples:                                               #
#    ./macro.sh Calib const     # Calibration constants   #
#    ./macro.sh Calib em 5      # Calib runs with emDRC   #
#    ./macro.sh em em 5         # EM energy scan M5T2     #
#    ./macro.sh em em 4         # EM energy scan M4T2     #
###########################################################

# macro for all good runs (Calib)
calib=(
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
module=(
    1 1 1 1  # -- M1, T1-4
    2 2 2 2  # -- M2, T1-4
    3 3 3 3  # -- M3, T1-4
    4 4 4 4  # -- M4, T1-4
    5 5 5 5  # -- M5, T1-4
    6 6 6 6  # -- M6, T1-4
    7 7 7 7  # -- M7, T1-4
    8 8 8 8  # -- M8, T1-4
    9 9 9 9  # -- M9, T1-4
)

# Energy values for energy scans
em_energies_M5T2=(
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
pion_energies_M5T2=(
    12352 12353 12354 12355 12356 12357 12358 12359 12360 12361 12363 # 120GeV
    12364 12365 12366 12367 12368 12369 12370 12371 12372 12373 # 100GeV
    12374 12375 12376 12377 12378 12379 12380 12381 12382 12383 # 80GeV
    12384 12385 12386 12387 12388 # 60GeV
    12389 12390 12391 # 40GeV
    12392 12393 # 20GeV
)
kp_energies_M5T2=(
    12399 12401 12402 12403 12404 12405 12406 12407 12408 # 120GeV
    12409 12410 12411 12412 12413 12414 12415 # 100GeV
    12416 12417 12418 12419 12420 12421 # 80GeV
    12422 # 60GeV
     # 40GeV
     # 20GeV
)

evt=-1

# Default module
target_module=${3:-5}  # Default to M5T2 if not specified

######### Run List and Energy List #########
if [ "$1" == "Calib" ]; then
    runList=${calib[@]}
    beam_type=""  # No beam type for calibration
elif [ "$1" == "em" ]; then
    beam_type="em"
    if [ "$target_module" == "4" ]; then
        energyList=${em_energies_M4T2[@]}
    else
        energyList=${em_energies_M5T2[@]}
    fi
elif [ "$1" == "pi" ]; then
    beam_type="pi"
    # Add energy lists for pi when available
    energyList=${pion_energies_M5T2[@]}
elif [ "$1" == "kaon" ] || [ "$1" == "proton" ]; then
    beam_type="$1"
    # Add energy lists for kaon/proton when available  
    echo "WARNING: $1 beam energy scan not yet implemented"
    exit 1
else
    echo "Usage: $0 [Calib|em|pi|kaon|proton] [ats|dwc|evt|const] [module]"
    echo "  module: 4 for M4T2, 5 for M5T2 (default: 5)"
    exit 1
fi

######### Program Selection #########
if [ "$2" == "ats" ]; then
    opt="avgTimeStruc"
elif [ "$2" == "dwc" ]; then
    opt="draw_DWC_ntuple"
elif [ "$2" == "evt" ]; then
    opt="evtloop_emDRC"
elif [ "$2" == "const" ]; then
    opt="calc_calib"
else
    echo "Usage: $0 [Calib|em|pi|kaon|proton] [ats|dwc|evt|const] [module]"
    echo "Available programs:"
    echo "  ats    - avgTimeStruc" 
    echo "  dwc    - draw_DWC"
    echo "  evt    - evtloop_emDRC"
    echo "  const  - calc_calib"
    exit 1
fi

######### Execution #########
if [ "$2" == "const" ]; then
    # For calibration constants, use module numbers
    index=0
    for run in ${runList[@]};
    do
        mod_num=${module[$index]}
        # Calculate tower number (T1-T4 for each module)
        tower_num=$((($index % 4) + 1))
        echo "Beam type: $1, Program: $2, Run number: ${run}, Module: ${mod_num}, Tower: ${tower_num}"
        RunCmd="./${opt} ${run} ${mod_num} ${tower_num}"
        echo $RunCmd
        eval $RunCmd
        ((index++))
    done
    
    # Combine all calibration results into a single file
    echo "Combining all calibration constants into unified files..."
    
    # Create unified directory
    unified_dir="./CalibConst/Unified"
    mkdir -p "$unified_dir"
    
    # Create unified config file
    unified_config="$unified_dir/All_Calib_Constants.txt"
    echo "# Unified Calibration Constants" > "$unified_config"
    echo "# Generated from Calib runs" >> "$unified_config"
    echo "# Format: {\"key\", value}" >> "$unified_config"
    echo "" >> "$unified_config"
    
    # Process each module (1-9)
    for mod in {1..9}; do
        module_line=""
        found_any=false
        
        echo "Processing Module $mod..."
        
        # Check each tower for this module
        for tower in {1..4}; do
            # Find the corresponding run number and file
            mod_index=-1
            run_index=0
            for test_run in ${runList[@]}; do
                test_mod=${module[$run_index]}
                test_tower=$((($run_index % 4) + 1))
                if [ "$test_mod" == "$mod" ] && [ "$test_tower" == "$tower" ]; then
                    mod_index=$run_index
                    break
                fi
                ((run_index++))
            done
            
            if [ $mod_index -ge 0 ]; then
                # Convert runList array to indexed array and get specific element
                runList_array=(${runList[@]})
                run_num=${runList_array[$mod_index]}
                calib_file="./CalibConst/Run_${run_num}_Module_${mod}_Tower_${tower}_calib.csv"
                echo "  Looking for file: $calib_file"
                
                if [ -f "$calib_file" ]; then
                    echo "  Found file, reading data..."
                    # Read only the first line of the CSV file (remove trailing comma and newline)
                    tower_data=$(head -n 1 "$calib_file" | sed 's/,$//')
                    echo "  Tower data: $tower_data"
                    
                    if [ -n "$tower_data" ]; then
                        if [ "$found_any" == "true" ]; then
                            module_line="$module_line, $tower_data"
                        else
                            module_line="$tower_data"
                            found_any=true
                        fi
                    fi
                else
                    echo "  File not found: $calib_file"
                fi
            fi
        done
        
        # Write module line if any data was found
        if [ "$found_any" == "true" ]; then
            echo "Writing module $mod data: $module_line"
            echo "$module_line," >> "$unified_config"
        else
            echo "No data found for module $mod"
        fi
    done
    
    echo "Unified calibration constants saved to: $unified_config"

elif [ "$1" == "Calib" ]; then
    # For calibration runs without const option
    for run in ${runList[@]};
    do
        echo "Beam type: $1, Program: $2, Run number: ${run}, Module: M${target_module}T2"
        RunCmd="./${opt} ${run} ${evt} ${target_module}"
        echo $RunCmd
        eval $RunCmd
    done
    
else
    for energy in ${energyList[@]};
    do
        echo ""
        echo "Beam type: $1, Program: $2, Run number: ${energy}, Module: M${target_module}T2"
        RunCmd="./${opt} ${energy} ${evt} ${target_module} ${beam_type}"
        echo $RunCmd
        eval $RunCmd
        
        if [ $? -ne 0 ]; then
            echo "WARNING: Failed to process ${energy}GeV"
        else
            echo "✓ Successfully processed ${energy}GeV"
        fi
    done
fi
