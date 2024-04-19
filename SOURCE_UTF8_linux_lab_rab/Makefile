# Script for Programming system generation
all: komppl.exe kompassr.exe absloadm.exe
#komppl.o: komppl.c
#	gcc -o komppl.o komppl.c
komppl.exe: komppl.c
	@echo "________k o m p p l . e x e        g e n e r a t i o n______"
	gcc -o komppl.exe komppl.c
	@echo "____________________________________________________________"
kompassr.exe: kompassr.c
	@echo "________k o m p a s s r . e x e    g e n e r a t i o n______"
	gcc -o kompassr.exe kompassr.c
	@echo "____________________________________________________________"
absloadm.exe: absloadm.c
	@echo "________a b s l o a d m . e x e    g e n e r a t i o n______"
        # 修改为自己的 -lncursesw 环境（以支持 UTF-8）
	gcc -o absloadm.exe absloadm.c -I/opt/homebrew/opt/ncurses/include -L/opt/homebrew/opt/ncurses/lib -lncursesw
	@echo "____________________________________________________________"

clean:
	rm -f *.exe
	rm -f *.o 
	rm -f *.ass
	rm -f *.tex
