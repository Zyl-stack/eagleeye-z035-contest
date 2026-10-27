`timescale 1ns / 1ps
`default_nettype none

module attention_heatmap_osd #(
    parameter IN_WIDTH   = 640,
    parameter IN_HEIGHT  = 360,
    parameter ROI_WIDTH  = 320,
    parameter ROI_HEIGHT = 180,
    parameter MAP_X      = 704,
    parameter MAP_Y      = 392,
    parameter GRID_COLS  = 32,
    parameter GRID_ROWS  = 18,
    parameter CELL_PITCH = 16,
    parameter CELL_SIZE  = 15
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
    localparam MAP_W = GRID_COLS * CELL_PITCH;
    localparam MAP_H = GRID_ROWS * CELL_PITCH;

    wire in_map_area = valid && (mode != 2'd0) &&
                       (pixel_x >= MAP_X) && (pixel_x < MAP_X + MAP_W) &&
                       (pixel_y >= MAP_Y) && (pixel_y < MAP_Y + MAP_H);
    wire [10:0] map_dx = pixel_x - MAP_X;
    wire [9:0]  map_dy = pixel_y - MAP_Y;
    wire [4:0] cell_x = map_dx[8:4];
    wire [4:0] cell_y = map_dy[8:4];
    wire inside_cell = (map_dx[3:0] < CELL_SIZE[3:0]) &&
                       (map_dy[3:0] < CELL_SIZE[3:0]);
    wire map_border = (map_dx == 11'd0) || (map_dx == MAP_W - 1) ||
                      (map_dy == 10'd0) || (map_dy == MAP_H - 1);

    wire [10:0] roi_center_x = {1'b0, roi_x} + (ROI_WIDTH >> 1);
    wire [9:0]  roi_center_y = {1'b0, roi_y} + (ROI_HEIGHT >> 1);
    wire [5:0] hot_x_shift = roi_center_x[9:4];
    wire [5:0] hot_y_shift = {1'b0, roi_center_y[8:4]};
    wire [4:0] hot_x = (hot_x_shift > 6'd31) ? 5'd31 : hot_x_shift[4:0];
    wire [4:0] hot_y = (hot_y_shift > 6'd17) ? 5'd17 : hot_y_shift[4:0];

    wire [4:0] dx = (cell_x >= hot_x) ? cell_x - hot_x : hot_x - cell_x;
    wire [4:0] dy = (cell_y >= hot_y) ? cell_y - hot_y : hot_y - cell_y;
    wire [5:0] distance = {1'b0, dx} + {1'b0, dy};
    reg [4:0] scan_col;
    reg [2:0] scan_div;

    always @(posedge clk) begin
        if (rst) begin
            scan_col <= 5'd0;
            scan_div <= 3'd0;
        end else if (frame_tick) begin
            if (scan_div == 3'd3) begin
                scan_div <= 3'd0;
                scan_col <= (scan_col == GRID_COLS - 1) ? 5'd0 : scan_col + 5'd1;
            end else begin
                scan_div <= scan_div + 3'd1;
            end
        end
    end

    wire scan_center = (cell_x == scan_col);
    wire scan_soft = ((scan_col != 5'd0) && (cell_x == scan_col - 5'd1)) ||
                     ((scan_col != GRID_COLS - 1) && (cell_x == scan_col + 5'd1));
    reg [23:0] heat_color;
    always @* begin
        case (distance)
            6'd0: heat_color = 24'hFF2020;
            6'd1: heat_color = 24'hFF5020;
            6'd2: heat_color = 24'hFF9030;
            6'd3: heat_color = 24'hFFD040;
            6'd4: heat_color = 24'hB8E850;
            6'd5: heat_color = 24'h58C878;
            6'd6: heat_color = 24'h20A8A0;
            6'd7: heat_color = 24'h187090;
            6'd8: heat_color = 24'h104060;
            default: heat_color = 24'h081C28;
        endcase
    end

    reg [23:0] cell_color;
    always @* begin
        if (scan_center) begin
            case (distance)
                6'd0: cell_color = 24'hFF3030;
                6'd1: cell_color = 24'hFF6030;
                6'd2: cell_color = 24'hFFA040;
                6'd3: cell_color = 24'hFFDC58;
                6'd4: cell_color = 24'hC8F068;
                6'd5: cell_color = 24'h70D890;
                6'd6: cell_color = 24'h38B8B0;
                6'd7: cell_color = 24'h2888A8;
                6'd8: cell_color = 24'h205878;
                default: cell_color = 24'h164056;
            endcase
        end else if (scan_soft && (distance > 6'd8)) begin
            cell_color = 24'h0C2A38;
        end else begin
            cell_color = heat_color;
        end
    end

    assign hit = in_map_area;
    assign pixel = map_border ? 24'hFF3030 :
                   inside_cell ? cell_color : 24'h02080C;
endmodule

`default_nettype wire
