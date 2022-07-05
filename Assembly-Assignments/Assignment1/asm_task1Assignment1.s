section	.rodata			; we define (global) read-only variables in .rodata section
	format_integer: db "%d", 10, 0	; format string

section .bss			; we define (global) uninitialized variables in .bss section
	an: resb 12		; enough to store integer in [-2,147,483,648 (-2^31) : 2,147,483,647 (2^31-1)]

section .text
	global assFunc
	extern c_checkValidity
    extern printf

assFunc:
	push ebp
	mov ebp, esp	
	pushad			

	mov ecx, dword [ebp+8]	; get function argument (pointer to string)

	; your code comes here...

	push ecx			; call printf with 2 arguments -  
	call c_checkValidity
	add esp, 4		; clean up stack after call
    cmp eax ,0
    je odd
    shl ecx,2
    jmp end

    odd:
        shl ecx,3

    end:
        push dword ecx
        push format_integer
        call printf
        add esp, 8
	    popad			
	    mov esp, ebp	
	    pop ebp
	    ret
