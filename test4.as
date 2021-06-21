    lw 0 1 one
    lw 0 2 two     
    lw 0 3 three
    lw 0 4 one
    noop
start beq 3 4 fin
    add 1 2 4     
    beq 3 4 start  
    noop
fin noop
done halt 
one .fill 1  
two .fill 2
three .fill 3
