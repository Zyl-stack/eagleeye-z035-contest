`timescale 1ns / 1ps
`default_nettype none

module fovea_axil_ctrl #(
    parameter C_S_AXI_DATA_WIDTH = 32,
    parameter C_S_AXI_ADDR_WIDTH = 6,
    parameter ROI_X_MAX          = 320,
    parameter ROI_Y_MAX          = 180
)(
    (* X_INTERFACE_INFO = "xilinx.com:signal:clock:1.0 S_AXI_ACLK CLK" *)
    (* X_INTERFACE_PARAMETER = "XIL_INTERFACENAME S_AXI_ACLK, ASSOCIATED_BUSIF S_AXI, ASSOCIATED_RESET s_axi_aresetn, FREQ_HZ 100000000" *)
    input  wire                             s_axi_aclk,
    (* X_INTERFACE_INFO = "xilinx.com:signal:reset:1.0 S_AXI_ARESETN RST" *)
    (* X_INTERFACE_PARAMETER = "XIL_INTERFACENAME S_AXI_ARESETN, POLARITY ACTIVE_LOW" *)
    input  wire                             s_axi_aresetn,

    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 S_AXI AWADDR" *)
    (* X_INTERFACE_PARAMETER = "XIL_INTERFACENAME S_AXI, DATA_WIDTH 32, PROTOCOL AXI4LITE, FREQ_HZ 100000000, ID_WIDTH 0, ADDR_WIDTH 6, AWUSER_WIDTH 0, ARUSER_WIDTH 0, WUSER_WIDTH 0, RUSER_WIDTH 0, BUSER_WIDTH 0, READ_WRITE_MODE READ_WRITE" *)
    input  wire [C_S_AXI_ADDR_WIDTH-1:0]    s_axi_awaddr,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 S_AXI AWVALID" *)
    input  wire                             s_axi_awvalid,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 S_AXI AWREADY" *)
    output reg                              s_axi_awready,

    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 S_AXI WDATA" *)
    input  wire [C_S_AXI_DATA_WIDTH-1:0]    s_axi_wdata,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 S_AXI WSTRB" *)
    input  wire [(C_S_AXI_DATA_WIDTH/8)-1:0] s_axi_wstrb,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 S_AXI WVALID" *)
    input  wire                             s_axi_wvalid,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 S_AXI WREADY" *)
    output reg                              s_axi_wready,

    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 S_AXI BRESP" *)
    output reg  [1:0]                       s_axi_bresp,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 S_AXI BVALID" *)
    output reg                              s_axi_bvalid,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 S_AXI BREADY" *)
    input  wire                             s_axi_bready,

    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 S_AXI ARADDR" *)
    input  wire [C_S_AXI_ADDR_WIDTH-1:0]    s_axi_araddr,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 S_AXI ARVALID" *)
    input  wire                             s_axi_arvalid,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 S_AXI ARREADY" *)
    output reg                              s_axi_arready,

    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 S_AXI RDATA" *)
    output reg  [C_S_AXI_DATA_WIDTH-1:0]    s_axi_rdata,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 S_AXI RRESP" *)
    output reg  [1:0]                       s_axi_rresp,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 S_AXI RVALID" *)
    output reg                              s_axi_rvalid,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 S_AXI RREADY" *)
    input  wire                             s_axi_rready,

    input  wire                             cmd_accept_i,
    input  wire [1:0]                       active_mode_i,

    output wire                             roi_cmd_valid_o,
    output reg  [9:0]                       roi_cmd_x_o,
    output reg  [8:0]                       roi_cmd_y_o,
    output reg  [15:0]                      roi_cmd_w_o,
    output reg  [15:0]                      roi_cmd_h_o,
    output reg  [15:0]                      target_id_o,
    output reg  [1:0]                       roi_cmd_mode_o,
    output reg  [15:0]                      output_buffer_o
);
    localparam REG_CONTROL         = 4'h0;
    localparam REG_STATUS          = 4'h1;
    localparam REG_REQUEST_ID      = 4'h2;
    localparam REG_SOURCE_FRAME_ID = 4'h3;
    localparam REG_TARGET_ID       = 4'h4;
    localparam REG_ROI_XY          = 4'h5;
    localparam REG_ROI_WH          = 4'h6;
    localparam REG_ENHANCE_MODE    = 4'h7;
    localparam REG_OUTPUT_BUFFER   = 4'h8;
    localparam REG_DEADLINE        = 4'h9;
    localparam REG_CYCLE_COUNT     = 4'hA;
    localparam REG_QUALITY_FLAGS   = 4'hB;

    localparam CTRL_ENABLE      = 0;
    localparam CTRL_COMMIT      = 1;
    localparam CTRL_CANCEL      = 2;
    localparam CTRL_CLEAR_ERROR = 3;

    localparam QF_ROI_CLIPPED          = 0;
    localparam QF_MODE_FALLBACK        = 1;
    localparam QF_TARGET_SWITCH        = 2;
    localparam QF_REQUEST_CANCELLED    = 3;
    localparam QF_FRAME_SAFE_LATCHED   = 4;
    localparam QF_OUTPUT_BUFFER_VALID  = 5;

    localparam [15:0] ROI_X_MAX_U16 = ROI_X_MAX;
    localparam [15:0] ROI_Y_MAX_U16 = ROI_Y_MAX;

    reg [C_S_AXI_ADDR_WIDTH-1:0] axi_awaddr;
    reg [C_S_AXI_ADDR_WIDTH-1:0] axi_araddr;

    reg        enable_reg;
    reg        pending_reg;
    reg        done_reg;
    reg        error_reg;
    reg [31:0] request_id_reg;
    reg [31:0] source_frame_id_reg;
    reg [31:0] deadline_reg;
    reg [31:0] cycle_count_reg;
    reg [31:0] quality_flags_reg;

    reg [15:0] roi_x_reg;
    reg [15:0] roi_y_reg;
    reg [15:0] roi_w_reg;
    reg [15:0] roi_h_reg;
    reg [1:0]  enhance_mode_reg;
    reg [15:0] output_buffer_reg;
    reg [15:0] target_id_reg;

    wire write_fire = s_axi_awready && s_axi_awvalid && s_axi_wready && s_axi_wvalid;
    wire read_fire  = s_axi_arready && s_axi_arvalid;
    wire [3:0] write_addr = axi_awaddr[5:2];
    wire [3:0] read_addr  = axi_araddr[5:2];

    wire roi_x_clip = (roi_x_reg > ROI_X_MAX_U16);
    wire roi_y_clip = (roi_y_reg > ROI_Y_MAX_U16);
    wire mode_bad   = (enhance_mode_reg > 2'd2);

    assign roi_cmd_valid_o = pending_reg && enable_reg;

    function [31:0] apply_wstrb;
        input [31:0] old_value;
        input [31:0] new_value;
        input [3:0]  wstrb;
        begin
            apply_wstrb[7:0]   = wstrb[0] ? new_value[7:0]   : old_value[7:0];
            apply_wstrb[15:8]  = wstrb[1] ? new_value[15:8]  : old_value[15:8];
            apply_wstrb[23:16] = wstrb[2] ? new_value[23:16] : old_value[23:16];
            apply_wstrb[31:24] = wstrb[3] ? new_value[31:24] : old_value[31:24];
        end
    endfunction

    function [15:0] clip_u16;
        input [15:0] value;
        input [15:0] max_value;
        begin
            clip_u16 = (value > max_value) ? max_value : value;
        end
    endfunction

    always @(posedge s_axi_aclk) begin
        if (!s_axi_aresetn) begin
            s_axi_awready <= 1'b0;
            s_axi_wready  <= 1'b0;
            axi_awaddr    <= {C_S_AXI_ADDR_WIDTH{1'b0}};
        end else begin
            if (!s_axi_awready && !s_axi_wready && s_axi_awvalid && s_axi_wvalid) begin
                s_axi_awready <= 1'b1;
                s_axi_wready  <= 1'b1;
                axi_awaddr    <= s_axi_awaddr;
            end else begin
                s_axi_awready <= 1'b0;
                s_axi_wready  <= 1'b0;
            end
        end
    end

    always @(posedge s_axi_aclk) begin
        if (!s_axi_aresetn) begin
            s_axi_bvalid <= 1'b0;
            s_axi_bresp  <= 2'b00;
        end else begin
            if (write_fire && !s_axi_bvalid) begin
                s_axi_bvalid <= 1'b1;
                s_axi_bresp  <= 2'b00;
            end else if (s_axi_bvalid && s_axi_bready) begin
                s_axi_bvalid <= 1'b0;
            end
        end
    end

    always @(posedge s_axi_aclk) begin
        if (!s_axi_aresetn) begin
            s_axi_arready <= 1'b0;
            axi_araddr    <= {C_S_AXI_ADDR_WIDTH{1'b0}};
        end else begin
            if (!s_axi_arready && s_axi_arvalid) begin
                s_axi_arready <= 1'b1;
                axi_araddr    <= s_axi_araddr;
            end else begin
                s_axi_arready <= 1'b0;
            end
        end
    end

    always @(posedge s_axi_aclk) begin
        if (!s_axi_aresetn) begin
            s_axi_rvalid <= 1'b0;
            s_axi_rresp  <= 2'b00;
            s_axi_rdata  <= 32'd0;
        end else begin
            if (read_fire && !s_axi_rvalid) begin
                s_axi_rvalid <= 1'b1;
                s_axi_rresp  <= 2'b00;
                case (read_addr)
                    REG_CONTROL:         s_axi_rdata <= {28'd0, 3'b000, enable_reg};
                    REG_STATUS:          s_axi_rdata <= {26'd0, active_mode_i, 1'b0, error_reg, done_reg, pending_reg};
                    REG_REQUEST_ID:      s_axi_rdata <= request_id_reg;
                    REG_SOURCE_FRAME_ID: s_axi_rdata <= source_frame_id_reg;
                    REG_TARGET_ID:       s_axi_rdata <= {16'd0, target_id_reg};
                    REG_ROI_XY:          s_axi_rdata <= {roi_y_reg, roi_x_reg};
                    REG_ROI_WH:          s_axi_rdata <= {roi_h_reg, roi_w_reg};
                    REG_ENHANCE_MODE:    s_axi_rdata <= {30'd0, enhance_mode_reg};
                    REG_OUTPUT_BUFFER:   s_axi_rdata <= {16'd0, output_buffer_reg};
                    REG_DEADLINE:        s_axi_rdata <= deadline_reg;
                    REG_CYCLE_COUNT:     s_axi_rdata <= cycle_count_reg;
                    REG_QUALITY_FLAGS:   s_axi_rdata <= quality_flags_reg;
                    default:             s_axi_rdata <= 32'd0;
                endcase
            end else if (s_axi_rvalid && s_axi_rready) begin
                s_axi_rvalid <= 1'b0;
            end
        end
    end

    always @(posedge s_axi_aclk) begin
        if (!s_axi_aresetn) begin
            enable_reg          <= 1'b0;
            pending_reg         <= 1'b0;
            done_reg            <= 1'b0;
            error_reg           <= 1'b0;
            request_id_reg      <= 32'd0;
            source_frame_id_reg <= 32'd0;
            deadline_reg        <= 32'd0;
            cycle_count_reg     <= 32'd0;
            quality_flags_reg   <= 32'd0;
            roi_x_reg           <= 16'd160;
            roi_y_reg           <= 16'd90;
            roi_w_reg           <= 16'd320;
            roi_h_reg           <= 16'd180;
            enhance_mode_reg    <= 2'd0;
            output_buffer_reg   <= 16'd0;
            target_id_reg       <= 16'd0;
            roi_cmd_x_o         <= 10'd160;
            roi_cmd_y_o         <= 9'd90;
            roi_cmd_w_o         <= 16'd320;
            roi_cmd_h_o         <= 16'd180;
            target_id_o         <= 16'd0;
            roi_cmd_mode_o      <= 2'd0;
            output_buffer_o     <= 16'd0;
        end else begin
            cycle_count_reg <= cycle_count_reg + 1'b1;

            if (cmd_accept_i && pending_reg) begin
                pending_reg <= 1'b0;
                done_reg    <= 1'b1;
                quality_flags_reg[QF_FRAME_SAFE_LATCHED] <= 1'b1;
            end

            if (write_fire) begin
                case (write_addr)
                    REG_CONTROL: begin
                        if (s_axi_wstrb[0]) begin
                            enable_reg <= s_axi_wdata[CTRL_ENABLE];

                            if (s_axi_wdata[CTRL_CLEAR_ERROR]) begin
                                error_reg <= 1'b0;
                            end

                            if (s_axi_wdata[CTRL_CANCEL]) begin
                                pending_reg <= 1'b0;
                                done_reg    <= 1'b0;
                                quality_flags_reg[QF_REQUEST_CANCELLED] <= 1'b1;
                            end

                            if (s_axi_wdata[CTRL_COMMIT]) begin
                                done_reg          <= 1'b0;
                                quality_flags_reg <= 32'd0;

                                if (!s_axi_wdata[CTRL_ENABLE] && !enable_reg) begin
                                    error_reg <= 1'b1;
                                end else begin
                                    pending_reg <= 1'b1;
                                    error_reg   <= 1'b0;

                                    roi_cmd_x_o    <= (roi_x_reg > ROI_X_MAX_U16) ? ROI_X_MAX_U16[9:0] : roi_x_reg[9:0];
                                    roi_cmd_y_o    <= (roi_y_reg > ROI_Y_MAX_U16) ? ROI_Y_MAX_U16[8:0] : roi_y_reg[8:0];
                                    roi_cmd_w_o    <= roi_w_reg;
                                    roi_cmd_h_o    <= roi_h_reg;
                                    target_id_o    <= target_id_reg;
                                    roi_cmd_mode_o <= mode_bad ? 2'd0 : enhance_mode_reg;
                                    output_buffer_o <= output_buffer_reg;

                                    quality_flags_reg[QF_ROI_CLIPPED] <= roi_x_clip || roi_y_clip;
                                    quality_flags_reg[QF_MODE_FALLBACK] <= mode_bad;
                                    quality_flags_reg[QF_OUTPUT_BUFFER_VALID] <= 1'b1;
                                    if (target_id_o != target_id_reg) begin
                                        quality_flags_reg[QF_TARGET_SWITCH] <= 1'b1;
                                    end
                                end
                            end
                        end
                    end

                    REG_REQUEST_ID:
                        request_id_reg <= apply_wstrb(request_id_reg, s_axi_wdata, s_axi_wstrb);

                    REG_SOURCE_FRAME_ID:
                        source_frame_id_reg <= apply_wstrb(source_frame_id_reg, s_axi_wdata, s_axi_wstrb);

                    REG_TARGET_ID:
                        if (s_axi_wstrb[0] || s_axi_wstrb[1]) begin
                            target_id_reg <= s_axi_wdata[15:0];
                        end

                    REG_ROI_XY: begin
                        if (s_axi_wstrb[0] || s_axi_wstrb[1]) begin
                            roi_x_reg <= s_axi_wdata[15:0];
                        end
                        if (s_axi_wstrb[2] || s_axi_wstrb[3]) begin
                            roi_y_reg <= s_axi_wdata[31:16];
                        end
                    end

                    REG_ROI_WH: begin
                        if (s_axi_wstrb[0] || s_axi_wstrb[1]) begin
                            roi_w_reg <= s_axi_wdata[15:0];
                        end
                        if (s_axi_wstrb[2] || s_axi_wstrb[3]) begin
                            roi_h_reg <= s_axi_wdata[31:16];
                        end
                    end

                    REG_ENHANCE_MODE:
                        if (s_axi_wstrb[0]) begin
                            enhance_mode_reg <= s_axi_wdata[1:0];
                        end

                    REG_OUTPUT_BUFFER:
                        if (s_axi_wstrb[0] || s_axi_wstrb[1]) begin
                            output_buffer_reg <= s_axi_wdata[15:0];
                        end

                    REG_DEADLINE:
                        deadline_reg <= apply_wstrb(deadline_reg, s_axi_wdata, s_axi_wstrb);

                    REG_CYCLE_COUNT:
                        cycle_count_reg <= apply_wstrb(cycle_count_reg, s_axi_wdata, s_axi_wstrb);

                    REG_QUALITY_FLAGS:
                        quality_flags_reg <= apply_wstrb(quality_flags_reg, s_axi_wdata, s_axi_wstrb);

                    default: begin
                    end
                endcase
            end
        end
    end
endmodule

`default_nettype wire
