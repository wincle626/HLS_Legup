add_llvm_loadable_module( LLVMLegUp
  HwOnly.cpp
  MemProf.cpp
  PostLTO.cpp
  PreLTO.cpp
  LegUpProfile.cpp
  SwOnly.cpp
  utils.cpp
  WatchVariables.cpp
  )

target_link_libraries(LLVMLegUp LLVMCodeGen)
  
