module z100_ms7210_init (
    input  wire clk_100m,
    input  wire resetn,
    output wire hdmi_rstn,
    output wire hdmi_i2c_scl,
    inout  wire hdmi_i2c_sda,
    output wire init_done
);
    wire sda_i;
    wire sda_o;
    wire sda_t;

    IOBUF hdmi_sda_iobuf (
        .I(sda_o),
        .O(sda_i),
        .T(sda_t),
        .IO(hdmi_i2c_sda)
    );

    ms72xx_ctl #(
        .CLK_FREQ(100_000_000)
    ) ms7210 (
        .clk(clk_100m),
        .rst_n(resetn),
        .rstn_out(hdmi_rstn),
        .init_over(init_done),
        .iic_scl(hdmi_i2c_scl),
        .iic_sda_i(sda_i),
        .iic_sda_o(sda_o),
        .iic_sda_t(sda_t)
    );
endmodule
