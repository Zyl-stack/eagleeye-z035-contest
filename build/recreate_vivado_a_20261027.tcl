# Recreate the Eyes source project with Vivado 2022.2 from this compact tree.
# Run from any directory: vivado -mode batch -source build/recreate_vivado.tcl
set root [file normalize [file join [file dirname [info script]] ..]]
# Vivado 2022.2 has Windows path-length and non-ASCII/space handling issues.
if {[string length $root] > 64 || [string first " " $root] >= 0 || [regexp {[^\x00-\x7f]} $root]} {
    error "Copy this source snapshot to a short ASCII path without spaces (for example D:/EEZ035), then rerun"
}
set part xc7z035ffg900-2
set work_root [file join $root build]
if {[info exists ::env(EAGLEEYE_BUILD_ROOT)] && $::env(EAGLEEYE_BUILD_ROOT) ne ""} {
    set work_root [file normalize $::env(EAGLEEYE_BUILD_ROOT)]
}
set project_dir [file join $work_root vivado_work]
set bd_parent [file join $work_root bd_source]
if {[file exists $project_dir] || [file exists $bd_parent]} {
    error "Refusing to overwrite existing build files under $work_root"
}
set sources [file join $root src vivado EagleEye_7035_Eyes.srcs]
set rtl [file join $sources sources_1 imports rtl]
set bd_source [file join $sources sources_1 bd system system.bd]
file mkdir $bd_parent
file copy [file join $sources sources_1 bd system] $bd_parent
set bd_copy [file join $bd_parent system system.bd]
# Relocate copied BD/XCI generated-output paths; the originals are not changed.
set gen_root [file normalize [file join $project_dir eagleeye_z035.gen]]
foreach source [concat [list $bd_copy] [glob -nocomplain [file join $bd_parent system ip * *.xci]]] {
    set in [open $source r]
    set content [read $in]
    close $in
    regsub -all {(\.\./)+EagleEye_7035_Eyes\.gen} $content $gen_root content
    set out [open $source w]
    puts -nonewline $out $content
    close $out
}

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
validate_bd_design
save_bd_design
generate_target all [get_files $bd_copy]
puts "EAGLEEYE_SOURCE_PROJECT_READY $project_dir"
puts "Review IP status and timing constraints before running synthesis/implementation."
