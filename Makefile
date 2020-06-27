CC=sdcc
PACK=packihx

PROJECTNAME=CH559USB
XRAMSIZE=0x0800
XRAMLOC=0x0600
CODESIZE=0xEFFF
DFREQSYS=48000000

CFLAGS=-V -mmcs51 --model-large --xram-size $(XRAMSIZE) --xram-loc $(XRAMLOC) --code-size $(CODESIZE) -I/ -DFREQ_SYS=$(DFREQSYS)

all: $(PROJECTNAME).bin

$(PROJECTNAME).hex: $(PROJECTNAME).ihx
	$(PACK) $^ > $@

$(PROJECTNAME).ihx: main.rel util.rel USBHost.rel uart.rel
	$(CC) $(CFLAGS) $^ -o $@

$(PROJECTNAME).bin: $(PROJECTNAME).ihx
	makebin -p $^ $@

%.rel: %.c
	$(CC) $(CFLAGS) -c $<


clean:
	-rm *.asm *.lst *.rel *.rst *.sym *.hex $(PROJECTNAME).lk $(PROJECTNAME).map $(PROJECTNAME).mem $(PROJECTNAME).ihx
