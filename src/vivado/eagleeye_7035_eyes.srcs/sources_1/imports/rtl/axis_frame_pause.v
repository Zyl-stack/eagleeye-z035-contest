`timescale 1ns / 1ps
`default_nettype none

module axis_frame_pause #(
    parameter WIDTH = 640,
    parameter HEIGHT = 360,
    parameter DEBOUNCE_BITS = 20
)(
    input wire clk,
    input wire rst,
    input wire pause_key,
    input wire [23:0] s_data,
    input wire s_valid,
    output wire s_ready,
    input wire s_last,
    input wire s_user,
    output wire [23:0] m_data,
    output wire m_valid,
    input wire m_ready,
    output wire m_last,
    output wire m_user,
    output reg paused
);
    localparam PIXELS = WIDTH * HEIGHT;
    localparam AW = $clog2(PIXELS);
    localparam XW = $clog2(WIDTH);

    (* ram_style = "block" *) reg [23:0] frame [0:PIXELS-1];
    reg key_meta, key_sync, key_stable, key_stable_d;
    reg [DEBOUNCE_BITS-1:0] debounce_count;
    wire key_pressed = ~key_sync;
    wire key_pulse = key_stable && !key_stable_d;

    always @(posedge clk) begin
        if (rst) begin
            key_meta <= 1'b1;
            key_sync <= 1'b1;
            key_stable <= 1'b0;
            key_stable_d <= 1'b0;
            debounce_count <= 0;
        end else begin
            key_meta <= pause_key;
            key_sync <= key_meta;
            key_stable_d <= key_stable;
            if (key_pressed == key_stable)
                debounce_count <= 0;
            else begin
                debounce_count <= debounce_count + 1'b1;
                if (&debounce_count) begin
                    key_stable <= key_pressed;
                    debounce_count <= 0;
                end
            end
        end
    end

    reg pause_pending, resume_pending, live_aligned;
    reg [AW-1:0] write_addr, read_addr;
    reg [XW-1:0] read_x;
    reg [23:0] replay_data;
    reg replay_valid, replay_last, replay_user;
    wire live_forward = !paused && (live_aligned || (s_valid && s_user));
    wire live_accept = s_valid && s_ready && live_forward;
    wire replay_accept = paused && replay_valid && m_ready;

    assign s_ready = paused || !live_forward || m_ready;
    assign m_data = paused ? replay_data : s_data;
    assign m_valid = paused ? replay_valid : (s_valid && live_forward);
    assign m_last = paused ? replay_last : s_last;
    assign m_user = paused ? replay_user : s_user;

    always @(posedge clk) begin
        if (rst) begin
            paused <= 1'b0;
            pause_pending <= 1'b0;
            resume_pending <= 1'b0;
            live_aligned <= 1'b0;
            write_addr <= 0;
            read_addr <= 0;
            read_x <= 0;
            replay_data <= 0;
            replay_valid <= 1'b0;
            replay_last <= 1'b0;
            replay_user <= 1'b0;
        end else begin
            if (key_pulse) begin
                if (paused) resume_pending <= 1'b1;
                else pause_pending <= 1'b1;
            end

            if (paused) begin
                if (!replay_valid || m_ready) begin
                    replay_data <= frame[read_addr];
                    replay_last <= (read_x == WIDTH-1);
                    replay_user <= (read_addr == 0);
                    replay_valid <= 1'b1;
                    read_addr <= (read_addr == PIXELS-1) ? 0 : read_addr + 1'b1;
                    read_x <= (read_x == WIDTH-1) ? 0 : read_x + 1'b1;
                end
                if (replay_accept && replay_last && (read_addr == 0) && resume_pending) begin
                    paused <= 1'b0;
                    resume_pending <= 1'b0;
                    replay_valid <= 1'b0;
                    live_aligned <= 1'b0;
                end
            end else begin
                replay_valid <= 1'b0;
                if (!live_aligned && s_valid && s_user && s_ready)
                    live_aligned <= 1'b1;
                if (live_accept) begin
                    frame[s_user ? {AW{1'b0}} : write_addr] <= s_data;
                    write_addr <= s_user ? 1 : write_addr + 1'b1;
                    if (write_addr == PIXELS-1 && s_last && (pause_pending || key_pulse)) begin
                        paused <= 1'b1;
                        pause_pending <= 1'b0;
                        read_addr <= 0;
                        read_x <= 0;
                        replay_valid <= 1'b0;
                    end
                end
            end
        end
    end
endmodule

`default_nettype wire
