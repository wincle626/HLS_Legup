#! /usr/bin/python
#
#
# USAGE:    python multi_cycle_remove_through_constraints.py <sdc_file_in> <sdc_file_out> <no_constraints_file> <rm_constraints_file>
#
#
# Example arguments: 
#
#   sdc_file_in
#               llvm_prof_multicycle_constraints.sdc
#               This is the original .sdc file produced by running "make" in LegUp
#
#   sdc_file_out
#               llvm_prof_multicycle_constraints_no_through_constraints.sdc
#               This is the output .sdc file. It's the same as the input except:
#                1) We remove through constraints for certain src/dst pairs which
#                   don't need them, and
#                2) We remove all constraints for certain src/dst pairs which
#                   we weren't able to print -through constraints for. Instead,
#                   this script replaces all those old constraints (that have 
#                   -through assignments) with a single constraint (which does not
#                   have -through assignments) and has the minimum slack of all
#                   those old -through assignment constraints.
#
#   no_constraints_file
#               src_dst_pairs_with_through_constraints.txt
#               The file listing all the src/dst pairs which don't need -through
#               constraints (#1 above). This is an optimization to improve compile
#               time and remove delays
#
#   rm_constraints_file
#               pairs_whose_through_constraints_must_be_removed.txt
#               The file listing all the src/dst pairs which we weren't able to 
#               print -through constraints for. This is needed for correctness.
#
# Read in the src/dst pairs which do need through constraints

import sys

sdc_file_in = sys.argv[1]
sdc_file_out = sys.argv[2]
no_constraints_file = sys.argv[3]
rm_constraints_file = sys.argv[4]



src_dst_pairs_with_through_constraints = []
try:
    f = open(no_constraints_file)
except:
    sys.exit(0);
for line in f:
    line = line.strip()
    if not line:
        continue
    src = line.split('\t')[0]
    dst = line.split('\t')[1]
    src_dst_pairs_with_through_constraints.append( (src, dst) )
f.close()

# Read in the src/dst pairs whose constraints we must remove and instead replace
# with a min-cycle constraint
pairs_whose_through_constraints_must_be_removed = []
try:
    f = open(rm_constraints_file)
except:
    sys.exit(0);
for line in f:
    line = line.strip()
    if not line:
        continue
    src = line.split('\t')[0]
    dst = line.split('\t')[1]
    min = line.split('\t')[2]
    pairs_whose_through_constraints_must_be_removed.append( (src, dst, min) )
f.close()


# Read in the llvm_prof_multicycle_constraints.sdc file (input argument)
try:
    f = open(sdc_file_in)
except:
    sys.exit(0);
sdc_file_without_through_constraints_str = ''
for line in f:
    line = line.strip()
    if not line:
        continue
    
    # First check if this is a pair which we must remove -through constraints for
    is_illegal_src_dst_pair = False
    for illegal_src_dst_pair in pairs_whose_through_constraints_must_be_removed:
        src = illegal_src_dst_pair[0]
        dst = illegal_src_dst_pair[1]
        min_constraint = illegal_src_dst_pair[2]
        # Check if this is a pair we must remove -through constraints for
        if ('set_multicycle_path -from [get_registers {*' + src + '*}] -to [get_registers {*' + dst + '*}] ') in line:
            is_illegal_src_dst_pair = True
            break
    # If this was found above, completely remove this assignment (skip to the next line of the .sdc file)
    if is_illegal_src_dst_pair:
        continue

    # Otherwise, maybe we want to keep this assignment, but only remove the -through constraint
    keep_through_for_this_line = False;
    for src_dst_pair_with_through_constraints in src_dst_pairs_with_through_constraints:
        src = src_dst_pair_with_through_constraints[0]
        dst = src_dst_pair_with_through_constraints[1]
        
        # Check if this is a pair we to keep the -through constraints for
        if ('set_multicycle_path -from [get_registers {*' + src + '*}] -to [get_registers {*' + dst + '*}] ') in line:
            keep_through_for_this_line = True;
            break
    if keep_through_for_this_line:
        sdc_file_without_through_constraints_str += line
    else:
        sdc_file_without_through_constraints_str += line.split('\t')[0]
    sdc_file_without_through_constraints_str += '\n'
f.close()

# Finally, write out constraints for each of the pairs in 
# pairs_whose_through_constraints_must_be_removed. Above we removed all the constraints
# previously printed for these pairs (all with -through constraints). Now, replace all
# the ones we removed with a single constraint for each path that uses the mininum slack.
for illegal_src_dst_pair in pairs_whose_through_constraints_must_be_removed:
    src = illegal_src_dst_pair[0]
    dst = illegal_src_dst_pair[1]
    min_constraint = illegal_src_dst_pair[2]
    sdc_file_without_through_constraints_str += \
        'set_multicycle_path -from [get_registers {*' + src + '*}] -to [get_registers {*' +  \
        dst + '*}] -setup -end ' +  min_constraint + '\n'
    sdc_file_without_through_constraints_str += \
        'set_multicycle_path -from [get_registers {*' + src + '*}] -to [get_registers {*' +  \
        dst + '*}] -hold -end ' +  str( int(min_constraint)-1 ) + '\n'

# Write the new file
try:
    f = open(sdc_file_out, 'w')
except:
    sys.exit(0);
f.write(sdc_file_without_through_constraints_str)
f.close()
