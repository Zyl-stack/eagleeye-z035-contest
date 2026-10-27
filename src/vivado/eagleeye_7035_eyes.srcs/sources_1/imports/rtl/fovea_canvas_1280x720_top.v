`timescale 1ns / 1ps
`default_nettype none

// Builds the competition first-stage UI:
//   left top  640x360: current camera frame
//   right top 640x360: previous-frame dynamic ROI scaled by Bilinear or Lanczos
//   bottom    1280x360: black
module fovea_canvas_1280x720_top #(
    parameter IN_WIDTH   = 640,
    parameter IN_HEIGHT  = 360,
    parameter OUT_WIDTH  = 1280,
    parameter OUT_HEIGHT = 720,
    parameter ROI_X      = 160,
    parameter ROI_Y      = 90,
    parameter ROI_WIDTH  = 320,
    parameter ROI_HEIGHT = 180
)(
    input  wire        sys_clk,
    input  wire        sys_rst,
    input  wire        enable,
    input  wire        use_lanczos,
    input  wire        edge_roi,
    input  wire        paused,
    input  wire [9:0]  roi_x,
    input  wire [8:0]  roi_y,

    input  wire [23:0] s_axis_tdata,
    input  wire        s_axis_tvalid,
    output wire        s_axis_tready,
    input  wire        s_axis_tlast,
    input  wire        s_axis_tuser,

    output wire [23:0] m_axis_tdata,
    output wire        m_axis_tvalid,
    input  wire        m_axis_tready,
    output wire        m_axis_tlast,
    output wire        m_axis_tuser,
    output wire        ready_for_sof
);
    localparam XW = (OUT_WIDTH  <= 2) ? 1 : $clog2(OUT_WIDTH);
    localparam YW = (OUT_HEIGHT <= 2) ? 1 : $clog2(OUT_HEIGHT);
    localparam [XW-1:0] IN_WIDTH_MINUS_1   = IN_WIDTH - 1;
    localparam [XW-1:0] IN_WIDTH_VALUE     = IN_WIDTH;
    localparam [XW-1:0] OUT_WIDTH_MINUS_1  = OUT_WIDTH - 1;
    localparam [YW-1:0] IN_HEIGHT_MINUS_1  = IN_HEIGHT - 1;
    localparam [YW-1:0] OUT_HEIGHT_MINUS_1 = OUT_HEIGHT - 1;
    localparam [XW-1:0] ROI_WIDTH_MINUS_1  = ROI_WIDTH - 1;
    localparam [YW-1:0] ROI_HEIGHT_MINUS_1 = ROI_HEIGHT - 1;
    localparam [XW-1:0] ROI_X_MAX          = IN_WIDTH - ROI_WIDTH;
    localparam [YW-1:0] ROI_Y_MAX          = IN_HEIGHT - ROI_HEIGHT;

    localparam ST_WAIT_SOF = 2'd0;
    localparam ST_LEFT     = 2'd1;
    localparam ST_RIGHT    = 2'd2;
    localparam ST_BOTTOM   = 2'd3;

    reg [1:0]    state;
    assign ready_for_sof = (state == ST_WAIT_SOF);
    reg [XW-1:0] out_x;
    reg [YW-1:0] out_y;

    wire in_left_area = (state == ST_WAIT_SOF) || (state == ST_LEFT);
    wire drop_until_sof = (state == ST_WAIT_SOF) && !(s_axis_tvalid && s_axis_tuser);

    wire [23:0] roi_tdata;
    wire        roi_tvalid;
    wire        roi_tready;
    wire        roi_tlast;
    wire        roi_tuser;

    wire [23:0] bilinear_tdata;
    wire        bilinear_tvalid;
    wire        bilinear_s_ready;
    wire        bilinear_m_ready;
    wire        bilinear_tlast;
    wire        bilinear_tuser;

    wire [23:0] lanczos_tdata;
    wire        lanczos_tvalid;
    wire        lanczos_s_ready;
    wire        lanczos_m_ready;
    wire        lanczos_tlast;
    wire        lanczos_tuser;

    localparam [7:0] EDGE_DISPLAY_THRESHOLD = 8'd80;
    wire [23:0] edge_display_data =
        (bilinear_tdata[23:16] >= EDGE_DISPLAY_THRESHOLD) ? 24'hE8F8FF : 24'h061018;
    wire [23:0] fovea_tdata  = use_lanczos ? lanczos_tdata :
                               edge_roi ? edge_display_data : bilinear_tdata;
    wire        fovea_tvalid = use_lanczos ? lanczos_tvalid : bilinear_tvalid;
    wire        fovea_tlast  = use_lanczos ? lanczos_tlast  : bilinear_tlast;

    wire [XW-1:0] roi_x_safe = ({2'b00, roi_x} > ROI_X_MAX) ? ROI_X_MAX : {2'b00, roi_x};
    wire [YW-1:0] roi_y_safe = ({1'b0, roi_y}  > ROI_Y_MAX) ? ROI_Y_MAX : {1'b0, roi_y};
    wire [XW-1:0] roi_x_end  = roi_x_safe + ROI_WIDTH_MINUS_1;
    wire [YW-1:0] roi_y_end  = roi_y_safe + ROI_HEIGHT_MINUS_1;

    wire in_visible_left = (state == ST_WAIT_SOF) || (state == ST_LEFT);
    wire in_roi_x_range  = (out_x >= roi_x_safe) && (out_x <= roi_x_end);
    wire in_roi_y_range  = (out_y >= roi_y_safe) && (out_y <= roi_y_end);
    wire in_roi = in_visible_left && in_roi_x_range && in_roi_y_range;
    wire [XW-1:0] accent_width_x = paused ? 3 : 2;
    wire [YW-1:0] accent_width_y = paused ? 3 : 2;
    wire draw_roi_shadow = in_roi && ((out_x == roi_x_safe) || (out_x == roi_x_end) ||
                                      (out_y == roi_y_safe) || (out_y == roi_y_end));
    wire draw_roi_accent = in_roi &&
        (((out_x > roi_x_safe) && (out_x <= roi_x_safe + accent_width_x)) ||
         ((out_x < roi_x_end) && (out_x >= roi_x_end - accent_width_x)) ||
         ((out_y > roi_y_safe) && (out_y <= roi_y_safe + accent_width_y)) ||
         ((out_y < roi_y_end) && (out_y >= roi_y_end - accent_width_y)));
    wire [23:0] roi_accent_color = paused ? 24'hFFE04A : 24'hFF3030;

    wire right_selected = (state == ST_RIGHT);
    wire right_ready = right_selected && m_axis_tready;
    wire right_transfer = right_selected && fovea_tvalid && m_axis_tready;

    assign s_axis_tready = enable &&
                           ((drop_until_sof && s_axis_tvalid) ||
                            (in_left_area && !drop_until_sof && m_axis_tready));

    wire src_transfer = in_left_area && !drop_until_sof && s_axis_tvalid && m_axis_tready;
    wire black_transfer = (state == ST_BOTTOM) && m_axis_tready;
    wire frame_start = src_transfer && s_axis_tuser;

    roi_frame_buffer_320x180 #(
        .SRC_WIDTH(IN_WIDTH),
        .SRC_HEIGHT(IN_HEIGHT),
        .ROI_X(ROI_X),
        .ROI_Y(ROI_Y),
        .ROI_WIDTH(ROI_WIDTH),
        .ROI_HEIGHT(ROI_HEIGHT)
    ) u_roi_buffer (
        .clk(sys_clk),
        .rst(sys_rst || !enable),
        .capture_tdata(s_axis_tdata),
        .capture_fire(src_transfer),
        .capture_tlast(s_axis_tlast),
        .capture_tuser(s_axis_tuser),
        .roi_x(roi_x),
        .roi_y(roi_y),
        .start_playback(frame_start),
        .m_axis_tdata(roi_tdata),
        .m_axis_tvalid(roi_tvalid),
        .m_axis_tready(roi_tready),
        .m_axis_tlast(roi_tlast),
        .m_axis_tuser(roi_tuser)
    );

    assign roi_tready = use_lanczos ? lanczos_s_ready : bilinear_s_ready;
    wire [23:0] roi_edge_data;
    axis_sobel_luma #(.WIDTH(ROI_WIDTH), .HEIGHT(ROI_HEIGHT)) u_roi_sobel (
        .clk(sys_clk), .rst(sys_rst || !enable), .pixel(roi_tdata),
        .accept(roi_tvalid && roi_tready), .sof(roi_tuser),
        .eol(roi_tlast), .edge_pixel(roi_edge_data)
    );

    bilinear_scaler_rgb_top #(
        .IMG_WIDTH(ROI_WIDTH),
        .IMG_HEIGHT(ROI_HEIGHT)
    ) u_roi_bilinear (
        .sys_clk(sys_clk),
        .sys_rst(sys_rst || !enable || use_lanczos),
        .s_axis_tdata(edge_roi ? roi_edge_data : roi_tdata),
        .s_axis_tvalid(roi_tvalid && !use_lanczos),
        .s_axis_tready(bilinear_s_ready),
        .s_axis_tlast(roi_tlast),
        .s_axis_tuser(roi_tuser),
        .m_axis_tdata(bilinear_tdata),
        .m_axis_tvalid(bilinear_tvalid),
        .m_axis_tready(bilinear_m_ready),
        .m_axis_tlast(bilinear_tlast),
        .m_axis_tuser(bilinear_tuser),
        .o_serial_x(),
        .o_serial_y()
    );

    lanczos_scaler_rgb_top #(
        .IMG_WIDTH(ROI_WIDTH),
        .IMG_HEIGHT(ROI_HEIGHT)
    ) u_roi_lanczos (
        .sys_clk(sys_clk),
        .sys_rst(sys_rst || !enable || !use_lanczos),
        .s_axis_tdata(roi_tdata),
        .s_axis_tvalid(roi_tvalid && use_lanczos),
        .s_axis_tready(lanczos_s_ready),
        .s_axis_tlast(roi_tlast),
        .s_axis_tuser(roi_tuser),
        .m_axis_tdata(lanczos_tdata),
        .m_axis_tvalid(lanczos_tvalid),
        .m_axis_tready(lanczos_m_ready),
        .m_axis_tlast(lanczos_tlast),
        .m_axis_tuser(lanczos_tuser),
        .o_serial_x(),
        .o_serial_y()
    );

    assign bilinear_m_ready = (!use_lanczos) && right_ready;
    assign lanczos_m_ready  = use_lanczos && right_ready;

    assign m_axis_tvalid = enable &&
                           ((state == ST_WAIT_SOF && s_axis_tvalid && s_axis_tuser) ||
                            (state == ST_LEFT && s_axis_tvalid) ||
                            (state == ST_RIGHT && fovea_tvalid) ||
                            (state == ST_BOTTOM));

    assign m_axis_tdata = draw_roi_shadow ? 24'h081018 :
                          draw_roi_accent ? roi_accent_color :
                          (state == ST_RIGHT) ? fovea_tdata :
                          (state == ST_BOTTOM) ? 24'h000000 :
                          ((state == ST_WAIT_SOF && !s_axis_tuser) ? 24'h000000 : s_axis_tdata);

    assign m_axis_tuser = (state == ST_WAIT_SOF) && s_axis_tvalid && s_axis_tuser;
    assign m_axis_tlast = m_axis_tvalid &&
                          ((state == ST_RIGHT) || (state == ST_BOTTOM)) &&
                          (out_x == OUT_WIDTH_MINUS_1);

    wire line_last_right = (out_x == OUT_WIDTH_MINUS_1);
    wire line_last_black = (out_x == OUT_WIDTH_MINUS_1);
    wire frame_last_row  = (out_y == OUT_HEIGHT_MINUS_1);
    wire last_left_col   = (out_x == IN_WIDTH_MINUS_1);
    wire last_left_row   = (out_y == IN_HEIGHT_MINUS_1);

    always @(posedge sys_clk) begin
        if (sys_rst) begin
            state <= ST_WAIT_SOF;
            out_x <= {XW{1'b0}};
            out_y <= {YW{1'b0}};
        end else if (!enable) begin
            state <= ST_WAIT_SOF;
            out_x <= {XW{1'b0}};
            out_y <= {YW{1'b0}};
        end else begin
            case (state)
                ST_WAIT_SOF: begin
                    out_x <= {XW{1'b0}};
                    out_y <= {YW{1'b0}};
                    if (src_transfer) begin
                        if (IN_WIDTH == 1) begin
                            state <= ST_RIGHT;
                            out_x <= IN_WIDTH_VALUE;
                        end else begin
                            state <= ST_LEFT;
                            out_x <= {{(XW-1){1'b0}}, 1'b1};
                        end
                    end
                end

                ST_LEFT: begin
                    if (src_transfer) begin
                        if (last_left_col) begin
                            state <= ST_RIGHT;
                            out_x <= IN_WIDTH_VALUE;
                        end else begin
                            out_x <= out_x + {{(XW-1){1'b0}}, 1'b1};
                        end
                    end
                end

                ST_RIGHT: begin
                    if (right_transfer) begin
                        if (line_last_right) begin
                            out_x <= {XW{1'b0}};
                            out_y <= out_y + {{(YW-1){1'b0}}, 1'b1};
                            state <= last_left_row ? ST_BOTTOM : ST_LEFT;
                        end else begin
                            out_x <= out_x + {{(XW-1){1'b0}}, 1'b1};
                        end
                    end
                end

                ST_BOTTOM: begin
                    if (black_transfer) begin
                        if (line_last_black) begin
                            out_x <= {XW{1'b0}};
                            if (frame_last_row) begin
                                out_y <= {YW{1'b0}};
                                state <= ST_WAIT_SOF;
                            end else begin
                                out_y <= out_y + {{(YW-1){1'b0}}, 1'b1};
                            end
                        end else begin
                            out_x <= out_x + {{(XW-1){1'b0}}, 1'b1};
                        end
                    end
                end

                default: begin
                    state <= ST_WAIT_SOF;
                    out_x <= {XW{1'b0}};
                    out_y <= {YW{1'b0}};
                end
            endcase
        end
    end

    wire unused_fovea_flags = fovea_tlast || bilinear_tuser || lanczos_tuser;

endmodule

`default_nettype wire
