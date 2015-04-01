SECTIONS {
    ._tests : {
        __tests = .;
        *(._tests)
        __etests = .;
    }
}
