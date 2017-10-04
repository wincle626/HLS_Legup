TARGETS = find_ra elf2sdram
SRC_DIR = ../../examples/chstone

all: $(TARGETS)

# make elf2sdram sensitive to changes of elf2sdram.cpp
elf2sdram: ../tool_source/elf2sdram.cpp

$(TARGETS):
	gcc ../tool_source/$@.cpp -lstdc++ -o $@

clean:
	rm -f $(TARGETS) *.s
