`timescale 1ns / 1ps

// Horizontal-half / vertical-half phase.  Four symmetric pixels are grouped
// before multiplication, reducing the 6x6 convolution to nine multiplies.
module lanczos_scaler_core3 #(
    parameter PIXEL_WIDTH = 8,
    parameter COEFF_WIDTH = 12,
    parameter COEFF_SHIFT = 10
)(
    input wire sys_clk,
    input wire sys_rst,
    input wire [PIXEL_WIDTH-1:0] i_pix_00, i_pix_01, i_pix_02, i_pix_03, i_pix_04, i_pix_05,
    input wire [PIXEL_WIDTH-1:0] i_pix_10, i_pix_11, i_pix_12, i_pix_13, i_pix_14, i_pix_15,
    input wire [PIXEL_WIDTH-1:0] i_pix_20, i_pix_21, i_pix_22, i_pix_23, i_pix_24, i_pix_25,
    input wire [PIXEL_WIDTH-1:0] i_pix_30, i_pix_31, i_pix_32, i_pix_33, i_pix_34, i_pix_35,
    input wire [PIXEL_WIDTH-1:0] i_pix_40, i_pix_41, i_pix_42, i_pix_43, i_pix_44, i_pix_45,
    input wire [PIXEL_WIDTH-1:0] i_pix_50, i_pix_51, i_pix_52, i_pix_53, i_pix_54, i_pix_55,
    input wire signed [COEFF_WIDTH-1:0] i_w00, i_w01, i_w02,
    input wire signed [COEFF_WIDTH-1:0] i_w10, i_w11, i_w12,
    input wire signed [COEFF_WIDTH-1:0] i_w20, i_w21, i_w22,
    input wire i_valid,
    output reg [PIXEL_WIDTH-1:0] o_pix,
    output reg o_valid
);
    localparam GROUP_W = PIXEL_WIDTH + 3;
    localparam PROD_W  = GROUP_W + COEFF_WIDTH;
    localparam SUM_W   = PROD_W + 5;
    localparam integer PIX_MAX = (1 << PIXEL_WIDTH) - 1;

    reg [PIXEL_WIDTH-1:0] p [0:5][0:5];
    reg signed [COEFF_WIDTH-1:0] w00, w01, w02;
    reg signed [COEFF_WIDTH-1:0] w10, w11, w12;
    reg signed [COEFF_WIDTH-1:0] w20, w21, w22;
    reg s1_valid;
    integer r, c;

    wire signed [GROUP_W-1:0] g00 =
        $signed({{(GROUP_W-PIXEL_WIDTH){1'b0}},p[0][0]}) +
        $signed({{(GROUP_W-PIXEL_WIDTH){1'b0}},p[0][5]}) +
        $signed({{(GROUP_W-PIXEL_WIDTH){1'b0}},p[5][0]}) +
        $signed({{(GROUP_W-PIXEL_WIDTH){1'b0}},p[5][5]});
    wire signed [GROUP_W-1:0] g01 =
        $signed({{(GROUP_W-PIXEL_WIDTH){1'b0}},p[0][1]}) +
        $signed({{(GROUP_W-PIXEL_WIDTH){1'b0}},p[0][4]}) +
        $signed({{(GROUP_W-PIXEL_WIDTH){1'b0}},p[5][1]}) +
        $signed({{(GROUP_W-PIXEL_WIDTH){1'b0}},p[5][4]});
    wire signed [GROUP_W-1:0] g02 =
        $signed({{(GROUP_W-PIXEL_WIDTH){1'b0}},p[0][2]}) +
        $signed({{(GROUP_W-PIXEL_WIDTH){1'b0}},p[0][3]}) +
        $signed({{(GROUP_W-PIXEL_WIDTH){1'b0}},p[5][2]}) +
        $signed({{(GROUP_W-PIXEL_WIDTH){1'b0}},p[5][3]});
    wire signed [GROUP_W-1:0] g10 =
        $signed({{(GROUP_W-PIXEL_WIDTH){1'b0}},p[1][0]}) +
        $signed({{(GROUP_W-PIXEL_WIDTH){1'b0}},p[1][5]}) +
        $signed({{(GROUP_W-PIXEL_WIDTH){1'b0}},p[4][0]}) +
        $signed({{(GROUP_W-PIXEL_WIDTH){1'b0}},p[4][5]});
    wire signed [GROUP_W-1:0] g11 =
        $signed({{(GROUP_W-PIXEL_WIDTH){1'b0}},p[1][1]}) +
        $signed({{(GROUP_W-PIXEL_WIDTH){1'b0}},p[1][4]}) +
        $signed({{(GROUP_W-PIXEL_WIDTH){1'b0}},p[4][1]}) +
        $signed({{(GROUP_W-PIXEL_WIDTH){1'b0}},p[4][4]});
    wire signed [GROUP_W-1:0] g12 =
        $signed({{(GROUP_W-PIXEL_WIDTH){1'b0}},p[1][2]}) +
        $signed({{(GROUP_W-PIXEL_WIDTH){1'b0}},p[1][3]}) +
        $signed({{(GROUP_W-PIXEL_WIDTH){1'b0}},p[4][2]}) +
        $signed({{(GROUP_W-PIXEL_WIDTH){1'b0}},p[4][3]});
    wire signed [GROUP_W-1:0] g20 =
        $signed({{(GROUP_W-PIXEL_WIDTH){1'b0}},p[2][0]}) +
        $signed({{(GROUP_W-PIXEL_WIDTH){1'b0}},p[2][5]}) +
        $signed({{(GROUP_W-PIXEL_WIDTH){1'b0}},p[3][0]}) +
        $signed({{(GROUP_W-PIXEL_WIDTH){1'b0}},p[3][5]});
    wire signed [GROUP_W-1:0] g21 =
        $signed({{(GROUP_W-PIXEL_WIDTH){1'b0}},p[2][1]}) +
        $signed({{(GROUP_W-PIXEL_WIDTH){1'b0}},p[2][4]}) +
        $signed({{(GROUP_W-PIXEL_WIDTH){1'b0}},p[3][1]}) +
        $signed({{(GROUP_W-PIXEL_WIDTH){1'b0}},p[3][4]});
    wire signed [GROUP_W-1:0] g22 =
        $signed({{(GROUP_W-PIXEL_WIDTH){1'b0}},p[2][2]}) +
        $signed({{(GROUP_W-PIXEL_WIDTH){1'b0}},p[2][3]}) +
        $signed({{(GROUP_W-PIXEL_WIDTH){1'b0}},p[3][2]}) +
        $signed({{(GROUP_W-PIXEL_WIDTH){1'b0}},p[3][3]});

    wire signed [PROD_W-1:0] q00 = g00 * w00;
    wire signed [PROD_W-1:0] q01 = g01 * w01;
    wire signed [PROD_W-1:0] q02 = g02 * w02;
    wire signed [PROD_W-1:0] q10 = g10 * w10;
    wire signed [PROD_W-1:0] q11 = g11 * w11;
    wire signed [PROD_W-1:0] q12 = g12 * w12;
    wire signed [PROD_W-1:0] q20 = g20 * w20;
    wire signed [PROD_W-1:0] q21 = g21 * w21;
    wire signed [PROD_W-1:0] q22 = g22 * w22;

    wire signed [SUM_W-1:0] e00 = {{(SUM_W-PROD_W){q00[PROD_W-1]}},q00};
    wire signed [SUM_W-1:0] e01 = {{(SUM_W-PROD_W){q01[PROD_W-1]}},q01};
    wire signed [SUM_W-1:0] e02 = {{(SUM_W-PROD_W){q02[PROD_W-1]}},q02};
    wire signed [SUM_W-1:0] e10 = {{(SUM_W-PROD_W){q10[PROD_W-1]}},q10};
    wire signed [SUM_W-1:0] e11 = {{(SUM_W-PROD_W){q11[PROD_W-1]}},q11};
    wire signed [SUM_W-1:0] e12 = {{(SUM_W-PROD_W){q12[PROD_W-1]}},q12};
    wire signed [SUM_W-1:0] e20 = {{(SUM_W-PROD_W){q20[PROD_W-1]}},q20};
    wire signed [SUM_W-1:0] e21 = {{(SUM_W-PROD_W){q21[PROD_W-1]}},q21};
    wire signed [SUM_W-1:0] e22 = {{(SUM_W-PROD_W){q22[PROD_W-1]}},q22};

    reg signed [SUM_W-1:0] s2_q00, s2_q01, s2_q02;
    reg signed [SUM_W-1:0] s2_q10, s2_q11, s2_q12;
    reg signed [SUM_W-1:0] s2_q20, s2_q21, s2_q22;
    reg s2_valid;
    reg signed [SUM_W-1:0] s3_sum0, s3_sum1, s3_sum2;
    reg s3_valid;
    reg signed [SUM_W-1:0] s4_sum;
    reg s4_valid;
    wire signed [SUM_W-1:0] scaled = s4_sum >>> COEFF_SHIFT;
    wire [PIXEL_WIDTH-1:0] clamped =
        (scaled > PIX_MAX) ? PIX_MAX[PIXEL_WIDTH-1:0] :
        (scaled < 0)       ? {PIXEL_WIDTH{1'b0}} : scaled[PIXEL_WIDTH-1:0];
    reg [PIXEL_WIDTH-1:0] s5_pix;
    reg s5_valid;

    always @(posedge sys_clk) begin
        if (sys_rst) begin
            for (r = 0; r < 6; r = r + 1)
                for (c = 0; c < 6; c = c + 1)
                    p[r][c] <= {PIXEL_WIDTH{1'b0}};
            w00 <= 0; w01 <= 0; w02 <= 0;
            w10 <= 0; w11 <= 0; w12 <= 0;
            w20 <= 0; w21 <= 0; w22 <= 0;
            s1_valid <= 1'b0;
            s2_q00 <= 0; s2_q01 <= 0; s2_q02 <= 0;
            s2_q10 <= 0; s2_q11 <= 0; s2_q12 <= 0;
            s2_q20 <= 0; s2_q21 <= 0; s2_q22 <= 0;
            s2_valid <= 1'b0;
            s3_sum0 <= 0; s3_sum1 <= 0; s3_sum2 <= 0;
            s3_valid <= 1'b0;
            s4_sum <= 0; s4_valid <= 1'b0;
            s5_pix <= 0; s5_valid <= 1'b0;
            o_pix <= 0; o_valid <= 1'b0;
        end else begin
            p[0][0]<=i_pix_00; p[0][1]<=i_pix_01; p[0][2]<=i_pix_02; p[0][3]<=i_pix_03; p[0][4]<=i_pix_04; p[0][5]<=i_pix_05;
            p[1][0]<=i_pix_10; p[1][1]<=i_pix_11; p[1][2]<=i_pix_12; p[1][3]<=i_pix_13; p[1][4]<=i_pix_14; p[1][5]<=i_pix_15;
            p[2][0]<=i_pix_20; p[2][1]<=i_pix_21; p[2][2]<=i_pix_22; p[2][3]<=i_pix_23; p[2][4]<=i_pix_24; p[2][5]<=i_pix_25;
            p[3][0]<=i_pix_30; p[3][1]<=i_pix_31; p[3][2]<=i_pix_32; p[3][3]<=i_pix_33; p[3][4]<=i_pix_34; p[3][5]<=i_pix_35;
            p[4][0]<=i_pix_40; p[4][1]<=i_pix_41; p[4][2]<=i_pix_42; p[4][3]<=i_pix_43; p[4][4]<=i_pix_44; p[4][5]<=i_pix_45;
            p[5][0]<=i_pix_50; p[5][1]<=i_pix_51; p[5][2]<=i_pix_52; p[5][3]<=i_pix_53; p[5][4]<=i_pix_54; p[5][5]<=i_pix_55;
            w00<=i_w00; w01<=i_w01; w02<=i_w02;
            w10<=i_w10; w11<=i_w11; w12<=i_w12;
            w20<=i_w20; w21<=i_w21; w22<=i_w22;
            s1_valid <= i_valid;

            s2_q00 <= e00; s2_q01 <= e01; s2_q02 <= e02;
            s2_q10 <= e10; s2_q11 <= e11; s2_q12 <= e12;
            s2_q20 <= e20; s2_q21 <= e21; s2_q22 <= e22;
            s2_valid <= s1_valid;
            s3_sum0 <= s2_q00 + s2_q01 + s2_q02;
            s3_sum1 <= s2_q10 + s2_q11 + s2_q12;
            s3_sum2 <= s2_q20 + s2_q21 + s2_q22;
            s3_valid <= s2_valid;
            s4_sum <= s3_sum0 + s3_sum1 + s3_sum2;
            s4_valid <= s3_valid;
            s5_pix <= clamped;
            s5_valid <= s4_valid;
            o_pix <= s5_pix;
            o_valid <= s5_valid;
        end
    end
endmodule
