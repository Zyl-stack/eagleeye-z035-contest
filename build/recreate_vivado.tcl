# Recreate the Eyes source project with Vivado 2022.2 from this compact tree.
# Run from any directory: vivado -mode batch -source build/recreate_vivado.tcl
set root [file normalize [file join [file dirname [info script]] ..]]
set part xc7z035ffg900-2
set project_dir [file join $root build vivado_work]
if {[file exists $project_dir]} {
    error "Refusing to overwrite $project_dir; use a fresh copy of this snapshot"
}
set sources [file join $root src vivado EagleEye_7035_Eyes.srcs]
set rtl [file join $sources sources_1 imports rtl]
set bd_source [file join $sources sources_1 bd system system.bd]
set bd_copy [file join $root build bd_source system.bd]
file mkdir [file dirname $bd_copy]
file copy $bd_source $bd_copy

create_project eagleeye_z035 $project_dir -part $part
set_property target_language Verilog [current_project]
set_property ip_repo_paths [list \
    [file join $root src ip_repo video] \
    [file join $root src ip_repo preproc]] [current_project]
update_ip_catalog

foreach source [concat [glob -nocomplain [file join $rtl *.v]] \
                       [glob -nocomplain [file join $rtl ms7210 *.v]]] {
    if {[file tail $source] eq "z100_top.v"} {continue}
    add_files -norecurse $source
}
add_files -norecurse [file join $root src servo roi_servo_pwm.v]
add_files -norecurse [file join $root src servo z100_top_servo.v]
add_files -norecurse [file join $sources sources_1 imports system_wrapper.v]
add_files -norecurse $bd_copy

foreach constraint [glob -nocomplain [file join $sources constrs_1 new *.xdc]] {
    add_files -fileset constrs_1 -norecurse $constraint
}
add_files -fileset constrs_1 -norecurse [file join $root src servo servo_j37.xdc]
set_property top z100_top [get_filesets sources_1]
update_compile_order -fileset sources_1
open_bd_design $bd_copy
foreach cell {fovea_axil_ctrl_0 roi_servo_pwm_0 video_mode_switch_top_0 z100_ms7210_init_0} {
    set instance [get_bd_cells -quiet $cell]
    if {[llength $instance]} {update_module_reference $instance}
}
validate_bd_design
save_bd_design
generate_target all [get_files $bd_copy]
puts "EAGLEEYE_SOURCE_PROJECT_READY $project_dir"
puts "Review IP status and timing constraints before running synthesis/implementation."
