;----------------------------------------
;Bearbeiter: 
;----------------------------------------
;.MODEL TINY
;.CODE
;IDEAL
ORG 100H

;PROGRAM:
;----------------------------------------

	call uhrstellen

  	cli              ;Initialisierung
        call IVTABinit   ;  der Vektortabelle
	call prePICinit
        call PICinit     ;  des Interruptcontrollers
        call PITinit     ;  des Zeitgeberschaltkreises
        sti
	
	call readhex	
	mov bx,ax	;"parameteruebergabe"

nochmal:
	call printadr	;adresse und inhalt anzeigen
	
	;taste abwarten
	mov ah,1
	int 5

	;taste auswerten
	cmp al,16h
	
	jne minus	;plus gedrueckt?
	inc bx
minus:	cmp al,17h
	jne eingabe	;minus gedrueckt?
	dec bx
eingabe:cmp al,10h
	jne go		;enter gedrueckt?
	call readhex	;neue adresse einlesen
	mov bx,ax
go:	cmp al,11h
	jne nochmal		;go gedrueckt?
	call uhraktivieren	;uhr anzeigen
	
	jmp nochmal	;von vorn


prePICinit:
	 mov al,00010011b
        out 0c0h,al             ;icw1
        jmp short $+2           ;I/O-Delay
        mov al,00001000b
        out 0c2h,al             ;icw2
;        jmp short $+2           ;I/O-Delay
        mov al,00000001b
        out 0c2h,al             ;icw4 (EOI-Modus)
;        jmp short $+2           ;I/O-Delay
SetMask: mov al,11111111b
        out 0c2h,al             ;ocw1 (alle Int's gesperrt)
;        jmp short $+2           ;I/O-Delay
        ret

;====================================================	
readhex:
;adresse einlesen
;rueckgabe in ax

	push bx
	push dx

;display loeschen	
	mov ah,0
	int 6

;eingabeaufforderung
	mov ah,2
	mov bx,string
	mov dl,7
	int 6	

;eingabe
	mov ah,2
	mov bx,0
	mov dl,3
	int 5
	
	pop dx
	pop bx

	ret

;====================================================
printadr:
; adresse/adressinhalt ausgeben
; adresse in bx uebergeben;
	push ax
	push dx
	push bx

	mov ah,0
	int 6
	
;adresse ausgeben	
	mov ah,3
	mov dl,7
	int 6

;speicherinhalt ausgeben
	mov ah,4	
	mov bl,[bx]
	mov dl,1
	int 6

	pop bx
	pop dx
	pop ax
	ret

;====================================================
uhrstellen:
	push ax
	push bx
	push cx

	mov bx,stu
	
nochmal2:
	mov ah,0
	mov al,[stu]
	cmp al, 255
	jne st
	mov al,23
st:
	div byte [stuueberlauf]
	mov [stu],ah

	mov ah,0
	mov al,[min]
	cmp al,255
	jne mi
	mov al,59
mi:
	div byte [minsekueberlauf]
	mov [min],ah

	mov ah,0
	mov al,[sek]
	cmp al,255
	jne se
	mov al,59
se:
	div byte [minsekueberlauf]
	mov [sek],ah
	call uhrzeitausgeben
	;taste abwarten
	mov ah,1
	int 5

	;taste auswerten
	cmp al,16h
	
	jne minus2	;plus gedrueckt?
	inc byte [bx]
minus2:	cmp al,17h
	jne eingabe2	;minus gedrueckt?
	dec byte [bx]
eingabe2:cmp al,10h
	jne go2		;enter gedrueckt?
	jmp return
go2:	cmp al,11h
	jne nochmal2		;go gedrueckt?
	
	cmp bx, stu	;zu stellenden wert um eins weiterrcken
	jne minset
	mov bx, min
	jmp nochmal2
minset:	cmp bx, min
	jne sekset
	mov bx, sek
	jmp nochmal2
sekset:	mov bx, stu

	jmp nochmal2	;von vorn		
return:
	pop cx
	pop bx
	pop ax
	ret
;====================================================	
;aktiviert die uhr
uhraktivieren:
	push ax
	mov byte [uhran],1 ;uhran ist flag fuer die anzeige der uhr

	;eine taste abwarten
	mov ah,1
	int 5
	mov byte [uhran],0	;uhr wieder aus

	pop ax
	ret
;====================================================	
;gibt uhrzeit auf dem display aus
uhrzeitausgeben:	
	push ax
	push dx	
	push bx

	;maske auf display legen (bindestriche)
	mov ah,2
	mov bx, uhrformat
	mov dl,7
	int 6		;maske ausgeben auf display
	
	;uhranzeige aktualisieren:
	mov al,[stu]
	mov dl,7
	call printdec		;aktuelle stunden anzeigen
	
	mov al,[min]
	mov dl,4		
	call printdec		;aktuelle minuten anzeigen
	
	mov al,[sek]
	mov dl,1
	call printdec		;aktuelle sekunden anzeigen

	pop bx
	pop dx
	pop ax	
	ret

;======================================
;uhr eine sekunde weiterschalten
uhrtakt:
	inc byte [sek]		;wieder eine sekunde mehr
	cmp byte [sek],60		;haben wir schon eine minute?
	jne ende		;nein -> ende
	mov byte [sek],0		;doch -> sekunden 0
	inc byte [min]		;eine minute mehr
	cmp byte [min],60		;haben wir schon eine stunde?
	jne ende		;nein -> ende
	mov byte [min],0		;doch -> minuten 0
	inc byte [stu]		;eine stunde mehr
	cmp byte [stu],24		;ist die uhr am ende?
	jne ende		;nein -> ende
	mov byte [stu],0		;ja -> neuer tag beginnt mit stunde 0
ende:
	ret

;======================================	

printdec:
;dezimalzahl in ax
;stelle in dl
;anzeige der 2stelligen zahlen jeweils durch ganzzahl und restdivision
	push cx
	mov ah,0
	div byte [d]

;divisionsergebnis in dx sichern
	mov cx,ax	

;erste stelle ausgeben
	mov ah,1 
	mov al,cl
	add al,48
	int 6

;zweite stelle ausgeben	
	mov ah,1
	mov al,ch
	add al,48
	dec dl
	int 6

	pop cx
	ret
;=============================================0

;Interruptcontroller initialisieren
;----------------------------------
PICinit:
         in al,0c2h       ;Lesen des Int.-Maskenregisters des PIC
         and al,11111110b
         out 0c2h,al      
         ret
;
;Zeitgeber initialisieren
;  Interrupt alle 10 ms
;------------------------
zk equ 18432         ;Zeitkonstante fuer 10-ms-Interrupt
PITinit:
         mov al,01110110b
         out 0a6h,al      ;Zaehler 1 im Mode 3
         mov al,zk&0000000011111111b
         out 0a2h,al      ;Low-Teil der Zeitkonstante laden
         mov al,zk>>8
         out 0a2h,al      ;Hi-Teil der Zeitkonstante laden
         ret
;

;----------------------------------------
IVTABinit:
     
;***** PROGRAMM
; Adresse der ISR (Offset und Segment) in der
; Interrupt-Vektor-Tabelle auf Vektor 8 eintragen
	mov ax, isr8 	;adresse der ISR in ax speichern
	mov [0020h],ax 		;speichern der ISR adresse in die ISR Tabelle (int 8)
        ret

;Interruptserviceroutine
;-----------------------
isr8:   push ax
; ev. weitere Register retten

;***** ISR-PROGRAMM
	cmp byte [uhran],1		;soll die Uhr angezeigt werden?
	jne takt		;nein? -> takt fortfuehren
	call uhrzeitausgeben	;wenn doch -> uhr anzeigen und takt fortfuehren
takt:
	inc byte [foo]  	;zaehlen
	cmp byte [foo],100	;alle 100 interrupts (jede sekunde) anzeige aktualisieren
	jne isrret
	mov byte [foo],0	;nach 100 interrupts den zaehler auf 0 setzen
	call uhrtakt	;uhrzeit im speicher aktualisieren

isrret: mov al,20h
        out 0c0h,al      ;PIC wieder freigeben
	pop ax		 ;ev. gerettete Register zurueckholen
        iret
;==============================================================
string db "Adr" ;eingabeaufforderung bei adresseingabe
       db 00
stu	db 18	;stunde
min	db 00	;minute
sek	db 00	;sekunde
d	db 10	;konstante fuer division
foo	db 00	;zaehlvariable fuer isr
uhran	db 00	;flag (uhranzeige an/aus)
stuueberlauf db 24
minsekueberlauf db 60

uhrformat db "  -  -  " ;uhrmaske
	  db 00


;END PROGRAM
