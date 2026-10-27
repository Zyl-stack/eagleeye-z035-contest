`timescale 1ns / 1ps

// Vertical half-pixel phase.  Arithmetic is identical to core1; only the
// six supplied samples come from a column instead of a row.
module lanczos_scaler_core2 #(
    parameter PIXEL_WIDTH = 8,
    parameter COEFF_WIDTH = 12,
    parameter COEFF_SHIFT = 10
)(
    input  wire                          sys_clk,
    input  wire                          sys_rst,
    input  wire [PIXEL_WIDTH-1:0]        i_p0,
    input  wire [PIXEL_WIDTH-1:0]        i_p1,
    input  wire [PIXEL_WIDTH-1:0]        i_p2,
    input  wire [PIXEL_WIDTH-1:0]        i_p3,
    input  wire [PIXEL_WIDTH-1:0]        i_p4,
    input  wire [PIXEL_WIDTH-1:0]        i_p5,
    input  wire signed [COEFF_WIDTH-1:0] i_w0,
    input  wire signed [COEFF_WIDTH-1:0] i_w1,
    input  wire signed [COEFF_WIDTH-1:0] i_w2,
    input  wire                          i_valid,
    output wire [PIXEL_WIDTH-1:0]        o_pix,
    output wire                          o_valid
);
    lanczos_scaler_core1 #(
        .PIXEL_WIDTH(PIXEL_WIDTH),
        .COEFF_WIDTH(COEFF_WIDTH),
        .COEFF_SHIFT(COEFF_SHIFT)
    ) u_vertical_mac (
        .sys_clk(sys_clk), .sys_rst(sys_rst),
        .i_p0(i_p0), .i_p1(i_p1), .i_p2(i_p2),
        .i_p3(i_p3), .i_p4(i_p4), .i_p5(i_p5),
        .i_w0(i_w0), .i_w1(i_w1), .i_w2(i_w2),
        .i_valid(i_valid), .o_pix(o_pix), .o_valid(o_valid)
    );
endmodule

