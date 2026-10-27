`timescale 1ns / 1ps

// Horizontal half-pixel phase.  Symmetry reduces six taps to three multiplies.
module lanczos_scaler_core1 #(
    parameter PIXEL_WIDTH = 8,
    parameter COEFF_WIDTH = 12,
    parameter COEFF_SHIFT = 10
)(
    input  wire                              sys_clk,
    input  wire                              sys_rst,
    input  wire [PIXEL_WIDTH-1:0]            i_p0,
    input  wire [PIXEL_WIDTH-1:0]            i_p1,
    input  wire [PIXEL_WIDTH-1:0]            i_p2,
    input  wire [PIXEL_WIDTH-1:0]            i_p3,
    input  wire [PIXEL_WIDTH-1:0]            i_p4,
    input  wire [PIXEL_WIDTH-1:0]            i_p5,
    input  wire signed [COEFF_WIDTH-1:0]     i_w0,
    input  wire signed [COEFF_WIDTH-1:0]     i_w1,
    input  wire signed [COEFF_WIDTH-1:0]     i_w2,
    input  wire                              i_valid,
    output reg  [PIXEL_WIDTH-1:0]            o_pix,
    output reg                               o_valid
);
    localparam PAIR_W = PIXEL_WIDTH + 2;
    localparam PROD_W = PAIR_W + COEFF_WIDTH;
    localparam SUM_W  = PROD_W + 3;
    localparam integer PIX_MAX = (1 << PIXEL_WIDTH) - 1;

    reg [PIXEL_WIDTH-1:0] s1_p0, s1_p1, s1_p2, s1_p3, s1_p4, s1_p5;
    reg signed [COEFF_WIDTH-1:0] s1_w0, s1_w1, s1_w2;
    reg s1_valid;

    wire signed [PAIR_W-1:0] pair0 = $signed({1'b0, s1_p0}) + $signed({1'b0, s1_p5});
    wire signed [PAIR_W-1:0] pair1 = $signed({1'b0, s1_p1}) + $signed({1'b0, s1_p4});
    wire signed [PAIR_W-1:0] pair2 = $signed({1'b0, s1_p2}) + $signed({1'b0, s1_p3});

    wire signed [PROD_W-1:0] prod0 = pair0 * s1_w0;
    wire signed [PROD_W-1:0] prod1 = pair1 * s1_w1;
    wire signed [PROD_W-1:0] prod2 = pair2 * s1_w2;
    wire signed [SUM_W-1:0] prod0_ext = {{(SUM_W-PROD_W){prod0[PROD_W-1]}}, prod0};
    wire signed [SUM_W-1:0] prod1_ext = {{(SUM_W-PROD_W){prod1[PROD_W-1]}}, prod1};
    wire signed [SUM_W-1:0] prod2_ext = {{(SUM_W-PROD_W){prod2[PROD_W-1]}}, prod2};

    reg signed [SUM_W-1:0] s2_prod0;
    reg signed [SUM_W-1:0] s2_prod1;
    reg signed [SUM_W-1:0] s2_prod2;
    reg s2_valid;
    reg signed [SUM_W-1:0] s3_sum;
    reg s3_valid;
    wire signed [SUM_W-1:0] scaled = s3_sum >>> COEFF_SHIFT;
    wire [PIXEL_WIDTH-1:0] clamped =
        (scaled > PIX_MAX) ? PIX_MAX[PIXEL_WIDTH-1:0] :
        (scaled < 0)       ? {PIXEL_WIDTH{1'b0}} : scaled[PIXEL_WIDTH-1:0];

    reg [PIXEL_WIDTH-1:0] s4_pix, s5_pix;
    reg s4_valid, s5_valid;

    always @(posedge sys_clk) begin
        if (sys_rst) begin
            s1_p0 <= 0; s1_p1 <= 0; s1_p2 <= 0;
            s1_p3 <= 0; s1_p4 <= 0; s1_p5 <= 0;
            s1_w0 <= 0; s1_w1 <= 0; s1_w2 <= 0;
            s1_valid <= 1'b0;
            s2_prod0 <= 0; s2_prod1 <= 0; s2_prod2 <= 0; s2_valid <= 1'b0;
            s3_sum <= 0; s3_valid <= 1'b0;
            s4_pix <= 0; s5_pix <= 0;
            s4_valid <= 1'b0; s5_valid <= 1'b0;
            o_pix <= 0; o_valid <= 1'b0;
        end else begin
            s1_p0 <= i_p0; s1_p1 <= i_p1; s1_p2 <= i_p2;
            s1_p3 <= i_p3; s1_p4 <= i_p4; s1_p5 <= i_p5;
            s1_w0 <= i_w0; s1_w1 <= i_w1; s1_w2 <= i_w2;
            s1_valid <= i_valid;

            s2_prod0 <= prod0_ext;
            s2_prod1 <= prod1_ext;
            s2_prod2 <= prod2_ext;
            s2_valid <= s1_valid;
            s3_sum <= s2_prod0 + s2_prod1 + s2_prod2;
            s3_valid <= s2_valid;
            s4_pix <= clamped;
            s4_valid <= s3_valid;
            s5_pix <= s4_pix;
            s5_valid <= s4_valid;
            o_pix <= s5_pix;
            o_valid <= s5_valid;
        end
    end
endmodule
