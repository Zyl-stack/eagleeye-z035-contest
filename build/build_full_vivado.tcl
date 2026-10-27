# Rebuild restored EagleEye Z035 snapshot with Vivado 2022.2 in a new output directory.
# Usage: vivado -mode batch -source build/build_full_vivado.tcl -tclargs <snapshot> <new-output> [jobs]
if {$argc < 2 || $argc > 3} {error "Expected: <restored-snapshot> <new-output> [jobs]"}
set ee_snapshot [file normalize [lindex $argv 0]]
set ee_out [file normalize [lindex $argv 1]]
set ee_jobs 4
if {$argc == 3} {set ee_jobs [lindex $argv 2]}
if {![string is integer -strict $ee_jobs] || $ee_jobs < 1 || $ee_jobs > 16} {error "jobs must be an integer from 1 to 16"}
if {![string match "2022.2*" [version -short]]} {error "Vivado 2022.2 is required"}
set ee_recreate [file join $ee_snapshot build recreate_vivado.tcl]
if {![file isfile $ee_recreate]} {error "Restored A snapshot missing build/recreate_vivado.tcl"}
if {[file exists $ee_out]} {error "Output already exists; refusing to overwrite: $ee_out"}
set ee_snapshot_cmp [string tolower [file normalize $ee_snapshot]]
set ee_out_cmp [string tolower [file normalize $ee_out]]
if {$ee_out_cmp eq $ee_snapshot_cmp || [string first "$ee_snapshot_cmp/" "$ee_out_cmp/"] == 0} {error "Output must be outside the source snapshot"}
file mkdir $ee_out
set ::env(EAGLEEYE_BUILD_ROOT) $ee_out
source $ee_recreate
set ee_reports [file join $ee_out reports]
file mkdir $ee_reports
update_compile_order -fileset sources_1
launch_runs synth_1 -jobs $ee_jobs
wait_on_run synth_1
set ee_synth_status [get_property STATUS [get_runs synth_1]]
puts "SYNTH_STATUS=$ee_synth_status"
if {$ee_synth_status ne "synth_design Complete!"} {error "Synthesis failed: $ee_synth_status"}
open_run synth_1
report_utilization -file [file join $ee_reports synth_utilization.rpt]
launch_runs impl_1 -to_step write_bitstream -jobs $ee_jobs
wait_on_run impl_1
set ee_impl_status [get_property STATUS [get_runs impl_1]]
puts "IMPL_STATUS=$ee_impl_status"
if {$ee_impl_status ne "write_bitstream Complete!"} {error "Implementation or bitstream failed: $ee_impl_status"}
open_run impl_1
report_utilization -file [file join $ee_reports impl_utilization.rpt]
report_timing_summary -report_unconstrained -file [file join $ee_reports timing_summary.rpt]
report_drc -file [file join $ee_reports drc.rpt]
report_cdc -file [file join $ee_reports cdc.rpt]
report_methodology -file [file join $ee_reports methodology.rpt]
set ee_bit [file join $ee_out vivado_work eagleeye_z035.runs impl_1 z100_top.bit]
if {![file isfile $ee_bit]} {error "BIT file not found: $ee_bit"}
write_hw_platform -fixed -include_bit -force -file [file join $ee_out eagleeye_z035.xsa]
puts "EAGLEEYE_BUILD_COMPLETE bit=$ee_bit xsa=[file join $ee_out eagleeye_z035.xsa]"
puts "NOTE: Review timing coverage, CDC and DRC; this is not board validation."