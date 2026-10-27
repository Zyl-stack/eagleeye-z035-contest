`timescale 1ns/1ps
// Repack 32-bit memory-order bytes into 24-bit RGB pixels.
// AXI DMA MM2S TLAST marks the whole 691200-byte frame. Output TLAST marks EOL.
module rgb888_mm2s_adapter (
    input wire aclk, input wire aresetn, input wire start,
    input wire [31:0] s_tdata, input wire [3:0] s_tkeep,
    input wire s_tvalid, output wire s_tready, input wire s_tlast,
    output wire [23:0] m_tdata, output wire [2:0] m_tkeep,
    output wire m_tvalid, input wire m_tready,
    output wire m_tlast, output wire m_tuser,
    output reg [1:0] error
);
    reg active;
    reg [63:0] bytes_buf;
    reg [3:0] byte_count;
    reg [17:0] words_in;
    reg [17:0] pixels_out;
    wire pop = m_tvalid && m_tready;
    wire push = s_tvalid && s_tready;
    reg [63:0] next_buf;
    reg [3:0] next_count;
    always @* begin
        next_buf = pop ? (bytes_buf >> 24) : bytes_buf;
        next_count = pop ? byte_count - 4'd3 : byte_count;
        if (push) begin
            next_buf = next_buf | ({32'b0,s_tdata} << (next_count*8));
            next_count = next_count + 4'd4;
        end
    end
    assign s_tready = active && byte_count<=4'd3 && words_in<18'd172800;
    assign m_tvalid = active && byte_count>=4'd3 && pixels_out<18'd230400;
    assign m_tdata = bytes_buf[23:0];
    assign m_tkeep = 3'b111;
    assign m_tuser = pixels_out==0;
    assign m_tlast = (pixels_out%18'd640)==18'd639;
    always @(posedge aclk) begin
        if (!aresetn) begin
            active<=0; bytes_buf<=0; byte_count<=0;
            words_in<=0; pixels_out<=0; error<=0;
        end else if (start) begin
            active<=1; bytes_buf<=0; byte_count<=0;
            words_in<=0; pixels_out<=0; error<=0;
        end else begin
            bytes_buf<=next_buf;
            byte_count<=next_count;
            if (push) begin
                words_in<=words_in+1'b1;
                if (s_tkeep!=4'hf) error[0]<=1;
                if (s_tlast != (words_in==18'd172799)) error[1]<=1;
            end
            if (pop) begin
                pixels_out<=pixels_out+1'b1;
                if (pixels_out==18'd230399) active<=0;
            end
        end
    end
endmodule
