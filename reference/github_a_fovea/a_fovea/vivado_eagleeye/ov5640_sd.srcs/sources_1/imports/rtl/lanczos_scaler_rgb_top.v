`timescale 1ns / 1ps
`default_nettype none

// RGB wrapper for three lock-stepped 8-bit single-channel Lanczos scalers.
//
// Pixel layout:
//   s_axis_tdata[23:16] = R
//   s_axis_tdata[15:8]  = G
//   s_axis_tdata[7:0]   = B
//
// The three lane controllers are intentionally kept in lockstep.  Input valid
// is presented to the lanes only when all three are ready, and output ready is
// presented to the lanes only when all three have a valid pixel.  Therefore no
// colour lane can consume or emit a pixel by itself.
module lanczos_scaler_rgb_top #(
    parameter IMG_WIDTH       = 640,
    parameter IMG_HEIGHT      = 480,
    parameter COEFF_WIDTH     = 12,
    parameter X_COORD_WIDTH   = (2*IMG_WIDTH  <= 2) ? 1 : $clog2(2*IMG_WIDTH),
    parameter Y_COORD_WIDTH   = (2*IMG_HEIGHT <= 2) ? 1 : $clog2(2*IMG_HEIGHT)
)(
    input  wire                         sys_clk,
    input  wire                         sys_rst,

    input  wire [23:0]                  s_axis_tdata,
    input  wire                         s_axis_tvalid,
    output wire                         s_axis_tready,
    input  wire                         s_axis_tlast,
    input  wire                         s_axis_tuser,

    output wire [23:0]                  m_axis_tdata,
    output wire                         m_axis_tvalid,
    input  wire                         m_axis_tready,
    output wire                         m_axis_tlast,
    output wire                         m_axis_tuser,

    output wire [X_COORD_WIDTH-1:0]     o_serial_x,
    output wire [Y_COORD_WIDTH-1:0]     o_serial_y
);
    wire [7:0] s_r = s_axis_tdata[23:16];
    wire [7:0] s_g = s_axis_tdata[15:8];
    wire [7:0] s_b = s_axis_tdata[7:0];

    wire r_s_ready;
    wire g_s_ready;
    wire b_s_ready;
    wire all_s_ready = r_s_ready && g_s_ready && b_s_ready;

    // Gate the internal valid with the combined ready.  If the lane ready
    // signals ever differ, none of the lanes is allowed to accept the pixel.
    wire lane_s_valid = s_axis_tvalid && all_s_ready;
    assign s_axis_tready = all_s_ready;

    wire [7:0] r_m_data;
    wire [7:0] g_m_data;
    wire [7:0] b_m_data;
    wire r_m_valid;
    wire g_m_valid;
    wire b_m_valid;
    wire r_m_last;
    wire g_m_last;
    wire b_m_last;
    wire r_m_user;
    wire g_m_user;
    wire b_m_user;
    wire [X_COORD_WIDTH-1:0] r_x;
    wire [X_COORD_WIDTH-1:0] g_x;
    wire [X_COORD_WIDTH-1:0] b_x;
    wire [Y_COORD_WIDTH-1:0] r_y;
    wire [Y_COORD_WIDTH-1:0] g_y;
    wire [Y_COORD_WIDTH-1:0] b_y;

    wire all_m_valid = r_m_valid && g_m_valid && b_m_valid;

    // A lane advances only when one complete RGB pixel is transferred.
    wire lane_m_ready = m_axis_tready && all_m_valid;

    lanczos_scaler_top #(
        .IMG_WIDTH(IMG_WIDTH),
        .IMG_HEIGHT(IMG_HEIGHT),
        .PIXEL_WIDTH(8),
        .COEFF_WIDTH(COEFF_WIDTH),
        .X_COORD_WIDTH(X_COORD_WIDTH),
        .Y_COORD_WIDTH(Y_COORD_WIDTH)
    ) u_scaler_r (
        .sys_clk(sys_clk),
        .sys_rst(sys_rst),
        .s_axis_tdata(s_r),
        .s_axis_tvalid(lane_s_valid),
        .s_axis_tready(r_s_ready),
        .s_axis_tlast(s_axis_tlast),
        .s_axis_tuser(s_axis_tuser),
        .m_axis_tdata(r_m_data),
        .m_axis_tvalid(r_m_valid),
        .m_axis_tready(lane_m_ready),
        .m_axis_tlast(r_m_last),
        .m_axis_tuser(r_m_user),
        .o_serial_x(r_x),
        .o_serial_y(r_y)
    );

    lanczos_scaler_top #(
        .IMG_WIDTH(IMG_WIDTH),
        .IMG_HEIGHT(IMG_HEIGHT),
        .PIXEL_WIDTH(8),
        .COEFF_WIDTH(COEFF_WIDTH),
        .X_COORD_WIDTH(X_COORD_WIDTH),
        .Y_COORD_WIDTH(Y_COORD_WIDTH)
    ) u_scaler_g (
        .sys_clk(sys_clk),
        .sys_rst(sys_rst),
        .s_axis_tdata(s_g),
        .s_axis_tvalid(lane_s_valid),
        .s_axis_tready(g_s_ready),
        .s_axis_tlast(s_axis_tlast),
        .s_axis_tuser(s_axis_tuser),
        .m_axis_tdata(g_m_data),
        .m_axis_tvalid(g_m_valid),
        .m_axis_tready(lane_m_ready),
        .m_axis_tlast(g_m_last),
        .m_axis_tuser(g_m_user),
        .o_serial_x(g_x),
        .o_serial_y(g_y)
    );

    lanczos_scaler_top #(
        .IMG_WIDTH(IMG_WIDTH),
        .IMG_HEIGHT(IMG_HEIGHT),
        .PIXEL_WIDTH(8),
        .COEFF_WIDTH(COEFF_WIDTH),
        .X_COORD_WIDTH(X_COORD_WIDTH),
        .Y_COORD_WIDTH(Y_COORD_WIDTH)
    ) u_scaler_b (
        .sys_clk(sys_clk),
        .sys_rst(sys_rst),
        .s_axis_tdata(s_b),
        .s_axis_tvalid(lane_s_valid),
        .s_axis_tready(b_s_ready),
        .s_axis_tlast(s_axis_tlast),
        .s_axis_tuser(s_axis_tuser),
        .m_axis_tdata(b_m_data),
        .m_axis_tvalid(b_m_valid),
        .m_axis_tready(lane_m_ready),
        .m_axis_tlast(b_m_last),
        .m_axis_tuser(b_m_user),
        .o_serial_x(b_x),
        .o_serial_y(b_y)
    );

    assign m_axis_tvalid = all_m_valid;
    assign m_axis_tdata  = all_m_valid ? {r_m_data, g_m_data, b_m_data} : 24'd0;
    assign m_axis_tlast  = all_m_valid && r_m_last;
    assign m_axis_tuser  = all_m_valid && r_m_user;
    assign o_serial_x    = r_x;
    assign o_serial_y    = r_y;

    // These signals are intentionally retained for waveform/debug inspection.
    // The testbench checks that all three copies remain equal on every cycle.
    wire lanes_input_ready_match = (r_s_ready == g_s_ready) &&
                                   (r_s_ready == b_s_ready);
    wire lanes_output_valid_match = (r_m_valid == g_m_valid) &&
                                    (r_m_valid == b_m_valid);
    wire lanes_output_control_match = (r_m_last == g_m_last) &&
                                      (r_m_last == b_m_last) &&
                                      (r_m_user == g_m_user) &&
                                      (r_m_user == b_m_user) &&
                                      (r_x == g_x) && (r_x == b_x) &&
                                      (r_y == g_y) && (r_y == b_y);
endmodule

`default_nettype wire
