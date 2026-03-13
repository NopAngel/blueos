[bits 32]
global bios_int_call


bios_int_call:
    pushad              
    mov ebp, esp      
    db 0xCD            
    int_no db 0         
    
    popad               
    ret