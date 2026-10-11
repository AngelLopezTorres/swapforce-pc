// GENERADO por gen_cputest.py a partir de runtime/ref/ppc_emit_altivec.cc (Xenia, InstrEmit_vpkd3d128).
// kXeniaPackCtl[pack][shift] = MakePermuteMask(x_sel,x_idx,y_sel,y_idx,z_sel,z_idx,w_sel,w_idx); sel 0 = VD previo, 1 = nuevo.
static const int kXeniaPackCtl[4][4][8] = {
    {{0}},
    {{0,0,0,1,0,2,1,3}, {0,0,0,1,1,3,0,3}, {0,0,1,3,0,2,0,3}, {1,3,0,1,0,2,0,3}},
    {{0,0,0,1,1,2,1,3}, {0,0,1,2,1,3,0,3}, {1,2,1,3,0,2,0,3}, {1,3,0,1,0,2,0,3}},
    {{0,0,0,1,1,2,1,3}, {0,0,1,2,1,3,0,3}, {1,2,1,3,0,2,0,3}, {0,0,0,1,0,2,1,2}},
};
