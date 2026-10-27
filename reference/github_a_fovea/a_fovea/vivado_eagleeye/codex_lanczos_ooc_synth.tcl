set src_dir {D:/7020_ov5640_ps_/25_ov5640_sd_646/ov5640_sd.srcs/sources_1/imports/rtl}
read_verilog -sv {D:/Vivado/Vivado/2022.2/data/ip/xpm/xpm_memory/hdl/xpm_memory.sv}
read_verilog [list \
    $src_dir/lanczos_coeff_2d_rom.v \
    $src_dir/lanczos_scaler_core0.v \
    $src_dir/lanczos_scaler_core1.v \
    $src_dir/lanczos_scaler_core2.v \
    $src_dir/lanczos_scaler_core3.v \
    $src_dir/lanczos_scaler_top.v \
    $src_dir/lanczos_scaler_rgb_top.v]
synth_design -top lanczos_scaler_rgb_top -part xc7z020clg400-2 -mode out_of_context -generic {IMG_WIDTH=640 IMG_HEIGHT=360}
report_utilization -file {D:/7020_ov5640_ps_/25_ov5640_sd_646/codex_lanczos_ooc_util.rpt}
