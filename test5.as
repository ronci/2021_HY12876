	lw 0 1 one	
	lw 0 2 two
    add 1 2 3
    jalr 0 0
    noop
    add 1 2 3
    halt
one .fill 1
two .fill 2
ans .fill 0
