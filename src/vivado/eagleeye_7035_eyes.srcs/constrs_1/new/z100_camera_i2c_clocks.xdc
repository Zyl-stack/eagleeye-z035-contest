# Provisional 50 MHz maximum OV5640 PCLK for the 640x360 bring-up mode.
# Measure the actual PCLK on hardware and tighten this period if necessary.
create_clock -name ov5640_pclk -period 20.000 [get_ports cmos1_pclk]

# The vendor I2C engine toggles dri_clk after 50 cycles of the 100 MHz PS clock.
create_generated_clock -name ms7210_dri_clk -divide_by 100 \
    -source [get_pins -hierarchical -filter {NAME =~ *u_i2c_dri/dri_clk_reg/C}] \
    [get_pins -hierarchical -filter {NAME =~ *u_i2c_dri/dri_clk_reg/Q}]
