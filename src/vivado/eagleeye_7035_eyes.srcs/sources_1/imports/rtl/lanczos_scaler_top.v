`timescale 1ns / 1ps
`default_nettype none

// Correctness-first single-channel 2x Lanczos-3 scaler.
//
// Window contract for reference source pixel (y,x):
//   win[r][c] = source[clamp(y+r-2)][clamp(x+c-2)]
// Therefore win[2][2] is exactly source[y][x].
//
// Six circular line stores retain the only source rows required by the 6x6
// window.  Input is paused while one reference row is processed and its two
// output rows are serialized.  No full-frame pixel buffer is used.
module lanczos_scaler_top #(
    parameter IMG_WIDTH       = 640,
    parameter IMG_HEIGHT      = 480,
    parameter PIXEL_WIDTH     = 8,
    parameter COEFF_WIDTH     = 12,
    parameter X_COORD_WIDTH   = (2*IMG_WIDTH  <= 2) ? 1 : $clog2(2*IMG_WIDTH),
    parameter Y_COORD_WIDTH   = (2*IMG_HEIGHT <= 2) ? 1 : $clog2(2*IMG_HEIGHT)
)(
    input  wire                         sys_clk,
    input  wire                         sys_rst,

    input  wire [PIXEL_WIDTH-1:0]       s_axis_tdata,
    input  wire                         s_axis_tvalid,
    output wire                         s_axis_tready,
    input  wire                         s_axis_tlast,
    input  wire                         s_axis_tuser,

    output wire [PIXEL_WIDTH-1:0]       m_axis_tdata,
    output wire                         m_axis_tvalid,
    input  wire                         m_axis_tready,
    output wire                         m_axis_tlast,
    output wire                         m_axis_tuser,

    output wire [X_COORD_WIDTH-1:0]     o_serial_x,
    output wire [Y_COORD_WIDTH-1:0]     o_serial_y
);
    localparam OUT_WIDTH = 2 * IMG_WIDTH;
    localparam IN_X_W  = (IMG_WIDTH  <= 2) ? 1 : $clog2(IMG_WIDTH);
    localparam IN_Y_W  = (IMG_HEIGHT <= 2) ? 1 : $clog2(IMG_HEIGHT);
    localparam OUT_X_W = (OUT_WIDTH  <= 2) ? 1 : $clog2(OUT_WIDTH);
    localparam [2:0] LAST_ROW_BANK = (IMG_HEIGHT - 1) % 6;

    localparam [3:0] ST_INPUT      = 4'd0;
    localparam [3:0] ST_PREP       = 4'd1;
    localparam [3:0] ST_PRELOAD    = 4'd2;
    localparam [3:0] ST_FEED       = 4'd3;
    localparam [3:0] ST_WAIT       = 4'd4;
    localparam [3:0] ST_OUT0_PRIME = 4'd5;
    localparam [3:0] ST_OUT0       = 4'd6;
    localparam [3:0] ST_OUT1_PRIME = 4'd7;
    localparam [3:0] ST_OUT1       = 4'd8;
    localparam [3:0] ST_PRELOAD_WAIT = 4'd9;
    localparam [3:0] ST_FEED_WAIT    = 4'd10;

    reg [3:0] state;
    reg [IN_X_W-1:0] in_x;
    reg [IN_Y_W-1:0] in_y;
    reg [IN_Y_W-1:0] target_y;
    reg [IN_Y_W-1:0] last_row_captured;
    reg frame_started;
    reg [IN_X_W-1:0] src_rd_col;
    reg [2:0] target_bank;
    reg [2:0] src_bank0;
    reg [2:0] src_bank1;
    reg [2:0] src_bank2;
    reg [2:0] src_bank3;
    reg [2:0] src_bank4;
    reg [2:0] src_bank5;

    // Six independent row banks.  Row y is stored in bank (y mod 6).
    wire [PIXEL_WIDTH-1:0] line0_rd_data;
    wire [PIXEL_WIDTH-1:0] line1_rd_data;
    wire [PIXEL_WIDTH-1:0] line2_rd_data;
    wire [PIXEL_WIDTH-1:0] line3_rd_data;
    wire [PIXEL_WIDTH-1:0] line4_rd_data;
    wire [PIXEL_WIDTH-1:0] line5_rd_data;

    function integer clamp_row;
        input integer value;
        begin
            if (value < 0)
                clamp_row = 0;
            else if (value >= IMG_HEIGHT)
                clamp_row = IMG_HEIGHT - 1;
            else
                clamp_row = value;
        end
    endfunction

    function integer clamp_col;
        input integer value;
        begin
            if (value < 0)
                clamp_col = 0;
            else if (value >= IMG_WIDTH)
                clamp_col = IMG_WIDTH - 1;
            else
                clamp_col = value;
        end
    endfunction

    function integer required_row;
        input integer reference_row;
        begin
            required_row = clamp_row(reference_row + 3);
        end
    endfunction

    function [2:0] bank_add;
        input [2:0] bank;
        input [2:0] offset;
        reg [3:0] sum;
        begin
            sum = bank + offset;
            bank_add = (sum >= 6) ? (sum - 6) : sum[2:0];
        end
    endfunction

    function [2:0] source_bank;
        input [IN_Y_W-1:0] reference_row;
        input [2:0] reference_bank;
        input integer window_slot;
        begin
            case (window_slot)
                0: source_bank = (reference_row < 2) ? 3'd0 : bank_add(reference_bank, 3'd4);
                1: source_bank = (reference_row < 1) ? 3'd0 : bank_add(reference_bank, 3'd5);
                2: source_bank = reference_bank;
                3: source_bank = (reference_row >= IMG_HEIGHT-1) ? LAST_ROW_BANK : bank_add(reference_bank, 3'd1);
                4: source_bank = (reference_row >= IMG_HEIGHT-2) ? LAST_ROW_BANK : bank_add(reference_bank, 3'd2);
                default: source_bank = (reference_row >= IMG_HEIGHT-3) ? LAST_ROW_BANK : bank_add(reference_bank, 3'd3);
            endcase
        end
    endfunction

    function [2:0] window_bank;
        input integer window_slot;
        begin
            case (window_slot)
                0: window_bank = src_bank0;
                1: window_bank = src_bank1;
                2: window_bank = src_bank2;
                3: window_bank = src_bank3;
                4: window_bank = src_bank4;
                default: window_bank = src_bank5;
            endcase
        end
    endfunction

    function [PIXEL_WIDTH-1:0] read_source_pixel_bank;
        input [2:0] bank;
        begin
            case (bank)
                0: read_source_pixel_bank = line0_rd_data;
                1: read_source_pixel_bank = line1_rd_data;
                2: read_source_pixel_bank = line2_rd_data;
                3: read_source_pixel_bank = line3_rd_data;
                4: read_source_pixel_bank = line4_rd_data;
                default: read_source_pixel_bank = line5_rd_data;
            endcase
        end
    endfunction

    wire need_next_row = (target_y < IMG_HEIGHT-1) &&
                         (last_row_captured < required_row(target_y + 1));
    wire capture_overlap_ok = (state == ST_WAIT) ||
                              (state == ST_OUT0_PRIME) ||
                              (state == ST_OUT0) ||
                              (state == ST_OUT1_PRIME) ||
                              (state == ST_OUT1);
    wire input_fire = s_axis_tvalid && s_axis_tready;
    wire input_eol  = s_axis_tlast || (in_x == IMG_WIDTH-1);
    // Once the current 6x6 row scan is finished, capture at most the one row
    // needed by the next reference row while the current output pair drains.
    assign s_axis_tready = !sys_rst &&
                           ((state == ST_INPUT) ||
                            (capture_overlap_ok && need_next_row));

    wire [2:0] input_bank = s_axis_tuser ? 3'd0 : (in_y % 6);
    wire [IN_X_W-1:0] line_wr_addr = s_axis_tuser ? {IN_X_W{1'b0}} : in_x;
    wire [5:0] line_wr_en = !input_fire       ? 6'b000000 :
                            (input_bank == 0) ? 6'b000001 :
                            (input_bank == 1) ? 6'b000010 :
                            (input_bank == 2) ? 6'b000100 :
                            (input_bank == 3) ? 6'b001000 :
                            (input_bank == 4) ? 6'b010000 :
                                                6'b100000;
    wire line_rd_en = (state == ST_PRELOAD_WAIT) || (state == ST_FEED_WAIT);

    lanczos_bram_sdpram #(
        .DATA_WIDTH(PIXEL_WIDTH), .ADDR_WIDTH(IN_X_W), .DEPTH(IMG_WIDTH)
    ) u_line0_ram (
        .clk(sys_clk), .wr_en(line_wr_en[0]), .wr_addr(line_wr_addr),
        .wr_data(s_axis_tdata), .rd_en(line_rd_en), .rd_addr(src_rd_col),
        .rd_data(line0_rd_data)
    );

    lanczos_bram_sdpram #(
        .DATA_WIDTH(PIXEL_WIDTH), .ADDR_WIDTH(IN_X_W), .DEPTH(IMG_WIDTH)
    ) u_line1_ram (
        .clk(sys_clk), .wr_en(line_wr_en[1]), .wr_addr(line_wr_addr),
        .wr_data(s_axis_tdata), .rd_en(line_rd_en), .rd_addr(src_rd_col),
        .rd_data(line1_rd_data)
    );

    lanczos_bram_sdpram #(
        .DATA_WIDTH(PIXEL_WIDTH), .ADDR_WIDTH(IN_X_W), .DEPTH(IMG_WIDTH)
    ) u_line2_ram (
        .clk(sys_clk), .wr_en(line_wr_en[2]), .wr_addr(line_wr_addr),
        .wr_data(s_axis_tdata), .rd_en(line_rd_en), .rd_addr(src_rd_col),
        .rd_data(line2_rd_data)
    );

    lanczos_bram_sdpram #(
        .DATA_WIDTH(PIXEL_WIDTH), .ADDR_WIDTH(IN_X_W), .DEPTH(IMG_WIDTH)
    ) u_line3_ram (
        .clk(sys_clk), .wr_en(line_wr_en[3]), .wr_addr(line_wr_addr),
        .wr_data(s_axis_tdata), .rd_en(line_rd_en), .rd_addr(src_rd_col),
        .rd_data(line3_rd_data)
    );

    lanczos_bram_sdpram #(
        .DATA_WIDTH(PIXEL_WIDTH), .ADDR_WIDTH(IN_X_W), .DEPTH(IMG_WIDTH)
    ) u_line4_ram (
        .clk(sys_clk), .wr_en(line_wr_en[4]), .wr_addr(line_wr_addr),
        .wr_data(s_axis_tdata), .rd_en(line_rd_en), .rd_addr(src_rd_col),
        .rd_data(line4_rd_data)
    );

    lanczos_bram_sdpram #(
        .DATA_WIDTH(PIXEL_WIDTH), .ADDR_WIDTH(IN_X_W), .DEPTH(IMG_WIDTH)
    ) u_line5_ram (
        .clk(sys_clk), .wr_en(line_wr_en[5]), .wr_addr(line_wr_addr),
        .wr_data(s_axis_tdata), .rd_en(line_rd_en), .rd_addr(src_rd_col),
        .rd_data(line5_rd_data)
    );

    // Column window state.  PRELOAD feeds clamped columns -2..3; thereafter
    // every FEED clock advances by one source column.
    reg [PIXEL_WIDTH-1:0] win [0:5][0:5];
    reg signed [31:0] preload_col;
    reg [IN_X_W-1:0] proc_x;
    integer rr, cc;

    wire core_i_valid = (state == ST_FEED);

    // Fixed coefficients.
    wire signed [COEFF_WIDTH-1:0] w0, w1, w2;
    wire signed [COEFF_WIDTH-1:0] w00, w01, w02;
    wire signed [COEFF_WIDTH-1:0] w10, w11, w12;
    wire signed [COEFF_WIDTH-1:0] w20, w21, w22;
    lanczos_coeff_2d_rom #(.COEFF_WIDTH(COEFF_WIDTH)) u_coeff (
        .o_w0(w0), .o_w1(w1), .o_w2(w2),
        .o_w00(w00), .o_w01(w01), .o_w02(w02),
        .o_w10(w10), .o_w11(w11), .o_w12(w12),
        .o_w20(w20), .o_w21(w21), .o_w22(w22)
    );

    wire [PIXEL_WIDTH-1:0] core_pix0, core_pix1, core_pix2, core_pix3;
    wire core_valid0, core_valid1, core_valid2, core_valid3;

    lanczos_scaler_core0 #(.PIXEL_WIDTH(PIXEL_WIDTH)) u_core0 (
        .sys_clk(sys_clk), .sys_rst(sys_rst),
        .i_pix_center(win[2][2]), .i_valid(core_i_valid),
        .o_pix(core_pix0), .o_valid(core_valid0)
    );

    lanczos_scaler_core1 #(
        .PIXEL_WIDTH(PIXEL_WIDTH), .COEFF_WIDTH(COEFF_WIDTH), .COEFF_SHIFT(10)
    ) u_core1 (
        .sys_clk(sys_clk), .sys_rst(sys_rst),
        .i_p0(win[2][0]), .i_p1(win[2][1]), .i_p2(win[2][2]),
        .i_p3(win[2][3]), .i_p4(win[2][4]), .i_p5(win[2][5]),
        .i_w0(w0), .i_w1(w1), .i_w2(w2), .i_valid(core_i_valid),
        .o_pix(core_pix1), .o_valid(core_valid1)
    );

    lanczos_scaler_core2 #(
        .PIXEL_WIDTH(PIXEL_WIDTH), .COEFF_WIDTH(COEFF_WIDTH), .COEFF_SHIFT(10)
    ) u_core2 (
        .sys_clk(sys_clk), .sys_rst(sys_rst),
        .i_p0(win[0][2]), .i_p1(win[1][2]), .i_p2(win[2][2]),
        .i_p3(win[3][2]), .i_p4(win[4][2]), .i_p5(win[5][2]),
        .i_w0(w0), .i_w1(w1), .i_w2(w2), .i_valid(core_i_valid),
        .o_pix(core_pix2), .o_valid(core_valid2)
    );

    lanczos_scaler_core3 #(
        .PIXEL_WIDTH(PIXEL_WIDTH), .COEFF_WIDTH(COEFF_WIDTH), .COEFF_SHIFT(10)
    ) u_core3 (
        .sys_clk(sys_clk), .sys_rst(sys_rst),
        .i_pix_00(win[0][0]), .i_pix_01(win[0][1]), .i_pix_02(win[0][2]), .i_pix_03(win[0][3]), .i_pix_04(win[0][4]), .i_pix_05(win[0][5]),
        .i_pix_10(win[1][0]), .i_pix_11(win[1][1]), .i_pix_12(win[1][2]), .i_pix_13(win[1][3]), .i_pix_14(win[1][4]), .i_pix_15(win[1][5]),
        .i_pix_20(win[2][0]), .i_pix_21(win[2][1]), .i_pix_22(win[2][2]), .i_pix_23(win[2][3]), .i_pix_24(win[2][4]), .i_pix_25(win[2][5]),
        .i_pix_30(win[3][0]), .i_pix_31(win[3][1]), .i_pix_32(win[3][2]), .i_pix_33(win[3][3]), .i_pix_34(win[3][4]), .i_pix_35(win[3][5]),
        .i_pix_40(win[4][0]), .i_pix_41(win[4][1]), .i_pix_42(win[4][2]), .i_pix_43(win[4][3]), .i_pix_44(win[4][4]), .i_pix_45(win[4][5]),
        .i_pix_50(win[5][0]), .i_pix_51(win[5][1]), .i_pix_52(win[5][2]), .i_pix_53(win[5][3]), .i_pix_54(win[5][4]), .i_pix_55(win[5][5]),
        .i_w00(w00), .i_w01(w01), .i_w02(w02),
        .i_w10(w10), .i_w11(w11), .i_w12(w12),
        .i_w20(w20), .i_w21(w21), .i_w22(w22),
        .i_valid(core_i_valid), .o_pix(core_pix3), .o_valid(core_valid3)
    );

    wire block_valid = core_valid0 && core_valid1 && core_valid2 && core_valid3;
    wire output_fire = m_axis_tvalid && m_axis_tready;

    // One pair of output rows.  They remain unchanged while AXI backpressure
    // holds m_axis_tready low.
    reg [IN_X_W:0] block_wr_count;
    reg row_pair_ready;
    reg [OUT_X_W-1:0] out_idx;
    reg [OUT_X_W-1:0] out_rd_addr;
    wire [IN_X_W-1:0] out_rd_word = out_rd_addr[OUT_X_W-1:1];
    wire [PIXEL_WIDTH-1:0] out_line0_even_data;
    wire [PIXEL_WIDTH-1:0] out_line0_odd_data;
    wire [PIXEL_WIDTH-1:0] out_line1_even_data;
    wire [PIXEL_WIDTH-1:0] out_line1_odd_data;
    wire out_wr_en = block_valid && (block_wr_count < IMG_WIDTH);
    wire out_rd_en = (state == ST_OUT0_PRIME) ||
                     (state == ST_OUT1_PRIME) ||
                     ((state == ST_OUT0) && output_fire && (out_idx != OUT_WIDTH-1)) ||
                     ((state == ST_OUT1) && output_fire && (out_idx != OUT_WIDTH-1));

    lanczos_bram_sdpram #(
        .DATA_WIDTH(PIXEL_WIDTH), .ADDR_WIDTH(IN_X_W), .DEPTH(IMG_WIDTH)
    ) u_out_line0_even_ram (
        .clk(sys_clk), .wr_en(out_wr_en), .wr_addr(block_wr_count[IN_X_W-1:0]),
        .wr_data(core_pix0), .rd_en(out_rd_en), .rd_addr(out_rd_word),
        .rd_data(out_line0_even_data)
    );

    lanczos_bram_sdpram #(
        .DATA_WIDTH(PIXEL_WIDTH), .ADDR_WIDTH(IN_X_W), .DEPTH(IMG_WIDTH)
    ) u_out_line0_odd_ram (
        .clk(sys_clk), .wr_en(out_wr_en), .wr_addr(block_wr_count[IN_X_W-1:0]),
        .wr_data(core_pix1), .rd_en(out_rd_en), .rd_addr(out_rd_word),
        .rd_data(out_line0_odd_data)
    );

    lanczos_bram_sdpram #(
        .DATA_WIDTH(PIXEL_WIDTH), .ADDR_WIDTH(IN_X_W), .DEPTH(IMG_WIDTH)
    ) u_out_line1_even_ram (
        .clk(sys_clk), .wr_en(out_wr_en), .wr_addr(block_wr_count[IN_X_W-1:0]),
        .wr_data(core_pix2), .rd_en(out_rd_en), .rd_addr(out_rd_word),
        .rd_data(out_line1_even_data)
    );

    lanczos_bram_sdpram #(
        .DATA_WIDTH(PIXEL_WIDTH), .ADDR_WIDTH(IN_X_W), .DEPTH(IMG_WIDTH)
    ) u_out_line1_odd_ram (
        .clk(sys_clk), .wr_en(out_wr_en), .wr_addr(block_wr_count[IN_X_W-1:0]),
        .wr_data(core_pix3), .rd_en(out_rd_en), .rd_addr(out_rd_word),
        .rd_data(out_line1_odd_data)
    );

    always @(posedge sys_clk) begin
        if (sys_rst || (state == ST_PREP)) begin
            block_wr_count <= 0;
            row_pair_ready <= 1'b0;
        end else begin
            row_pair_ready <= 1'b0;
            if (block_valid && (block_wr_count < IMG_WIDTH)) begin
                if (block_wr_count == IMG_WIDTH-1) begin
                    row_pair_ready <= 1'b1;
                end else begin
                    block_wr_count <= block_wr_count + 1'b1;
                end
            end
        end
    end

    assign m_axis_tvalid = (state == ST_OUT0) || (state == ST_OUT1);
    // The output row memories are intentionally not reset so that synthesis can
    // infer RAM instead of a large bank of resettable registers.  Their contents
    // are therefore unknown before the first row pair has been calculated.  Do
    // not expose those don't-care values while TVALID is low; only read the
    // selected output row during a real AXI output state.
    assign m_axis_tdata  = !m_axis_tvalid ? {PIXEL_WIDTH{1'b0}} :
                           (state == ST_OUT1) ?
                               (out_idx[0] ? out_line1_odd_data : out_line1_even_data) :
                               (out_idx[0] ? out_line0_odd_data : out_line0_even_data);
    assign m_axis_tlast  = m_axis_tvalid && (out_idx == OUT_WIDTH-1);
    assign m_axis_tuser  = m_axis_tvalid && (state == ST_OUT0) &&
                           (target_y == 0) && (out_idx == 0);
    assign o_serial_x = out_idx;
    assign o_serial_y = (target_y << 1) + ((state == ST_OUT1) ? 1'b1 : 1'b0);

    // Input capture, window construction, row processing and serialization.
    always @(posedge sys_clk) begin
        if (sys_rst) begin
            state <= ST_INPUT;
            in_x <= 0;
            in_y <= 0;
            target_y <= 0;
            last_row_captured <= 0;
            frame_started <= 1'b0;
            preload_col <= -2;
            src_rd_col <= 0;
            target_bank <= 3'd0;
            src_bank0 <= 3'd0;
            src_bank1 <= 3'd0;
            src_bank2 <= 3'd0;
            src_bank3 <= 3'd0;
            src_bank4 <= 3'd0;
            src_bank5 <= 3'd0;
            proc_x <= 0;
            out_idx <= 0;
            out_rd_addr <= 0;
            for (rr = 0; rr < 6; rr = rr + 1)
                for (cc = 0; cc < 6; cc = cc + 1)
                    win[rr][cc] <= {PIXEL_WIDTH{1'b0}};
        end else begin
            // Source-row capture is independent of output serialization.  It
            // is enabled in ST_INPUT and, when safe, overlaps ST_WAIT/OUT0/OUT1.
            if (input_fire) begin
                if (s_axis_tuser) begin
                    in_y <= 0;
                    target_y <= 0;
                    last_row_captured <= 0;
                    target_bank <= 3'd0;
                    frame_started <= 1'b1;
                    if (IMG_WIDTH == 1)
                        in_x <= 0;
                    else
                        in_x <= 1;
                end else begin
                    if (input_eol) begin
                        in_x <= 0;
                        last_row_captured <= in_y;
                        if (in_y < IMG_HEIGHT-1)
                            in_y <= in_y + 1'b1;
                    end else begin
                        in_x <= in_x + 1'b1;
                    end
                end
            end

            case (state)
                ST_INPUT: begin
                    if (input_fire && !s_axis_tuser && input_eol &&
                        ((in_y >= required_row(target_y)) ||
                         (in_y == IMG_HEIGHT-1)))
                        state <= ST_PREP;
                end

                ST_PREP: begin
                    preload_col <= -2;
                    src_rd_col <= clamp_col(-2);
                    src_bank0 <= source_bank(target_y, target_bank, 0);
                    src_bank1 <= source_bank(target_y, target_bank, 1);
                    src_bank2 <= source_bank(target_y, target_bank, 2);
                    src_bank3 <= source_bank(target_y, target_bank, 3);
                    src_bank4 <= source_bank(target_y, target_bank, 4);
                    src_bank5 <= source_bank(target_y, target_bank, 5);
                    proc_x <= 0;
                    for (rr = 0; rr < 6; rr = rr + 1)
                        for (cc = 0; cc < 6; cc = cc + 1)
                            win[rr][cc] <= {PIXEL_WIDTH{1'b0}};
                    state <= ST_PRELOAD_WAIT;
                end

                ST_PRELOAD_WAIT: begin
                    state <= ST_PRELOAD;
                end

                ST_PRELOAD: begin
                    for (rr = 0; rr < 6; rr = rr + 1) begin
                        win[rr][0] <= win[rr][1];
                        win[rr][1] <= win[rr][2];
                        win[rr][2] <= win[rr][3];
                        win[rr][3] <= win[rr][4];
                        win[rr][4] <= win[rr][5];
                        win[rr][5] <= read_source_pixel_bank(window_bank(rr));
                    end
                    if (preload_col == 3) begin
                        src_rd_col <= clamp_col(4);
                        state <= ST_FEED_WAIT;
                    end else begin
                        src_rd_col <= clamp_col(preload_col + 1);
                        preload_col <= preload_col + 1;
                        state <= ST_PRELOAD_WAIT;
                    end
                end

                ST_FEED_WAIT: begin
                    state <= ST_FEED;
                end

                ST_FEED: begin
                    // Cores capture the current window on this edge.  Shift in
                    // column x+4 to prepare the window for reference x+1.
                    if (proc_x == IMG_WIDTH-1) begin
                        state <= ST_WAIT;
                    end else begin
                        for (rr = 0; rr < 6; rr = rr + 1) begin
                            win[rr][0] <= win[rr][1];
                            win[rr][1] <= win[rr][2];
                            win[rr][2] <= win[rr][3];
                            win[rr][3] <= win[rr][4];
                            win[rr][4] <= win[rr][5];
                            win[rr][5] <= read_source_pixel_bank(window_bank(rr));
                        end
                        src_rd_col <= clamp_col(proc_x + 5);
                        proc_x <= proc_x + 1'b1;
                        state <= ST_FEED_WAIT;
                    end
                end

                ST_WAIT: begin
                    if (row_pair_ready) begin
                        out_idx <= 0;
                        out_rd_addr <= 0;
                        state <= ST_OUT0_PRIME;
                    end
                end

                ST_OUT0_PRIME: begin
                    out_idx <= 0;
                    out_rd_addr <= 1;
                    state <= ST_OUT0;
                end

                ST_OUT0: begin
                    if (output_fire) begin
                        if (out_idx == OUT_WIDTH-1) begin
                            out_idx <= 0;
                            out_rd_addr <= 0;
                            state <= ST_OUT1_PRIME;
                        end else begin
                            out_idx <= out_idx + 1'b1;
                            if (out_idx < OUT_WIDTH-2)
                                out_rd_addr <= out_rd_addr + 1'b1;
                        end
                    end
                end

                ST_OUT1_PRIME: begin
                    out_idx <= 0;
                    out_rd_addr <= 1;
                    state <= ST_OUT1;
                end

                ST_OUT1: begin
                    if (output_fire) begin
                        if (out_idx == OUT_WIDTH-1) begin
                            out_idx <= 0;
                            if (target_y == IMG_HEIGHT-1) begin
                                // Frame complete; accept the next SOF.
                                state <= ST_INPUT;
                                in_x <= 0;
                                in_y <= 0;
                                target_y <= 0;
                                target_bank <= 3'd0;
                                last_row_captured <= 0;
                                frame_started <= 1'b0;
                            end else begin
                                target_y <= target_y + 1'b1;
                                target_bank <= bank_add(target_bank, 3'd1);
                                if ((last_row_captured >= required_row(target_y + 1)) ||
                                    (input_fire && !s_axis_tuser && input_eol &&
                                     (in_y >= required_row(target_y + 1))))
                                    state <= ST_PREP;
                                else
                                    state <= ST_INPUT;
                            end
                        end else begin
                            out_idx <= out_idx + 1'b1;
                            if (out_idx < OUT_WIDTH-2)
                                out_rd_addr <= out_rd_addr + 1'b1;
                        end
                    end
                end

                default: state <= ST_INPUT;
            endcase
        end
    end

    // Keep the variable visible to synthesis/debug tools; normal operation
    // starts with tuser but fixed-size counting also works without it.
    wire unused_frame_started = frame_started;
endmodule

module lanczos_bram_sdpram #(
    parameter DATA_WIDTH = 8,
    parameter ADDR_WIDTH = 10,
    parameter DEPTH      = 640
)(
    input  wire                  clk,
    input  wire                  wr_en,
    input  wire [ADDR_WIDTH-1:0] wr_addr,
    input  wire [DATA_WIDTH-1:0] wr_data,
    input  wire                  rd_en,
    input  wire [ADDR_WIDTH-1:0] rd_addr,
    output wire [DATA_WIDTH-1:0] rd_data
);
    xpm_memory_sdpram #(
        .ADDR_WIDTH_A(ADDR_WIDTH),
        .ADDR_WIDTH_B(ADDR_WIDTH),
        .AUTO_SLEEP_TIME(0),
        .BYTE_WRITE_WIDTH_A(DATA_WIDTH),
        .CASCADE_HEIGHT(0),
        .CLOCKING_MODE("common_clock"),
        .ECC_MODE("no_ecc"),
        .MEMORY_INIT_FILE("none"),
        .MEMORY_INIT_PARAM("0"),
        .MEMORY_OPTIMIZATION("true"),
        .MEMORY_PRIMITIVE("block"),
        .MEMORY_SIZE(DEPTH * DATA_WIDTH),
        .MESSAGE_CONTROL(0),
        .READ_DATA_WIDTH_B(DATA_WIDTH),
        .READ_LATENCY_B(1),
        .READ_RESET_VALUE_B("0"),
        .RST_MODE_A("SYNC"),
        .RST_MODE_B("SYNC"),
        .SIM_ASSERT_CHK(0),
        .USE_EMBEDDED_CONSTRAINT(0),
        .USE_MEM_INIT(0),
        .USE_MEM_INIT_MMI(0),
        .WAKEUP_TIME("disable_sleep"),
        .WRITE_DATA_WIDTH_A(DATA_WIDTH),
        .WRITE_MODE_B("read_first")
    ) u_xpm_sdpram (
        .dbiterrb(),
        .doutb(rd_data),
        .sbiterrb(),
        .addra(wr_addr),
        .addrb(rd_addr),
        .clka(clk),
        .clkb(clk),
        .dina(wr_data),
        .ena(wr_en),
        .enb(rd_en),
        .injectdbiterra(1'b0),
        .injectsbiterra(1'b0),
        .regceb(1'b1),
        .rstb(1'b0),
        .sleep(1'b0),
        .wea(wr_en)
    );
endmodule

`default_nettype wire
