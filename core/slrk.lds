SECTIONS {
    .text : {
        __start_dr_code = .;
        *debug_reg.o (.text);
        __end_dr_code = .;
    }
}
