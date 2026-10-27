`timescale 1ns/1ps
module picodet_preproc_ip (
    input wire aclk, input wire aresetn,
    input wire [5:0] s_axi_awaddr, input wire s_axi_awvalid, output wire s_axi_awready,
    input wire [31:0] s_axi_wdata, input wire [3:0] s_axi_wstrb,
    input wire s_axi_wvalid, output wire s_axi_wready,
    output reg [1:0] s_axi_bresp, output reg s_axi_bvalid, input wire s_axi_bready,
    input wire [5:0] s_axi_araddr, input wire s_axi_arvalid, output wire s_axi_arready,
    output reg [31:0] s_axi_rdata, output reg [1:0] s_axi_rresp,
    output reg s_axi_rvalid, input wire s_axi_rready,
    input wire [31:0] s_axis_tdata, input wire [3:0] s_axis_tkeep,
    input wire s_axis_tvalid, output wire s_axis_tready,
    input wire s_axis_tlast,
    output wire [31:0] m_axis_r_tdata, m_axis_g_tdata, m_axis_b_tdata,
    output wire [3:0] m_axis_r_tkeep, m_axis_g_tkeep, m_axis_b_tkeep,
    output wire m_axis_r_tvalid, m_axis_g_tvalid, m_axis_b_tvalid,
    input wire m_axis_r_tready, m_axis_g_tready, m_axis_b_tready,
    output wire m_axis_r_tlast, m_axis_g_tlast, m_axis_b_tlast,
    output wire m_axis_r_tuser, m_axis_g_tuser, m_axis_b_tuser,
    output wire irq
);
    reg aw_pending,w_pending;
    reg [5:0] aw_addr;
    reg [31:0] w_data;
    reg [3:0] w_strb;
    reg start_pulse, done_sticky, start_busy_error;
    wire busy,done_pulse;
    wire [3:0] core_error;
    wire [1:0] adapter_error;
    wire [23:0] pixel_data;
    wire [2:0] pixel_keep;
    wire pixel_valid,pixel_ready,pixel_last,pixel_user;
    wire [31:0] cycles,input_pixels,output_pixels;
    assign s_axi_awready = !aw_pending && !s_axi_bvalid;
    assign s_axi_wready = !w_pending && !s_axi_bvalid;
    assign s_axi_arready = !s_axi_rvalid;
    assign irq = done_sticky;

    rgb888_mm2s_adapter u_unpack (
        .aclk(aclk),.aresetn(aresetn),.start(start_pulse),
        .s_tdata(s_axis_tdata),.s_tkeep(s_axis_tkeep),
        .s_tvalid(s_axis_tvalid),.s_tready(s_axis_tready),.s_tlast(s_axis_tlast),
        .m_tdata(pixel_data),.m_tkeep(pixel_keep),
        .m_tvalid(pixel_valid),.m_tready(pixel_ready),
        .m_tlast(pixel_last),.m_tuser(pixel_user),.error(adapter_error)
    );

    preproc_axis_core u_core (
        .aclk(aclk),.aresetn(aresetn),.start(start_pulse),
        .busy(busy),.done_pulse(done_pulse),.error(core_error),.cycles(cycles),
        .input_pixels(input_pixels),.output_pixels(output_pixels),
        .s_axis_tdata(pixel_data),.s_axis_tkeep(pixel_keep),
        .s_axis_tvalid(pixel_valid),.s_axis_tready(pixel_ready),
        .s_axis_tlast(pixel_last),.s_axis_tuser(pixel_user),
        .m_axis_r_tdata(m_axis_r_tdata),.m_axis_g_tdata(m_axis_g_tdata),
        .m_axis_b_tdata(m_axis_b_tdata),
        .m_axis_r_tkeep(m_axis_r_tkeep),.m_axis_g_tkeep(m_axis_g_tkeep),
        .m_axis_b_tkeep(m_axis_b_tkeep),
        .m_axis_r_tvalid(m_axis_r_tvalid),.m_axis_g_tvalid(m_axis_g_tvalid),
        .m_axis_b_tvalid(m_axis_b_tvalid),
        .m_axis_r_tready(m_axis_r_tready),.m_axis_g_tready(m_axis_g_tready),
        .m_axis_b_tready(m_axis_b_tready),
        .m_axis_r_tlast(m_axis_r_tlast),.m_axis_g_tlast(m_axis_g_tlast),
        .m_axis_b_tlast(m_axis_b_tlast),
        .m_axis_b_tuser(m_axis_b_tuser),.m_axis_g_tuser(m_axis_g_tuser),
        .m_axis_r_tuser(m_axis_r_tuser)
    );
    always @(posedge aclk) begin
        if (!aresetn) begin
            aw_pending<=0; w_pending<=0; aw_addr<=0; w_data<=0; w_strb<=0;
            s_axi_bvalid<=0; s_axi_bresp<=0;
            s_axi_rvalid<=0; s_axi_rresp<=0; s_axi_rdata<=0;
            start_pulse<=0; done_sticky<=0; start_busy_error<=0;
        end else begin
            start_pulse<=0;
            if (done_pulse) done_sticky<=1;
            if (s_axi_awvalid && s_axi_awready) begin
                aw_addr<=s_axi_awaddr; aw_pending<=1;
            end
            if (s_axi_wvalid && s_axi_wready) begin
                w_data<=s_axi_wdata; w_strb<=s_axi_wstrb; w_pending<=1;
            end
            if (aw_pending && w_pending && !s_axi_bvalid) begin
                aw_pending<=0; w_pending<=0;
                s_axi_bvalid<=1; s_axi_bresp<=2'b00;
                if (aw_addr==6'h00 && w_strb[0] && w_data[0]) begin
                    if (busy || start_pulse) start_busy_error<=1;
                    else begin start_pulse<=1; done_sticky<=0; start_busy_error<=0; end
                end else if (aw_addr==6'h04 && w_strb[0] && w_data[1])
                    done_sticky<=0;
            end
            if (s_axi_bvalid && s_axi_bready) s_axi_bvalid<=0;
            if (s_axi_arvalid && s_axi_arready) begin
                s_axi_rvalid<=1; s_axi_rresp<=2'b00;
                case (s_axi_araddr[5:2])
                    4'h0: s_axi_rdata<=32'b0;
                    4'h1: s_axi_rdata<={23'b0,adapter_error,start_busy_error,core_error,done_sticky,busy};
                    4'h2: s_axi_rdata<=cycles;
                    4'h3: s_axi_rdata<=input_pixels;
                    4'h4: s_axi_rdata<=output_pixels;
                    4'h5: s_axi_rdata<=32'h0001_0000;
                    default: begin s_axi_rdata<=0; s_axi_rresp<=2'b10; end
                endcase
            end
            if (s_axi_rvalid && s_axi_rready) s_axi_rvalid<=0;
        end
    end
endmodule
