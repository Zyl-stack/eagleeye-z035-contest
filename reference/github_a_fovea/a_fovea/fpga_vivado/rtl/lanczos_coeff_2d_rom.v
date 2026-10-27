`timescale 1ns / 1ps

// Fixed Q10 coefficients for the four phases of a 2x Lanczos-3 scaler.
// Integer/integer phase is implemented as a direct pixel copy in core0.
module lanczos_coeff_2d_rom #(
    parameter COEFF_WIDTH = 12
)(
    output wire signed [COEFF_WIDTH-1:0] o_w0,
    output wire signed [COEFF_WIDTH-1:0] o_w1,
    output wire signed [COEFF_WIDTH-1:0] o_w2,

    output wire signed [COEFF_WIDTH-1:0] o_w00,
    output wire signed [COEFF_WIDTH-1:0] o_w01,
    output wire signed [COEFF_WIDTH-1:0] o_w02,
    output wire signed [COEFF_WIDTH-1:0] o_w10,
    output wire signed [COEFF_WIDTH-1:0] o_w11,
    output wire signed [COEFF_WIDTH-1:0] o_w12,
    output wire signed [COEFF_WIDTH-1:0] o_w20,
    output wire signed [COEFF_WIDTH-1:0] o_w21,
    output wire signed [COEFF_WIDTH-1:0] o_w22
);
    // 1-D half-pixel kernel:
    // [25, -139, 626, 626, -139, 25], sum = 1024.
    assign o_w0 = 25;
    assign o_w1 = -139;
    assign o_w2 = 626;

    // Compressed 2-D half/half kernel.  Each coefficient multiplies a
    // four-pixel symmetry group, so 9 multiplies cover the 6x6 window.
    assign o_w00 = 1;
    assign o_w01 = -3;
    assign o_w02 = 15;
    assign o_w10 = -3;
    assign o_w11 = 19;
    assign o_w12 = -85;
    assign o_w20 = 15;
    assign o_w21 = -85;
    assign o_w22 = 383;
endmodule

