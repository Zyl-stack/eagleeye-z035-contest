`timescale 1ns/1ps
module tb_control;
  reg clk=0,resetn=0;
  always #5 clk=~clk;
  reg [5:0] awaddr=0,araddr=0;
  reg awvalid=0,wvalid=0,arvalid=0;
  reg [31:0] wdata=0;
  reg [3:0] wstrb=0;
  wire awready,wready,arready,bvalid,rvalid,irq;
  wire [1:0] bresp,rresp;
  wire [31:0] rdata;
  picodet_preproc_ip dut(
    .aclk(clk),.aresetn(resetn),
    .s_axi_awaddr(awaddr),.s_axi_awvalid(awvalid),.s_axi_awready(awready),
    .s_axi_wdata(wdata),.s_axi_wstrb(wstrb),.s_axi_wvalid(wvalid),.s_axi_wready(wready),
    .s_axi_bresp(bresp),.s_axi_bvalid(bvalid),.s_axi_bready(1'b1),
    .s_axi_araddr(araddr),.s_axi_arvalid(arvalid),.s_axi_arready(arready),
    .s_axi_rdata(rdata),.s_axi_rresp(rresp),.s_axi_rvalid(rvalid),.s_axi_rready(1'b1),
    .s_axis_tdata(32'b0),.s_axis_tkeep(4'hf),.s_axis_tvalid(1'b0),
    .s_axis_tready(),.s_axis_tlast(1'b0),
    .m_axis_r_tdata(),.m_axis_g_tdata(),.m_axis_b_tdata(),
    .m_axis_r_tkeep(),.m_axis_g_tkeep(),.m_axis_b_tkeep(),
    .m_axis_r_tvalid(),.m_axis_g_tvalid(),.m_axis_b_tvalid(),
    .m_axis_r_tready(1'b1),.m_axis_g_tready(1'b1),.m_axis_b_tready(1'b1),
    .m_axis_r_tlast(),.m_axis_g_tlast(),.m_axis_b_tlast(),
    .m_axis_r_tuser(),.m_axis_g_tuser(),.m_axis_b_tuser(),.irq(irq)
  );
  initial begin
    repeat(5) @(negedge clk); resetn=1;
    @(negedge clk); awaddr=0; awvalid=1;
    @(negedge clk); awvalid=0;
    repeat(2) @(negedge clk); wdata=1;wstrb=4'h1;wvalid=1;
    @(negedge clk);wvalid=0;
    repeat(4) @(negedge clk);
    araddr=6'h04;arvalid=1;
    @(negedge clk);arvalid=0;
    if(!rvalid || !rdata[0] || rdata[1] || rresp!=0)
      $fatal(1,"AXI-Lite start/status failed: %h",rdata);
    $display("PASS AXI-Lite independent AW/W and busy status");
    $finish;
  end
endmodule
