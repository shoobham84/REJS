`timescale 1ps/1ps

module all_zeros_and_ones();
    reg clk, rst_n, enable, I;
    wire [7:0] O;
    wire success;

    puzzle_extracted dut (
        .clk(clk),
        .rst_n(rst_n),
        .enable(enable),
        .I(I),
        .O(O),
        .success(success)
    );

    initial begin
        clk = 0;
        forever #5000 clk = ~clk;
    end

    task run_test(input [120:0] key, input [128*8:1] name);
        integer k;
        begin
            $display(" RUNNING TEST: %0s", name);

            rst_n = 0; enable = 0; I = 0;
            #30000;
            rst_n = 1;
            #10000;

            enable = 1;
            for (k = 120; k >= 0; k = k - 1) 
            begin
                I = key[k];
                #10000;
            end
            enable = 0;
            I = 0;

            #10000;
            $write(" OUTPUT STRING: \"");
            for (k = 0; k < 19; k = k + 1) begin
                if (O >= 32 && O <= 126)
                    $write("%c", O);
                #10000;
            end
            $display("\n SUCCESS FLAG:  %b", success);
            #30000;
        end
    endtask

    initial begin
        $dumpfile("outputs/all_zeros_and_ones.vcd");
        $dumpvars(0, all_zeros_and_ones);

        run_test(121'b0, "ALL ZEROS ");

        run_test({121{1'b1}}, "ALL ONES ");

        $finish;
    end
endmodule
