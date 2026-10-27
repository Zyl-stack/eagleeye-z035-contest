`timescale 1ns / 1ps
`default_nettype none

module video_mode_switch_top #(
    parameter IN_WIDTH        = 640,
    parameter IN_HEIGHT       = 360,
    parameter OUT_WIDTH       = 1280,
    parameter OUT_HEIGHT      = 720,
    parameter KEY_ACTIVE_LOW  = 1,
    parameter DEBOUNCE_BITS   = 20
)(
    input  wire        sys_clk,
    input  wire        sys_rst,
    input  wire        mode_key,

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

    output wire        mode_active
);
    localparam OUT_XW = (OUT_WIDTH  <= 2) ? 1 : $clog2(OUT_WIDTH);
    localparam OUT_YW = (OUT_HEIGHT <= 2) ? 1 : $clog2(OUT_HEIGHT);
    localparam [OUT_XW-1:0] OUT_WIDTH_MINUS_1  = OUT_WIDTH - 1;
    localparam [OUT_YW-1:0] OUT_HEIGHT_MINUS_1 = OUT_HEIGHT - 1;
    localparam [OUT_XW-1:0] OSD_TEXT_X         = 48;
    localparam [OUT_YW-1:0] OSD_TEXT_Y         = 396;
    localparam [OUT_XW-1:0] OSD_TEXT_WIDTH     = 208;
    localparam [OUT_YW-1:0] OSD_TEXT_HEIGHT    = 16;

    localparam [1:0] MODE_DIRECT   = 2'd0;
    localparam [1:0] MODE_BILINEAR = 2'd1;
    localparam [1:0] MODE_LANCZOS  = 2'd2;

    localparam ROI_WIDTH_VALUE  = 320;
    localparam ROI_HEIGHT_VALUE = 180;
    localparam ROI_INIT_X       = 160;
    localparam ROI_INIT_Y       = 90;
    localparam ROI_STEP_X       = 4;
    localparam ROI_STEP_Y       = 2;
    localparam ROI_HOLD_FRAMES  = 2;
    localparam ROI_XW = (IN_WIDTH  <= 2) ? 1 : $clog2(IN_WIDTH);
    localparam ROI_YW = (IN_HEIGHT <= 2) ? 1 : $clog2(IN_HEIGHT);
    localparam [ROI_XW-1:0] ROI_X_MAX        = IN_WIDTH - ROI_WIDTH_VALUE;
    localparam [ROI_YW-1:0] ROI_Y_MAX        = IN_HEIGHT - ROI_HEIGHT_VALUE;
    localparam [ROI_XW-1:0] ROI_INIT_X_VALUE = ROI_INIT_X;
    localparam [ROI_YW-1:0] ROI_INIT_Y_VALUE = ROI_INIT_Y;
    localparam [ROI_XW-1:0] ROI_STEP_X_VALUE = ROI_STEP_X;
    localparam [ROI_YW-1:0] ROI_STEP_Y_VALUE = ROI_STEP_Y;

    wire [1:0] mode_req;

    // Future PS/target-tracker hook. These internal stubs keep the current BD
    // unchanged; replace them with top-level or AXI-Lite driven signals later.
    wire        roi_cmd_valid = 1'b0;
    wire [9:0]  roi_cmd_x     = 10'd0;
    wire [8:0]  roi_cmd_y     = 9'd0;
    wire [1:0]  roi_cmd_mode  = 2'd0;

    key_mode_cycle_debounce #(
        .KEY_ACTIVE_LOW(KEY_ACTIVE_LOW),
        .DEBOUNCE_BITS(DEBOUNCE_BITS)
    ) u_key_mode (
        .clk(sys_clk),
        .rst(sys_rst),
        .key_in(mode_key),
        .mode_state(mode_req)
    );

    reg [1:0] mode_sel;
    reg stream_busy;
    reg output_in_frame;
    reg [OUT_XW-1:0] output_x;
    reg [OUT_YW-1:0] output_y;

    reg [ROI_XW-1:0] roi_x_req;
    reg [ROI_YW-1:0] roi_y_req;
    reg               roi_dir_x;
    reg               roi_dir_y;
    reg [3:0]         roi_hold_cnt;

    reg [ROI_XW-1:0] roi_x_active;
    reg [ROI_YW-1:0] roi_y_active;

    wire [ROI_XW-1:0] roi_cmd_x_sized = roi_cmd_x[ROI_XW-1:0];
    wire [ROI_YW-1:0] roi_cmd_y_sized = roi_cmd_y[ROI_YW-1:0];
    wire [ROI_XW-1:0] roi_cmd_x_safe  = (roi_cmd_x_sized > ROI_X_MAX) ? ROI_X_MAX : roi_cmd_x_sized;
    wire [ROI_YW-1:0] roi_cmd_y_safe  = (roi_cmd_y_sized > ROI_Y_MAX) ? ROI_Y_MAX : roi_cmd_y_sized;
    wire [1:0] cmd_mode_safe = (roi_cmd_mode <= MODE_LANCZOS) ? roi_cmd_mode : MODE_DIRECT;
    wire [1:0] requested_mode = roi_cmd_valid ? cmd_mode_safe : mode_req;
    wire [ROI_XW-1:0] requested_roi_x = roi_cmd_valid ? roi_cmd_x_safe : roi_x_req;
    wire [ROI_YW-1:0] requested_roi_y = roi_cmd_valid ? roi_cmd_y_safe : roi_y_req;

    wire can_start_new_frame = !stream_busy;
    wire [1:0] route_mode = (can_start_new_frame && s_axis_tvalid && s_axis_tuser) ? requested_mode : mode_sel;
    wire input_sof_accept = s_axis_tvalid && s_axis_tuser && s_axis_tready;
    wire output_accept = m_axis_tvalid && m_axis_tready;
    wire output_last_pixel = output_in_frame &&
                             (output_x == OUT_WIDTH_MINUS_1) &&
                             (output_y == OUT_HEIGHT_MINUS_1);

    always @(posedge sys_clk) begin
        if (sys_rst) begin
            mode_sel        <= MODE_DIRECT;
            stream_busy     <= 1'b0;
            output_in_frame <= 1'b0;
            output_x        <= {OUT_XW{1'b0}};
            output_y        <= {OUT_YW{1'b0}};
            roi_x_active    <= ROI_INIT_X_VALUE;
            roi_y_active    <= ROI_INIT_Y_VALUE;
        end else begin
            if (input_sof_accept && can_start_new_frame) begin
                mode_sel     <= requested_mode;
                roi_x_active <= requested_roi_x;
                roi_y_active <= requested_roi_y;
                stream_busy  <= 1'b1;
            end

            if (output_accept && m_axis_tuser) begin
                output_in_frame <= 1'b1;
                output_x        <= {{(OUT_XW-1){1'b0}}, 1'b1};
                output_y        <= {OUT_YW{1'b0}};
            end else if (output_accept && output_in_frame) begin
                if (output_last_pixel) begin
                    stream_busy     <= 1'b0;
                    output_in_frame <= 1'b0;
                    output_x        <= {OUT_XW{1'b0}};
                    output_y        <= {OUT_YW{1'b0}};
                end else if (output_x == OUT_WIDTH_MINUS_1) begin
                    output_x <= {OUT_XW{1'b0}};
                    output_y <= output_y + {{(OUT_YW-1){1'b0}}, 1'b1};
                end else begin
                    output_x <= output_x + {{(OUT_XW-1){1'b0}}, 1'b1};
                end
            end
        end
    end

    wire roi_update_frame = output_accept && output_last_pixel;

    always @(posedge sys_clk) begin
        if (sys_rst) begin
            roi_x_req    <= ROI_INIT_X_VALUE;
            roi_y_req    <= ROI_INIT_Y_VALUE;
            roi_dir_x    <= 1'b0;
            roi_dir_y    <= 1'b0;
            roi_hold_cnt <= 4'd0;
        end else if (roi_update_frame) begin
            if (roi_hold_cnt == (ROI_HOLD_FRAMES - 1)) begin
                roi_hold_cnt <= 4'd0;

                if (!roi_dir_x) begin
                    if (roi_x_req >= (ROI_X_MAX - ROI_STEP_X_VALUE)) begin
                        roi_x_req <= ROI_X_MAX;
                        roi_dir_x <= 1'b1;
                    end else begin
                        roi_x_req <= roi_x_req + ROI_STEP_X_VALUE;
                    end
                end else begin
                    if (roi_x_req <= ROI_STEP_X_VALUE) begin
                        roi_x_req <= {ROI_XW{1'b0}};
                        roi_dir_x <= 1'b0;
                    end else begin
                        roi_x_req <= roi_x_req - ROI_STEP_X_VALUE;
                    end
                end

                if (!roi_dir_y) begin
                    if (roi_y_req >= (ROI_Y_MAX - ROI_STEP_Y_VALUE)) begin
                        roi_y_req <= ROI_Y_MAX;
                        roi_dir_y <= 1'b1;
                    end else begin
                        roi_y_req <= roi_y_req + ROI_STEP_Y_VALUE;
                    end
                end else begin
                    if (roi_y_req <= ROI_STEP_Y_VALUE) begin
                        roi_y_req <= {ROI_YW{1'b0}};
                        roi_dir_y <= 1'b0;
                    end else begin
                        roi_y_req <= roi_y_req - ROI_STEP_Y_VALUE;
                    end
                end
            end else begin
                roi_hold_cnt <= roi_hold_cnt + 4'd1;
            end
        end
    end
    wire [23:0] direct_m_data;
    wire        direct_m_valid;
    wire        direct_m_ready;
    wire        direct_m_last;
    wire        direct_m_user;
    wire        direct_s_ready;

    wire [23:0] fovea_m_data;
    wire        fovea_m_valid;
    wire        fovea_m_ready;
    wire        fovea_m_last;
    wire        fovea_m_user;
    wire        fovea_s_ready;

    wire direct_selected   = (route_mode == MODE_DIRECT);
    wire bilinear_selected = (route_mode == MODE_BILINEAR);
    wire lanczos_selected  = (route_mode == MODE_LANCZOS);
    wire fovea_selected    = bilinear_selected || lanczos_selected;

    assign s_axis_tready = fovea_selected ? fovea_s_ready : direct_s_ready;

    axis_canvas_640x360_to_1280x720 #(
        .IN_WIDTH(IN_WIDTH),
        .IN_HEIGHT(IN_HEIGHT),
        .OUT_WIDTH(OUT_WIDTH),
        .OUT_HEIGHT(OUT_HEIGHT)
    ) u_direct_canvas (
        .clk(sys_clk),
        .rst(sys_rst || !direct_selected),
        .enable(direct_selected),
        .s_axis_tdata(s_axis_tdata),
        .s_axis_tvalid(s_axis_tvalid && direct_selected),
        .s_axis_tready(direct_s_ready),
        .s_axis_tlast(s_axis_tlast),
        .s_axis_tuser(s_axis_tuser),
        .m_axis_tdata(direct_m_data),
        .m_axis_tvalid(direct_m_valid),
        .m_axis_tready(direct_m_ready),
        .m_axis_tlast(direct_m_last),
        .m_axis_tuser(direct_m_user)
    );

    fovea_canvas_1280x720_top #(
        .IN_WIDTH(IN_WIDTH),
        .IN_HEIGHT(IN_HEIGHT),
        .OUT_WIDTH(OUT_WIDTH),
        .OUT_HEIGHT(OUT_HEIGHT),
        .ROI_X(ROI_INIT_X),
        .ROI_Y(ROI_INIT_Y),
        .ROI_WIDTH(ROI_WIDTH_VALUE),
        .ROI_HEIGHT(ROI_HEIGHT_VALUE)
    ) u_fovea_canvas (
        .sys_clk(sys_clk),
        .sys_rst(sys_rst || !fovea_selected),
        .enable(fovea_selected),
        .use_lanczos(lanczos_selected),
        .roi_x(roi_x_active),
        .roi_y(roi_y_active),
        .s_axis_tdata(s_axis_tdata),
        .s_axis_tvalid(s_axis_tvalid && fovea_selected),
        .s_axis_tready(fovea_s_ready),
        .s_axis_tlast(s_axis_tlast),
        .s_axis_tuser(s_axis_tuser),
        .m_axis_tdata(fovea_m_data),
        .m_axis_tvalid(fovea_m_valid),
        .m_axis_tready(fovea_m_ready),
        .m_axis_tlast(fovea_m_last),
        .m_axis_tuser(fovea_m_user)
    );

    assign direct_m_ready   = direct_selected   && m_axis_tready;
    assign fovea_m_ready    = fovea_selected    && m_axis_tready;

    wire [23:0] mux_m_data  = fovea_selected ? fovea_m_data  : direct_m_data;
    wire        mux_m_valid = fovea_selected ? fovea_m_valid : direct_m_valid;
    wire        mux_m_last  = fovea_selected ? fovea_m_last  : direct_m_last;
    wire        mux_m_user  = fovea_selected ? fovea_m_user  : direct_m_user;

    wire osd_area = (output_x >= OSD_TEXT_X) &&
                    (output_x < (OSD_TEXT_X + OSD_TEXT_WIDTH)) &&
                    (output_y >= OSD_TEXT_Y) &&
                    (output_y < (OSD_TEXT_Y + OSD_TEXT_HEIGHT));
    wire [OUT_XW-1:0] osd_dx = output_x - OSD_TEXT_X;
    wire [OUT_YW-1:0] osd_dy = output_y - OSD_TEXT_Y;
    wire [3:0] osd_char_index = osd_dx[7:4];
    wire [2:0] osd_font_col   = osd_dx[3:1];
    wire [2:0] osd_font_row_i = osd_dy[3:1];
    wire [7:0] osd_char_code  = osd_char(route_mode, osd_char_index);
    wire [7:0] osd_font_bits  = osd_font_row(osd_char_code, osd_font_row_i);
    wire       osd_text_pixel = mux_m_valid && osd_area &&
                                osd_font_bits[3'd7 - osd_font_col];
    wire [23:0] osd_text_color = (route_mode == MODE_LANCZOS)  ? 24'hFF0000 :
                                 (route_mode == MODE_BILINEAR) ? 24'hFFFF00 :
                                                                 24'hC0C0C0;

    wire        heatmap_hit;
    wire [23:0] heatmap_pixel;

    attention_heatmap_osd #(
        .IN_WIDTH(IN_WIDTH),
        .IN_HEIGHT(IN_HEIGHT),
        .ROI_WIDTH(ROI_WIDTH_VALUE),
        .ROI_HEIGHT(ROI_HEIGHT_VALUE),
        .MAP_X(352)
    ) u_attention_heatmap_osd (
        .clk(sys_clk),
        .rst(sys_rst),
        .frame_tick(output_accept && mux_m_user),
        .pixel_x(output_x),
        .pixel_y(output_y),
        .roi_x(roi_x_active),
        .roi_y(roi_y_active),
        .mode(route_mode),
        .valid(mux_m_valid),
        .hit(heatmap_hit),
        .pixel(heatmap_pixel)
    );

    assign m_axis_tdata  = osd_text_pixel ? osd_text_color :
                           heatmap_hit    ? heatmap_pixel   :
                                            mux_m_data;
    assign m_axis_tvalid = mux_m_valid;
    assign m_axis_tlast  = mux_m_last;
    assign m_axis_tuser  = mux_m_user;

    assign mode_active = (route_mode != MODE_DIRECT);

    function [7:0] osd_char;
        input [1:0] mode;
        input [3:0] index;
        begin
            case (mode)
                MODE_BILINEAR: begin
                    case (index)
                        4'd0: osd_char = "M";
                        4'd1: osd_char = "O";
                        4'd2: osd_char = "D";
                        4'd3: osd_char = "E";
                        4'd4: osd_char = " ";
                        4'd5: osd_char = "B";
                        4'd6: osd_char = "I";
                        4'd7: osd_char = "L";
                        4'd8: osd_char = "I";
                        4'd9: osd_char = "N";
                        4'd10: osd_char = "E";
                        4'd11: osd_char = "A";
                        4'd12: osd_char = "R";
                        default: osd_char = " ";
                    endcase
                end
                MODE_LANCZOS: begin
                    case (index)
                        4'd0: osd_char = "M";
                        4'd1: osd_char = "O";
                        4'd2: osd_char = "D";
                        4'd3: osd_char = "E";
                        4'd4: osd_char = " ";
                        4'd5: osd_char = "L";
                        4'd6: osd_char = "A";
                        4'd7: osd_char = "N";
                        4'd8: osd_char = "C";
                        4'd9: osd_char = "Z";
                        4'd10: osd_char = "O";
                        4'd11: osd_char = "S";
                        default: osd_char = " ";
                    endcase
                end
                default: begin
                    case (index)
                        4'd0: osd_char = "M";
                        4'd1: osd_char = "O";
                        4'd2: osd_char = "D";
                        4'd3: osd_char = "E";
                        4'd4: osd_char = " ";
                        4'd5: osd_char = "B";
                        4'd6: osd_char = "Y";
                        4'd7: osd_char = "P";
                        4'd8: osd_char = "A";
                        4'd9: osd_char = "S";
                        4'd10: osd_char = "S";
                        default: osd_char = " ";
                    endcase
                end
            endcase
        end
    endfunction

    function [7:0] osd_font_row;
        input [7:0] ch;
        input [2:0] row;
        begin
            case (ch)
                "A": begin
                    case (row)
                        3'd0: osd_font_row = 8'b00111100;
                        3'd1: osd_font_row = 8'b01100110;
                        3'd2: osd_font_row = 8'b01100110;
                        3'd3: osd_font_row = 8'b01111110;
                        3'd4: osd_font_row = 8'b01100110;
                        3'd5: osd_font_row = 8'b01100110;
                        3'd6: osd_font_row = 8'b01100110;
                        default: osd_font_row = 8'b00000000;
                    endcase
                end
                "B": begin
                    case (row)
                        3'd0: osd_font_row = 8'b01111100;
                        3'd1: osd_font_row = 8'b01100110;
                        3'd2: osd_font_row = 8'b01100110;
                        3'd3: osd_font_row = 8'b01111100;
                        3'd4: osd_font_row = 8'b01100110;
                        3'd5: osd_font_row = 8'b01100110;
                        3'd6: osd_font_row = 8'b01111100;
                        default: osd_font_row = 8'b00000000;
                    endcase
                end
                "C": begin
                    case (row)
                        3'd0: osd_font_row = 8'b00111100;
                        3'd1: osd_font_row = 8'b01100110;
                        3'd2: osd_font_row = 8'b01100000;
                        3'd3: osd_font_row = 8'b01100000;
                        3'd4: osd_font_row = 8'b01100000;
                        3'd5: osd_font_row = 8'b01100110;
                        3'd6: osd_font_row = 8'b00111100;
                        default: osd_font_row = 8'b00000000;
                    endcase
                end
                "D": begin
                    case (row)
                        3'd0: osd_font_row = 8'b01111000;
                        3'd1: osd_font_row = 8'b01101100;
                        3'd2: osd_font_row = 8'b01100110;
                        3'd3: osd_font_row = 8'b01100110;
                        3'd4: osd_font_row = 8'b01100110;
                        3'd5: osd_font_row = 8'b01101100;
                        3'd6: osd_font_row = 8'b01111000;
                        default: osd_font_row = 8'b00000000;
                    endcase
                end
                "E": begin
                    case (row)
                        3'd0: osd_font_row = 8'b01111110;
                        3'd1: osd_font_row = 8'b01100000;
                        3'd2: osd_font_row = 8'b01100000;
                        3'd3: osd_font_row = 8'b01111100;
                        3'd4: osd_font_row = 8'b01100000;
                        3'd5: osd_font_row = 8'b01100000;
                        3'd6: osd_font_row = 8'b01111110;
                        default: osd_font_row = 8'b00000000;
                    endcase
                end
                "I": begin
                    case (row)
                        3'd0: osd_font_row = 8'b00111100;
                        3'd1: osd_font_row = 8'b00011000;
                        3'd2: osd_font_row = 8'b00011000;
                        3'd3: osd_font_row = 8'b00011000;
                        3'd4: osd_font_row = 8'b00011000;
                        3'd5: osd_font_row = 8'b00011000;
                        3'd6: osd_font_row = 8'b00111100;
                        default: osd_font_row = 8'b00000000;
                    endcase
                end
                "L": begin
                    case (row)
                        3'd0: osd_font_row = 8'b01100000;
                        3'd1: osd_font_row = 8'b01100000;
                        3'd2: osd_font_row = 8'b01100000;
                        3'd3: osd_font_row = 8'b01100000;
                        3'd4: osd_font_row = 8'b01100000;
                        3'd5: osd_font_row = 8'b01100000;
                        3'd6: osd_font_row = 8'b01111110;
                        default: osd_font_row = 8'b00000000;
                    endcase
                end
                "M": begin
                    case (row)
                        3'd0: osd_font_row = 8'b01100011;
                        3'd1: osd_font_row = 8'b01110111;
                        3'd2: osd_font_row = 8'b01111111;
                        3'd3: osd_font_row = 8'b01101011;
                        3'd4: osd_font_row = 8'b01100011;
                        3'd5: osd_font_row = 8'b01100011;
                        3'd6: osd_font_row = 8'b01100011;
                        default: osd_font_row = 8'b00000000;
                    endcase
                end
                "N": begin
                    case (row)
                        3'd0: osd_font_row = 8'b01100011;
                        3'd1: osd_font_row = 8'b01110011;
                        3'd2: osd_font_row = 8'b01111011;
                        3'd3: osd_font_row = 8'b01101111;
                        3'd4: osd_font_row = 8'b01100111;
                        3'd5: osd_font_row = 8'b01100011;
                        3'd6: osd_font_row = 8'b01100011;
                        default: osd_font_row = 8'b00000000;
                    endcase
                end
                "O": begin
                    case (row)
                        3'd0: osd_font_row = 8'b00111100;
                        3'd1: osd_font_row = 8'b01100110;
                        3'd2: osd_font_row = 8'b01100110;
                        3'd3: osd_font_row = 8'b01100110;
                        3'd4: osd_font_row = 8'b01100110;
                        3'd5: osd_font_row = 8'b01100110;
                        3'd6: osd_font_row = 8'b00111100;
                        default: osd_font_row = 8'b00000000;
                    endcase
                end
                "P": begin
                    case (row)
                        3'd0: osd_font_row = 8'b01111100;
                        3'd1: osd_font_row = 8'b01100110;
                        3'd2: osd_font_row = 8'b01100110;
                        3'd3: osd_font_row = 8'b01111100;
                        3'd4: osd_font_row = 8'b01100000;
                        3'd5: osd_font_row = 8'b01100000;
                        3'd6: osd_font_row = 8'b01100000;
                        default: osd_font_row = 8'b00000000;
                    endcase
                end
                "R": begin
                    case (row)
                        3'd0: osd_font_row = 8'b01111100;
                        3'd1: osd_font_row = 8'b01100110;
                        3'd2: osd_font_row = 8'b01100110;
                        3'd3: osd_font_row = 8'b01111100;
                        3'd4: osd_font_row = 8'b01111000;
                        3'd5: osd_font_row = 8'b01101100;
                        3'd6: osd_font_row = 8'b01100110;
                        default: osd_font_row = 8'b00000000;
                    endcase
                end
                "S": begin
                    case (row)
                        3'd0: osd_font_row = 8'b00111110;
                        3'd1: osd_font_row = 8'b01100000;
                        3'd2: osd_font_row = 8'b01100000;
                        3'd3: osd_font_row = 8'b00111100;
                        3'd4: osd_font_row = 8'b00000110;
                        3'd5: osd_font_row = 8'b00000110;
                        3'd6: osd_font_row = 8'b01111100;
                        default: osd_font_row = 8'b00000000;
                    endcase
                end
                "Y": begin
                    case (row)
                        3'd0: osd_font_row = 8'b01100110;
                        3'd1: osd_font_row = 8'b01100110;
                        3'd2: osd_font_row = 8'b00111100;
                        3'd3: osd_font_row = 8'b00011000;
                        3'd4: osd_font_row = 8'b00011000;
                        3'd5: osd_font_row = 8'b00011000;
                        3'd6: osd_font_row = 8'b00011000;
                        default: osd_font_row = 8'b00000000;
                    endcase
                end
                "Z": begin
                    case (row)
                        3'd0: osd_font_row = 8'b01111110;
                        3'd1: osd_font_row = 8'b00000110;
                        3'd2: osd_font_row = 8'b00001100;
                        3'd3: osd_font_row = 8'b00011000;
                        3'd4: osd_font_row = 8'b00110000;
                        3'd5: osd_font_row = 8'b01100000;
                        3'd6: osd_font_row = 8'b01111110;
                        default: osd_font_row = 8'b00000000;
                    endcase
                end
                default: osd_font_row = 8'b00000000;
            endcase
        end
    endfunction
endmodule

module key_mode_cycle_debounce #(
    parameter KEY_ACTIVE_LOW = 1,
    parameter DEBOUNCE_BITS  = 20
)(
    input  wire clk,
    input  wire rst,
    input  wire key_in,
    output reg [1:0] mode_state
);
    reg key_meta;
    reg key_sync;
    reg stable_pressed;
    reg stable_pressed_d;
    reg [DEBOUNCE_BITS-1:0] debounce_cnt;

    wire pressed_raw = KEY_ACTIVE_LOW ? ~key_sync : key_sync;

    always @(posedge clk) begin
        if (rst) begin
            key_meta         <= 1'b0;
            key_sync         <= 1'b0;
            stable_pressed   <= 1'b0;
            stable_pressed_d <= 1'b0;
            debounce_cnt     <= {DEBOUNCE_BITS{1'b0}};
            mode_state       <= 2'd0;
        end else begin
            key_meta <= key_in;
            key_sync <= key_meta;

            if (pressed_raw == stable_pressed) begin
                debounce_cnt <= {DEBOUNCE_BITS{1'b0}};
            end else begin
                debounce_cnt <= debounce_cnt + {{(DEBOUNCE_BITS-1){1'b0}}, 1'b1};
                if (&debounce_cnt) begin
                    stable_pressed <= pressed_raw;
                    debounce_cnt   <= {DEBOUNCE_BITS{1'b0}};
                end
            end

            stable_pressed_d <= stable_pressed;
            if (stable_pressed && !stable_pressed_d) begin
                mode_state <= (mode_state == 2'd2) ? 2'd0 : (mode_state + 2'd1);
            end
        end
    end
endmodule

module axis_canvas_640x360_to_1280x720 #(
    parameter IN_WIDTH   = 640,
    parameter IN_HEIGHT  = 360,
    parameter OUT_WIDTH  = 1280,
    parameter OUT_HEIGHT = 720
)(
    input  wire        clk,
    input  wire        rst,
    input  wire        enable,

    input  wire [23:0] s_axis_tdata,
    input  wire        s_axis_tvalid,
    output wire        s_axis_tready,
    input  wire        s_axis_tlast,
    input  wire        s_axis_tuser,

    output wire [23:0] m_axis_tdata,
    output wire        m_axis_tvalid,
    input  wire        m_axis_tready,
    output wire        m_axis_tlast,
    output wire        m_axis_tuser
);
    localparam XW = (OUT_WIDTH  <= 2) ? 1 : $clog2(OUT_WIDTH);
    localparam YW = (OUT_HEIGHT <= 2) ? 1 : $clog2(OUT_HEIGHT);
    localparam [XW-1:0] IN_WIDTH_MINUS_1   = IN_WIDTH - 1;
    localparam [XW-1:0] IN_WIDTH_VALUE     = IN_WIDTH;
    localparam [XW-1:0] OUT_WIDTH_MINUS_1  = OUT_WIDTH - 1;
    localparam [YW-1:0] IN_HEIGHT_MINUS_1  = IN_HEIGHT - 1;
    localparam [YW-1:0] OUT_HEIGHT_MINUS_1 = OUT_HEIGHT - 1;

    localparam ST_WAIT_SOF = 2'd0;
    localparam ST_SRC      = 2'd1;
    localparam ST_RIGHT    = 2'd2;
    localparam ST_BOTTOM   = 2'd3;

    reg [1:0]    state;
    reg [XW-1:0] out_x;
    reg [YW-1:0] out_y;

    wire in_source_area = (state == ST_WAIT_SOF) || (state == ST_SRC);
    wire drop_until_sof = (state == ST_WAIT_SOF) && !(s_axis_tvalid && s_axis_tuser);

    assign s_axis_tready = enable &&
                           ((drop_until_sof && s_axis_tvalid) ||
                            (in_source_area && !drop_until_sof && m_axis_tready));

    assign m_axis_tvalid = enable &&
                           ((state == ST_SRC && s_axis_tvalid) ||
                            (state == ST_RIGHT) ||
                            (state == ST_BOTTOM) ||
                            ((state == ST_WAIT_SOF) && s_axis_tvalid && s_axis_tuser));

    assign m_axis_tdata = (state == ST_RIGHT || state == ST_BOTTOM) ? 24'h000000 :
                          ((state == ST_WAIT_SOF && !s_axis_tuser) ? 24'h000000 : s_axis_tdata);

    assign m_axis_tuser = (state == ST_WAIT_SOF) && s_axis_tvalid && s_axis_tuser;
    assign m_axis_tlast = ((state == ST_RIGHT) || (state == ST_BOTTOM)) &&
                          (out_x == OUT_WIDTH_MINUS_1);

    wire src_transfer    = in_source_area && !drop_until_sof && s_axis_tvalid && m_axis_tready;
    wire black_transfer  = ((state == ST_RIGHT) || (state == ST_BOTTOM)) && m_axis_tready;
    wire line_last_black = (out_x == OUT_WIDTH_MINUS_1);
    wire frame_last_row  = (out_y == OUT_HEIGHT_MINUS_1);
    wire last_src_col    = (out_x == IN_WIDTH_MINUS_1);
    wire last_src_row    = (out_y == IN_HEIGHT_MINUS_1);

    always @(posedge clk) begin
        if (rst) begin
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
                            state <= (OUT_WIDTH == 1) ? ST_BOTTOM : ST_RIGHT;
                            out_x <= {{(XW-1){1'b0}}, 1'b1};
                        end else begin
                            state <= ST_SRC;
                            out_x <= {{(XW-1){1'b0}}, 1'b1};
                        end
                    end
                end

                ST_SRC: begin
                    if (src_transfer) begin
                        if (last_src_col) begin
                            state <= ST_RIGHT;
                            out_x <= IN_WIDTH_VALUE;
                        end else begin
                            out_x <= out_x + {{(XW-1){1'b0}}, 1'b1};
                        end
                    end
                end

                ST_RIGHT: begin
                    if (black_transfer) begin
                        if (line_last_black) begin
                            out_x <= {XW{1'b0}};
                            out_y <= out_y + {{(YW-1){1'b0}}, 1'b1};
                            state <= last_src_row ? ST_BOTTOM : ST_SRC;
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

    wire unused_input_flags = s_axis_tlast;
endmodule

`default_nettype wire
