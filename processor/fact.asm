main:
    alert enter a positive integer
	in

    pop ax

    call arg_check
	
    ;mov ax cx
    mov bx 1
	
    call fact

fact:
    push 1
    push ax

    jle end

    dec ax

    push bx

    mul

    pop bx

    pop

    call fact


arg_check: 
    push 0
    push ax

    jl less_than_zero

    pop
    pop

    ret

less_than_zero:
    alert learn to read, please
    out

    end

end:
    alert result:
    push bx
    out 

    end
