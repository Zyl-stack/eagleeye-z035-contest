`timescale 1ns/1ps
module tb_preproc;
    reg clk=0, resetn=0, start=0;
    always #5 clk=~clk;
    reg [23:0] input_mem [0:230399];
    integer in_idx=0, r_idx=0, g_idx=0, b_idx=0, tick=0;
    integer fr,fg,fb;
    reg [1023:0] input_file, output_prefix;
    wire busy,done;
    wire [3:0] error;
    wire [31:0] cycles,input_pixels,output_pixels;
    wire s_ready;
    wire [31:0] r_data,g_data,b_data;
    wire [3:0] r_keep,g_keep,b_keep;
    wire r_valid,g_valid,b_valid,r_last,g_last,b_last,r_user,g_user,b_user;
    wire r_ready=(tick%7!=0),g_ready=(tick%11!=0),b_ready=(tick%13!=0);
    wire s_valid=in_idx<230400;
    wire [23:0] s_data=input_mem[in_idx];
    wire s_last=(in_idx%640)==639;
    wire s_user=in_idx==0;
    preproc_axis_core dut(
        .aclk(clk),.aresetn(resetn),.start(start),.busy(busy),
        .done_pulse(done),.error(error),.cycles(cycles),
        .input_pixels(input_pixels),.output_pixels(output_pixels),
        .s_axis_tdata(s_data),.s_axis_tkeep(3'b111),
        .s_axis_tvalid(s_valid),.s_axis_tready(s_ready),
        .s_axis_tlast(s_last),.s_axis_tuser(s_user),
        .m_axis_r_tdata(r_data),.m_axis_g_tdata(g_data),.m_axis_b_tdata(b_data),
        .m_axis_r_tkeep(r_keep),.m_axis_g_tkeep(g_keep),.m_axis_b_tkeep(b_keep),
        .m_axis_r_tvalid(r_valid),.m_axis_g_tvalid(g_valid),.m_axis_b_tvalid(b_valid),
        .m_axis_r_tready(r_ready),.m_axis_g_tready(g_ready),.m_axis_b_tready(b_ready),
        .m_axis_r_tlast(r_last),.m_axis_g_tlast(g_last),.m_axis_b_tlast(b_last),
        .m_axis_r_tuser(r_user),.m_axis_g_tuser(g_user),.m_axis_b_tuser(b_user)
    );
    initial begin
        if (!$value$plusargs("INPUT=%s",input_file)) $fatal(1,"missing +INPUT=");
        if (!$value$plusargs("OUTPUT=%s",output_prefix)) $fatal(1,"missing +OUTPUT=");
        $readmemh(input_file,input_mem);
        fr=$fopen({output_prefix,"_r.hex"},"w");
        fg=$fopen({output_prefix,"_g.hex"},"w");
        fb=$fopen({output_prefix,"_b.hex"},"w");
        if (!fr || !fg || !fb) $fatal(1,"cannot open output");
        repeat(5) @(negedge clk);
        resetn=1;
        @(negedge clk); start=1;
        @(negedge clk); start=0;
        wait(done);
        @(negedge clk);
        if (error || input_pixels!=230400 || output_pixels!=102400 ||
            in_idx!=230400 || r_idx!=102400 || g_idx!=102400 || b_idx!=102400)
            $fatal(1,"count/status mismatch err=%h in=%d out=%d r=%d g=%d b=%d",error,in_idx,output_pixels,r_idx,g_idx,b_idx);
        $display("PASS cycles=%d latency_ms_at_100MHz=%f",cycles,cycles/100000.0);
        $fclose(fr);$fclose(fg);$fclose(fb);
        $finish;
    end
    always @(posedge clk) if(resetn) begin
        tick<=tick+1;
        if(tick>1200000) $fatal(1,"timeout");
        if(s_valid && s_ready) in_idx<=in_idx+1;
        if(r_valid && r_ready) begin
            if(r_keep!=4'hf || r_last!=(r_idx==102399) || r_user!=(r_idx==0))
                $fatal(1,"R sideband at %d",r_idx);
            $fdisplay(fr,"%08x",r_data); r_idx<=r_idx+1;
        end
        if(g_valid && g_ready) begin
            if(g_keep!=4'hf || g_last!=(g_idx==102399) || g_user!=(g_idx==0))
                $fatal(1,"G sideband at %d",g_idx);
            $fdisplay(fg,"%08x",g_data); g_idx<=g_idx+1;
        end
        if(b_valid && b_ready) begin
            if(b_keep!=4'hf || b_last!=(b_idx==102399) || b_user!=(b_idx==0))
                $fatal(1,"B sideband at %d",b_idx);
            $fdisplay(fb,"%08x",b_data); b_idx<=b_idx+1;
        end
    end
endmodule
