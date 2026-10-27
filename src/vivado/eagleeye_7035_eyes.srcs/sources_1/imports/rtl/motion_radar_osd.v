`timescale 1ns / 1ps
`default_nettype none

module motion_radar_osd #(
    parameter ROI_WIDTH    = 320,
    parameter ROI_HEIGHT   = 180,
    parameter PANEL_X      = 960,
    parameter PANEL_Y      = 408,
    parameter PANEL_W      = 224,
    parameter PANEL_H      = 248,
    parameter CENTER_X     = 112,
    parameter CENTER_Y     = 112,
    parameter MAX_VEC_LEN  = 76,
    parameter DEAD_ZONE    = 10
)(
    input  wire        clk,
    input  wire        rst,
    input  wire        frame_tick,
    input  wire [10:0] pixel_x,
    input  wire [9:0]  pixel_y,
    input  wire [9:0]  roi_x,
    input  wire [8:0]  roi_y,
    input  wire [7:0]  target_id,
    input  wire [1:0]  mode,
    input  wire        valid,

    output wire        hit,
    output wire [23:0] pixel
);
    localparam [1:0] MODE_DIRECT  = 2'd0;
    localparam [1:0] MODE_LANCZOS = 2'd2;

    wire active_mode = (mode != MODE_DIRECT);
    wire in_panel = valid && active_mode &&
                    (pixel_x >= PANEL_X) && (pixel_x < (PANEL_X + PANEL_W)) &&
                    (pixel_y >= PANEL_Y) && (pixel_y < (PANEL_Y + PANEL_H));

    wire [10:0] local_x = pixel_x - PANEL_X;
    wire [9:0]  local_y = pixel_y - PANEL_Y;
    wire [10:0] rel_abs_x = (local_x >= CENTER_X) ? (local_x - CENTER_X) : (CENTER_X - local_x);
    wire [9:0]  rel_abs_y = (local_y >= CENTER_Y) ? (local_y - CENTER_Y) : (CENTER_Y - local_y);
    wire [10:0] rel_abs_y_ext = {1'b0, rel_abs_y};
    wire [10:0] max_abs = (rel_abs_x >= rel_abs_y_ext) ? rel_abs_x : rel_abs_y_ext;

    wire [10:0] roi_center_x = {1'b0, roi_x} + (ROI_WIDTH >> 1);
    wire [9:0]  roi_center_y = {1'b0, roi_y} + (ROI_HEIGHT >> 1);

    reg        history_valid;
    reg [2:0]  history_count;
    reg [7:0]  prev_target_id;
    reg [10:0] center_x_d0;
    reg [10:0] center_x_d1;
    reg [10:0] center_x_d2;
    reg [10:0] center_x_d3;
    reg [9:0]  center_y_d0;
    reg [9:0]  center_y_d1;
    reg [9:0]  center_y_d2;
    reg [9:0]  center_y_d3;
    reg signed [11:0] vx_filt;
    reg signed [11:0] vy_filt;
    reg [10:0] arrow_len;
    reg [10:0] target_len;
    reg [10:0] bar_len;
    reg signed [11:0] raw_vx_reg;
    reg signed [11:0] raw_vy_reg;
    reg [10:0] speed_metric_reg;
    reg [2:0]  dir_code;

    wire target_changed = (target_id != prev_target_id);
    wire history_ready = history_valid && (history_count >= 3'd4);
    wire signed [11:0] delta_x_4f = $signed({1'b0, roi_center_x}) - $signed({1'b0, center_x_d3});
    wire signed [11:0] delta_y_4f = $signed({2'b00, roi_center_y}) - $signed({2'b00, center_y_d3});
    wire [10:0] abs_delta_x = abs12(delta_x_4f);
    wire [10:0] abs_delta_y = abs12(delta_y_4f);
    wire [11:0] delta_sum = {1'b0, abs_delta_x} + {1'b0, abs_delta_y};
    wire in_dead_zone = !history_ready || (delta_sum < DEAD_ZONE);

    wire signed [11:0] raw_vx_next = in_dead_zone ? 12'sd0 : (delta_x_4f >>> 2);
    wire signed [11:0] raw_vy_next = in_dead_zone ? 12'sd0 : (delta_y_4f >>> 2);
    wire signed [13:0] vx_filter_sum = ($signed(vx_filt) <<< 1) + $signed(vx_filt) + $signed(raw_vx_reg);
    wire signed [13:0] vy_filter_sum = ($signed(vy_filt) <<< 1) + $signed(vy_filt) + $signed(raw_vy_reg);
    wire signed [11:0] next_vx_filt = vx_filter_sum[13:2];
    wire signed [11:0] next_vy_filt = vy_filter_sum[13:2];
    wire [10:0] abs_vx = abs12(vx_filt);
    wire [10:0] abs_vy = abs12(vy_filt);
    wire [11:0] speed_sum = {1'b0, abs_vx} + {1'b0, abs_vy};
    wire [10:0] speed_metric_next = (speed_sum > 12'd96) ? 11'd96 : {4'd0, speed_sum[6:0]};
    wire [10:0] target_len_next = speed_to_len(speed_metric_reg);
    wire [10:0] bar_len_next = speed_metric_reg + {1'b0, speed_metric_reg[10:1]};
    wire [2:0] dir_code_next = quantize_dir(vx_filt, vy_filt);

    always @(posedge clk) begin
        if (rst) begin
            history_valid <= 1'b0;
            history_count <= 3'd0;
            prev_target_id <= 8'd0;
            center_x_d0 <= 11'd0;
            center_x_d1 <= 11'd0;
            center_x_d2 <= 11'd0;
            center_x_d3 <= 11'd0;
            center_y_d0 <= 10'd0;
            center_y_d1 <= 10'd0;
            center_y_d2 <= 10'd0;
            center_y_d3 <= 10'd0;
            vx_filt <= 12'sd0;
            vy_filt <= 12'sd0;
            arrow_len <= 11'd0;
            target_len <= 11'd0;
            bar_len <= 11'd0;
            raw_vx_reg <= 12'sd0;
            raw_vy_reg <= 12'sd0;
            speed_metric_reg <= 11'd0;
            dir_code <= 3'd0;
        end else if (frame_tick) begin
            if (!active_mode) begin
                history_valid <= 1'b0;
                history_count <= 3'd0;
                vx_filt <= 12'sd0;
                vy_filt <= 12'sd0;
                arrow_len <= 11'd0;
                target_len <= 11'd0;
                bar_len <= 11'd0;
                raw_vx_reg <= 12'sd0;
                raw_vy_reg <= 12'sd0;
                speed_metric_reg <= 11'd0;
                dir_code <= 3'd0;
            end else if (!history_valid || target_changed) begin
                history_valid <= 1'b1;
                history_count <= 3'd1;
                prev_target_id <= target_id;
                center_x_d0 <= roi_center_x;
                center_x_d1 <= roi_center_x;
                center_x_d2 <= roi_center_x;
                center_x_d3 <= roi_center_x;
                center_y_d0 <= roi_center_y;
                center_y_d1 <= roi_center_y;
                center_y_d2 <= roi_center_y;
                center_y_d3 <= roi_center_y;
                vx_filt <= 12'sd0;
                vy_filt <= 12'sd0;
                arrow_len <= 11'd0;
                target_len <= 11'd0;
                bar_len <= 11'd0;
                raw_vx_reg <= 12'sd0;
                raw_vy_reg <= 12'sd0;
                speed_metric_reg <= 11'd0;
                dir_code <= 3'd0;
            end else begin
                prev_target_id <= target_id;
                center_x_d0 <= roi_center_x;
                center_x_d1 <= center_x_d0;
                center_x_d2 <= center_x_d1;
                center_x_d3 <= center_x_d2;
                center_y_d0 <= roi_center_y;
                center_y_d1 <= center_y_d0;
                center_y_d2 <= center_y_d1;
                center_y_d3 <= center_y_d2;
                if (history_count < 3'd4) begin
                    history_count <= history_count + 3'd1;
                end
                raw_vx_reg <= raw_vx_next;
                raw_vy_reg <= raw_vy_next;
                vx_filt <= next_vx_filt;
                vy_filt <= next_vy_filt;
                speed_metric_reg <= speed_metric_next;
                target_len <= target_len_next;
                bar_len <= bar_len_next;
                dir_code <= dir_code_next;

                if (arrow_len < target_len) begin
                    arrow_len <= (target_len - arrow_len > 11'd3) ? (arrow_len + 11'd3) : target_len;
                end else if (arrow_len > target_len) begin
                    arrow_len <= (arrow_len - target_len > 11'd3) ? (arrow_len - 11'd3) : target_len;
                end
            end
        end
    end

    wire panel_border = in_panel &&
                        ((local_x < 11'd2) || (local_x >= (PANEL_W - 2)) ||
                         (local_y < 10'd2) || (local_y >= (PANEL_H - 2)));
    wire title_mark = (local_y >= 10'd16) && (local_y < 10'd20) &&
                      (local_x >= 11'd50) && (local_x < 11'd174);
    wire title_dot = (local_y >= 10'd14) && (local_y < 10'd22) &&
                     (local_x >= 11'd34) && (local_x < 11'd42);

    wire ring0 = (max_abs >= 11'd26) && (max_abs <= 11'd28);
    wire ring1 = (max_abs >= 11'd52) && (max_abs <= 11'd54);
    wire ring2 = (max_abs >= 11'd78) && (max_abs <= 11'd80);
    wire axis_line = ((rel_abs_x <= 11'd1) && (rel_abs_y <= 10'd82)) ||
                     ((rel_abs_y <= 10'd1) && (rel_abs_x <= 11'd82));
    wire center_dot = (rel_abs_x <= 11'd3) && (rel_abs_y <= 10'd3);

    wire right_ray = (dir_code == 3'd0) && (local_y >= (CENTER_Y - 2)) && (local_y <= (CENTER_Y + 2)) &&
                     (local_x >= CENTER_X) && (local_x <= (CENTER_X + arrow_len));
    wire down_right_ray = (dir_code == 3'd1) &&
                          (local_x >= CENTER_X) && (local_x <= (CENTER_X + arrow_len)) &&
                          (local_y >= CENTER_Y) && (local_y <= (CENTER_Y + arrow_len)) &&
                          (rel_abs_x[10:2] == rel_abs_y[9:1]);
    wire down_ray = (dir_code == 3'd2) && (local_x >= (CENTER_X - 2)) && (local_x <= (CENTER_X + 2)) &&
                    (local_y >= CENTER_Y) && (local_y <= (CENTER_Y + arrow_len[9:0]));
    wire down_left_ray = (dir_code == 3'd3) &&
                         (local_x <= CENTER_X) && (local_x >= (CENTER_X - arrow_len)) &&
                         (local_y >= CENTER_Y) && (local_y <= (CENTER_Y + arrow_len)) &&
                         (rel_abs_x[10:2] == rel_abs_y[9:1]);
    wire left_ray = (dir_code == 3'd4) && (local_y >= (CENTER_Y - 2)) && (local_y <= (CENTER_Y + 2)) &&
                    (local_x <= CENTER_X) && (local_x >= (CENTER_X - arrow_len));
    wire up_left_ray = (dir_code == 3'd5) &&
                       (local_x <= CENTER_X) && (local_x >= (CENTER_X - arrow_len)) &&
                       (local_y <= CENTER_Y) && (local_y >= (CENTER_Y - arrow_len[9:0])) &&
                       (rel_abs_x[10:2] == rel_abs_y[9:1]);
    wire up_ray = (dir_code == 3'd6) && (local_x >= (CENTER_X - 2)) && (local_x <= (CENTER_X + 2)) &&
                  (local_y <= CENTER_Y) && (local_y >= (CENTER_Y - arrow_len[9:0]));
    wire up_right_ray = (dir_code == 3'd7) &&
                        (local_x >= CENTER_X) && (local_x <= (CENTER_X + arrow_len)) &&
                        (local_y <= CENTER_Y) && (local_y >= (CENTER_Y - arrow_len[9:0])) &&
                        (rel_abs_x[10:2] == rel_abs_y[9:1]);
    wire arrow_line = (arrow_len > 11'd5) &&
                      (right_ray || down_right_ray || down_ray || down_left_ray ||
                       left_ray || up_left_ray || up_ray || up_right_ray);

    wire end_dot =
        ((dir_code == 3'd0) && near_point(local_x, local_y, CENTER_X + arrow_len, CENTER_Y)) ||
        ((dir_code == 3'd1) && near_point(local_x, local_y, CENTER_X + arrow_len, CENTER_Y + arrow_len[9:0])) ||
        ((dir_code == 3'd2) && near_point(local_x, local_y, CENTER_X, CENTER_Y + arrow_len[9:0])) ||
        ((dir_code == 3'd3) && near_point(local_x, local_y, CENTER_X - arrow_len, CENTER_Y + arrow_len[9:0])) ||
        ((dir_code == 3'd4) && near_point(local_x, local_y, CENTER_X - arrow_len, CENTER_Y)) ||
        ((dir_code == 3'd5) && near_point(local_x, local_y, CENTER_X - arrow_len, CENTER_Y - arrow_len[9:0])) ||
        ((dir_code == 3'd6) && near_point(local_x, local_y, CENTER_X, CENTER_Y - arrow_len[9:0])) ||
        ((dir_code == 3'd7) && near_point(local_x, local_y, CENTER_X + arrow_len, CENTER_Y - arrow_len[9:0]));

    wire speed_bar_bg = (local_x >= 11'd42) && (local_x < 11'd188) &&
                        (local_y >= 10'd226) && (local_y < 10'd234);
    wire speed_bar_fg = speed_bar_bg && ((local_x - 11'd42) < bar_len);

    wire [23:0] border_color = (mode == MODE_LANCZOS) ? 24'hFF0000 : 24'hFFFF00;
    wire [23:0] arrow_color  = (mode == MODE_LANCZOS) ? 24'hFF3030 : 24'hFFE040;

    assign hit = in_panel;
    assign pixel = panel_border ? border_color :
                   title_dot    ? arrow_color :
                   title_mark   ? 24'h2A8FC0 :
                   speed_bar_fg ? arrow_color :
                   speed_bar_bg ? 24'h143044 :
                   end_dot      ? arrow_color :
                   arrow_line   ? arrow_color :
                   center_dot   ? 24'hFFFFFF :
                   (ring0 || ring1 || ring2) ? 24'h1C7090 :
                   axis_line    ? 24'h0E3048 :
                                  24'h000912;

    function [10:0] abs12;
        input signed [11:0] v;
        begin
            abs12 = v[11] ? (~v[10:0] + 11'd1) : v[10:0];
        end
    endfunction

    function [10:0] speed_to_len;
        input [10:0] speed;
        begin
            if      (speed >= 11'd32) speed_to_len = 11'd76;
            else if (speed >= 11'd24) speed_to_len = 11'd62;
            else if (speed >= 11'd16) speed_to_len = 11'd48;
            else if (speed >= 11'd10) speed_to_len = 11'd34;
            else if (speed >= 11'd5)  speed_to_len = 11'd20;
            else                      speed_to_len = 11'd0;
        end
    endfunction

    function [2:0] quantize_dir;
        input signed [11:0] vx;
        input signed [11:0] vy;
        reg [10:0] ax;
        reg [10:0] ay;
        begin
            ax = abs12(vx);
            ay = abs12(vy);
            if ((ax + ay) < 11'd2) begin
                quantize_dir = 3'd0;
            end else if (ax > (ay << 1)) begin
                quantize_dir = vx[11] ? 3'd4 : 3'd0;
            end else if (ay > (ax << 1)) begin
                quantize_dir = vy[11] ? 3'd6 : 3'd2;
            end else begin
                case ({vy[11], vx[11]})
                    2'b00: quantize_dir = 3'd1;
                    2'b01: quantize_dir = 3'd3;
                    2'b11: quantize_dir = 3'd5;
                    default: quantize_dir = 3'd7;
                endcase
            end
        end
    endfunction

    function near_point;
        input [10:0] px;
        input [9:0]  py;
        input [10:0] tx;
        input [9:0]  ty;
        reg [10:0] dx;
        reg [9:0]  dy;
        begin
            dx = (px >= tx) ? (px - tx) : (tx - px);
            dy = (py >= ty) ? (py - ty) : (ty - py);
            near_point = (dx <= 11'd4) && (dy <= 10'd4);
        end
    endfunction
endmodule

`default_nettype wire
