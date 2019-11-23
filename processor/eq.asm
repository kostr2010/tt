alert enter coefficients of ax^2+bx+c=0 equation in given order

alert a
in
pop ax

alert b
in
pop bx

alert c
in
pop cx

push ax

jnz quadratic
jz  linear
end

quadratic:
    push 4
    push ax
    push cx
    mul
    mul
    
    push bx
    push bx
    mul

    sub

    pop dx

    push dx
    push 0 

    jg complex_roots
    jl real_roots
    je full_square

complex_roots:
    alert given equatin has two complex roots

    push dx
    push -1
    mul
    sqrt
    pop dx

    push 2
    push ax
    mul

    push bx
    push -1
    mul

    div

    alert real part
    out

    push 2
    push ax
    mul

    push dx

    div

    alert imaginary part
    out

    push -1
    mul

    out

    jmp end


full_square:
    push 2
    push ax
    mul

    push bx
    push -1
    mul

    div

    alert given equation is a full square, it has only one root
    out

    jmp end

real_roots:
    alert given equation has two real roots
    push 2
    push ax
    mul

    push dx
    sqrt

    push bx
    push -1
    mul

    sub
    div

    out

    push 2
    push ax
    mul

    push dx
    sqrt

    push bx
    push -1
    mul

    add
    div

    out

    jmp end

linear:
    push bx
    jnz one_root

    push cx 
    jnz no_roots

    alert any number of R satisfies this equation
    jmp end

one_root:
    push bx
    push -1
    mul

    push cx

    div

    alert eq has only one root
    out

    jmp end

no_roots:
    alert given equation has no roots

    jmp end

end:
    end
