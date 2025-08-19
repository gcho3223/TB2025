# plotter codes

## `DrawATS`
- drawing average time structure of event
- usage
```bash
root -l -b -q Plotter_ATS.C'("run_number","center")'
```

## `DrawDWC`
- drawing correlation & beam spot of DWC
- usage
```bash
root -l -b -q Plotter_DWC.C'("run_number")'
```

## `Plotter_Plots`
- draw all histograms of run you choose
- usage
```bash
root -l -b -q Plotter_Plots.C'("run_number","beam_type","center")'
```
- NOTE: if name of histogram is changed, update ./header/histlist.h

## `produce_Resol`
- draw energy deposit distribution for each energy point
- draw energy resolution and linearty
- usage
```bash
root -l -b -q produce_Resol.cc'("beam_type","rotation","interaction","option")'
```
    - beam_type: em, pi, kaon, proton
    - rotation: on / off
    - interaction: on / off (run with interaction taget installed)
    - option: nnr (remove noise term and reversed x axis, this is for hadronic energy resolution)