`timescale 1ns / 1ps
`default_nettype none

// Captures a dynamic RGB888 ROI from the incoming 640x360 stream and replays the
// previous captured ROI as a 320x180 AXI4-Stream frame.  The replay path emits a
// black frame until one complete ROI has been captured.
module roi_frame_buffer_320x180 #(
    parameter SRC_WIDTH  = 640,
    parameter SRC_HEIGHT = 360,
    parameter ROI_X      = 160,
    parameter ROI_Y      = 90,
    parameter ROI_WIDTH  = 320,
    parameter ROI_HEIGHT = 180
)(
    input  wire        clk,
    input  wire        rst,

    input  wire [23:0] capture_tdata,
    input  wire        capture_fire,
    input  wire        capture_tlast,
    input  wire        capture_tuser,
    input  wire [9:0]  roi_x,
    input  wire [8:0]  roi_y,

    input  wire        start_playback,
    output wire [23:0] m_axis_tdata,
    output wire        m_axis_tvalid,
    input  wire        m_axis_tready,
    output wire        m_axis_tlast,
    output wire        m_axis_tuser
);
    localparam SRC_XW = (SRC_WIDTH  <= 2) ? 1 : $clog2(SRC_WIDTH);
    localparam SRC_YW = (SRC_HEIGHT <= 2) ? 1 : $clog2(SRC_HEIGHT);
    localparam ROI_PIXELS = ROI_WIDTH * ROI_HEIGHT;
    localparam ADDR_W = (ROI_PIXELS <= 2) ? 1 : $clog2(ROI_PIXELS);

    localparam [SRC_XW-1:0] SRC_WIDTH_MINUS_1 = SRC_WIDTH - 1;
    localparam [SRC_XW-1:0] ROI_X_MAX         = SRC_WIDTH - ROI_WIDTH;
    localparam [SRC_YW-1:0] ROI_Y_MAX         = SRC_HEIGHT - ROI_HEIGHT;
    localparam [ADDR_W-1:0] ROI_PIXELS_MINUS_1 = ROI_PIXELS - 1;
    localparam [ADDR_W-1:0] ROI_WIDTH_MINUS_1  = ROI_WIDTH - 1;

    wire [SRC_XW-1:0] roi_x_safe = (roi_x[SRC_XW-1:0] > ROI_X_MAX) ? ROI_X_MAX : roi_x[SRC_XW-1:0];
    wire [SRC_YW-1:0] roi_y_safe = (roi_y[SRC_YW-1:0] > ROI_Y_MAX) ? ROI_Y_MAX : roi_y[SRC_YW-1:0];
    wire [SRC_XW-1:0] roi_x_end  = roi_x_safe + (ROI_WIDTH - 1);
    wire [SRC_YW-1:0] roi_y_end  = roi_y_safe + (ROI_HEIGHT - 1);

    (* ram_style = "block" *) reg [23:0] roi_mem [0:ROI_PIXELS-1];

    reg [SRC_XW-1:0] cap_x;
    reg [SRC_YW-1:0] cap_y;
    reg              roi_valid;

    wire [SRC_XW-1:0] cur_x = capture_tuser ? {SRC_XW{1'b0}} : cap_x;
    wire [SRC_YW-1:0] cur_y = capture_tuser ? {SRC_YW{1'b0}} : cap_y;
    wire in_roi = (cur_x >= roi_x_safe) && (cur_x <= roi_x_end) &&
                  (cur_y >= roi_y_safe) && (cur_y <= roi_y_end);
    wire roi_first_pixel = (cur_x == roi_x_safe) && (cur_y == roi_y_safe);
    wire roi_last_pixel  = (cur_x == roi_x_end)  && (cur_y == roi_y_end);

    reg [ADDR_W-1:0] wr_addr_seq;
    wire [ADDR_W-1:0] wr_addr = roi_first_pixel ? {ADDR_W{1'b0}} : wr_addr_seq;

    reg [ADDR_W-1:0] rd_addr;
    reg [ADDR_W-1:0] issue_addr;
    reg              issuing;
    reg              issued_valid;
    reg              issued_user;
    reg              issued_last;
    reg              rd_valid;
    reg              rd_user;
    reg              rd_last;
    reg [23:0]       rd_data;
    reg [23:0]       out_data;
    reg              out_valid;
    reg              out_user;
    reg              out_last;
    reg [ADDR_W-1:0] issue_x;

    wire source_advance = !out_valid || m_axis_tready;
    wire issue_last_pixel = (issue_addr == ROI_PIXELS_MINUS_1);
    wire issue_line_last = (issue_x == ROI_WIDTH_MINUS_1);

    always @(posedge clk) begin
        rd_data <= roi_valid ? roi_mem[rd_addr] : 24'h000000;

        if (rst) begin
            cap_x        <= {SRC_XW{1'b0}};
            cap_y        <= {SRC_YW{1'b0}};
            roi_valid    <= 1'b0;
            wr_addr_seq  <= {ADDR_W{1'b0}};
            rd_addr      <= {ADDR_W{1'b0}};
            issue_addr   <= {ADDR_W{1'b0}};
            issuing      <= 1'b0;
            issued_valid <= 1'b0;
            issued_user  <= 1'b0;
            issued_last  <= 1'b0;
            rd_valid     <= 1'b0;
            rd_user      <= 1'b0;
            rd_last      <= 1'b0;
            out_data     <= 24'h000000;
            out_valid    <= 1'b0;
            out_user     <= 1'b0;
            out_last     <= 1'b0;
            issue_x      <= {ADDR_W{1'b0}};
        end else begin
            if (capture_fire) begin
                if (capture_tuser) begin
                    cap_x <= {{(SRC_XW-1){1'b0}}, 1'b1};
                    cap_y <= {SRC_YW{1'b0}};
                end else if (capture_tlast || (cap_x == SRC_WIDTH_MINUS_1)) begin
                    cap_x <= {SRC_XW{1'b0}};
                    cap_y <= cap_y + {{(SRC_YW-1){1'b0}}, 1'b1};
                end else begin
                    cap_x <= cap_x + {{(SRC_XW-1){1'b0}}, 1'b1};
                end

                if (capture_tuser) begin
                    wr_addr_seq <= {ADDR_W{1'b0}};
                end

                if (in_roi) begin
                    roi_mem[wr_addr] <= capture_tdata;
                    wr_addr_seq <= roi_last_pixel ? {ADDR_W{1'b0}} :
                                   (wr_addr + {{(ADDR_W-1){1'b0}}, 1'b1});
                    if (roi_last_pixel) begin
                        roi_valid <= 1'b1;
                    end
                end
            end

            if (start_playback) begin
                rd_addr      <= {ADDR_W{1'b0}};
                issue_addr   <= {ADDR_W{1'b0}};
                issuing      <= 1'b1;
                issued_valid <= 1'b0;
                issued_user  <= 1'b0;
                issued_last  <= 1'b0;
                rd_valid     <= 1'b0;
                rd_user      <= 1'b0;
                rd_last      <= 1'b0;
                out_valid    <= 1'b0;
                out_user     <= 1'b0;
                out_last     <= 1'b0;
                out_data     <= 24'h000000;
                issue_x      <= {ADDR_W{1'b0}};
            end else if (source_advance) begin
                out_valid <= rd_valid;
                out_user  <= rd_user;
                out_last  <= rd_last;
                out_data  <= rd_valid ? rd_data : 24'h000000;

                rd_valid <= issued_valid;
                rd_user  <= issued_user;
                rd_last  <= issued_last;

                if (issuing) begin
                    rd_addr      <= issue_addr;
                    issued_valid <= 1'b1;
                    issued_user  <= (issue_addr == {ADDR_W{1'b0}});
                    issued_last  <= issue_line_last;

                    if (issue_last_pixel) begin
                        issuing    <= 1'b0;
                        issue_addr <= {ADDR_W{1'b0}};
                        issue_x    <= {ADDR_W{1'b0}};
                    end else begin
                        issue_addr <= issue_addr + {{(ADDR_W-1){1'b0}}, 1'b1};
                        issue_x    <= issue_line_last ? {ADDR_W{1'b0}} :
                                      (issue_x + {{(ADDR_W-1){1'b0}}, 1'b1});
                    end
                end else begin
                    issued_valid <= 1'b0;
                    issued_user  <= 1'b0;
                    issued_last  <= 1'b0;
                end
            end
        end
    end

    assign m_axis_tdata  = out_data;
    assign m_axis_tvalid = out_valid;
    assign m_axis_tlast  = out_valid && out_last;
    assign m_axis_tuser  = out_valid && out_user;

endmodule

`default_nettype wire
