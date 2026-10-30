# Experimental migration of the restored Eyes snapshot to Vivado 2025.2.
# Usage: vivado -mode batch -source build/build_vivado_2025_2.tcl \
#        -tclargs <restored-snapshot> <new-output> ?project|bit? ?jobs?
# The source snapshot is copied into the new output directory before opening the BD.
if {$argc < 2 || $argc > 4} {
    error "Expected: <restored-snapshot> <new-output> ?project|bit? ?jobs?"
}
if {![string match "2025.2*" [version -short]]} {
    error "This experimental migration requires Vivado 2025.2"
}
set ee_source [file normalize [lindex $argv 0]]
set ee_out [file normalize [lindex $argv 1]]
set ee_mode project
set ee_jobs 4
if {$argc >= 3} {set ee_mode [lindex $argv 2]}
if {$argc == 4} {set ee_jobs [lindex $argv 3]}
if {$ee_mode ni {project bit}} {error "Mode must be project or bit"}
if {![string is integer -strict $ee_jobs] || $ee_jobs < 1 || $ee_jobs > 16} {
    error "jobs must be an integer from 1 to 16"
}
if {[file exists $ee_out]} {error "Output already exists; refusing to overwrite: $ee_out"}
set ee_source_cmp [string tolower $ee_source]
set ee_out_cmp [string tolower $ee_out]
if {$ee_out_cmp eq $ee_source_cmp || [string first "$ee_source_cmp/" "$ee_out_cmp/"] == 0} {
    error "Output must be outside the source snapshot"
}
foreach p [list $ee_source $ee_out] {
    if {[string length $p] > 64 || [string first " " $p] >= 0 || [regexp {[^\x00-\x7f]} $p]} {
        error "Use short ASCII paths without spaces, for example D:/EEA and D:/EE25"
    }
}
set ee_src [file join $ee_source src]
set ee_expected [list \
    [file join $ee_src vivado EagleEye_7035_Eyes.srcs sources_1 bd system system.bd] \
    [file join $ee_src vivado EagleEye_7035_Eyes.srcs sources_1 imports system_wrapper.v] \
    [file join $ee_src servo roi_servo_pwm.v] \
    [file join $ee_src servo z100_top_servo.v] \
    [file join $ee_src servo servo_j37.xdc] \
    [file join $ee_src ip_repo video] \
    [file join $ee_src ip_repo preproc]]
foreach p $ee_expected {
    if {![file exists $p]} {error "Missing restored snapshot input: $p"}
}

file mkdir $ee_out
set ee_copy [file join $ee_out source_copy]
file mkdir $ee_copy
foreach dir {vivado servo ip_repo} {
    file copy [file join $ee_src $dir] [file join $ee_copy $dir]
}
set ee_reports [file join $ee_out reports]
file mkdir $ee_reports
set ee_project [file join $ee_out vivado_work]
set ee_sources [file join $ee_copy vivado EagleEye_7035_Eyes.srcs]
set ee_rtl [file join $ee_sources sources_1 imports rtl]
set ee_bd [file join $ee_sources sources_1 bd system system.bd]

# Repoint generated-output paths in the private BD/XCI copies only.
set ee_gen [file normalize [file join $ee_project eagleeye_z035.gen]]
foreach f [concat [list $ee_bd] [glob -nocomplain [file join $ee_sources sources_1 bd system ip * *.xci]]] {
    set channel [open $f r]
    set content [read $channel]
    close $channel
    regsub -all {(\.\./)+EagleEye_7035_Eyes\.gen} $content $ee_gen content
    set channel [open $f w]
    puts -nonewline $channel $content
    close $channel
}

create_project eagleeye_z035 $ee_project -part xc7z035ffg900-2
set_property target_language Verilog [current_project]
set_property ip_repo_paths [list [file join $ee_copy ip_repo video] [file join $ee_copy ip_repo preproc]] [current_project]
update_ip_catalog
foreach f [concat [glob -nocomplain [file join $ee_rtl *.v]] [glob -nocomplain [file join $ee_rtl ms7210 *.v]]] {
    if {[file tail $f] eq "z100_top.v"} {continue}
    add_files -norecurse $f
}
add_files -norecurse [file join $ee_copy servo roi_servo_pwm.v]
add_files -norecurse [file join $ee_copy servo z100_top_servo.v]
add_files -norecurse [file join $ee_sources sources_1 imports system_wrapper.v]
add_files -norecurse $ee_bd
foreach f [glob -nocomplain [file join $ee_sources constrs_1 new *.xdc]] {
    add_files -fileset constrs_1 -norecurse $f
}
add_files -fileset constrs_1 -norecurse [file join $ee_copy servo servo_j37.xdc]
set_property top z100_top [get_filesets sources_1]
update_compile_order -fileset sources_1

open_bd_design $ee_bd
report_ip_status -file [file join $ee_reports ip_status_before.rpt]
set ee_cells {}
foreach cell [get_bd_cells -hierarchical *] {
    if {[get_property TYPE $cell] eq "ip" && ![string match "*:module_ref:*" [get_property VLNV $cell]]} {
        lappend ee_cells $cell
    }
}
if {[llength $ee_cells] == 0} {error "Block design is empty; refusing to continue"}
# Upgrade catalog IP only: hierarchies and RTL module references are not IP instances.
# Never use get_ips -all.
upgrade_ip -log [file join $ee_reports ip_upgrade.log] $ee_cells
validate_bd_design
save_bd_design
report_ip_status -file [file join $ee_reports ip_status_after.rpt]
set ee_locked {}
foreach ip [get_ips] {
    if {[get_property IS_LOCKED $ip]} {lappend ee_locked $ip}
}
if {[llength $ee_locked] > 0} {
    error "Locked IP after upgrade: $ee_locked; inspect reports and Vivado log"
}
generate_target all [get_files $ee_bd]
update_compile_order -fileset sources_1
puts "EAGLEEYE_2025_PROJECT_READY [file join $ee_project eagleeye_z035.xpr]"
puts "REVIEW: inspect IP upgrade/status, BD, address map, clocks, reset, constraints and licenses."
if {$ee_mode eq "project"} {return}

launch_runs synth_1 -jobs $ee_jobs
wait_on_run synth_1
set ee_status [get_property STATUS [get_runs synth_1]]
puts "SYNTH_STATUS=$ee_status"
if {$ee_status ne "synth_design Complete!"} {error "Synthesis failed: $ee_status"}
open_run synth_1
report_utilization -file [file join $ee_reports synth_utilization.rpt]
launch_runs impl_1 -to_step write_bitstream -jobs $ee_jobs
wait_on_run impl_1
set ee_status [get_property STATUS [get_runs impl_1]]
puts "IMPL_STATUS=$ee_status"
if {$ee_status ne "write_bitstream Complete!"} {error "Implementation failed: $ee_status"}
open_run impl_1
report_utilization -file [file join $ee_reports impl_utilization.rpt]
report_timing_summary -report_unconstrained -file [file join $ee_reports timing_summary.rpt]
report_drc -file [file join $ee_reports drc.rpt]
report_cdc -file [file join $ee_reports cdc.rpt]
report_methodology -file [file join $ee_reports methodology.rpt]
set ee_bit [file join $ee_project eagleeye_z035.runs impl_1 z100_top.bit]
if {![file isfile $ee_bit]} {error "BIT not found: $ee_bit"}
write_hw_platform -fixed -include_bit -force -file [file join $ee_out eagleeye_z035.xsa]
puts "EAGLEEYE_2025_BIT_CREATED bit=$ee_bit xsa=[file join $ee_out eagleeye_z035.xsa]"
puts "NOT BOARD VALIDATED: inspect reports and revalidate hardware/software compatibility."
