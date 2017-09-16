
#########  AVR Project Makefile Template   #########
###### please include a link to our site,     ######
###### http://electrons.psychogenic.com  or   ######
###### http://www.psychogenic.com             ######
######                                        ######
####################################################


#####         Target Specific Details          #####
#####     Customize these for your project     #####

# Name of target controller 
# (e.g. 'at90s8515', see the available avr-gcc mmcu 
# options for possible values)
MCU=attiny24

# Name of our project
# (use a single word, e.g. 'myproject')
PRJNAME=test_eep

# Assembly source files:
# Use *.asm for assembly source code files.
PRJSRC=$(wildcard *.asm)

# additional includes (e.g. -I /path/to/inc-dir)
INC=/usr/local/AVR/AvrAssembler2/Appnotes/

# verbosity select.
#-v verbosity [0-9][s]:
#       -vs Include target resource usage statistics
#       -vl Output low-level assembly code to stdout
#       -v0 Silent, only error messages printed
#       -v1 Error and warning messages printed
#       -v2 Error, warning, and  info messages printed (default)
#       -v3-v9 Unspecified, increasing amounts of assembler internal dumps.
VERB=2s

#####      AVR Dude 'writeflash' options       #####
#####  If you are using the avrdude program
#####  (http://www.bsdhome.com/avrdude/) to write
#####  to the MCU, you can set the following config
#####  options and use 'make writeflash' to program
#####  the device.


# programmer id--check the avrdude for complete list
# of available opts.  These should include stk500,
# avr910, avrisp, bsd, pony and more.  Set this to
# one of the valid "-c PROGRAMMER-ID" values 
# described in the avrdude info page.
# 
AVRDUDE_PROGRAMMERID=stk500v2

# port--serial or parallel port to which your 
# hardware programmer is attached
#
AVRDUDE_PORT=/dev/ttyUSB0


####################################################
#####                Config Done               #####
#####                                          #####
##### You shouldn't need to edit anything      #####
##### below to use the makefile but may wish   #####
##### to override a few of the flags           #####
##### nonetheless                              #####
#####                                          #####
####################################################

##### Flags ####

# output file format:
# -fO Debug info for simulation in AVR Studio (default)
# -fO1 | -fO2 - force format version 1 or 2 (default: auto)
# -fM Motorola hex
# -fI Intel hex
# -fG Generic hex format
# -f- No output file

OUTFMT=I

#assembler
ASMFLAGS =-I $(INC) 

# linker
LDFLAGS=-f$(OUTFMT) -m $(TRG).map -l $(TRG).lst -e $(TRG).eep -v$(VERB)

##### executables ####
AVRASM=wine /usr/local/AVR/AvrAssembler2/avrasm2.exe
AVRDUDE=avrdude
REMOVE=rm -f

##### automatic target names ####
TRG=$(PRJNAME)

HEXROMTRG=$(TRG).hex 

# Define all object files.

# Make targets:
hex: $(TRG)

writeflash: $(TRG)
	$(AVRDUDE) -c $(AVRDUDE_PROGRAMMERID) \
	 -p $(MCU) -P $(AVRDUDE_PORT) -e -u -v \
	 -U flash:w:$(HEXROMTRG)

install: writeflash

$(TRG): $(PRJSRC)
	$(AVRASM) $(ASMFLAGS) $(LDFLAGS) -o $(HEXROMTRG) $<


#### Cleanup ####
clean:
	$(REMOVE) $(HEXROMTRG) $(TRG).map $(TRG).lst


#####                    EOF                   #####

