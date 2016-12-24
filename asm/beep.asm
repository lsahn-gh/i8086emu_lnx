start:
    mov cx,200
    call part1
    call part11
    call part1
    call part12
    call part2
    call part3
    loop start


part1:
    call c
    call h
    call c
    call h
    call c
    call g
    call b
    call gis
    mov cx,600
    call f
    call gis1
    call c1
    call f
    mov cx,600
    call g
    call c1
    ret

part11:
    call e
    call g
    mov cx,600
    call gis
    call c1
    ret

part12:
    call gis
    call g
    mov cx,600
    call f
    ret

part2:
    call g
    call gis
    call b
    mov cx,600
    call c
    call es1
    call cis
    call c
    mov cx,600
    call b
    call es1
    call c
    call b
    mov cx,600
    call gis
    call es1
    call b
    call gis
    call g
    ret

part3:
    call c1
    call c1
    call c
    call c1
    call c
    call c    
    call c3
    call h
    


sleep:
        mov cx,0xfff

lo1:
        loop lo1
	mov cx,0xff
lo2:
	loop lo2
        ret

play:
    push ax
    mov ax,cx
    out 0xef,ax
    pop ax
    out 0xff,ax
    call sleep
    mov cx,200
    ret

;Tondefinitionen ... fast wohltemperiert :o)
gis1:
    mov ax, 210
    call play
    ret
a1: mov ax, 220
    call play
    ret

b1:
    mov ax,232
    call play
    ret
    
h1:
    mov ax, 247
    call play
    ret
c1:
    mov ax, 262
    call play
    ret
    
cis1:
    mov ax, 272
    call play
    ret
d:
    mov ax, 294
    call play
    ret
es1:
    mov ax, 310
    call play
    ret
e:
    mov ax, 330
    call play
    ret
f:
    mov ax,349
    call play
    ret
fis:
    mov ax, 368
    call play
    ret
g:
    mov ax,392
    call play
    ret
gis:
    mov ax, 413
    call play
    ret
a:
    mov ax,440
    call play
    ret
b:  mov ax,463
    call play
    ret
h:  mov ax,494
    call play
    ret
c:  mov ax, 523
    call play
    ret
cis:  mov ax, 551
    call play
    ret
c3:
    mov ax,1046
    call play
    ret