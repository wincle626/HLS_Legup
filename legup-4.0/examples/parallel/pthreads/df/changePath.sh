#!/bin/bash

SCINET_QUARTUS='C:\\Users\\jin\\Desktop\\james\\altera_lib'


    #change path from mine to scinet quartus directory
    sed -i 's|^`include\s".*/altera_mf.v"|`include "'${SCINET_QUARTUS}'\\altera_mf.v"|' tiger/tiger.v
	sed -i 's|^`include\s".*/220model.v"|`include "'${SCINET_QUARTUS}'\\220model.v"|' tiger/tiger.v
    sed -i 's|^`include\s".*/sgate.v"|`include "'${SCINET_QUARTUS}'\\sgate.v"|' tiger/tiger.v
    sed -i 's|^`include\s".*/altera_primitives.v"|`include "'${SCINET_QUARTUS}'\\altera_primitives.v"|' tiger/tiger.v
    sed -i 's|^`include\s".*/stratixiv_atoms.v"|`include "'${SCINET_QUARTUS}'\\stratixiv_atoms.v"|' tiger/tiger.v
    sed -i 's|^`include\s".*/stratixiii_atoms.v"|`include "'${SCINET_QUARTUS}'\\stratixiii_atoms.v"|' tiger/tiger.v
    echo "Replacing Path Complete!"
