attributes: attributes.c
	xlc -g -qasm -q64 -qlist=$(basename $<).s -o $@ $<

clean:
	rm -f *.o *.dbg *.s attributes
