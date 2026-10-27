`timescale 1ns / 1ps
`default_nettype none

module roi_servo_pwm #(
    parameter integer CLK_HZ = 100_000_000,
    parameter integer STARTUP_FRAMES = 50,
    parameter integer REVERSE_PAN = 0,
    parameter integer REVERSE_TILT = 0
)(
    (* X_INTERFACE_INFO = "xilinx.com:signal:clock:1.0 clk CLK" *)
    (* X_INTERFACE_PARAMETER = "FREQ_HZ 100000000, ASSOCIATED_RESET rst" *)
    input  wire       clk,
    (* X_INTERFACE_INFO = "xilinx.com:signal:reset:1.0 rst RST" *)
    (* X_INTERFACE_PARAMETER = "POLARITY ACTIVE_HIGH" *)
    input  wire       rst,
    input  wire [9:0] roi_x,
    input  wire [8:0] roi_y,
    output wire       pan_pwm,
    output wire       tilt_pwm
);
    localparam integer TICKS_PER_US = CLK_HZ / 1_000_000;
    localparam integer DIV_W = (TICKS_PER_US <= 1) ? 1 : $clog2(TICKS_PER_US);
    localparam integer START_W = (STARTUP_FRAMES <= 1) ? 1 : $clog2(STARTUP_FRAMES + 1);
    localparam [11:0] MIN_US = 12'd1300;
    localparam [11:0] CENTER_US = 12'd1500;
    localparam [11:0] MAX_US = 12'd1700;
    localparam [11:0] STEP_US = 12'd10;

    reg [DIV_W-1:0] div_count;
    reg [14:0] us_count;
    reg [START_W-1:0] startup_count;
    reg [11:0] pan_us;
    reg [11:0] tilt_us;

    wire [9:0] x_safe = (roi_x > 10'd320) ? 10'd320 : roi_x;
    wire [8:0] y_safe = (roi_y > 9'd180) ? 9'd180 : roi_y;
    wire [11:0] x_times_five = ({2'b00, x_safe} << 2) + {2'b00, x_safe};
    wire [11:0] y_scaled = ({3'b000, y_safe} << 1) + ({3'b000, y_safe} >> 2);
    wire [11:0] pan_raw = MIN_US + (x_times_five >> 2);
    wire [11:0] tilt_unclamped = MIN_US + y_scaled;
    wire [11:0] tilt_raw = (tilt_unclamped > MAX_US) ? MAX_US : tilt_unclamped;
    wire [11:0] pan_target = REVERSE_PAN ? (MIN_US + MAX_US - pan_raw) : pan_raw;
    wire [11:0] tilt_target = REVERSE_TILT ? (MIN_US + MAX_US - tilt_raw) : tilt_raw;

    always @(posedge clk) begin
        if (rst) begin
            div_count <= 0;
            us_count <= 0;
            startup_count <= 0;
            pan_us <= CENTER_US;
            tilt_us <= CENTER_US;
        end else if (div_count == TICKS_PER_US - 1) begin
            div_count <= 0;
            if (us_count == 15'd19999) begin
                us_count <= 0;
                if (startup_count < STARTUP_FRAMES) begin
                    startup_count <= startup_count + 1'b1;
                end else begin
                    if (pan_target > pan_us + STEP_US)
                        pan_us <= pan_us + STEP_US;
                    else if (pan_us > pan_target + STEP_US)
                        pan_us <= pan_us - STEP_US;
                    else
                        pan_us <= pan_target;

                    if (tilt_target > tilt_us + STEP_US)
                        tilt_us <= tilt_us + STEP_US;
                    else if (tilt_us > tilt_target + STEP_US)
                        tilt_us <= tilt_us - STEP_US;
                    else
                        tilt_us <= tilt_target;
                end
            end else begin
                us_count <= us_count + 1'b1;
            end
        end else begin
            div_count <= div_count + 1'b1;
        end
    end

    assign pan_pwm = !rst && (us_count < pan_us);
    assign tilt_pwm = !rst && (us_count < tilt_us);
endmodule

`default_nettype wire
