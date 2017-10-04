:tocdepth: 3

.. _constraints:

Constraints Manual
==================================

High-level synthesis (HLS) tools typically accept user-provided constraints that impact the automatically generated hardware.  This guide explains the constraints available for LegUp HLS.

Constraints are primarily represented in a TCL file(s) that are read by LegUp HLS on execution.  The defaults are mainly to be found in ``examples/legup.tcl``.  While it is possible to directly modify ``examples/legup.tcl``, for design-specific constraints, we recommend you create a ``config.tcl`` file in the local design directory that includes ``examples/legup.tcl`` then overrides default constraint values, as appropriate.  See ``examples/mult/config.tcl`` for an example of the recommended methodology.

--------------------------------------------------------------------------------

Most Commonly Used Constraints
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


.. include:: constraint_docs/CASE_FSM.rst
.. include:: constraint_docs/CLOCK_PERIOD.rst
.. include:: constraint_docs/GROUP_RAMS.rst
.. include:: constraint_docs/GROUP_RAMS_SIMPLE_OFFSET.rst
.. include:: constraint_docs/LOCAL_RAMS.rst
.. include:: constraint_docs/MB_MINIMIZE_HW.rst
.. include:: constraint_docs/NO_INLINE.rst
.. include:: constraint_docs/NO_OPT.rst
.. include:: constraint_docs/SET_ACCELERATOR_FUNCTION.rst
.. include:: constraint_docs/UNROLL.rst
.. include:: constraint_docs/VSIM_NO_ASSERT.rst
.. include:: constraint_docs/loop_pipeline.rst
.. include:: constraint_docs/set_combine_basicblock.rst
.. include:: constraint_docs/set_operation_latency.rst
.. include:: constraint_docs/set_project.rst
.. include:: constraint_docs/set_resource_constraint.rst

Other Advanced Constraints
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. include:: constraint_docs/CASEX.rst
.. include:: constraint_docs/DEBUG_DB_HOST.rst
.. include:: constraint_docs/DEBUG_DB_NAME.rst
.. include:: constraint_docs/DEBUG_DB_PASSWORD.rst
.. include:: constraint_docs/DEBUG_DB_SCRIPT_FILE.rst
.. include:: constraint_docs/DEBUG_DB_USER.rst
.. include:: constraint_docs/DEBUG_SDC_CONSTRAINTS.rst
.. include:: constraint_docs/DFG_SHOW_DUMMY_CALLS.rst
.. include:: constraint_docs/DISABLE_REG_SHARING.rst
.. include:: constraint_docs/DONT_CHAIN_GET_ELEM_PTR.rst
.. include:: constraint_docs/DUAL_PORT_BINDING.rst
.. include:: constraint_docs/ENABLE_PATTERN_SHARING.rst
.. include:: constraint_docs/EXPLICIT_LPM_MULTS.rst
.. include:: constraint_docs/FREQ_THRESHOLD.rst
.. include:: constraint_docs/INCLUDE_INST_IN_FSM_DOT_GRAPH.rst
.. include:: constraint_docs/INCREMENTAL_SDC.rst
.. include:: constraint_docs/INFERRED_RAMS.rst
.. include:: constraint_docs/INSPECT_DEBUG.rst
.. include:: constraint_docs/INSPECT_DEBUG_DB_NAME.rst
.. include:: constraint_docs/INSPECT_DEBUG_DB_SCRIPT_FILE.rst
.. include:: constraint_docs/INSPECT_ONCHIP_BUG_DETECT_DEBUG.rst
.. include:: constraint_docs/KEEP_SIGNALS_WITH_NO_FANOUT.rst
.. include:: constraint_docs/LLVM_PROFILE.rst
.. include:: constraint_docs/MB_MAX_BACK_PASSES.rst
.. include:: constraint_docs/MB_PRINT_STATS.rst
.. include:: constraint_docs/MB_RANGE_FILE.rst
.. include:: constraint_docs/MODULO_DEBUG.rst
.. include:: constraint_docs/MODULO_SCHEDULER.rst
.. include:: constraint_docs/MULTIPLIER_NO_CHAIN.rst
.. include:: constraint_docs/MULTIPUMPING.rst
.. include:: constraint_docs/MULTI_CYCLE_ADD_THROUGH_CONSTRAINTS.rst
.. include:: constraint_docs/MULTI_CYCLE_DEBUG.rst
.. include:: constraint_docs/MULTI_CYCLE_DISABLE_REG_MERGING.rst
.. include:: constraint_docs/MULTI_CYCLE_DUPLICATE_LOAD_REG.rst
.. include:: constraint_docs/MULTI_CYCLE_REMOVE_CMP_REG.rst
.. include:: constraint_docs/MULTI_CYCLE_REMOVE_REG.rst
.. include:: constraint_docs/MULTI_CYCLE_REMOVE_REG_DIVIDERS.rst
.. include:: constraint_docs/MULT_BY_CONST_INFER_DSP.rst
.. include:: constraint_docs/NO_DFG_DOT_FILES.rst
.. include:: constraint_docs/NO_LOOP_PIPELINING.rst
.. include:: constraint_docs/NO_ROMS.rst
.. include:: constraint_docs/PATTERN_SHARE_ADD.rst
.. include:: constraint_docs/PATTERN_SHARE_BITOPS.rst
.. include:: constraint_docs/PATTERN_SHARE_SHIFT.rst
.. include:: constraint_docs/PATTERN_SHARE_SUB.rst
.. include:: constraint_docs/PIPELINE_ALL.rst
.. include:: constraint_docs/PIPELINE_RESOURCE_SHARING.rst
.. include:: constraint_docs/PIPELINE_SAVE_REG.rst
.. include:: constraint_docs/PRINTF_CYCLES.rst
.. include:: constraint_docs/PRINT_BB_STATS.rst
.. include:: constraint_docs/PRINT_FUNCTION_START_FINISH.rst
.. include:: constraint_docs/PRINT_STATES.rst
.. include:: constraint_docs/PS_BIT_DIFF_THRESHOLD.rst
.. include:: constraint_docs/PS_MAX_SIZE.rst
.. include:: constraint_docs/PS_MIN_SIZE.rst
.. include:: constraint_docs/PS_MIN_WIDTH.rst
.. include:: constraint_docs/RESTRICT_TO_MAXDSP.rst
.. include:: constraint_docs/SDC_BACKTRACKING_PRIORITY.rst
.. include:: constraint_docs/SDC_MULTIPUMP.rst
.. include:: constraint_docs/SDC_NO_CHAINING.rst
.. include:: constraint_docs/SDC_ONLY_CHAIN_CRITICAL.rst
.. include:: constraint_docs/SDC_PRIORITY.rst
.. include:: constraint_docs/SYSTEM_BARRIER_BASE_ADDRESS.rst
.. include:: constraint_docs/SYSTEM_CLOCK_INTERFACE.rst
.. include:: constraint_docs/SYSTEM_CLOCK_MODULE.rst
.. include:: constraint_docs/SYSTEM_DATA_CACHE_TYPE.rst
.. include:: constraint_docs/SYSTEM_MEMORY_BASE_ADDRESS.rst
.. include:: constraint_docs/SYSTEM_MEMORY_INTERFACE.rst
.. include:: constraint_docs/SYSTEM_MEMORY_MODULE.rst
.. include:: constraint_docs/SYSTEM_MEMORY_SIM_INIT_FILE_NAME.rst
.. include:: constraint_docs/SYSTEM_MEMORY_SIM_INIT_FILE_TYPE.rst
.. include:: constraint_docs/SYSTEM_MEMORY_SIZE.rst
.. include:: constraint_docs/SYSTEM_MEMORY_WIDTH.rst
.. include:: constraint_docs/SYSTEM_MUTEX_BASE_ADDRESS.rst
.. include:: constraint_docs/SYSTEM_PROCESSOR_ARCHITECTURE.rst
.. include:: constraint_docs/SYSTEM_PROCESSOR_DATA_MASTER.rst
.. include:: constraint_docs/SYSTEM_PROCESSOR_NAME.rst
.. include:: constraint_docs/SYSTEM_PROJECT_NAME.rst
.. include:: constraint_docs/SYSTEM_RESET_INTERFACE.rst
.. include:: constraint_docs/SYSTEM_RESET_MODULE.rst
.. include:: constraint_docs/TEST_WAITREQUEST.rst
.. include:: constraint_docs/TIMING_NO_IGNORE_GETELEMENTPTR_AND_STORE.rst
.. include:: constraint_docs/TIMING_NUM_PATHS.rst
.. include:: constraint_docs/debugger_capture_mode.rst
.. include:: constraint_docs/set_custom_test_bench_module.rst
.. include:: constraint_docs/set_custom_top_level_module.rst
.. include:: constraint_docs/set_custom_verilog_file.rst
.. include:: constraint_docs/set_custom_verilog_function.rst
.. include:: constraint_docs/set_device_specs.rst
.. include:: constraint_docs/set_legup_config_board.rst
.. include:: constraint_docs/set_legup_output_path.rst
.. include:: constraint_docs/set_memory_global.rst
.. include:: constraint_docs/set_memory_local.rst
.. include:: constraint_docs/set_operation_attributes.rst
.. include:: constraint_docs/set_operation_sharing.rst
.. include:: constraint_docs/use_debugger.rst
