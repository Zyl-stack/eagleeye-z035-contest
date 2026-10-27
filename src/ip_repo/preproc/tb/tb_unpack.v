`timescale 1ns/1ps
module tb_unpack;
    reg clk=0,resetn=0,start=0;
    always #5 clk=~clk;
    reg [31:0] words [0:172799];
    reg [23:0] pixels [0:230399];
    reg [1023:0] word_file,pixel_file;
    integer wi=0,pi=0,tick=0;
    wire s_ready,m_valid,m_last,m_user;
    wire [23:0] m_data;
    wire [2:0] m_keep;
    wire [1:0] error;
    wire m_ready=tick%9!=0;
    rgb888_mm2s_adapter dut(
      .aclk(clk),.aresetn(resetn),.start(start),
      .s_tdata(words[wi]),.s_tkeep(4'hf),.s_tvalid(wi<172800),
      .s_tready(s_ready),.s_tlast(wi==172799),
      .m_tdata(m_data),.m_tkeep(m_keep),.m_tvalid(m_valid),
      .m_tready(m_ready),.m_tlast(m_last),.m_tuser(m_user),.error(error)
    );
    initial begin
      if(!$value$plusargs("WORDS=%s",word_file)) $fatal(1,"missing WORDS");
      if(!$value$plusargs("PIXELS=%s",pixel_file)) $fatal(1,"missing PIXELS");
      $readmemh(word_file,words); $readmemh(pixel_file,pixels);
      repeat(5) @(negedge clk); resetn=1;
      @(negedge clk); start=1;
      @(negedge clk); start=0;
      wait(pi==230400);
      @(negedge clk);
      if(wi!=172800 || error) $fatal(1,"incomplete/error wi=%d err=%h",wi,error);
      $display("PASS unpack 691200 bytes -> 230400 RGB pixels, cycles=%d",tick);
      $finish;
    end
    always @(posedge clk) if(resetn) begin
      tick<=tick+1;
      if(tick>600000) $fatal(1,"timeout");
      if(wi<172800 && s_ready) wi<=wi+1;
      if(m_valid && m_ready) begin
        if(m_data!==pixels[pi] || m_keep!=3'b111 ||
           m_user!=(pi==0) || m_last!=((pi%640)==639))
          $fatal(1,"pixel mismatch at %d got=%h expected=%h",pi,m_data,pixels[pi]);
        pi<=pi+1;
      end
    end
endmodule
