.PHONY: clean


main: main.o
	ld -macosx_version_min 10.7.0 main.o -o main 
main.o: program.s
	nasm -f macho64 program.s -o main.o
clean:
	rm main.o main
