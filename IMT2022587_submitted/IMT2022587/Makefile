CC = gcc
OBJ = bst.o unix.o IMT2022587_pds.o

%.o: %.c
	$(CC) -c $<

non_interactive_test: $(OBJ) pds_tester.o
	$(CC) -o pds_tester bst.o unix.o IMT2022587_pds.o pds_tester.o

clean:
	rm -f *.o pds_tester *.exe

clear_data:
	rm -f *.dat *.ndx

restart:
	rm -f *.o pds_tester *.exe
	rm -f *.dat *.ndx