;----------------------------------------
;Bearbeiter: 
;----------------------------------------
;.MODEL TINY
;.CODE
;IDEAL
ORG 100H

viel equ 20000

;PROGRAM:

;----------------------------------------

links:
	mov cx,8
	mov al,1
s2:
	out 0,al
	shl al,1

	mov bx,viel
nochmal:	
	dec bx
	jnz nochmal

	loop s2

	push ax
	in al,0
	cmp ax,1
	je rechts
	pop ax

	loop links


rechts:
	mov cx,8
	mov al,10000000b
s4:
	out 0,al
	shr al,1

	mov bx,viel
nochmal2:	
	dec bx
	jnz nochmal2

	loop s4

	push ax
	in al,0
	cmp ax,1
	je links
	pop ax	


	loop rechts

;----------------------------------------
;END PROGRAM
