alert enter coefficients of ax^2+bx+c=0 equation in given order; [0]
alert a; [62]
in ; [68]
pop ax; [69]
alert b; [71]
in ; [77]
pop bx; [78]
alert c; [80]
in ; [86]
pop cx; [87]
push ax; [89]
jnz 103; [92]
jz 469; [97]
end ; [102]
push 40000; [103]
push ax; [109]
push cx; [112]
mul ; [115]
mul ; [116]
push bx; [117]
push bx; [120]
mul ; [123]
sub ; [124]
pop dx; [125]
push dx; [127]
push 0; [130]
jg 151; [136]
jl 372; [141]
je 287; [146]
alert given equatin has two complex roots; [151]
push dx; [191]
push -10000; [194]
mul ; [200]
sqrt ; [201]
pop dx; [202]
push 20000; [204]
push ax; [210]
mul ; [213]
push bx; [214]
push -10000; [217]
mul ; [223]
div ; [224]
alert real part; [225]
out ; [239]
push 20000; [240]
push ax; [246]
mul ; [249]
push dx; [250]
div ; [253]
alert imaginary part; [254]
out ; [273]
push -10000; [274]
mul ; [280]
out ; [281]
jmp 616; [282]
push 20000; [287]
push ax; [293]
mul ; [296]
push bx; [297]
push -10000; [300]
mul ; [306]
div ; [307]
alert given equation is a full square, it has only one root; [308]
out ; [366]
jmp 616; [367]
alert given equation has two real roots; [372]
push 20000; [410]
push ax; [416]
mul ; [419]
push dx; [420]
sqrt ; [423]
push bx; [424]
push -10000; [427]
mul ; [433]
sub ; [434]
div ; [435]
out ; [436]
push 20000; [437]
push ax; [443]
mul ; [446]
push dx; [447]
sqrt ; [450]
push bx; [451]
push -10000; [454]
mul ; [460]
add ; [461]
div ; [462]
out ; [463]
jmp 616; [464]
push bx; [469]
jnz 534; [472]
push cx; [477]
jnz 579; [480]
alert any number of R satisfies this equation; [485]
jmp 616; [529]
push bx; [534]
push -10000; [537]
mul ; [543]
push cx; [544]
div ; [547]
alert eq has only one root; [548]
out ; [573]
jmp 616; [574]
alert given equation has no roots; [579]
jmp 616; [611]
end ; [616]
