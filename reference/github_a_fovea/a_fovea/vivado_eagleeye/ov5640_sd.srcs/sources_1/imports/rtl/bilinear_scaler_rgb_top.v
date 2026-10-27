`timescale 1ns / 1ps
`default_nettype none

// Fixed 2x RGB888 bilinear scaler for the same AXI4-Stream video style used by
// lanczos_scaler_rgb_top.  The line buffers use synchronous block-RAM style
// reads, and the output pixel is registered to keep the video clock timing sane.
module bilinear_scaler_rgb_top #(
    parameter IMG_WIDTH       = 640,
    parameter IMG_HEIGHT      = 360,
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
    localparam IN_XW = (IMG_WIDTH  <= 2) ? 1 : $clog2(IMG_WIDTH);
    localparam IN_YW = (IMG_HEIGHT <= 2) ? 1 : $clog2(IMG_HEIGHT);

    localparam [IN_XW-1:0] IMG_WIDTH_MINUS_1  = IMG_WIDTH - 1;
    localparam [IN_YW-1:0] IMG_HEIGHT_MINUS_1 = IMG_HEIGHT - 1;
    localparam [IN_YW-1:0] IMG_HEIGHT_MINUS_2 = (IMG_HEIGHT > 1) ? (IMG_HEIGHT - 2) : 0;
    localparam [IN_YW-1:0] IMG_HEIGHT_MINUS_3 = (IMG_HEIGHT > 2) ? (IMG_HEIGHT - 3) : 0;

    localparam ST_WAIT_SOF    = 3'd0;
    localparam ST_FILL_TOP    = 3'd1;
    localparam ST_FILL_BOTTOM = 3'd2;
    localparam ST_OUTPUT      = 3'd3;
    localparam ST_WAIT_NEXT   = 3'd4;
    localparam ST_FLUSH       = 3'd5;

    reg [2:0]       state;
    reg [IN_XW-1:0] fill_x;
    reg [IN_XW-1:0] capture_x;
    reg [IN_XW-1:0] src_x;
    reg [IN_YW-1:0] src_y;
    reg             x_phase;
    reg             y_phase;
    reg [1:0]       top_sel;
    reg [1:0]       bottom_sel;
    reg [1:0]       write_sel;
    reg             capture_active;
    reg             capture_done;

    reg [IN_XW-1:0] rd_addr_a;
    reg [IN_XW-1:0] rd_addr_b;

    wire [23:0] line0_a_dout;
    wire [23:0] line0_b_dout;
    wire [23:0] line1_a_dout;
    wire [23:0] line1_b_dout;
    wire [23:0] line2_a_dout;
    wire [23:0] line2_b_dout;

    reg             pipe_valid;
    reg             pipe_x_phase;
    reg             pipe_y_phase;
    reg             pipe_last;
    reg             pipe_user;
    reg [1:0]       pipe_top_sel;
    reg [1:0]       pipe_bottom_sel;
    reg [X_COORD_WIDTH-1:0] pipe_x_coord;
    reg [Y_COORD_WIDTH-1:0] pipe_y_coord;

    reg             calc_valid;
    reg             calc_x_phase;
    reg             calc_y_phase;
    reg             calc_last;
    reg             calc_user;
    reg [1:0]       calc_top_sel;
    reg [1:0]       calc_bottom_sel;
    reg [X_COORD_WIDTH-1:0] calc_x_coord;
    reg [Y_COORD_WIDTH-1:0] calc_y_coord;

    reg             out_valid;
    reg             out_last;
    reg             out_user;
    reg [23:0]      out_data;
    reg [X_COORD_WIDTH-1:0] out_x_coord;
    reg [Y_COORD_WIDTH-1:0] out_y_coord;

    wire in_fill_top = (state == ST_WAIT_SOF && s_axis_tuser) || (state == ST_FILL_TOP);
    wire in_fill_bottom = (state == ST_FILL_BOTTOM);
    wire in_capture_state = ((state == ST_OUTPUT) || (state == ST_WAIT_NEXT)) &&
                            capture_active && !capture_done;

    assign s_axis_tready = (state == ST_WAIT_SOF)    ? 1'b1 :
                           (state == ST_FILL_TOP)    ? 1'b1 :
                           (state == ST_FILL_BOTTOM) ? 1'b1 :
                           in_capture_state;

    wire s_fire = s_axis_tvalid && s_axis_tready;
    wire out_stage_ready = !out_valid || m_axis_tready;
    wire issue_output = (state == ST_OUTPUT) && out_stage_ready;

    wire capture_finishes_now = s_fire && in_capture_state &&
                                ((capture_x == IMG_WIDTH_MINUS_1) || s_axis_tlast);
    wire capture_ready = capture_done || capture_finishes_now || !capture_active;

    wire output_line_last = (src_x == IMG_WIDTH_MINUS_1) && x_phase;
    wire output_frame_last = (src_y == IMG_HEIGHT_MINUS_1) && y_phase &&
                             (src_x == IMG_WIDTH_MINUS_1) && x_phase;
    wire [IN_XW-1:0] right_x = (src_x == IMG_WIDTH_MINUS_1) ? src_x :
                               (src_x + {{(IN_XW-1){1'b0}}, 1'b1});

    wire [IN_XW-1:0] wr_addr = in_capture_state ? capture_x :
                               (state == ST_WAIT_SOF) ? {IN_XW{1'b0}} : fill_x;

    wire line0_we = s_fire && (in_fill_top || (in_capture_state && (write_sel == 2'd0)));
    wire line1_we = s_fire && (in_fill_bottom || (in_capture_state && (write_sel == 2'd1)));
    wire line2_we = s_fire && (in_capture_state && (write_sel == 2'd2));

    bilinear_line_ram #(
        .DATA_WIDTH(24),
        .ADDR_WIDTH(IN_XW),
        .DEPTH(IMG_WIDTH)
    ) u_line0_a (
        .clk(sys_clk),
        .we(line0_we),
        .wr_addr(wr_addr),
        .din(s_axis_tdata),
        .rd_addr(rd_addr_a),
        .dout(line0_a_dout)
    );

    bilinear_line_ram #(
        .DATA_WIDTH(24),
        .ADDR_WIDTH(IN_XW),
        .DEPTH(IMG_WIDTH)
    ) u_line0_b (
        .clk(sys_clk),
        .we(line0_we),
        .wr_addr(wr_addr),
        .din(s_axis_tdata),
        .rd_addr(rd_addr_b),
        .dout(line0_b_dout)
    );

    bilinear_line_ram #(
        .DATA_WIDTH(24),
        .ADDR_WIDTH(IN_XW),
        .DEPTH(IMG_WIDTH)
    ) u_line1_a (
        .clk(sys_clk),
        .we(line1_we),
        .wr_addr(wr_addr),
        .din(s_axis_tdata),
        .rd_addr(rd_addr_a),
        .dout(line1_a_dout)
    );

    bilinear_line_ram #(
        .DATA_WIDTH(24),
        .ADDR_WIDTH(IN_XW),
        .DEPTH(IMG_WIDTH)
    ) u_line1_b (
        .clk(sys_clk),
        .we(line1_we),
        .wr_addr(wr_addr),
        .din(s_axis_tdata),
        .rd_addr(rd_addr_b),
        .dout(line1_b_dout)
    );

    bilinear_line_ram #(
        .DATA_WIDTH(24),
        .ADDR_WIDTH(IN_XW),
        .DEPTH(IMG_WIDTH)
    ) u_line2_a (
        .clk(sys_clk),
        .we(line2_we),
        .wr_addr(wr_addr),
        .din(s_axis_tdata),
        .rd_addr(rd_addr_a),
        .dout(line2_a_dout)
    );

    bilinear_line_ram #(
        .DATA_WIDTH(24),
        .ADDR_WIDTH(IN_XW),
        .DEPTH(IMG_WIDTH)
    ) u_line2_b (
        .clk(sys_clk),
        .we(line2_we),
        .wr_addr(wr_addr),
        .din(s_axis_tdata),
        .rd_addr(rd_addr_b),
        .dout(line2_b_dout)
    );

    wire [23:0] pix_a = select_line(calc_top_sel,    line0_a_dout, line1_a_dout, line2_a_dout);
    wire [23:0] pix_b = select_line(calc_top_sel,    line0_b_dout, line1_b_dout, line2_b_dout);
    wire [23:0] pix_c = select_line(calc_bottom_sel, line0_a_dout, line1_a_dout, line2_a_dout);
    wire [23:0] pix_d = select_line(calc_bottom_sel, line0_b_dout, line1_b_dout, line2_b_dout);

    function [23:0] select_line;
        input [1:0]  sel;
        input [23:0] p0;
        input [23:0] p1;
        input [23:0] p2;
        begin
            case (sel)
                2'd0: select_line = p0;
                2'd1: select_line = p1;
                default: select_line = p2;
            endcase
        end
    endfunction

    function [7:0] avg2_8;
        input [7:0] p0;
        input [7:0] p1;
        reg [8:0] sum;
        begin
            sum = {1'b0, p0} + {1'b0, p1} + 9'd1;
            avg2_8 = sum[8:1];
        end
    endfunction

    function [7:0] avg4_8;
        input [7:0] p0;
        input [7:0] p1;
        input [7:0] p2;
        input [7:0] p3;
        reg [9:0] sum;
        begin
            sum = {2'b00, p0} + {2'b00, p1} + {2'b00, p2} + {2'b00, p3} + 10'd2;
            avg4_8 = sum[9:2];
        end
    endfunction

    function [23:0] avg2_rgb;
        input [23:0] p0;
        input [23:0] p1;
        begin
            avg2_rgb = {
                avg2_8(p0[23:16], p1[23:16]),
                avg2_8(p0[15:8],  p1[15:8]),
                avg2_8(p0[7:0],   p1[7:0])
            };
        end
    endfunction

    function [23:0] avg4_rgb;
        input [23:0] p0;
        input [23:0] p1;
        input [23:0] p2;
        input [23:0] p3;
        begin
            avg4_rgb = {
                avg4_8(p0[23:16], p1[23:16], p2[23:16], p3[23:16]),
                avg4_8(p0[15:8],  p1[15:8],  p2[15:8],  p3[15:8]),
                avg4_8(p0[7:0],   p1[7:0],   p2[7:0],   p3[7:0])
            };
        end
    endfunction

    function [23:0] bilinear_pixel;
        input [23:0] a;
        input [23:0] b;
        input [23:0] c;
        input [23:0] d;
        input        xp;
        input        yp;
        begin
            case ({yp, xp})
                2'b00: bilinear_pixel = a;
                2'b01: bilinear_pixel = avg2_rgb(a, b);
                2'b10: bilinear_pixel = avg2_rgb(a, c);
                default: bilinear_pixel = avg4_rgb(a, b, c, d);
            endcase
        end
    endfunction

    wire [X_COORD_WIDTH-1:0] src_x_ext = {{(X_COORD_WIDTH-IN_XW){1'b0}}, src_x};
    wire [Y_COORD_WIDTH-1:0] src_y_ext = {{(Y_COORD_WIDTH-IN_YW){1'b0}}, src_y};
    wire [X_COORD_WIDTH-1:0] issue_x_coord = (src_x_ext << 1) + {{(X_COORD_WIDTH-1){1'b0}}, x_phase};
    wire [Y_COORD_WIDTH-1:0] issue_y_coord = (src_y_ext << 1) + {{(Y_COORD_WIDTH-1){1'b0}}, y_phase};
    wire issue_user = (src_y == {IN_YW{1'b0}}) &&
                      (src_x == {IN_XW{1'b0}}) &&
                      !x_phase && !y_phase;

    assign m_axis_tvalid = out_valid;
    assign m_axis_tdata  = out_data;
    assign m_axis_tlast  = out_valid && out_last;
    assign m_axis_tuser  = out_valid && out_user;
    assign o_serial_x    = out_x_coord;
    assign o_serial_y    = out_y_coord;

    always @(posedge sys_clk) begin
        if (sys_rst) begin
            state           <= ST_WAIT_SOF;
            fill_x          <= {IN_XW{1'b0}};
            capture_x       <= {IN_XW{1'b0}};
            src_x           <= {IN_XW{1'b0}};
            src_y           <= {IN_YW{1'b0}};
            x_phase         <= 1'b0;
            y_phase         <= 1'b0;
            top_sel         <= 2'd0;
            bottom_sel      <= 2'd1;
            write_sel       <= 2'd2;
            capture_active  <= 1'b0;
            capture_done    <= 1'b1;
            rd_addr_a       <= {IN_XW{1'b0}};
            rd_addr_b       <= {IN_XW{1'b0}};
            pipe_valid      <= 1'b0;
            pipe_x_phase    <= 1'b0;
            pipe_y_phase    <= 1'b0;
            pipe_last       <= 1'b0;
            pipe_user       <= 1'b0;
            pipe_top_sel    <= 2'd0;
            pipe_bottom_sel <= 2'd1;
            pipe_x_coord    <= {X_COORD_WIDTH{1'b0}};
            pipe_y_coord    <= {Y_COORD_WIDTH{1'b0}};
            calc_valid      <= 1'b0;
            calc_x_phase    <= 1'b0;
            calc_y_phase    <= 1'b0;
            calc_last       <= 1'b0;
            calc_user       <= 1'b0;
            calc_top_sel    <= 2'd0;
            calc_bottom_sel <= 2'd1;
            calc_x_coord    <= {X_COORD_WIDTH{1'b0}};
            calc_y_coord    <= {Y_COORD_WIDTH{1'b0}};
            out_valid       <= 1'b0;
            out_last        <= 1'b0;
            out_user        <= 1'b0;
            out_data        <= 24'd0;
            out_x_coord     <= {X_COORD_WIDTH{1'b0}};
            out_y_coord     <= {Y_COORD_WIDTH{1'b0}};
        end else begin
            if (out_stage_ready) begin
                out_valid   <= calc_valid;
                out_last    <= calc_last;
                out_user    <= calc_user;
                out_data    <= calc_valid ? bilinear_pixel(pix_a, pix_b, pix_c, pix_d,
                                                           calc_x_phase, calc_y_phase) : 24'd0;
                out_x_coord <= calc_x_coord;
                out_y_coord <= calc_y_coord;

                calc_valid      <= pipe_valid;
                calc_x_phase    <= pipe_x_phase;
                calc_y_phase    <= pipe_y_phase;
                calc_last       <= pipe_last;
                calc_user       <= pipe_user;
                calc_top_sel    <= pipe_top_sel;
                calc_bottom_sel <= pipe_bottom_sel;
                calc_x_coord    <= pipe_x_coord;
                calc_y_coord    <= pipe_y_coord;

                pipe_valid <= issue_output;
                if (issue_output) begin
                    rd_addr_a       <= src_x;
                    rd_addr_b       <= right_x;
                    pipe_x_phase    <= x_phase;
                    pipe_y_phase    <= y_phase;
                    pipe_last       <= output_line_last;
                    pipe_user       <= issue_user;
                    pipe_top_sel    <= top_sel;
                    pipe_bottom_sel <= bottom_sel;
                    pipe_x_coord    <= issue_x_coord;
                    pipe_y_coord    <= issue_y_coord;
                end else begin
                    pipe_last <= 1'b0;
                    pipe_user <= 1'b0;
                end
            end

            if (s_fire && in_capture_state) begin
                if ((capture_x == IMG_WIDTH_MINUS_1) || s_axis_tlast) begin
                    capture_x      <= {IN_XW{1'b0}};
                    capture_done   <= 1'b1;
                    capture_active <= 1'b0;
                end else begin
                    capture_x <= capture_x + {{(IN_XW-1){1'b0}}, 1'b1};
                end
            end

            case (state)
                ST_WAIT_SOF: begin
                    if (s_fire && s_axis_tuser) begin
                        fill_x <= {{(IN_XW-1){1'b0}}, 1'b1};
                        state  <= ST_FILL_TOP;
                    end
                end

                ST_FILL_TOP: begin
                    if (s_fire) begin
                        if ((fill_x == IMG_WIDTH_MINUS_1) || s_axis_tlast) begin
                            fill_x <= {IN_XW{1'b0}};
                            state  <= ST_FILL_BOTTOM;
                        end else begin
                            fill_x <= fill_x + {{(IN_XW-1){1'b0}}, 1'b1};
                        end
                    end
                end

                ST_FILL_BOTTOM: begin
                    if (s_fire) begin
                        if ((fill_x == IMG_WIDTH_MINUS_1) || s_axis_tlast) begin
                            fill_x         <= {IN_XW{1'b0}};
                            src_x          <= {IN_XW{1'b0}};
                            src_y          <= {IN_YW{1'b0}};
                            x_phase        <= 1'b0;
                            y_phase        <= 1'b0;
                            top_sel        <= 2'd0;
                            bottom_sel     <= 2'd1;
                            write_sel      <= 2'd2;
                            capture_x      <= {IN_XW{1'b0}};
                            capture_active <= (IMG_HEIGHT > 2);
                            capture_done   <= !(IMG_HEIGHT > 2);
                            state          <= ST_OUTPUT;
                        end else begin
                            fill_x <= fill_x + {{(IN_XW-1){1'b0}}, 1'b1};
                        end
                    end
                end

                ST_OUTPUT: begin
                    if (issue_output) begin
                        if (output_frame_last) begin
                            state          <= ST_FLUSH;
                            src_x          <= {IN_XW{1'b0}};
                            src_y          <= {IN_YW{1'b0}};
                            x_phase        <= 1'b0;
                            y_phase        <= 1'b0;
                            capture_active <= 1'b0;
                            capture_done   <= 1'b1;
                        end else if (output_line_last && y_phase) begin
                            if (src_y == IMG_HEIGHT_MINUS_2) begin
                                src_x          <= {IN_XW{1'b0}};
                                src_y          <= src_y + {{(IN_YW-1){1'b0}}, 1'b1};
                                x_phase        <= 1'b0;
                                y_phase        <= 1'b0;
                                top_sel        <= bottom_sel;
                                bottom_sel     <= bottom_sel;
                                capture_active <= 1'b0;
                                capture_done   <= 1'b1;
                            end else if (capture_ready) begin
                                src_x          <= {IN_XW{1'b0}};
                                src_y          <= src_y + {{(IN_YW-1){1'b0}}, 1'b1};
                                x_phase        <= 1'b0;
                                y_phase        <= 1'b0;
                                top_sel        <= bottom_sel;
                                bottom_sel     <= write_sel;
                                write_sel      <= top_sel;
                                capture_x      <= {IN_XW{1'b0}};
                                capture_active <= (src_y < IMG_HEIGHT_MINUS_3);
                                capture_done   <= !(src_y < IMG_HEIGHT_MINUS_3);
                            end else begin
                                state <= ST_WAIT_NEXT;
                            end
                        end else if (output_line_last) begin
                            src_x   <= {IN_XW{1'b0}};
                            x_phase <= 1'b0;
                            y_phase <= 1'b1;
                        end else if (x_phase) begin
                            src_x   <= src_x + {{(IN_XW-1){1'b0}}, 1'b1};
                            x_phase <= 1'b0;
                        end else begin
                            x_phase <= 1'b1;
                        end
                    end
                end

                ST_WAIT_NEXT: begin
                    if (capture_ready && out_stage_ready) begin
                        state          <= ST_OUTPUT;
                        src_x          <= {IN_XW{1'b0}};
                        src_y          <= src_y + {{(IN_YW-1){1'b0}}, 1'b1};
                        x_phase        <= 1'b0;
                        y_phase        <= 1'b0;
                        top_sel        <= bottom_sel;
                        bottom_sel     <= write_sel;
                        write_sel      <= top_sel;
                        capture_x      <= {IN_XW{1'b0}};
                        capture_active <= (src_y < IMG_HEIGHT_MINUS_3);
                        capture_done   <= !(src_y < IMG_HEIGHT_MINUS_3);
                    end
                end

                ST_FLUSH: begin
                    if (out_stage_ready && !pipe_valid && !calc_valid) begin
                        state <= ST_WAIT_SOF;
                    end
                end

                default: begin
                    state <= ST_WAIT_SOF;
                end
            endcase
        end
    end

endmodule

module bilinear_line_ram #(
    parameter DATA_WIDTH = 24,
    parameter ADDR_WIDTH = 10,
    parameter DEPTH      = 640
)(
    input  wire                  clk,
    input  wire                  we,
    input  wire [ADDR_WIDTH-1:0] wr_addr,
    input  wire [DATA_WIDTH-1:0] din,
    input  wire [ADDR_WIDTH-1:0] rd_addr,
    output reg  [DATA_WIDTH-1:0] dout
);
    (* ram_style = "block" *) reg [DATA_WIDTH-1:0] ram [0:DEPTH-1];

    always @(posedge clk) begin
        if (we) begin
            ram[wr_addr] <= din;
        end
        dout <= ram[rd_addr];
    end
endmodule

`default_nettype wire
