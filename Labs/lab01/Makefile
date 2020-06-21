all: buildcat buildtac

buildtac:
	gcc ex2.c -o tac
buildcat:
	gcc ex1.c -o cat
runtac: buildtac
	./tac gigel.txt
runcat: buildcat
	./cat gigel.txt
diffcat: buildcat
	./cat gigel.txt > outmycat.txt
	cat gigel.txt > outcat.txt
	diff outmycat.txt outcat.txt
	rm outmycat.txt outcat.txt
difftac: buildtac
	./tac gigel.txt > outmytac.txt
	tac gigel.txt > outtac.txt
	diff outmytac.txt outtac.txt
	rm outmytac.txt outtac.txt
clean: 
	rm tac cat
