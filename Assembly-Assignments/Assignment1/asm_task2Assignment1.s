section	.rodata			; we define (global) read-only variables in .rodata section
	format_string: db "%s", 10, 0	; format string

section .bss			; we define (global) uninitialized variables in .bss section
	an: resb 33		; enough to store integer in [-2,147,483,648 (-2^31) : 2,147,483,647 (2^31-1)]

	 
	
section .text
	global convertor
	extern printf



convertor:
	push ebp
	mov ebp, esp
	sub esp, 4
	mov dword[ebp-4], 0	
	pushad
	
	


	mov ebx, 0

initAn:
	mov byte [an + ebx], 0	
	inc ebx	
	cmp ebx, 34	
	jl initAn

	mov ecx, dword [ebp+8]	; get function argument (pointer to string)
	cmp byte [ecx], 113 ;jump and end the loop.
	je endFuncWithQ 

	mov ebx, 0

mainWhile:
	mov dl, byte [ecx + ebx] ;move the curr byte to dl
	cmp dl, 10 ;if dl is a null end the function
	je printBinary 
	cmp dl, 0 ;if dl is new line end the function.
	je printBinary
	cmp dl, 65	; check if the byte is a letter
	jge isLetter
	sub dl, 48
	jmp convertBinary
isLetter:
	sub dl,55

convertBinary:	
	mov eax, 0
	shl dl, 4

whileBinaryCon:
	cmp eax, 4
	je endLoop
	shl dl,1
	jc addBinaryOne
	mov byte [an + eax + ebx * 4], 48
	inc eax
	jmp whileBinaryCon
addBinaryOne:
	mov byte [an + ebx * 4 + eax], 49
	inc eax
	jmp whileBinaryCon


endLoop:
	inc ebx 
	jmp mainWhile

printBinary:
	push an			; call printf with 2 arguments -  
	push format_string	; pointer to str and pointer to format string
	call printf
	add esp, 8		; clean up stack after call
	jmp endFunc

	endFuncWithQ:
		mov dword[ebp-4] ,1
	

	endFunc:
		popad	
		mov eax, dword[ebp-4]		
		mov esp, ebp	
		pop ebp
		ret
