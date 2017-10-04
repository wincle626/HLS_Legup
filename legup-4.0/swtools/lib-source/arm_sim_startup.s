.global __arm_sim_reset
__arm_sim_reset:
 LDR sp, =stack_top
 BL main
 B .
