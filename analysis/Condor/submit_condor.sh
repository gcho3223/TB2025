#!/bin/bash

if [ "$#" -lt 4 ]; then
    echo "Usage: ./submit_condor.sh particle:[calib|em|pi|kp] program:[ats|dwc|evt] rotation:[on|off] interaction target:[on|off]"
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

for run in "${runlist[@]}";
do
    sample_path="./${version}/${particle}/${program}/${run}"
    cd ${sample_path}
    condor_submit "condor_sub.sub"
    echo "submitted ${run} jobs!!"
    cd - > /dev/null  # 경로 출력을 숨김
done
