`timescale 1ns / 1ps

// SkyWater 130nm Standard Cell Library Behavioral Simulation Models

module sky130_fd_sc_hd__inv_2 (input wire A, output wire Y);
    assign Y = ~A;
endmodule

module sky130_fd_sc_hd__buf_2 (input wire A, output wire X);
    assign X = A;
endmodule

module sky130_fd_sc_hd__clkbuf_4 (input wire A, output wire X);
    assign X = A;
endmodule

module sky130_fd_sc_hd__clkbuf_8 (input wire A, output wire X);
    assign X = A;
endmodule

module sky130_fd_sc_hd__clkbuf_16 (input wire A, output wire X);
    assign X = A;
endmodule

module sky130_fd_sc_hd__conb_1 (output wire HI, output wire LO);
    assign HI = 1'b1;
    assign LO = 1'b0;
endmodule

module sky130_fd_sc_hd__mux2_1 (input wire A0, input wire A1, input wire S, output wire X);
    assign X = S ? A1 : A0;
endmodule

// AND gates
module sky130_fd_sc_hd__and2_2 (input wire A, input wire B, output wire X);
    assign X = A & B;
endmodule

module sky130_fd_sc_hd__and2b_2 (input wire A_N, input wire B, output wire X);
    assign X = (~A_N) & B;
endmodule

module sky130_fd_sc_hd__and3_2 (input wire A, input wire B, input wire C, output wire X);
    assign X = A & B & C;
endmodule

module sky130_fd_sc_hd__and3b_2 (input wire A_N, input wire B, input wire C, output wire X);
    assign X = (~A_N) & B & C;
endmodule

module sky130_fd_sc_hd__and4_2 (input wire A, input wire B, input wire C, input wire D, output wire X);
    assign X = A & B & C & D;
endmodule

module sky130_fd_sc_hd__and4b_2 (input wire A_N, input wire B, input wire C, input wire D, output wire X);
    assign X = (~A_N) & B & C & D;
endmodule

module sky130_fd_sc_hd__and4bb_2 (input wire A_N, input wire B_N, input wire C, input wire D, output wire X);
    assign X = (~A_N) & (~B_N) & C & D;
endmodule

// NAND gates
module sky130_fd_sc_hd__nand2_2 (input wire A, input wire B, output wire Y);
    assign Y = ~(A & B);
endmodule

module sky130_fd_sc_hd__nand2b_2 (input wire A_N, input wire B, output wire Y);
    assign Y = ~((~A_N) & B);
endmodule

module sky130_fd_sc_hd__nand3_2 (input wire A, input wire B, input wire C, output wire Y);
    assign Y = ~(A & B & C);
endmodule

module sky130_fd_sc_hd__nand3b_2 (input wire A_N, input wire B, input wire C, output wire Y);
    assign Y = ~((~A_N) & B & C);
endmodule

module sky130_fd_sc_hd__nand4_2 (input wire A, input wire B, input wire C, input wire D, output wire Y);
    assign Y = ~(A & B & C & D);
endmodule

// OR gates
module sky130_fd_sc_hd__or2_2 (input wire A, input wire B, output wire X);
    assign X = A | B;
endmodule

module sky130_fd_sc_hd__or3_2 (input wire A, input wire B, input wire C, output wire X);
    assign X = A | B | C;
endmodule

module sky130_fd_sc_hd__or3b_2 (input wire A, input wire B, input wire C_N, output wire X);
    assign X = A | B | (~C_N);
endmodule

module sky130_fd_sc_hd__or4_2 (input wire A, input wire B, input wire C, input wire D, output wire X);
    assign X = A | B | C | D;
endmodule

module sky130_fd_sc_hd__or4b_2 (input wire A, input wire B, input wire C, input wire D_N, output wire X);
    assign X = A | B | C | (~D_N);
endmodule

module sky130_fd_sc_hd__or4bb_2 (input wire A, input wire B, input wire C_N, input wire D_N, output wire X);
    assign X = A | B | (~C_N) | (~D_N);
endmodule

// NOR gates
module sky130_fd_sc_hd__nor2_2 (input wire A, input wire B, output wire Y);
    assign Y = ~(A | B);
endmodule

module sky130_fd_sc_hd__nor3_2 (input wire A, input wire B, input wire C, output wire Y);
    assign Y = ~(A | B | C);
endmodule

module sky130_fd_sc_hd__nor3b_2 (input wire A, input wire B, input wire C_N, output wire Y);
    assign Y = ~(A | B | (~C_N));
endmodule

module sky130_fd_sc_hd__nor4_2 (input wire A, input wire B, input wire C, input wire D, output wire Y);
    assign Y = ~(A | B | C | D);
endmodule

module sky130_fd_sc_hd__nor4b_2 (input wire A, input wire B, input wire C, input wire D_N, output wire Y);
    assign Y = ~(A | B | C | (~D_N));
endmodule

// XOR / XNOR
module sky130_fd_sc_hd__xor2_2 (input wire A, input wire B, output wire X);
    assign X = A ^ B;
endmodule

module sky130_fd_sc_hd__xnor2_2 (input wire A, input wire B, output wire Y);
    assign Y = ~(A ^ B);
endmodule

// Compound gates
module sky130_fd_sc_hd__a21o_2 (input wire A1, input wire A2, input wire B1, output wire X);
    assign X = (A1 & A2) | B1;
endmodule

module sky130_fd_sc_hd__a21oi_2 (input wire A1, input wire A2, input wire B1, output wire Y);
    assign Y = ~((A1 & A2) | B1);
endmodule

module sky130_fd_sc_hd__a22o_2 (input wire A1, input wire A2, input wire B1, input wire B2, output wire X);
    assign X = (A1 & A2) | (B1 & B2);
endmodule

module sky130_fd_sc_hd__a22oi_2 (input wire A1, input wire A2, input wire B1, input wire B2, output wire Y);
    assign Y = ~((A1 & A2) | (B1 & B2));
endmodule

module sky130_fd_sc_hd__a31o_2 (input wire A1, input wire A2, input wire A3, input wire B1, output wire X);
    assign X = (A1 & A2 & A3) | B1;
endmodule

module sky130_fd_sc_hd__a31oi_2 (input wire A1, input wire A2, input wire A3, input wire B1, output wire Y);
    assign Y = ~((A1 & A2 & A3) | B1);
endmodule

module sky130_fd_sc_hd__a32o_2 (input wire A1, input wire A2, input wire A3, input wire B1, input wire B2, output wire X);
    assign X = (A1 & A2 & A3) | (B1 & B2);
endmodule

module sky130_fd_sc_hd__a211o_2 (input wire A1, input wire A2, input wire B1, input wire C1, output wire X);
    assign X = (A1 & A2) | B1 | C1;
endmodule

module sky130_fd_sc_hd__a211oi_2 (input wire A1, input wire A2, input wire B1, input wire C1, output wire Y);
    assign Y = ~((A1 & A2) | B1 | C1);
endmodule

module sky130_fd_sc_hd__a221o_2 (input wire A1, input wire A2, input wire B1, input wire B2, input wire C1, output wire X);
    assign X = (A1 & A2) | (B1 & B2) | C1;
endmodule

module sky130_fd_sc_hd__a221oi_2 (input wire A1, input wire A2, input wire B1, input wire B2, input wire C1, output wire Y);
    assign Y = ~((A1 & A2) | (B1 & B2) | C1);
endmodule

module sky130_fd_sc_hd__a311o_2 (input wire A1, input wire A2, input wire A3, input wire B1, input wire C1, output wire X);
    assign X = (A1 & A2 & A3) | B1 | C1;
endmodule

module sky130_fd_sc_hd__a2111oi_2 (input wire A1, input wire A2, input wire B1, input wire C1, input wire D1, output wire Y);
    assign Y = ~((A1 & A2) | B1 | C1 | D1);
endmodule

module sky130_fd_sc_hd__a41oi_2 (input wire A1, input wire A2, input wire A3, input wire A4, input wire B1, output wire Y);
    assign Y = ~((A1 & A2 & A3 & A4) | B1);
endmodule

module sky130_fd_sc_hd__a21bo_2 (input wire A1, input wire A2, input wire B1_N, output wire X);
    assign X = (A1 & A2) | (~B1_N);
endmodule

module sky130_fd_sc_hd__a21boi_2 (input wire A1, input wire A2, input wire B1_N, output wire Y);
    assign Y = ~((A1 & A2) | (~B1_N));
endmodule

module sky130_fd_sc_hd__o21a_2 (input wire A1, input wire A2, input wire B1, output wire X);
    assign X = (A1 | A2) & B1;
endmodule

module sky130_fd_sc_hd__o21ai_2 (input wire A1, input wire A2, input wire B1, output wire Y);
    assign Y = ~((A1 | A2) & B1);
endmodule

module sky130_fd_sc_hd__o22a_2 (input wire A1, input wire A2, input wire B1, input wire B2, output wire X);
    assign X = (A1 | A2) & (B1 | B2);
endmodule

module sky130_fd_sc_hd__o22ai_2 (input wire A1, input wire A2, input wire B1, input wire B2, output wire Y);
    assign Y = ~((A1 | A2) & (B1 | B2));
endmodule

module sky130_fd_sc_hd__o31a_2 (input wire A1, input wire A2, input wire A3, input wire B1, output wire X);
    assign X = (A1 | A2 | A3) & B1;
endmodule

module sky130_fd_sc_hd__o31ai_2 (input wire A1, input wire A2, input wire A3, input wire B1, output wire Y);
    assign Y = ~((A1 | A2 | A3) & B1);
endmodule

module sky130_fd_sc_hd__o32a_2 (input wire A1, input wire A2, input wire A3, input wire B1, input wire B2, output wire X);
    assign X = (A1 | A2 | A3) & (B1 | B2);
endmodule

module sky130_fd_sc_hd__o32ai_2 (input wire A1, input wire A2, input wire A3, input wire B1, input wire B2, output wire Y);
    assign Y = ~((A1 | A2 | A3) & (B1 | B2));
endmodule

module sky130_fd_sc_hd__o211a_2 (input wire A1, input wire A2, input wire B1, input wire C1, output wire X);
    assign X = (A1 | A2) & B1 & C1;
endmodule

module sky130_fd_sc_hd__o211ai_2 (input wire A1, input wire A2, input wire B1, input wire C1, output wire Y);
    assign Y = ~((A1 | A2) & B1 & C1);
endmodule

module sky130_fd_sc_hd__o221a_2 (input wire A1, input wire A2, input wire B1, input wire B2, input wire C1, output wire X);
    assign X = (A1 | A2) & (B1 | B2) & C1;
endmodule

module sky130_fd_sc_hd__o311a_2 (input wire A1, input wire A2, input wire A3, input wire B1, input wire C1, output wire X);
    assign X = (A1 | A2 | A3) & B1 & C1;
endmodule

module sky130_fd_sc_hd__o21ba_2 (input wire A1, input wire A2, input wire B1_N, output wire X);
    assign X = (A1 | A2) & (~B1_N);
endmodule

module sky130_fd_sc_hd__o21bai_2 (input wire A1, input wire A2, input wire B1_N, output wire Y);
    assign Y = ~((A1 | A2) & (~B1_N));
endmodule

module sky130_fd_sc_hd__o2bb2a_2 (input wire A1_N, input wire A2_N, input wire B1, input wire B2, output wire X);
    assign X = ((~A1_N) | (~A2_N)) & (B1 | B2);
endmodule

// Sequential elements (Flip-Flops)
module sky130_fd_sc_hd__dfrtp_2 (
    input wire CLK,
    input wire D,
    input wire RESET_B,
    output reg Q
);
    always @(posedge CLK or negedge RESET_B) begin
        if (!RESET_B)
            Q <= 1'b0;
        else
            Q <= D;
    end
endmodule

module sky130_fd_sc_hd__dfstp_2 (
    input wire CLK,
    input wire D,
    input wire SET_B,
    output reg Q
);
    always @(posedge CLK or negedge SET_B) begin
        if (!SET_B)
            Q <= 1'b1;
        else
            Q <= D;
    end
endmodule

module sky130_fd_sc_hd__dfxtp_2 (
    input wire CLK,
    input wire D,
    output reg Q
);
    always @(posedge CLK) begin
        Q <= D;
    end
endmodule

module sky130_fd_sc_hd__diode_2 (
    input wire DIODE
);
endmodule
