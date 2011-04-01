set(barevm_lib_src 
    basicobjects.cpp  
    bobobject.cpp  
    builtins.cpp  
    bytecode.cpp  
    environment.cpp  
    main.cpp  
    serialization.cpp  
    utils.cpp
    vm.cpp)

add_library(barevm_lib ${barevm_lib_src})

