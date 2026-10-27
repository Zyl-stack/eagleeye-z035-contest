# vivado -mode batch -source scripts/sim_xsim.tcl -tclargs <frame-name>
# Example: ... -tclargs captured
set root [file normalize [file join [file dirname [info script]] ..]]
set frame [lindex $argv 0]
if {$frame ni {captured zero full distinct edge}} {error "unknown frame name"}
set out [file join $root build xsim_$frame]
file mkdir $out
create_project -force preproc_sim $out -part xc7z035ffg900-2
add_files -fileset sim_1 [list \
    [file join $root rtl preproc_axis_core.v] \
    [file join $root tb tb_preproc.v] \
    [file join $root rtl normalize_r.mem] \
    [file join $root rtl normalize_g.mem] \
    [file join $root rtl normalize_b.mem]]
set_property top tb_preproc [get_filesets sim_1]
set input_file [file normalize [file join $root vectors ${frame}.hex]]
set output_prefix [file normalize [file join $root build ${frame}_xsim]]
set_property -dict [list xsim.simulate.xsim.more_options "-testplusarg INPUT=$input_file -testplusarg OUTPUT=$output_prefix"] [get_filesets sim_1]
set_property xsim.simulate.runtime 0ns [get_filesets sim_1]
launch_simulation -mode behavioral
run all
close_sim
close_project
if {[info exists ::env(PYTHONHOME)]} {unset ::env(PYTHONHOME)}
if {[info exists ::env(PYTHONPATH)]} {unset ::env(PYTHONPATH)}
set python_exe [expr {[info exists ::env(PYTHON)] ? $::env(PYTHON) : "python"}]
puts [exec $python_exe [file join $root tools compare.py] $frame --prefix ${frame}_xsim]
