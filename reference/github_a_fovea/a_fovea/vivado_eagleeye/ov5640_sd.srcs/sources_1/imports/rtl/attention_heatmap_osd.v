`timescale 1ns / 1ps
`default_nettype none

module attention_heatmap_osd #(
    parameter IN_WIDTH        = 640,
    parameter IN_HEIGHT       = 360,
    parameter ROI_WIDTH       = 320,
    parameter ROI_HEIGHT      = 180,
    parameter MAP_X           = 704,
    parameter MAP_Y           = 392,
    parameter GRID_COLS       = 32,
    parameter GRID_ROWS       = 18,
    parameter CELL_PITCH      = 16,
    parameter CELL_SIZE       = 15
)(
    input  wire        clk,
    input  wire        rst,
    input  wire        frame_tick,
    input  wire [10:0] pixel_x,
    input  wire [9:0]  pixel_y,
    input  wire [9:0]  roi_x,
    input  wire [8:0]  roi_y,
    input  wire [1:0]  mode,
    input  wire        valid,

    output wire        hit,
    output wire [23:0] pixel
);
    localparam [1:0] MODE_DIRECT  = 2'd0;
    localparam [1:0] MODE_LANCZOS = 2'd2;
    localparam MAP_W = GRID_COLS * CELL_PITCH;
    localparam MAP_H = GRID_ROWS * CELL_PITCH;
    localparam [4:0] INIT_HOT_X = 5'd20;
    localparam [4:0] INIT_HOT_Y = 5'd11;

    wire active_mode = (mode != MODE_DIRECT);
    wire in_map_area = valid && active_mode &&
                       (pixel_x >= MAP_X) && (pixel_x < (MAP_X + MAP_W)) &&
                       (pixel_y >= MAP_Y) && (pixel_y < (MAP_Y + MAP_H));

    wire [10:0] map_dx = pixel_x - MAP_X;
    wire [9:0]  map_dy = pixel_y - MAP_Y;
    wire [4:0]  cell_x = map_dx[8:4];
    wire [4:0]  cell_y = map_dy[8:4];
    wire        inside_cell = (map_dx[3:0] < CELL_SIZE[3:0]) &&
                              (map_dy[3:0] < CELL_SIZE[3:0]);

    wire map_border = in_map_area &&
                      ((map_dx < 11'd2) || (map_dx >= (MAP_W - 2)) ||
                       (map_dy < 10'd2) || (map_dy >= (MAP_H - 2)));

    wire [10:0] roi_center_x = {1'b0, roi_x} + (ROI_WIDTH >> 1);
    wire [9:0]  roi_center_y = {1'b0, roi_y} + (ROI_HEIGHT >> 1);
    wire [5:0]  hot_x_shift = roi_center_x[9:4];
    wire [5:0]  hot_y_shift = {1'b0, roi_center_y[8:4]};
    wire [4:0]  hot_x = (hot_x_shift > 6'd31) ? 5'd31 : hot_x_shift[4:0];
    wire [4:0]  hot_y = (hot_y_shift > 6'd17) ? 5'd17 : hot_y_shift[4:0];

    reg [4:0] last_hot_x;
    reg [4:0] last_hot_y;
    reg [4:0] trail0_x;
    reg [4:0] trail0_y;
    reg [4:0] trail1_x;
    reg [4:0] trail1_y;
    reg [4:0] trail2_x;
    reg [4:0] trail2_y;
    reg [4:0] trail3_x;
    reg [4:0] trail3_y;
    reg [4:0] trail4_x;
    reg [4:0] trail4_y;
    reg [4:0] trail5_x;
    reg [4:0] trail5_y;
    reg [4:0] scan_col;
    reg [1:0] scan_div;

    wire hot_changed = (hot_x != last_hot_x) || (hot_y != last_hot_y);

    always @(posedge clk) begin
        if (rst) begin
            last_hot_x <= INIT_HOT_X;
            last_hot_y <= INIT_HOT_Y;
            trail0_x   <= INIT_HOT_X;
            trail0_y   <= INIT_HOT_Y;
            trail1_x   <= INIT_HOT_X;
            trail1_y   <= INIT_HOT_Y;
            trail2_x   <= INIT_HOT_X;
            trail2_y   <= INIT_HOT_Y;
            trail3_x   <= INIT_HOT_X;
            trail3_y   <= INIT_HOT_Y;
            trail4_x   <= INIT_HOT_X;
            trail4_y   <= INIT_HOT_Y;
            trail5_x   <= INIT_HOT_X;
            trail5_y   <= INIT_HOT_Y;
            scan_col   <= 5'd0;
            scan_div   <= 2'd0;
        end else if (frame_tick) begin
            scan_div <= scan_div + 2'd1;
            if (scan_div == 2'd0) begin
                scan_col <= (scan_col == 5'd31) ? 5'd0 : (scan_col + 5'd1);
            end

            if (active_mode && hot_changed) begin
                last_hot_x <= hot_x;
                last_hot_y <= hot_y;
                trail0_x   <= last_hot_x;
                trail0_y   <= last_hot_y;
                trail1_x   <= trail0_x;
                trail1_y   <= trail0_y;
                trail2_x   <= trail1_x;
                trail2_y   <= trail1_y;
                trail3_x   <= trail2_x;
                trail3_y   <= trail2_y;
                trail4_x   <= trail3_x;
                trail4_y   <= trail3_y;
                trail5_x   <= trail4_x;
                trail5_y   <= trail4_y;
            end
        end
    end

    wire [5:0] current_distance = manhattan(cell_x, cell_y, hot_x, hot_y);
    wire [5:0] trail0_distance  = manhattan(cell_x, cell_y, trail0_x, trail0_y);
    wire [5:0] trail1_distance  = manhattan(cell_x, cell_y, trail1_x, trail1_y);
    wire [5:0] trail2_distance  = manhattan(cell_x, cell_y, trail2_x, trail2_y);
    wire [5:0] trail3_distance  = manhattan(cell_x, cell_y, trail3_x, trail3_y);
    wire [5:0] trail4_distance  = manhattan(cell_x, cell_y, trail4_x, trail4_y);
    wire [5:0] trail5_distance  = manhattan(cell_x, cell_y, trail5_x, trail5_y);

    wire [23:0] border_color = (mode == MODE_LANCZOS) ? 24'hFF0000 : 24'hFFFF00;
    wire [23:0] grid_gap_color = 24'h01050A;
    wire [23:0] bg_color = 24'h001020;
    wire [23:0] current_color = current_heat_color(current_distance);
    wire [23:0] trail_color = trail_heat_color(
        trail0_distance,
        trail1_distance,
        trail2_distance,
        trail3_distance,
        trail4_distance,
        trail5_distance
    );

    wire current_core = (current_distance <= 6'd2);
    wire current_halo = (current_distance <= 6'd8);
    wire trail_hit = (trail0_distance <= 6'd2) ||
                     (trail1_distance <= 6'd2) ||
                     (trail2_distance <= 6'd2) ||
                     (trail3_distance <= 6'd1) ||
                     (trail4_distance <= 6'd1) ||
                     (trail5_distance <= 6'd1);
    wire scan_hit = (cell_x == scan_col) || (cell_x == (scan_col + 5'd1));
    wire [23:0] scan_color = (mode == MODE_LANCZOS) ? 24'h402028 : 24'h303820;

    wire [23:0] cell_color = current_core ? current_color :
                              scan_hit     ? scan_color    :
                              current_halo ? current_color :
                              trail_hit    ? trail_color   :
                                             bg_color;

    assign hit = in_map_area;
    assign pixel = map_border ? border_color :
                   inside_cell ? cell_color : grid_gap_color;

    function [5:0] manhattan;
        input [4:0] ax;
        input [4:0] ay;
        input [4:0] bx;
        input [4:0] by;
        reg [4:0] dx;
        reg [4:0] dy;
        begin
            dx = (ax >= bx) ? (ax - bx) : (bx - ax);
            dy = (ay >= by) ? (ay - by) : (by - ay);
            manhattan = {1'b0, dx} + {1'b0, dy};
        end
    endfunction

    function [23:0] current_heat_color;
        input [5:0] distance;
        begin
            case (distance)
                6'd0: current_heat_color = 24'hFF1010;
                6'd1: current_heat_color = 24'hFF4000;
                6'd2: current_heat_color = 24'hFF7800;
                6'd3: current_heat_color = 24'hFFD000;
                6'd4: current_heat_color = 24'hA8F000;
                6'd5: current_heat_color = 24'h40D840;
                6'd6: current_heat_color = 24'h00A890;
                6'd7: current_heat_color = 24'h0068B0;
                6'd8: current_heat_color = 24'h003060;
                default: current_heat_color = 24'h001020;
            endcase
        end
    endfunction

    function [23:0] trail_heat_color;
        input [5:0] d0;
        input [5:0] d1;
        input [5:0] d2;
        input [5:0] d3;
        input [5:0] d4;
        input [5:0] d5;
        begin
            if      (d0 <= 6'd1) trail_heat_color = 24'hC05010;
            else if (d1 <= 6'd1) trail_heat_color = 24'h984018;
            else if (d2 <= 6'd1) trail_heat_color = 24'h703820;
            else if (d3 <= 6'd1) trail_heat_color = 24'h404820;
            else if (d4 <= 6'd1) trail_heat_color = 24'h205040;
            else if (d5 <= 6'd1) trail_heat_color = 24'h184060;
            else if (d0 <= 6'd2) trail_heat_color = 24'h804020;
            else if (d1 <= 6'd2) trail_heat_color = 24'h603828;
            else if (d2 <= 6'd2) trail_heat_color = 24'h403830;
            else                  trail_heat_color = 24'h001020;
        end
    endfunction
endmodule

`default_nettype wire
