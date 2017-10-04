#!/bin/bash
#generate alsyncRAMs with megawizard according to given cache parameters
FAMILY=${1}
DCACHESIZE=${2}
DBLOCKSIZE=${3}
ICACHESIZE=${4}
IBLOCKSIZE=${5}

echo FPGA family : ${FAMILY}

#For data cache
DblockSizeBits=$[8*(2**${DBLOCKSIZE})]
DtagSizeBits=31
DWIDTH=$[${DblockSizeBits}+${DtagSizeBits}+1]
DNUMWORDS=$[2**${DCACHESIZE}]

echo Data cache line size : $DWIDTH bits
echo Number of data cache lines : $DNUMWORDS lines
rm -f dcacheMem.v

qmegawiz -silent module=altsyncram wizard="RAM: 2-PORT" OPERATION_MODE=BIDIR_DUAL_PORT intended_device_family="${FAMILY}" WIDTH_A=${DWIDTH} WIDTH_B=${DWIDTH} NUMWORDS_A=${DNUMWORDS} NUMWORDS_B=${DNUMWORDS} WIDTHAD_A=${DCACHESIZE} WIDTHAD_B=${DCACHESIZE} CLOCK_ENABLE_INPUT_A=BYPASS CLOCK_ENABLE_OUTPUT_A=BYPASS CLOCK_ENABLE_INPUT_B=BYPASS CLOCK_ENABLE_OUTPUT_B=BYPASS BYTE_SIZE=8 INDATA_REG_B=CLOCK0 OUTDATA_ACLR_A=NONE OUTDATA_ACLR_B=NONE OUTDATA_REG_A=UNREGISTERED OUTDATA_REG_B=UNREGISTERED POWER_UP_UNINITIALIZED=FALSE READ_DURING_WRITE_MODE_MIXED_PORTS=OLD_DATA READ_DURING_WRITE_MODE_PORT_A=NEW_DATA_NO_NBE_READ READ_DURING_WRITE_MODE_PORT_B=NEW_DATA_NO_NBE_READ WIDTH_BYTEENA_A=132 WIDTH_BYTEENA_B=132 WRCONTROL_WRADDRESS_REG_B=CLOCK0 ADDRESS_REG_B=CLOCK0 BYTEENA_REG_B=CLOCK0 LOW_POWER_MODE=NONE address_b=used data_b=used byteena_b=used data_b=used wre_b=used clocken2=unused clock1=unused clocken1=unused byteena_a=used aclr0=unused q_b=used rden_a=unused rden_b=unused aclr1=unused clocken0=unused OPTIONAL_FILES=-BB dcacheMem.v

#For instruction Cache
IblockSizeBits=$[8*(2**${IBLOCKSIZE})]
ItagSizeBits=$[32-${ICACHESIZE}-${IBLOCKSIZE}]
IWIDTH=$[${IblockSizeBits}+${ItagSizeBits}+1]
INUMWORDS=$[2**${ICACHESIZE}]

echo Instruction cache line size : $IWIDTH bits
echo Number of instruction cache lines : $INUMWORDS lines
rm -f icacheMem.v

#run megawizard to produce icacheMem.v
#icacheMem.v (single-port)
 qmegawiz -silent module=altsyncram wizard="RAM: 1-PORT" OPERATION_MODE=SINGLE_PORT intended_device_family="${FAMILY}" WIDTH_A=${IWIDTH} NUMWORDS_A=${INUMWORDS} WIDTHAD_A=${ICACHESIZE} CLOCK_ENABLE_INPUT_A=NORMAL CLOCK_ENABLE_OUTPUT_A=BYPASS OUTDATA_ACLR_A=NONE OUTDATA_REG_A=UNREGISTERED POWER_UP_UNINITIALIZED=FALSE READ_DURING_WRITE_MODE_PORT_A=NEW_DATA_NO_NBE_READ WIDTH_BYTEENA_A=1 LOW_POWER_MODE=NONE address_b=unused data_b=unused byteena_b=unused data_b=unused wre_b=unused clocken2=unused clock1=unused clocken1=unused byteena_a=unused aclr0=unused q_b=unused rden_b=unused aclr1=unused CLOCK_ENABLE_OUTPUT_B=BYPASS clocken0=used OPTIONAL_FILES=-BB icacheMem.v

#For Multipumping Data Cache
DtagSizeBits_MP=31

DWIDTH_MP=$[${DblockSizeBits}+${DtagSizeBits_MP}+1]
DBYTE=$[(2**${DBLOCKSIZE})+4]

#	qmegawiz -silent module=altsyncram wizard="RAM: 2-PORT" intended_device_family="Stratix IV" OPERATION_MODE=BIDIR_DUAL_PORT WIDTH_A=${DWIDTH_MP} WIDTHAD_A=${DCACHESIZE} NUMWORDS_A=${DNUMWORDS} WIDTH_B=${DWIDTH_MP} WIDTHAD_B=${DCACHESIZE} NUMWORDS_B=${DNUMWORDS} LOW_POWER_MODE=NONE clock0=USED ADDRESS_REG_B=CLOCK0 INDATA_REG_B=CLOCK0 WRCONTROL_WRADDRESS_REG_B=CLOCK0 read_during_write_mode_mixed_ports=OLD_DATA BYTEENA_A=USED BYTEENA_B=USED width_byteena_a=${DBYTE} width_byteena_b=${DBYTE} byteena_reg_b=CLOCK0 byte_size=8 OPTIONAL_FILES=-BB dcacheMem.v
#	mv dcacheMem.v dcacheMem_MP_${DBLOCKSIZE}_${DCACHESIZE}.v

