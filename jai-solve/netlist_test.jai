#import "Basic";

#import "netlist_codegen";

test_netlist :: #string END
* RC Lowpass Filter
V1 vi 0 PULSE(-1 1 0 20m 1u 1u 20m)
R1 vo vi 1k
C1 vo 0 1u IC=0
.tran 0 80m 0 20.0u
.wav "vin.wav" 32 48k V(vi)
.wav "vout.wav" 32 48k V(vo)
.options plotwinsize=0
.backanno
.end
END


main :: () {
    code := netlist_codegen(test_netlist);
    print("----------------------------------\n");
    print(code);
}
