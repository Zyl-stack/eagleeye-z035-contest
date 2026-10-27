# Run: vivado -mode batch -source scripts/build.tcl -tclargs <synth|impl|package>
# Uses only Verilog, MEM and XDC source files. Intended for Vivado 2022.2+.
set root [file normalize [file join [file dirname [info script]] ..]]
set mode [lindex $argv 0]
if {$mode ni {synth impl package}} {error "mode must be synth, impl, or package"}
set part xc7z035ffg900-2
set out [file join $root build vivado_$mode]
file mkdir $out
create_project -force preproc $out -part $part
set_property target_language Verilog [current_project]
add_files [list [file join $root rtl preproc_axis_core.v] [file join $root rtl rgb888_mm2s_adapter.v] [file join $root rtl picodet_preproc_ip.v]]
add_files [list [file join $root rtl normalize_r.mem] [file join $root rtl normalize_g.mem] [file join $root rtl normalize_b.mem]]
set_property top picodet_preproc_ip [current_fileset]
add_files -fileset constrs_1 [file join $root constraints clock_100mhz.xdc]
update_compile_order -fileset sources_1
if {$mode eq "package"} {
    ipx::package_project -root_dir $root -vendor eagleeye.local -library user -taxonomy /UserIP -import_files -set_current true
    set core [ipx::current_core]
    set_property name picodet_preproc_ip $core
    set_property display_name "PicoDet RGB888 bilinear FP32 NCHW" $core
    set_property description "640x360 RGB888 AXIS to three 320x320 FP32 AXIS planes" $core
    set_property version 1.0 $core
    set axi [ipx::get_bus_interfaces s_axi -of_objects $core]
    set_property value AXI4LITE [ipx::add_bus_parameter PROTOCOL $axi]
    set clkif [ipx::get_bus_interfaces aclk -of_objects $core]
    set_property value 100000000 [ipx::add_bus_parameter FREQ_HZ $clkif]
    ipx::create_xgui_files $core
    ipx::update_checksums $core
    ipx::save_core $core
    puts "PACKAGED: [file join $root component.xml]"
} else {
    synth_design -top picodet_preproc_ip -part $part -flatten_hierarchy rebuilt
    report_utilization -file [file join $out utilization_synth.rpt]
    report_timing_summary -file [file join $out timing_synth.rpt]
    write_checkpoint -force [file join $out post_synth.dcp]
    if {$mode eq "impl"} {
        opt_design
        place_design
        phys_opt_design
        route_design
        report_utilization -file [file join $out utilization_impl.rpt]
        report_timing_summary -file [file join $out timing_impl.rpt]
        write_checkpoint -force [file join $out post_route.dcp]
    }
}
close_project
