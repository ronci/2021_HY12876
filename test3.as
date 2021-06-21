    lw 0 1 ten     
    lw 0 2 one     
    lw 0 3 one  
    noop
start add 2 3 5
    sw 0 5 chv
    lw 0 2 chv
    beq 1 2 2      
    beq 0 0 start  
    noop
done halt          
ten .fill 10
one .fill 1
chv .fill 0
