`timescale 1ns / 1ps

// Integer/integer phase.  Four registered stages keep its valid latency
// identical to core1/core2/core3 without synthesizing a redundant 36-MAC.
module lanczos_scaler_core0 #(
    parameter PIXEL_WIDTH = 8
)(
    input  wire                   sys_clk,
    input  wire                   sys_rst,
    input  wire [PIXEL_WIDTH-1:0] i_pix_center,
    input  wire                   i_valid,
    output reg  [PIXEL_WIDTH-1:0] o_pix,
    output reg                    o_valid
);
    reg [PIXEL_WIDTH-1:0] s1_pix, s2_pix, s3_pix, s4_pix, s5_pix;
    reg s1_valid, s2_valid, s3_valid, s4_valid, s5_valid;

    always @(posedge sys_clk) begin
        if (sys_rst) begin
            s1_pix   <= {PIXEL_WIDTH{1'b0}};
            s2_pix   <= {PIXEL_WIDTH{1'b0}};
            s3_pix   <= {PIXEL_WIDTH{1'b0}};
            s4_pix   <= {PIXEL_WIDTH{1'b0}};
            s5_pix   <= {PIXEL_WIDTH{1'b0}};
            o_pix    <= {PIXEL_WIDTH{1'b0}};
            s1_valid <= 1'b0;
            s2_valid <= 1'b0;
            s3_valid <= 1'b0;
            s4_valid <= 1'b0;
            s5_valid <= 1'b0;
            o_valid  <= 1'b0;
        end else begin
            s1_pix   <= i_pix_center;
            s2_pix   <= s1_pix;
            s3_pix   <= s2_pix;
            s4_pix   <= s3_pix;
            s5_pix   <= s4_pix;
            o_pix    <= s5_pix;
            s1_valid <= i_valid;
            s2_valid <= s1_valid;
            s3_valid <= s2_valid;
            s4_valid <= s3_valid;
            s5_valid <= s4_valid;
            o_valid  <= s5_valid;
        end
    end
endmodule
