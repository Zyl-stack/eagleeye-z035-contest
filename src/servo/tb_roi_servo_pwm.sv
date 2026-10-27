`timescale 1ns / 1ps

module tb_roi_servo_pwm;
    reg clk = 0;
    reg rst = 1;
    reg [9:0] roi_x = 320;
    reg [8:0] roi_y = 180;
    wire pan_pwm;
    wire tilt_pwm;

    always #5 clk = ~clk;

    roi_servo_pwm #(.CLK_HZ(1_000_000), .STARTUP_FRAMES(1)) dut (
        .clk(clk), .rst(rst), .roi_x(roi_x), .roi_y(roi_y),
        .pan_pwm(pan_pwm), .tilt_pwm(tilt_pwm)
    );

    task period;
        repeat (20000) @(posedge clk);
        #1;
    endtask

    initial begin
        repeat (3) @(negedge clk);
        if (pan_pwm || tilt_pwm) $fatal(1, "reset must hold both outputs low");
        rst = 0;
        #1;
        if (!pan_pwm || !tilt_pwm) $fatal(1, "center pulses did not start");

        period();
        if (dut.pan_us !== 1500 || dut.tilt_us !== 1500)
            $fatal(1, "startup center hold failed");
        period();
        if (dut.pan_us !== 1510 || dut.tilt_us !== 1510)
            $fatal(1, "maximum step toward ROI failed");

        roi_x = 0;
        roi_y = 0;
        period();
        if (dut.pan_us !== 1500 || dut.tilt_us !== 1500)
            $fatal(1, "ROI reverse slew failed");
        if (dut.us_count !== 0 || !pan_pwm || !tilt_pwm)
            $fatal(1, "20 ms PWM boundary failed");

        repeat (1500) @(posedge clk);
        #1;
        if (pan_pwm || tilt_pwm) $fatal(1, "center high phase exceeds 1.5 ms");
        $display("PASS servo center hold, 20 ms period, 10 us slew, ROI reversal");
        $finish;
    end
endmodule
