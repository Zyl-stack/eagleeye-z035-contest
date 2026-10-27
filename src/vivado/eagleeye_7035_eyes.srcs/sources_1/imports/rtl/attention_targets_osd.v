`timescale 1ns / 1ps
`default_nettype none

module attention_targets_osd (
    input  wire [10:0]   pixel_x,
    input  wire [9:0]    pixel_y,
    input  wire          valid,
    input  wire [1:0]    mode,
    input  wire          paused,
    input  wire [2303:0] metadata,
    output wire          hit,
    output wire [23:0]   pixel
);
    wire [23:0] status_pixel;
    attention_status_hud u_status_hud (
        .pixel_x(pixel_x), .pixel_y(pixel_y), .mode(mode), .paused(paused),
        .metadata(metadata), .pixel(status_pixel)
    );
    assign hit = valid && (pixel_y >= 10'd360);
    assign pixel = status_pixel;
endmodule

module attention_status_hud (
    input  wire [10:0]   pixel_x,
    input  wire [9:0]    pixel_y,
    input  wire [1:0]    mode,
    input  wire          paused,
    input  wire [2303:0] metadata,
    output wire [23:0]   pixel
);
    wire title_lit;
    wire subtitle_lit;
    wire lanczos_lit;
    wire bilinear_lit;
    wire direct_lit;
    wire edges_lit;
    wire auto_lit;
    wire manual_lit;

    osd_text5x7 #(.X(108), .Y(476), .SCALE_SHIFT(3), .STEP(48),
                  .CHARS(9), .TEXT("Eagle Eye")) u_title (
        .pixel_x(pixel_x), .pixel_y(pixel_y), .lit(title_lit)
    );
    osd_text5x7 #(.X(24), .Y(552), .SCALE_SHIFT(2), .STEP(22),
                  .CHARS(27), .TEXT("FPGA Video Broadcast System")) u_subtitle (
        .pixel_x(pixel_x), .pixel_y(pixel_y), .lit(subtitle_lit)
    );
    osd_text5x7 #(.X(166), .Y(604), .SCALE_SHIFT(2), .STEP(24),
                  .CHARS(13), .TEXT("Mode: Lanczos")) u_lanczos (
        .pixel_x(pixel_x), .pixel_y(pixel_y), .lit(lanczos_lit)
    );
    osd_text5x7 #(.X(154), .Y(604), .SCALE_SHIFT(2), .STEP(24),
                  .CHARS(14), .TEXT("Mode: Bilinear")) u_bilinear (
        .pixel_x(pixel_x), .pixel_y(pixel_y), .lit(bilinear_lit)
    );
    osd_text5x7 #(.X(178), .Y(604), .SCALE_SHIFT(2), .STEP(24),
                  .CHARS(12), .TEXT("Mode: Direct")) u_direct (
        .pixel_x(pixel_x), .pixel_y(pixel_y), .lit(direct_lit)
    );
    osd_text5x7 #(.X(190), .Y(604), .SCALE_SHIFT(2), .STEP(24),
                  .CHARS(11), .TEXT("Mode: Edges")) u_edges (
        .pixel_x(pixel_x), .pixel_y(pixel_y), .lit(edges_lit)
    );
    osd_text5x7 #(.X(166), .Y(652), .SCALE_SHIFT(2), .STEP(24),
                  .CHARS(13), .TEXT("AUTO TRACKING")) u_auto_tracking (
        .pixel_x(pixel_x), .pixel_y(pixel_y), .lit(auto_lit)
    );
    osd_text5x7 #(.X(118), .Y(652), .SCALE_SHIFT(2), .STEP(24),
                  .CHARS(17), .TEXT("MANUAL INSPECTION")) u_manual_inspection (
        .pixel_x(pixel_x), .pixel_y(pixel_y), .lit(manual_lit)
    );

    wire mode_lit = (mode == 2'd3) ? edges_lit :
                    (mode == 2'd2) ? lanczos_lit :
                    (mode == 2'd1) ? bilinear_lit : direct_lit;
    wire divider = (pixel_y == 10'd374) ||
                   ((pixel_x == 11'd640) && (pixel_y >= 10'd384));
    assign pixel = title_lit ? 24'hF5FAFF :
                   subtitle_lit ? 24'hB8C7E0 :
                   mode_lit ? 24'hFF6070 :
                   (paused && manual_lit) ? 24'h79B8FF :
                   (!paused && auto_lit) ? 24'h60DCA0 :
                   divider  ? 24'h188C96 : 24'h080C12;
endmodule

module osd_text5x7 #(
    parameter integer X = 0,
    parameter integer Y = 0,
    parameter integer SCALE_SHIFT = 2,
    parameter integer STEP = 24,
    parameter integer CHARS = 8,
    parameter [8*CHARS-1:0] TEXT = "EagleEye"
)(
    input  wire [10:0] pixel_x,
    input  wire [9:0]  pixel_y,
    output wire        lit
);
    wire [CHARS-1:0] glyph_lit;
    genvar i;
    generate
        for (i = 0; i < CHARS; i = i + 1) begin : g_letter
            localparam [7:0] LETTER = TEXT[(CHARS-i)*8-1 -: 8];
            osd_letter5x7 #(
                .X(X + i*STEP), .Y(Y), .SCALE_SHIFT(SCALE_SHIFT), .CHAR(LETTER)
            ) u_letter (
                .pixel_x(pixel_x), .pixel_y(pixel_y), .lit(glyph_lit[i])
            );
        end
    endgenerate
    assign lit = |glyph_lit;
endmodule

module osd_letter5x7 #(
    parameter integer X = 0,
    parameter integer Y = 0,
    parameter integer SCALE_SHIFT = 2,
    parameter [7:0] CHAR = " "
)(
    input  wire [10:0] pixel_x,
    input  wire [9:0]  pixel_y,
    output wire        lit
);
    localparam integer SCALE = 1 << SCALE_SHIFT;
    wire in_glyph = (pixel_x >= X) && (pixel_x < X + 5*SCALE) &&
                    (pixel_y >= Y) && (pixel_y < Y + 7*SCALE);
    wire [2:0] glyph_x = (pixel_x - X) >> SCALE_SHIFT;
    wire [2:0] glyph_y = (pixel_y - Y) >> SCALE_SHIFT;
    function [34:0] glyph_bitmap;
        input [7:0] c;
        reg [34:0] bitmap;
        begin
        case (c)
            "A": bitmap = 35'b01110_10001_10001_11111_10001_10001_10001;
            "C": bitmap = 35'b01111_10000_10000_10000_10000_10000_01111;
            "E": bitmap = 35'b11111_10000_11110_10000_10000_10000_11111;
            "F": bitmap = 35'b11111_10000_11110_10000_10000_10000_10000;
            "G": bitmap = 35'b01111_10000_10000_10111_10001_10001_01111;
            "I": bitmap = 35'b01110_00100_00100_00100_00100_00100_01110;
            "K": bitmap = 35'b10001_10010_10100_11000_10100_10010_10001;
            "M": bitmap = 35'b10001_11011_10101_10101_10001_10001_10001;
            "L": bitmap = 35'b10000_10000_10000_10000_10000_10000_11111;
            "N": bitmap = 35'b10001_11001_10101_10011_10001_10001_10001;
            "O": bitmap = 35'b01110_10001_10001_10001_10001_10001_01110;
            "B": bitmap = 35'b11110_10001_10001_11110_10001_10001_11110;
            "D": bitmap = 35'b11110_10001_10001_10001_10001_10001_11110;
            "P": bitmap = 35'b11110_10001_10001_11110_10000_10000_10000;
            "R": bitmap = 35'b11110_10001_10001_11110_10100_10010_10001;
            "S": bitmap = 35'b01111_10000_10000_01110_00001_00001_11110;
            "T": bitmap = 35'b11111_00100_00100_00100_00100_00100_00100;
            "U": bitmap = 35'b10001_10001_10001_10001_10001_10001_01110;
            "V": bitmap = 35'b10001_10001_10001_10001_10001_01010_00100;
            "a": bitmap = 35'b00000_01110_00001_01111_10001_10011_01101;
            "g": bitmap = 35'b00000_01111_10001_10001_01111_00001_01110;
            "l": bitmap = 35'b10000_10000_10000_10000_10000_10000_11100;
            "e": bitmap = 35'b00000_01110_10001_11111_10000_10001_01110;
            "y": bitmap = 35'b00000_10001_10001_01111_00001_10001_01110;
            "o": bitmap = 35'b00000_01110_10001_10001_10001_10001_01110;
            "d": bitmap = 35'b00001_00001_01101_10011_10001_10011_01101;
            "n": bitmap = 35'b00000_11110_10001_10001_10001_10001_10001;
            "c": bitmap = 35'b00000_01111_10000_10000_10000_10000_01111;
            "z": bitmap = 35'b00000_11111_00010_00100_01000_10000_11111;
            "s": bitmap = 35'b00000_01111_10000_01110_00001_00001_11110;
            "i": bitmap = 35'b00100_00000_01100_00100_00100_00100_01110;
            "m": bitmap = 35'b00000_11010_10101_10101_10101_10101_10101;
            "r": bitmap = 35'b00000_10110_11001_10000_10000_10000_10000;
            "t": bitmap = 35'b01000_01000_11100_01000_01000_01001_00110;
            ":": bitmap = 35'b00000_00100_00100_00000_00100_00100_00000;
            default: bitmap = 35'b0;
        endcase
        glyph_bitmap = bitmap;
        end
    endfunction
    wire [34:0] bitmap = glyph_bitmap(CHAR);
    assign lit = in_glyph && bitmap[34 - (glyph_y*5 + glyph_x)];
endmodule

`default_nettype wire
