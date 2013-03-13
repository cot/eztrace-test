TESTCASES_=$(patsubst %.c,%,$(wildcard *.c))
ifeq "32" "$(shell getconf LONG_BIT)"
	BITS=32
else
	BITS=32 64
endif
TESTCASES=$(foreach testcase,$(TESTCASES_),$(foreach b,$(BITS),$(testcase).$(b) $(testcase).$(b).so))

all:	$(TESTCASES)

echo:
	echo '$(BITS) + $(TESTCASES_) = $(TESTCASES)'
	
%.32:	%.c
	@$(CC) -m32 -o $@ $^

%.64:	%.c
	@$(CC) -m64 -o $@ $^

%.32.so:	%.c
	@$(CC) -m32 -shared -fPIC -Wl,-Bsymbolic -o $@ $^

%.64.so:	%.c
	@$(CC) -m64 -shared -fPIC -Wl,-Bsymbolic -o $@ $^

clean:
	@rm -f $(TESTCASES)
