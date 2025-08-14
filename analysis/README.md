# TB2025 Analysis Package - Refactored Version
This analysis package is refactored code to analyze for TB2025 beam test data.

### Condor
- you can use condor job to run scripts for all runs

### header
## preset.h
- preset contains all configuration values (e.g. integration range, PID cut, equalization constant, scale factor, atteneuation correction)

### output
- `***merge_energy_histograms.cc***` is a tool to merge output files for each beam energy points

### plots
- `***produce_Resol.cc***`, `***Resolution.h***`: plotter for energy deposit distribution, energy resolution per points, and linearity

### Analysis codes
## `***draw_DWC***`: for DWC distribution(beam position)
- ussage
```bash
./draw_DWC <run_number> <max_event> <beam_type>
./draw_DWC 12345 -1 pi
```
- beam type: em, pi, kp(kaon, proton)
## `***avgTimeStruc***`: average time structure of signal of each detector
- usage
```bash
./avgTimeStruc <run_number> <max_event> <beam_type>
./avgTimeStruc 12345 1000 kp
```
- beam type: em, pi, kp(kaon, proton)
## `***evtloop_DRC***`: analysis code.
- usage
```bash
./evtloop_DRC <run_number> <max_event> <module> <beam_type> <rotation_option> <interaction_target_option>
./evtloop_DRC 13425 -1 5 proton on off
```
- module: center module(M5T2 -> 5, M4T2 -> 4)
- beam_type: em, pi, kaon, proton
- rotation_option: on|off
    - on: DRC has 1.5 degree rotated
    - **only for pi, kaon, and proton**
- interaction_target_option: on|off
    - on: there is interaction target installed between PS and DRC
    - **only for pi, kaon, and proton**