
WIDTH EQU 160
HEIGHT EQU 199
  ;Change the video mode to 13h
    mov  ax, 0013h              ;Put the desired graphics mode into AL
    ;int  10h
  ;Prepare for writing to the video buffer
    mov  di, cs
    add  di, 1000h
    mov  ds, di
    mov  di, 0a000h             ;Put the video segment into DI
    mov  es, di                 ;so it can easily be put into DS
    mov  cx, -1
    mov  dx, 0
    mov  di, 0
clear_t_buffer
    mov  [ds:di], dx
    add  di, 2
    loop clear_t_buffer
  ;------------Set the palette
  ;  The first step is to zero all of the colors in the palette.  This
  ;is important is both the highest-numbered and lowest-numbered colors
  ;used should be set to black
    mov  cx, 255                ;Number of colors to change
    mov  dx, 03c8h              ;Port address of color-write selector
clear_palette:
    mov  al, cl                 ;Load Color number to change
    out  dx, al                 ;Set the color number
    inc  dx                     ;Change port number to 03c9h, which is the
                                ;port to write to actually change the
                                ;color
    mov  al, 0
    out  dx, al                 ;Set RED compenent
    out  dx, al                 ;and GREEN
    out  dx, al                 ;and BLUE
    dec  dx                     ;Change port addr back to 03c8h
    loop clear_palette
  ;The next step is to set colors 10-73 to a smooth black-to red
  ;progression.
    mov  cl, 63                 ;Number of colors to change
black_to_red:
    mov  al, cl                 ;Load Color number to change
    add  al, 10
    out  dx, al                 ;Set the color number
    inc  dx                     ;Change port number to 03c9h, which is the
                                ;port to write to actually change the
                                ;color
    mov  al, cl                 ;Red is the progressing color
    out  dx, al                 ;Set RED compenent
    mov  al, 0
    out  dx, al
    out  dx, al
    dec  dx                     ;Change port addr back to 03c8h
    loop black_to_red
  ;The final step in the color setting is to set colors 74-136 to a smooth
  ;red-to-yellow progression.
    mov  cl, 63                 ;Number of colors to change
red_to_yellow:
    mov  al, cl                 ;Load Color number to change
    add  al, 73
    out  dx, al                 ;Set the color number
    inc  dx                     ;Change port number to 03c9h, which is the
                                ;port to write to actually change the
                                ;color
    mov  al, 63                 ;Here RED is always the brightest possible
    out  dx, al                 ;Set RED compenent
    mov  al, cl                 ;Green is the element that is changed to
                                ;provide a progression from red to yellow
    out  dx, al                 ;and GREEN
    mov  al, 0                  ;Blue is still zero
    out  dx, al                 ;and BLUE
    dec  dx                     ;Change port addr back to 03c8h
    loop red_to_yellow
  ;------------End of palette setting (at last!)
  ;Set-up random number generation
    mov  si, 0100h
main_loop:
    mov  di, 321
    mov  cl, HEIGHT-1
  ;Here is the main processing loop
    mov  bh, 0                  ;If the upper part of BX is 0, a byte
                                ;can be converted to a word with it
                                ;by simply moving the byte into BL.
process_image:
    push cx                     ;Get outer loop counter out of the way
    mov  cl, WIDTH
process_a_line:
    mov  bl, [ds:di]            ;This pixel is loaded and mov-ed into
    mov  ax, bx                 ;ax to clear whatever might have
                                ;been there before
    mov  bl, [ds:di-1]          ;The pixel immediately to the left
    add  ax, bx
    mov  bl, [ds:di+1]          ;The pixel immediately to the right
    add  ax, bx
    mov  bl, [ds:di+320]        ;The pixel immediately below
    add  ax, bx
    sar  ax,1
    ;, 2                  ;Shortcut to divide by four
    sar ax,1
    mov  [ds:di-320], al
    mov  [es:di-320], al
    inc  di
    loop process_a_line
    pop  cx                     ;Restore outer loop counter
    add  di, 320-WIDTH
    loop process_image
  ;Here is our phony random number generator
    mov  cl, WIDTH
    mov  di, (HEIGHT-1)*320
    mov  bx, 168
drawrandomheat:
    mov  [ds:di], bh            ;Clear this point (we may plot it again
                                ;later)
    mov  al, [cs:si]            ;Load part of our own code
    cmp  al, 240                ;Negative ~93.75% of the time
    jae  nospark                ;In this (unlikely) case no spark
                                ;is generated at this point
    mov  [ds:di], bl            ;Set the memory byte
nospark:
    inc  di                     ;Move on to the next pixel
    inc  si                     ;Increment the pointer thru our code
                                ;segment (this is quite important,
                                ;as without it, the "random" values would
                                ;be completely homogeneous without it!)
    cmp  si, 021e7h             ;Reset the registers in case it wanders
    jnz  no_overflow
    mov  si, 0100h
no_overflow:
    loop drawrandomheat
    mov  di, 321
    mov  ah, 1                  ;Check if the user pushed a kee
    ;int  16h
    ;jnz  byebye
    jmp  main_loop
byebye:
    mov  ax, 0003h              ;Just incase the user actually liked
    ;int  10h                    ;Mode 3 (which many idiots do...)
    mov  ax, 4c00h              ;Exit to Dumb Operating System
    ;int  21h                    ;-)
