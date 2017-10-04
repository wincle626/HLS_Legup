# This makefile create a ipx file for the legup components add places it in
# the boards directory

default: ../boards/legup_components.ipx

../boards/legup_components.ipx: */*/*_hw.tcl */*/*/*.v */*/*/*_hw.tcl */*/*/*/*.v
	-ip-make-ipx --source-directory=. --thorough-descent --output=../boards/legup_components.ipx

clean:
	rm -rf ../boards/legup_components.ipx

.PHONY: default clean ../boards/legup_components.ipx

