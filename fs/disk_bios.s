; disk on bios (x86)
;    - NopAngel

DAPACK:
	db	0x10
	db	0
blkcnt:	dw	16			
db_add:	dw	0x7C00		
	dw	0				
d_lba:	dd	1			
	dd	0				

	mov si, DAPACK		
	mov ah, 0x42		
	mov dl, 0x80		
	int 0x13
	jc short .error
	
check_edd:

	mov ah, 0x41 
	mov bx, 0x55aa
	int 0x13
	jc .no_edd
	cmp bx, 0xaa55
	jne .no_edd
	test cx, 1		
	jz .no_edd
.no_edd: