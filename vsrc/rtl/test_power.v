module test_power(clk,rst_n,a,b,working_flag);
input [23:0]a;
input [23:0]b;
input working_flag;

input clk;
input rst_n;

reg [119:0] ax5;

always @(posedge clk or negedge rst_n)begin
if(~rst_n)begin
    ax5 <= 120'd0;
end else begin
    if(working_flag) ax5 <= {a,a,a,a,a};
    else ax5 <= 120'd0;
end
end


endmodule