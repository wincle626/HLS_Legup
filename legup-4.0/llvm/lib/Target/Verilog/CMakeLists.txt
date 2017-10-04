add_llvm_target(Verilog
  Allocation.cpp
  Binding.cpp
  FiniteStateMachine.cpp
  GenerateRTL.cpp
  GlobalNames.cpp
  Hungarian.cpp
  LegupPass.cpp
  LegupSchedulerDAG.cpp
  MetaScheduler.cpp
  Ram.cpp
  ResourceEstimator.cpp
  RTL.cpp
  SchedulerPass.cpp
  SimpleASAPScheduler.cpp
  State.cpp
  utils.cpp
  Verilog.cpp
  VerilogWriter.cpp
  SDCScheduler.cpp
)

add_subdirectory(TargetInfo)

