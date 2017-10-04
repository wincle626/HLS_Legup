#!/usr/bin/python3

import sys
import re
import os
import codecs

def printer(x):
    sys.stdout.write(str(x) + "\t")

fit_rpt = sys.argv[1]
timing_rpt = sys.argv[2]

benchmark = os.path.split(os.path.dirname(os.path.abspath(fit_rpt)))[1]

if os.path.isfile(fit_rpt):
    fit = codecs.open(fit_rpt, 'r', 'iso8859').read()
else:
    fit = ""

timing = open(timing_rpt).read()

total_logic = 0
top = (0,0)
main = (0,0)
traceSched = (0,0)

fit_total_logic = 0
fit_top = 0
fit_main = 0
fit_traceSched = 0
fit_hlsd = 0
fit_state_muxer = 0
fit_hlsd_total = 0
fit_dsp = 0
timing_fmax = 0
timing_fmax_units = "_"


if re.search("^;\s+Family\s+;\s+Stratix IV\s+;$", fit, flags=re.M|re.S):
    # Stratix IV
    m = re.search("^;\s+Fitter Resource Usage Summary\s+;$.*?^;\s+Logic utilization\s+;\s+([\d,]+) ", fit, flags=re.M|re.S)
    if m:
        fit_total_logic = int(m.group(1).replace(',',''))

    m = re.search("^;\s+Fitter Resource Utilization by Entity\s+;$.*?^;\s+\|top\s+; (\d+) ", fit, flags=re.M|re.S)
    if m:
        fit_top = m.group(1)
    
    m = re.search("^;\s+Fitter Resource Utilization by Entity\s+;$.*?^;\s+\|main:main_inst\|\s+; (\d+) ", fit, flags=re.M|re.S)
    if m:
        fit_main = m.group(1)
    
    m = re.search("^;\s+Fitter Resource Utilization by Entity\s+;$.*?^;\s+\|traceScheduler:traceScheduler_inst\|\s+; (\d+) ", fit, flags=re.M|re.S)
    if m:
        fit_traceSched = m.group(1)

    m = re.search("^;\s+Fitter Resource Utilization by Entity\s+;$.*?^;\s+\|hlsd:hlsd_inst\|\s+; (\d+) ", fit, flags=re.M|re.S)
    if m:
        fit_hlsd = m.group(1)
        
    m = re.search("^;\s+Fitter Resource Utilization by Entity\s+;$.*?^;\s+\|dbgStateMuxer:dbgStateMuxer_inst\|\s+; (\d+) ", fit, flags=re.M|re.S)
    if m:
        fit_state_muxer = m.group(1)
    m = re.search("^;\s+Fitter Resource Usage Summary\s+;$.*?^;\s+DSP block 18-bit elements\s+;\s+([\d,]+) ", fit, flags=re.M|re.S)
    if m:
        fit_dsp = int(m.group(1).replace(',',''))
            
    m = re.search("^;\s+Slow 900mV 85C Model Fmax Summary\s+;$.*?^;\s+(\d+\.\d+)\s+(.*?)\s+;.*?;\s+clk\s+", timing, flags=re.M|re.S)
    if m:
        timing_fmax = m.group(1)
        timing_fmax_units = m.group(2)

    fit_hlsd_total = int(fit_hlsd) + int(fit_state_muxer)

    printer(benchmark)
    printer(fit_total_logic)
    printer(fit_top)
    printer(fit_main)
    printer(fit_hlsd_total)
    printer(fit_traceSched)
    printer(fit_state_muxer)
    printer(fit_dsp)
    printer(timing_fmax)
    printer(timing_fmax_units)
    print("")
    
elif re.search("^;\s+Family\s+;\s+Cyclone II\s+;$", fit, flags=re.M|re.S):
    # Cyclone II
    m = re.search("^;\s+Fitter Summary\s+;$.*?^;\s+Total logic elements\s+;\s+([\d,]+) ", fit, flags=re.M|re.S)
    if m:
        fit_total_logic = int(m.group(1).replace(',',''))

    m = re.search("^;\s+Fitter Resource Utilization by Entity\s+;$.*?^;\s+\|top\s+; (\d+) ", fit, flags=re.M|re.S)
    if m:
        fit_top = m.group(1)
    
    m = re.search("^;\s+Fitter Resource Utilization by Entity\s+;$.*?^;\s+\|main:main_inst\|\s+; (\d+) ", fit, flags=re.M|re.S)
    if m:
        fit_main = m.group(1)
    
    m = re.search("^;\s+Fitter Resource Utilization by Entity\s+;$.*?^;\s+\|traceScheduler:traceScheduler_inst\|\s+; (\d+) ", fit, flags=re.M|re.S)
    if m:
        fit_traceSched = m.group(1)

    m = re.search("^;\s+Fitter Resource Utilization by Entity\s+;$.*?^;\s+\|hlsd:hlsd_inst\|\s+; (\d+) ", fit, flags=re.M|re.S)
    if m:
        fit_hlsd = m.group(1)
        
    m = re.search("^;\s+Fitter Resource Utilization by Entity\s+;$.*?^;\s+\|dbgStateMuxer:dbgStateMuxer_inst\|\s+; (\d+) ", fit, flags=re.M|re.S)
    if m:
        fit_state_muxer = m.group(1)
    
    m = re.search("^;\s+Fitter Summary\s+;$.*?^;\s+Embedded Multiplier 9-bit elements\s+;\s+([\d,]+) ", fit, flags=re.M|re.S)
    if m:
        fit_dsp = int(m.group(1).replace(',',''))

    m = re.search("^;\s+Slow Model Fmax Summary\s+;$.*?^;\s+(\d+\.\d+)\s+(.*?)\s+;", timing, flags=re.M|re.S)
    if m:
        timing_fmax = m.group(1)
        timing_fmax_units = m.group(2)

    fit_hlsd_total = int(fit_hlsd) + int(fit_state_muxer)

    printer(benchmark)
    printer(fit_total_logic)
    printer(fit_dsp)
    printer(timing_fmax)
    printer(timing_fmax_units)
    print("")

else: 
    # Xilinx
    m = re.search("^\s*Number of Slice Registers:\s*([\d,]+) out of", fit, flags=re.M|re.S)
    slice_regs = int(m.group(1).replace(',',''))

    m = re.search("^\s*Number of Slice LUTs:\s+([\d,]+) out of", fit, flags=re.M|re.S)
    slice_luts = int(m.group(1).replace(',',''))

    m = re.search("^\s*Number of occupied Slices:\s+([\d,]+) out of", fit, flags=re.M|re.S)
    occupied_slices = int(m.group(1).replace(',',''))

    m = re.search("^\s*Number of LUT Flip Flop pairs used:\s*([\d,]+)\s*$", fit, flags=re.M|re.S)
    lut_ff_pairs = int(m.group(1).replace(',',''))

    m = re.search("^\s*Minimum period is\s+(\d+.\d+)(.*?)\.", timing, flags=re.M|re.S)
    crit_path = m.group(1)
    crit_path_units = m.group(2)
    
    printer(benchmark)
    printer(slice_regs)
    printer(slice_luts)
    printer(occupied_slices)
    printer(lut_ff_pairs)
    printer(crit_path)
    printer(crit_path_units)
    print("")

