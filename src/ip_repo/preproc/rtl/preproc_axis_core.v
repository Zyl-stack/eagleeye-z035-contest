`timescale 1ns/1ps
// Fixed 640x360 RGB888 to 3 x 320x320 FP32 NCHW-plane AXI streams.
// Input byte lanes: [7:0]=R, [15:8]=G, [23:16]=B.
module preproc_axis_core (
    input wire aclk, input wire aresetn, input wire start,
    output reg busy, output reg done_pulse, output reg [3:0] error,
    output reg [31:0] cycles,
    input wire [23:0] s_axis_tdata, input wire [2:0] s_axis_tkeep,
    input wire s_axis_tvalid, output wire s_axis_tready,
    input wire s_axis_tlast, input wire s_axis_tuser,
    output wire [31:0] m_axis_r_tdata, m_axis_g_tdata, m_axis_b_tdata,
    output wire [3:0] m_axis_r_tkeep, m_axis_g_tkeep, m_axis_b_tkeep,
    output wire m_axis_r_tvalid, m_axis_g_tvalid, m_axis_b_tvalid,
    input wire m_axis_r_tready, m_axis_g_tready, m_axis_b_tready,
    output wire m_axis_r_tlast, m_axis_g_tlast, m_axis_b_tlast,
    output wire m_axis_r_tuser, m_axis_g_tuser, m_axis_b_tuser,
    output reg [31:0] input_pixels, output reg [31:0] output_pixels
);
    localparam IDLE=3'd0, LOAD=3'd1, READ=3'd2,
               CALC=3'd3, ROM_WAIT=3'd4, OUT=3'd5, FINISH=3'd6;
    reg [2:0] state;
    reg [9:0] sx;
    reg [8:0] sy, dy;
    reg [8:0] dx;
    wire [12:0] ycoord = (dy * 13'd18) + 13'd1;
    wire [8:0] y0 = ycoord[12:4];
    wire [8:0] y1 = (y0 == 9'd359) ? 9'd359 : y0 + 9'd1;
    wire [3:0] fy = ycoord[3:0];
    reg [23:0] line00 [0:319], line01 [0:319];
    reg [23:0] line10 [0:319], line11 [0:319];
    reg [23:0] q00,q01,q10,q11;
    reg [12:0] sum_r,sum_g,sum_b;
    (* rom_style = "block" *) reg [31:0] lut_r [0:8160];
    (* rom_style = "block" *) reg [31:0] lut_g [0:8160];
    (* rom_style = "block" *) reg [31:0] lut_b [0:8160];
    reg [31:0] val_r,val_g,val_b;
    reg sent_r,sent_g,sent_b;
    wire take_r = (state==OUT) && !sent_r && m_axis_r_tready;
    wire take_g = (state==OUT) && !sent_g && m_axis_g_tready;
    wire take_b = (state==OUT) && !sent_b && m_axis_b_tready;
    wire all_taken = (sent_r || take_r) && (sent_g || take_g) && (sent_b || take_b);
    wire [23:0] top0 = y0[0] ? q10 : q00;
    wire [23:0] top1 = y0[0] ? q11 : q01;
    wire [23:0] bot0 = y0[0] ? q00 : q10;
    wire [23:0] bot1 = y0[0] ? q01 : q11;
    wire [4:0] inv_fy = 5'd16 - {1'b0,fy};

    initial begin
        $readmemh("normalize_r.mem", lut_r);
        $readmemh("normalize_g.mem", lut_g);
        $readmemh("normalize_b.mem", lut_b);
    end

    assign s_axis_tready = busy && state==LOAD;
    assign m_axis_r_tdata = val_r;
    assign m_axis_g_tdata = val_g;
    assign m_axis_b_tdata = val_b;
    assign m_axis_r_tkeep = 4'hf;
    assign m_axis_g_tkeep = 4'hf;
    assign m_axis_b_tkeep = 4'hf;
    assign m_axis_r_tvalid = state==OUT && !sent_r;
    assign m_axis_g_tvalid = state==OUT && !sent_g;
    assign m_axis_b_tvalid = state==OUT && !sent_b;
    assign m_axis_r_tlast = dy==9'd319 && dx==9'd319;
    assign m_axis_g_tlast = m_axis_r_tlast;
    assign m_axis_b_tlast = m_axis_r_tlast;
    assign m_axis_r_tuser = dy==0 && dx==0;
    assign m_axis_g_tuser = m_axis_r_tuser;
    assign m_axis_b_tuser = m_axis_r_tuser;

    always @(posedge aclk) begin
        // Four small 24-bit synchronous-read RAMs, split by row/pixel parity.
        q00 <= line00[dx]; q01 <= line01[dx];
        q10 <= line10[dx]; q11 <= line11[dx];
        if (s_axis_tvalid && s_axis_tready) begin
            if (!sy[0] && !sx[0]) line00[sx[9:1]] <= s_axis_tdata;
            if (!sy[0] &&  sx[0]) line01[sx[9:1]] <= s_axis_tdata;
            if ( sy[0] && !sx[0]) line10[sx[9:1]] <= s_axis_tdata;
            if ( sy[0] &&  sx[0]) line11[sx[9:1]] <= s_axis_tdata;
        end
        val_r <= lut_r[sum_r]; val_g <= lut_g[sum_g]; val_b <= lut_b[sum_b];
    end

    always @(posedge aclk) begin
        if (!aresetn) begin
            state<=IDLE; busy<=0; done_pulse<=0; error<=0; cycles<=0;
            sx<=0; sy<=0; dx<=0; dy<=0;
            sum_r<=0; sum_g<=0; sum_b<=0;
            sent_r<=0; sent_g<=0; sent_b<=0;
            input_pixels<=0; output_pixels<=0;
        end else begin
            done_pulse<=0;
            if (busy) cycles<=cycles+1'b1;
            case (state)
                IDLE: if (start) begin
                    state<=LOAD; busy<=1; error<=0; cycles<=0;
                    sx<=0; sy<=0; dx<=0; dy<=0;
                    sent_r<=0; sent_g<=0; sent_b<=0;
                    input_pixels<=0; output_pixels<=0;
                end
                LOAD: if (s_axis_tvalid && s_axis_tready) begin
                    input_pixels<=input_pixels+1'b1;
                    if (s_axis_tkeep!=3'b111) error[0]<=1;
                    if (s_axis_tuser != (sx==0 && sy==0)) error[1]<=1;
                    if (s_axis_tlast != (sx==10'd639)) error[2]<=1;
                    if (sx==10'd639) begin
                        sx<=0; sy<=sy+1'b1;
                        if (sy==y1) state<=READ;
                    end else sx<=sx+1'b1;
                end
                READ: state<=CALC;
                CALC: begin
                    sum_r <= (({1'b0,top0[7:0]}+{1'b0,top1[7:0]})*inv_fy)
                           + (({1'b0,bot0[7:0]}+{1'b0,bot1[7:0]})*fy);
                    sum_g <= (({1'b0,top0[15:8]}+{1'b0,top1[15:8]})*inv_fy)
                           + (({1'b0,bot0[15:8]}+{1'b0,bot1[15:8]})*fy);
                    sum_b <= (({1'b0,top0[23:16]}+{1'b0,top1[23:16]})*inv_fy)
                           + (({1'b0,bot0[23:16]}+{1'b0,bot1[23:16]})*fy);
                    state<=ROM_WAIT;
                end
                ROM_WAIT: begin
                    sent_r<=0; sent_g<=0; sent_b<=0;
                    state<=OUT;
                end
                OUT: begin
                    if (take_r) sent_r<=1;
                    if (take_g) sent_g<=1;
                    if (take_b) sent_b<=1;
                    if (all_taken) begin
                        output_pixels<=output_pixels+1'b1;
                        if (dx==9'd319) begin
                            dx<=0;
                            if (dy==9'd319) state<=FINISH;
                            else begin
                                dy<=dy+1'b1;
                                if (sy>(((dy+1'b1)*13'd18+13'd1)>>4)+1'b1)
                                    state<=READ;
                                else state<=LOAD;
                            end
                        end else begin dx<=dx+1'b1; state<=READ; end
                    end
                end
                FINISH: begin
                    busy<=0; done_pulse<=1;
                    if (input_pixels!=32'd230400 || output_pixels!=32'd102400)
                        error[3]<=1;
                    state<=IDLE;
                end
                default: state<=IDLE;
            endcase
        end
    end
endmodule
