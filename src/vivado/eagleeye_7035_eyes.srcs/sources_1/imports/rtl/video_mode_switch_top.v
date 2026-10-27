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
    (* X_INTERFACE_INFO = "xilinx.com:signal:clock:1.0 sys_clk CLK" *)
    (* X_INTERFACE_PARAMETER = "ASSOCIATED_BUSIF s_axis:m_axis, ASSOCIATED_RESET sys_rst" *)
    input  wire        sys_clk,
    (* X_INTERFACE_INFO = "xilinx.com:signal:reset:1.0 sys_rst RST" *)
    (* X_INTERFACE_PARAMETER = "POLARITY ACTIVE_HIGH" *)
    input  wire        sys_rst,
    input  wire        mode_key,
    input  wire        pause_key,

    input  wire        roi_cmd_valid,
    input  wire [9:0]  roi_cmd_x,
    input  wire [8:0]  roi_cmd_y,
    input  wire [1:0]  roi_cmd_mode,
    input  wire        roi_cmd_manual,
    output wire        roi_cmd_accept,
    input  wire        osd_cmd_valid,
    input  wire [2303:0] osd_cmd_data,
    output wire        osd_cmd_accept,

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

    output wire [1:0]  active_mode,
    output wire        mode_active,
    output wire        paused_state,
    output wire [9:0]  active_roi_x,
    output wire [8:0]  active_roi_y
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
    localparam [1:0] MODE_EDGES    = 2'd3;

    localparam ROI_WIDTH_VALUE  = 320;
    localparam ROI_HEIGHT_VALUE = 180;
    localparam ROI_INIT_X       = 160;
    localparam ROI_INIT_Y       = 90;
    localparam ROI_XW = (IN_WIDTH  <= 2) ? 1 : $clog2(IN_WIDTH);
    localparam ROI_YW = (IN_HEIGHT <= 2) ? 1 : $clog2(IN_HEIGHT);
    localparam [ROI_XW-1:0] ROI_X_MAX        = IN_WIDTH - ROI_WIDTH_VALUE;
    localparam [ROI_YW-1:0] ROI_Y_MAX        = IN_HEIGHT - ROI_HEIGHT_VALUE;
    localparam [ROI_XW-1:0] ROI_INIT_X_VALUE = ROI_INIT_X;
    localparam [ROI_YW-1:0] ROI_INIT_Y_VALUE = ROI_INIT_Y;

    wire key_press;
    wire [1:0] requested_mode;
    wire [23:0] frame_data;
    wire frame_valid, frame_ready, frame_last, frame_user;
    wire frame_paused;
    axis_frame_pause #(.WIDTH(IN_WIDTH), .HEIGHT(IN_HEIGHT),
                       .DEBOUNCE_BITS(DEBOUNCE_BITS)) u_frame_pause (
        .clk(sys_clk), .rst(sys_rst), .pause_key(pause_key),
        .s_data(s_axis_tdata), .s_valid(s_axis_tvalid),
        .s_ready(s_axis_tready), .s_last(s_axis_tlast), .s_user(s_axis_tuser),
        .m_data(frame_data), .m_valid(frame_valid), .m_ready(frame_ready),
        .m_last(frame_last), .m_user(frame_user), .paused(frame_paused)
    );

    key_press_debounce #(
        .KEY_ACTIVE_LOW(KEY_ACTIVE_LOW),
        .DEBOUNCE_BITS(DEBOUNCE_BITS)
    ) u_key_mode (
        .clk(sys_clk),
        .rst(sys_rst),
        .key_in(mode_key),
        .key_press(key_press)
    );

    roi_mode_policy u_mode_policy (
        .clk(sys_clk),
        .rst(sys_rst),
        .paused(frame_paused),
        .key_press(key_press),
        .requested_mode(requested_mode)
    );

    reg [1:0] mode_sel;
    reg stream_busy;
    reg output_in_frame;
    reg [OUT_XW-1:0] output_x;
    reg [OUT_YW-1:0] output_y;
    reg source_in_frame;
    reg [OUT_XW-1:0] source_x;
    reg [OUT_YW-1:0] source_y;

    reg [ROI_XW-1:0] roi_x_active;
    reg [ROI_YW-1:0] roi_y_active;
    reg [2303:0]      osd_active_data;

    wire [ROI_XW-1:0] roi_cmd_x_sized = roi_cmd_x[ROI_XW-1:0];
    wire [ROI_YW-1:0] roi_cmd_y_sized = roi_cmd_y[ROI_YW-1:0];
    wire [ROI_XW-1:0] roi_cmd_x_safe  = (roi_cmd_x_sized > ROI_X_MAX) ? ROI_X_MAX : roi_cmd_x_sized;
    wire [ROI_YW-1:0] roi_cmd_y_safe  = (roi_cmd_y_sized > ROI_Y_MAX) ? ROI_Y_MAX : roi_cmd_y_sized;
    wire roi_cmd_changes_position = roi_cmd_valid && (!frame_paused || roi_cmd_manual);
    wire [ROI_XW-1:0] requested_roi_x = roi_cmd_changes_position ? roi_cmd_x_safe : roi_x_active;
    wire [ROI_YW-1:0] requested_roi_y = roi_cmd_changes_position ? roi_cmd_y_safe : roi_y_active;

    wire can_start_new_frame = !stream_busy;
    wire fovea_ready_for_sof;
    wire [1:0] route_mode = (can_start_new_frame && fovea_ready_for_sof &&
                             frame_valid && frame_user) ? requested_mode : mode_sel;
    wire input_sof_accept = frame_valid && frame_user && frame_ready;
    wire output_accept = m_axis_tvalid && m_axis_tready;
    wire output_last_pixel = output_in_frame &&
                             (output_x == OUT_WIDTH_MINUS_1) &&
                             (output_y == OUT_HEIGHT_MINUS_1);
    assign roi_cmd_accept = input_sof_accept && can_start_new_frame && roi_cmd_valid;
    assign osd_cmd_accept = input_sof_accept && can_start_new_frame && osd_cmd_valid && !frame_paused;

    always @(posedge sys_clk) begin
        if (sys_rst)
            osd_active_data <= 2304'd0;
        else if (osd_cmd_accept)
            osd_active_data <= osd_cmd_data;
    end

    always @(posedge sys_clk) begin
        if (sys_rst) begin
            mode_sel        <= MODE_LANCZOS;
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
    wire edge_selected     = (route_mode == MODE_EDGES);
    wire fovea_selected    = bilinear_selected || lanczos_selected || edge_selected;

    assign frame_ready = fovea_selected ? fovea_s_ready : direct_s_ready;

    axis_canvas_640x360_to_1280x720 #(
        .IN_WIDTH(IN_WIDTH),
        .IN_HEIGHT(IN_HEIGHT),
        .OUT_WIDTH(OUT_WIDTH),
        .OUT_HEIGHT(OUT_HEIGHT)
    ) u_direct_canvas (
        .clk(sys_clk),
        .rst(sys_rst || !direct_selected),
        .enable(direct_selected),
        .s_axis_tdata(frame_data),
        .s_axis_tvalid(frame_valid && direct_selected),
        .s_axis_tready(direct_s_ready),
        .s_axis_tlast(frame_last),
        .s_axis_tuser(frame_user),
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
        .edge_roi(edge_selected),
        .paused(frame_paused),
        .roi_x(roi_x_active),
        .roi_y(roi_y_active),
        .s_axis_tdata(frame_data),
        .s_axis_tvalid(frame_valid && fovea_selected),
        .s_axis_tready(fovea_s_ready),
        .s_axis_tlast(frame_last),
        .s_axis_tuser(frame_user),
        .m_axis_tdata(fovea_m_data),
        .m_axis_tvalid(fovea_m_valid),
        .m_axis_tready(fovea_m_ready),
        .m_axis_tlast(fovea_m_last),
        .m_axis_tuser(fovea_m_user),
        .ready_for_sof(fovea_ready_for_sof)
    );

    wire output_pipe_s_ready;

    assign direct_m_ready   = direct_selected   && output_pipe_s_ready;
    assign fovea_m_ready    = fovea_selected    && output_pipe_s_ready;

    wire [23:0] mux_m_data  = fovea_selected ? fovea_m_data  : direct_m_data;
    wire        mux_m_valid = fovea_selected ? fovea_m_valid : direct_m_valid;
    wire        mux_m_last  = fovea_selected ? fovea_m_last  : direct_m_last;
    wire        mux_m_user  = fovea_selected ? fovea_m_user  : direct_m_user;
    wire        source_accept = mux_m_valid && output_pipe_s_ready;
    wire        source_last_pixel = source_in_frame &&
                               (source_x == OUT_WIDTH_MINUS_1) &&
                               (source_y == OUT_HEIGHT_MINUS_1);

    // The OSD is evaluated before the output register slice. Track that
    // incoming stream independently from the FIFO-facing output stream.
    always @(posedge sys_clk) begin
        if (sys_rst) begin
            source_in_frame <= 1'b0;
            source_x        <= {OUT_XW{1'b0}};
            source_y        <= {OUT_YW{1'b0}};
        end else if (source_accept && mux_m_user) begin
            source_in_frame <= 1'b1;
            source_x        <= {{(OUT_XW-1){1'b0}}, 1'b1};
            source_y        <= {OUT_YW{1'b0}};
        end else if (source_accept && source_in_frame) begin
            if (source_last_pixel) begin
                source_in_frame <= 1'b0;
                source_x        <= {OUT_XW{1'b0}};
                source_y        <= {OUT_YW{1'b0}};
            end else if (source_x == OUT_WIDTH_MINUS_1) begin
                source_x <= {OUT_XW{1'b0}};
                source_y <= source_y + {{(OUT_YW-1){1'b0}}, 1'b1};
            end else begin
                source_x <= source_x + {{(OUT_XW-1){1'b0}}, 1'b1};
            end
        end
    end

    wire attention_osd_hit;
    wire [23:0] attention_osd_pixel;
    attention_targets_osd u_attention_targets_osd (
        .pixel_x(source_x), .pixel_y(source_y), .valid(mux_m_valid),
        .mode(route_mode), .paused(frame_paused), .metadata(osd_active_data),
        .hit(attention_osd_hit), .pixel(attention_osd_pixel)
    );

    // Keep the heat-map in front of the bottom HUD.  This is intentionally
    // evaluated in the existing pre-register OSD stage so the output stream
    // still crosses only the original register slice.
    wire        heatmap_hit;
    wire [23:0] heatmap_pixel;
    attention_heatmap_osd #(
        .IN_WIDTH(IN_WIDTH),
        .IN_HEIGHT(IN_HEIGHT),
        .ROI_WIDTH(ROI_WIDTH_VALUE),
        .ROI_HEIGHT(ROI_HEIGHT_VALUE)
    ) u_attention_heatmap_osd (
        .clk(sys_clk),
        .rst(sys_rst),
        .frame_tick(source_accept && mux_m_user),
        .pixel_x(source_x),
        .pixel_y(source_y),
        .roi_x(roi_x_active),
        .roi_y(roi_y_active),
        .mode(route_mode),
        .valid(mux_m_valid),
        .hit(heatmap_hit),
        .pixel(heatmap_pixel)
    );

    axis_rgb24_reg_slice u_output_reg_slice (
        .clk(sys_clk),
        .rst(sys_rst),
        .s_axis_tdata(heatmap_hit ? heatmap_pixel :
                      (attention_osd_hit ? attention_osd_pixel : mux_m_data)),
        .s_axis_tvalid(mux_m_valid),
        .s_axis_tready(output_pipe_s_ready),
        .s_axis_tlast(mux_m_last),
        .s_axis_tuser(mux_m_user),
        .m_axis_tdata(m_axis_tdata),
        .m_axis_tvalid(m_axis_tvalid),
        .m_axis_tready(m_axis_tready),
        .m_axis_tlast(m_axis_tlast),
        .m_axis_tuser(m_axis_tuser)
    );

    assign active_mode = mode_sel;
    assign mode_active = (route_mode != MODE_DIRECT);
    assign paused_state = frame_paused;
    assign active_roi_x = roi_x_active[9:0];
    assign active_roi_y = roi_y_active[8:0];

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

// One-deep elastic AXI4-Stream register slice.  All video sideband signals
// advance with the pixel, and the source is stalled only when this register is
// full and the downstream FIFO is not ready.
module axis_rgb24_reg_slice (
    input  wire        clk,
    input  wire        rst,

    input  wire [23:0] s_axis_tdata,
    input  wire        s_axis_tvalid,
    output wire        s_axis_tready,
    input  wire        s_axis_tlast,
    input  wire        s_axis_tuser,

    output reg  [23:0] m_axis_tdata,
    output reg         m_axis_tvalid,
    input  wire        m_axis_tready,
    output reg         m_axis_tlast,
    output reg         m_axis_tuser
);
    assign s_axis_tready = !m_axis_tvalid || m_axis_tready;

    always @(posedge clk) begin
        if (rst) begin
            m_axis_tdata  <= 24'd0;
            m_axis_tvalid <= 1'b0;
            m_axis_tlast  <= 1'b0;
            m_axis_tuser  <= 1'b0;
        end else if (s_axis_tready) begin
            m_axis_tvalid <= s_axis_tvalid;
            if (s_axis_tvalid) begin
                m_axis_tdata <= s_axis_tdata;
                m_axis_tlast <= s_axis_tlast;
                m_axis_tuser <= s_axis_tuser;
            end else begin
                m_axis_tlast <= 1'b0;
                m_axis_tuser <= 1'b0;
            end
        end
    end
endmodule

module key_press_debounce #(
    parameter KEY_ACTIVE_LOW = 1,
    parameter DEBOUNCE_BITS  = 20
)(
    input  wire clk,
    input  wire rst,
    input  wire key_in,
    output wire key_press
);
    reg key_meta;
    reg key_sync;
    reg stable_pressed;
    reg stable_pressed_d;
    reg [DEBOUNCE_BITS-1:0] debounce_cnt;

    wire pressed_raw = KEY_ACTIVE_LOW ? ~key_sync : key_sync;
    assign key_press = stable_pressed && !stable_pressed_d;

    always @(posedge clk) begin
        if (rst) begin
            key_meta         <= 1'b0;
            key_sync         <= 1'b0;
            stable_pressed   <= 1'b0;
            stable_pressed_d <= 1'b0;
            debounce_cnt     <= {DEBOUNCE_BITS{1'b0}};
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
        end
    end
endmodule

module roi_mode_policy (
    input  wire       clk,
    input  wire       rst,
    input  wire       paused,
    input  wire       key_press,
    output wire [1:0] requested_mode
);
    localparam [1:0] MODE_BILINEAR = 2'd1;
    localparam [1:0] MODE_LANCZOS  = 2'd2;
    localparam [1:0] MODE_EDGES    = 2'd3;

    reg live_lanczos;
    reg paused_d;
    reg [1:0] pause_mode;
    wire [1:0] live_mode = live_lanczos ? MODE_LANCZOS : MODE_BILINEAR;

    assign requested_mode = paused ? (paused_d ? pause_mode : live_mode) : live_mode;

    always @(posedge clk) begin
        if (rst) begin
            live_lanczos <= 1'b1;
            paused_d <= 1'b0;
            pause_mode <= MODE_LANCZOS;
        end else begin
            paused_d <= paused;
            if (paused && !paused_d) begin
                pause_mode <= live_mode;
            end else if (paused && key_press) begin
                if (live_lanczos)
                    pause_mode <= (pause_mode == MODE_LANCZOS) ? MODE_EDGES : MODE_LANCZOS;
                else begin
                    case (pause_mode)
                        MODE_BILINEAR: pause_mode <= MODE_LANCZOS;
                        MODE_LANCZOS:  pause_mode <= MODE_EDGES;
                        default:       pause_mode <= MODE_BILINEAR;
                    endcase
                end
            end else if (!paused && !paused_d && key_press) begin
                live_lanczos <= !live_lanczos;
            end
        end
    end
endmodule

module axis_sobel_luma #(
    parameter WIDTH = 640,
    parameter HEIGHT = 360
)(
    input wire clk,
    input wire rst,
    input wire [23:0] pixel,
    input wire accept,
    input wire sof,
    input wire eol,
    output wire [23:0] edge_pixel
);
    localparam XW = $clog2(WIDTH);
    localparam YW = $clog2(HEIGHT);
    (* ram_style = "distributed" *) reg [7:0] previous_row [0:WIDTH-1];
    (* ram_style = "distributed" *) reg [7:0] older_row [0:WIDTH-1];
    reg [XW-1:0] x;
    reg [YW-1:0] y;
    reg [7:0] top_left, top_mid, mid_left, mid_mid, bot_left, bot_mid;
    wire [XW-1:0] at_x = sof ? {XW{1'b0}} : x;
    wire [YW-1:0] at_y = sof ? {YW{1'b0}} : y;
    wire [7:0] top_right = older_row[at_x];
    wire [7:0] mid_right = previous_row[at_x];
    wire [7:0] bot_right = (pixel[23:16] >> 2) +
                           (pixel[15:8] >> 1) + (pixel[7:0] >> 2);
    wire signed [11:0] gx = $signed({4'b0,top_right}) +
                            ($signed({4'b0,mid_right}) <<< 1) +
                            $signed({4'b0,bot_right}) -
                            $signed({4'b0,top_left}) -
                            ($signed({4'b0,mid_left}) <<< 1) -
                            $signed({4'b0,bot_left});
    wire signed [11:0] gy = $signed({4'b0,bot_left}) +
                            ($signed({4'b0,bot_mid}) <<< 1) +
                            $signed({4'b0,bot_right}) -
                            $signed({4'b0,top_left}) -
                            ($signed({4'b0,top_mid}) <<< 1) -
                            $signed({4'b0,top_right});
    wire [11:0] abs_x = gx[11] ? -gx : gx;
    wire [11:0] abs_y = gy[11] ? -gy : gy;
    wire [12:0] magnitude = {1'b0,abs_x} + {1'b0,abs_y};
    wire [7:0] edge_value = (at_x < 2 || at_y < 2) ? 8'd0 :
                      (magnitude > 255 ? 8'hff : magnitude[7:0]);
    assign edge_pixel = {edge_value,edge_value,edge_value};

    always @(posedge clk) begin
        if (rst) begin
            x <= 0; y <= 0;
            top_left <= 0; top_mid <= 0;
            mid_left <= 0; mid_mid <= 0;
            bot_left <= 0; bot_mid <= 0;
        end else if (accept) begin
            older_row[at_x] <= mid_right;
            previous_row[at_x] <= bot_right;
            top_left <= (at_x == 0) ? 8'd0 : top_mid;
            top_mid <= top_right;
            mid_left <= (at_x == 0) ? 8'd0 : mid_mid;
            mid_mid <= mid_right;
            bot_left <= (at_x == 0) ? 8'd0 : bot_mid;
            bot_mid <= bot_right;
            if (eol) begin
                x <= 0;
                y <= (at_y == HEIGHT-1) ? 0 : at_y + 1'b1;
            end else begin
                x <= at_x + 1'b1;
                y <= at_y;
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
