%define	STK_UNIT	4

%macro	print_num	1 
    pushad 
    push %1 
    push format_digit
    call printf
    add	esp, STK_UNIT*2
    popad
%endmacro

%macro	print_octal	1
    pushad 
    push %1
    push format_Octal
    call printf
    add	esp, STK_UNIT*2
    popad
%endmacro


%macro	print_string	1
    pushad 
    push %1
    push format_string
    call printf
    add	esp, STK_UNIT*2
    popad
%endmacro

%macro	print_char 1
    pushad 
	push %1
    push format_char
    call printf
    add	esp, STK_UNIT
    popad
%endmacro

%macro pushToStack	0 ;gets a link-list of number- push it to the operand stack
	push eax
	push ebx
	mov eax, dword [stackUsed] ;the operands in the stack counter
	mov ecx, [headLink] ;the pointer to the new list head
	mov ebx, dword[operandsStack] ;pointer to the operand stack
    mov dword[ebx+eax*STK_UNIT], ecx ;the next unused spot in the stack array
	inc dword[stackUsed] 
	pop ebx
	pop eax
%endmacro

%macro addLinkData	1 ;set the data and the current link that was created
	pushad
    mov eax, dword[headLink] ;move the new link head pointer to eax
    mov byte[eax],%1   ;set the new link data- dl   
    mov ebx, [currLink]  
    mov dword[eax+1],ebx ;set the previous head link as the new link next's    
	mov dword[currLink],eax   ;set current link to point to the new head       
	mov eax, dword[currLink] ;set the currnt link as the head
	popad
%endmacro

%macro initLinks 0 ;initialize the link pointes
	mov dword[headLink], 0    
    mov dword[currLink], 0 
%endmacro

%macro createNewLink 0 ;create new link and save pointer in headLink
    pushad
    push 5
	push 1
    call calloc
    add esp, STK_UNIT*2
    mov dword[headLink], eax ;point headlink to the new allocated link
    popad
%endmacro

%macro printDebug	1	
	push ebp ;push current frame
	mov ebp, esp
	pushad
	cmp dword[debug_flag],0 ;check if debug is on
	je %%end
	mov ecx, %1	 ;mov the first link pointer to ecx		
	mov ebx, 0	;set ebx as links counter	

%%pushloop:
	cmp ecx, 0		;if link==null - finished the iteration
	je %%print_links
	movzx edx, byte[ecx]	;edx get the link data
	push edx				;save the data in the stack
	mov ecx, [ecx+1] 		;next link
	inc ebx					;increase counter
	jmp %%pushloop
%%print_links:		;print all the data of the link
	cmp ebx, 0			
	je %%endOfLine				;no links left to print
	mov byte[linkData], 0		;reset the linkData var	
	pop edx						;pop the last link data
	add edx, '0'				;convet to int
	mov byte[linkData], dl		;mov the data the linkData		
	push ebx							
	push linkData
	push format_string
	push dword[stderr]
	call fprintf				;print the last link digit to stderr
	add esp, STK_UNIT*3			;reset the stack
	pop ebx						
	dec ebx		
	jmp %%print_links				;
	
%%endOfLine:		;print '\n'
	mov byte[linkData], 10		
 	push linkData			    
	push format_string		
	push dword[stderr]
	call fprintf
	add esp, STK_UNIT*3

%%end:
	popad
	mov esp,ebp
	pop ebp
%endmacro	

%macro free_list	0
	mov eax, dword [stackUsed]	;how much we used
	mov ebx, dword[operandsStack] ;pointer to the stack
    mov ecx, dword[ebx+eax*STK_UNIT-STK_UNIT] ;pointer to the list to delete
	push ecx ; push the arg
	call clean_list ;call clean list to free ll the links
	add esp, STK_UNIT ;add 4 to recover the stack frame
%endmacro

%macro switch_links	0 ;take a data from eax, add it to a new link in the list
	pop eax ;conatins data
	mov ebx, dword[headLink]  ;move the new link to ebx
    mov byte[ebx],al		;set the data to ebx                
    mov edx, [currLink]		;set edx as the previous head
    mov dword[ebx+1],edx    ;set edx as the new head next  
	mov dword[currLink],ebx       ;set the curr link as the new head link    
%endmacro

section	.rodata	

	format_char db "%c", 0
	format_digit: db "%d", 0    
	format_string: db "%s", 0	
	format_Octal: db "%o", 10, 0    ; format octal number
	stackOverFlowmessage: 		db "Error: Operand Stack Overflow",10,0 
	emptyStackmessage: 	db "Error: Insufficient Number of Arguments on Stack",10,0
	

section .data
	stackUsed:	dd 0 ;how many operntds currently in op-stack
	stackSize: 		dd 0x5 ;the opernads stack init size
	message: db 'calc: ', 0 ;the calc printing format 	
	carry: dd 0 ;used for the addition
	totalOp: dd 0 ;succesful and unsuccesful operation counter
	debug_flag: dd 0 ; if got -d as input
	counter: dd 1	;used for the arguments init 
    argc: dd 0 		;used to keep the argc


section .bss			   
	input: resb 80		;the buffer for the input	
    headLink: resd  1	;link pointer for first link
    currLink: resd 1    ;another link pointer  
    operandsStack: resd 1   ;the pointer the the top of the operandsstack    
	linkData: resb 1		;pointer to the data of a link
	

section .text
	align 16
	global main
	extern printf
	extern fprintf 
	extern fflush
	extern malloc 
	extern calloc 
	extern free 
	extern getchar 
	extern fgets 
	extern stdin
	extern stderr

initArguments:
	mov eax, [counter] ;the arguments counter for the loop
	mov edx, [argc]		;the amount of arguments
	cmp edx, eax		;check if finished iteration
	je main_init
	mov ecx, [ebx+eax*STK_UNIT]		;ebx points to argv
	movzx edx, byte[ecx]	
	cmp edx, 45 ;the '-' ;check if its -d - Debug flag
	jne setSize ;got number as input
	mov dword[debug_flag],1 ;set debug on
	inc byte [counter] 
	jmp initArguments

setSize:
	mov eax, 0 ;init eax- to compute the new stack size
charToOctal:  ;convet the char one by one to int - octal
	movzx edx, byte[ecx] ;point to next char
	cmp edx, 0  ;finish the string - point to null
	je initStackSize
	sub edx,'0' ;convert char to int
	add eax, edx	;add the value to eax
	cmp byte[ecx+1], 0  
	je initStackSize	;next char is null
	imul eax,8	;two digits number
	inc ecx  	;increase string pointer
	jmp charToOctal

initStackSize:
	mov dword[stackSize], eax ;init stack size
	inc dword[counter]  ;inc the arguments loop counter
	jmp initArguments


main:
	push ebp
	mov ebp, esp	
	pushad
	mov edx, dword[ebp+STK_UNIT*2] ;edx point to argc
	mov dword[argc],edx 		
	mov ebx, dword[ebp+STK_UNIT*3] ;ebx point to argv
	jmp initArguments

main_init:
	mov eax,dword[stackSize]   
	push 4
	push eax    
	call calloc     ;set the operands stack size
	add esp, STK_UNIT*2
	mov dword[operandsStack], eax ;set operandsStack to point to the top of the stack
	call calculator
    
calculator:
	push ebp
	mov ebp, esp
	pushad
	print_string message ;print 'calc:'
	push dword [stdin]		
	push 80					
	push input				
	call fgets			;get input into the input buffer
	add esp, 12			;clean the stack
    movzx ebx, byte[input]  ;check the first char of the input
	push calculator
    cmp ebx, 113 	;got 'q'
	je quit
	cmp ebx, 43 	;got '+'
    je addition
	cmp ebx, 100  ;got 'd'
    je duplicate
    cmp ebx, 112	;got 'p'
    je popPrint
    cmp ebx, 38		;got '&'
    je bitAnd
    cmp ebx, 110	;got 'n'
    je numOfBytes
    jmp gotNumber	;else -got opernad (number)


quit:
	push ebp
	mov ebp, esp
	pushad
	
.free_loop: ;iterate over the operands stack and free allocations
	mov ebx, dword[operandsStack] ;point to the head of the stack
	mov eax, dword [stackUsed] 
	cmp dword[stackUsed],0		
	je .loop_End 
    mov ecx, dword[ebx+eax*STK_UNIT-STK_UNIT]	;point to the next operand	
	push ecx
	call clean_list ;free the operands' link list
	add esp, STK_UNIT ;clean stack
	dec dword[stackUsed]	
	cmp dword[stackUsed],0 
	jg .free_loop

.loop_End:
	mov eax, dword[operandsStack]
	push eax
	call free ;free the operands stack
	add esp, STK_UNIT ;reset the stack
    mov edx, dword[totalOp]	     
	print_octal edx  ;print total operations in octal
	
	popad
	mov esp,ebp
	pop ebp
    mov eax,1
    int 0x80 ;system call to exit

gotNumber: 
    push ebp
	mov ebp, esp
	pushad
    mov ebx, dword [stackUsed]
	mov eax, dword [stackSize] 
	cmp eax, ebx	;check if theres place left in the operands stack
    jg .valid 
    print_string stackOverFlowmessage	 	
	popad
	mov esp,ebp
	pop ebp
    ret 

.valid:
   	mov ecx, 0  ;set counter - for the characters of the input number
	initLinks 	;init link variables

.loop_Start:
    movzx edx, byte[input+ecx]	;iterate on the charaters of the input
    cmp edx, 10	;if next char is '\n'
	je .loop_End
	cmp edx, 0	;if next char is null			
	je .loop_End

.add_new_link: ;create link and insert
    sub edx, '0' ;convert to digit
    createNewLink  ;create new link
    addLinkData dl ;add new data to the link and set pointers
    inc ecx	
	jmp .loop_Start

.loop_End:
	pushToStack	;add new opernd to the top of the stack
	mov ecx, [headLink]
	printDebug ecx
    popad
	mov esp,ebp
	pop ebp
	ret

duplicate:
	push ebp
	mov ebp, esp
	pushad		
	inc dword[totalOp]
				
    mov eax, dword [stackSize]
    mov ebx, dword [stackUsed]
	cmp eax, ebx
	
    jne .emptyStackCheck

    print_string stackOverFlowmessage			
	jmp .done

.emptyStackCheck: ;check if the stack is empty
	mov eax, dword[stackUsed] ;check how many space we used
	cmp eax, 0	;check if there is an operand							
	jne .start ;if there is operand start the execution
	jmp emptyStackError ;if no operand throw an error and return

.start:
	mov ecx, dword[operandsStack] ;the first stack operand
	mov ebx, dword[ecx+eax*STK_UNIT-STK_UNIT]	; the first stack link			
	mov ecx,0
	cmp ebx, 0 ; to check how many push to data we done
	je .endloop ;check if null (cant be but in any case :)...)
	initLinks

.accumData:
	movzx edx, byte[ebx] ;move the data in the link		
	push edx ; save the data from the most significent bit in the number
	inc ecx ; check how many push we did
	mov ebx, dword[ebx+1] ;next link
	cmp ebx, 0 ;if the next link is null
	jne .accumData ;we done so only have to create the list for the number
	
.createDataLoop:
	cmp ecx , 0 ; if we done jump to the end
	je .endloop
	createNewLink ; create new link
	pop edx ; pop the data
    addLinkData dl ;add the 8 bytes of edx (dl) to the current link
	dec ecx
	jmp .createDataLoop ;continue

.endloop:
	pushToStack	;push the list we created to the stack
	mov ecx, [headLink]
	printDebug ecx ;check for debug
	
.done: ; done this section..
	popad
	mov esp,ebp
	pop ebp
	ret



addition:
	push ebp
	mov ebp, esp
	pushad

	inc dword[totalOp]					
	mov eax, dword[stackUsed] ;check if there are enough operands - 2
	cmp eax, 1
	jle emptyStackError
.init:	
	mov dword[carry], 0	  
	mov ecx, dword[operandsStack]
	initLinks 
	mov edx, dword[ecx+eax*STK_UNIT-STK_UNIT]		 ;pointer to first operand	
	mov ebx, dword[ecx+eax*STK_UNIT-STK_UNIT*2]		 ;pointer to 2nd operand
     
	mov ecx, 0	 ;init digits counter
.while_Add:
	cmp edx, 0
	je .firstEnd ;if the finished all digits
	cmp ebx, 0
	je .secondEnd	;if the finished all digits
.else:
	movzx eax, byte[edx] ;put the 1st link data in edx(digit)
	add eax, [carry]	 ;add carry
	mov dword[carry],0	 ;reset carry	 	
	add al, byte[ebx]	;add the 2nd operand link data				
	cmp eax, 7				;check if has carry
	jg .hasCarry					
	push eax ;save sum in stack
	mov edx, dword[edx+1]	;move 1st to next digit
	inc ecx	
	mov ebx, dword[ebx+1]	;move 2nd to next digit	
	jmp .while_Add

.hasCarry:
	mov dword[carry], 1	;turn on the carry var flag
	sub eax, STK_UNIT*2	;add the rest of sum(greater then 8)
	push eax  ;save sum in stack
	mov edx, dword[edx+1] ;iterate to next digit		
	mov ebx, dword[ebx+1] ;iterate to next digit
	inc ecx
	jmp .while_Add

.firstEnd:			
	cmp ebx, 0	;check if the 2nd is finished too
	je .addMSB
	movzx eax, byte[ebx] ;add the rest of the 2nd with the carry
	add eax, [carry]
	mov dword[carry],0 
	cmp eax, 7 ;check if has carry
	ja .addFirst_Carry	
	push eax	
	mov ebx, dword[ebx+1]	 ;next digit of the 1st
	inc ecx
	jmp addition.firstEnd

.addFirst_Carry:
	mov dword[carry], 1	;add the carry
	sub eax, 2*STK_UNIT ;remove 8 from the sum
	inc ecx	
	mov ebx, dword[ebx+1]	;get the 1st next digit
	push eax ;save sum in stack
	jmp addition.firstEnd

.secondEnd:	
	cmp edx, 0 	;check if the first ended as well
	je .addMSB
	movzx eax, byte[edx]	;get the next link data of the 1st
	add eax, [carry]	;add the carry
	mov dword[carry],0
	cmp eax, 7	;check if has carry
	jg .addSecond_Carry
	inc ecx
	push eax ;save sum in stack
	mov edx, dword[edx+1]	;get the next link of the first
	jmp .secondEnd

.addSecond_Carry:
	mov dword[carry], 1			;turn on the carry flag
	sub eax, 2*STK_UNIT			;remove 8 from the sum
	inc ecx
	push eax  ;save sum in stack
	mov edx, dword[edx+1]		;get the next link of the first
	jmp .secondEnd

.addMSB:
	cmp dword[carry],1	;check if has carry
	jl .setSum				
	push dword[carry] ;save MSB in stack
	mov dword[carry], 0
	inc ecx	

.setSum:
	createNewLink ;create a new link
	switch_links ;set the link value from the stuck
	dec ecx 
	cmp ecx,1 		;check if theres digits left
	jge .addMSB

.clean:
	mov ebx, dword[operandsStack] 
	mov edx, dword [stackUsed]
    mov ecx, dword[ebx+edx*STK_UNIT-STK_UNIT]	;point to first opernad
	push ecx
	call clean_list		;clean first operand
	add esp, STK_UNIT	;clean stack
    mov ecx, dword[ebx+edx*STK_UNIT-STK_UNIT*2]	;point to 2nd op
	push ecx
	call clean_list ;clean 2nd operand
	add esp, STK_UNIT	;clean stack
	dec dword [stackUsed] ;decrease on in the stack used
	mov ecx, [headLink]
	mov ebx, dword[operandsStack]
	mov edx, dword [stackUsed]
    mov dword[ebx+edx*STK_UNIT-STK_UNIT], ecx	 ;print the sum if on debug			
	printDebug ecx

	popad
	mov esp,ebp
	pop ebp
	ret

popPrint: ;remove from operandsStack[stackUsed], decrease stackUsed & print the number
	push ebp
	mov ebp, esp
	pushad

	inc dword[totalOp]					; inc num of operations
	mov eax, dword[stackUsed]
	cmp eax, 0							; stackUsed=0 there's nothing to pop
	je emptyStackError
	mov ecx, dword[operandsStack]
	mov ebx, dword[ecx+eax*STK_UNIT-STK_UNIT]	; ebx will hold the pointer to the lat digit of the number;
	
	mov ecx,0
.pushListToStack:	; pushing the whole number-list to the stack
	cmp ebx, 0					
	je .print_links
	inc ecx	  ;the links counter
	movzx edx, byte[ebx]	;save the data of the curr link in the stack	
	push edx				
	mov ebx, dword[ebx+1]	;next link
	jmp popPrint.pushListToStack
	
.print_links:		
	pop edx		;get the link data
	add edx, '0' ;convert digit to char
	mov byte[linkData], dl			
	push ecx						
	push linkData
	push format_string
	call printf			;print the links' data
	add esp, STK_UNIT*2
	pop ecx
	dec ecx
	cmp ecx, 0
	jg .print_links

	mov byte[linkData], 10 ;print '\n'
 	push linkData			  
	push format_string		
	call printf
	add esp, STK_UNIT*2
	free_list			;free the printed operand
	dec dword [stackUsed]	;decrease the number of operands in the stack

	popad
	mov esp,ebp
	pop ebp
	ret

emptyStackError:
	print_string emptyStackmessage
	popad
	mov esp,ebp
	pop ebp
	ret

bitAnd:
	push ebp
	mov ebp, esp
	pushad


	inc dword[totalOp]	; increment total operands				
	mov eax, dword[stackUsed] ;check gow much operands in stack
	cmp eax, 1 ; check if we have lower then 2 operands
	jle emptyStackError ; if the sufficient num of operands print error						
	
	mov ecx, dword[operandsStack] ; pointer to stack
	mov edx, dword[stackUsed] ; pointer to the last cell in stack
	mov ebx, dword[ecx+edx*STK_UNIT-STK_UNIT] ;pointer to the first operand
	mov eax, dword[ecx+edx*STK_UNIT-STK_UNIT*2] ;pointer to the second operand

	initLinks ;initial the links      
	
	mov ecx, 0 ;start an counter
.accumDataLoop:
	
	movzx edx, byte[ebx] ;move the byte			
	and dl, byte[eax] ;use and on the bits from the last significent
	push edx ;save the answer'
	mov eax, dword[eax+1] ;next link			
	mov ebx, dword[ebx+1]	;next link			
	inc ecx ;inc the pointer
	cmp eax, 0 ;check if the first is null
	je .createList ;jump to create the operand
	cmp ebx, 0 ;check if secont is null
	je .createList ;jump to create the operand
	jmp .accumDataLoop ;continue the link

.createList: ;iterate over the digints and create list of the digints
	createNewLink ;create a new link	
	pop edx ;pop the digits from the most significant bit
	addLinkData dl ; add the data to the new link we create
	dec ecx ;dec the counter of the digits
	cmp ecx,0
	jne .createList

					
	free_list ;free the link
	dec dword[stackUsed]
	free_list
	inc dword[stackUsed]

	dec dword [stackUsed] ;deck once
	dec dword [stackUsed] ;deck two nd time
    pushToStack ; push to last cell of the stack the list we made
	mov ecx, [headLink]
	printDebug ecx ;print debugg if need
	popad
	mov esp,ebp
	pop ebp
	ret


numOfBytes: ;check how if there is any stack operands
	push ebp
	mov ebp, esp
	pushad
	inc dword[totalOp]	;inc the total operations			
	mov eax, dword[stackUsed] ;see how much space we used
	cmp eax, 0 ; check if there is operand			
	je emptyStackError ; error if no operands avaidable in stack

	mov edx, dword[operandsStack] ;pointer to the stack
	mov ebx, dword[edx+eax*STK_UNIT-STK_UNIT] ;pointer to the first list		
	push ebx
	call num_of_bytes ; number of bytes
	add esp, STK_UNIT
	initLinks ;initial the new link						
.continuewhile:	
	mov eax, edx	
	mov edx , 0
	mov ecx, 8
	div ecx
	cmp edx, 0 ;check if there is a carry
	je .resault
	add eax, 1

.resault:
	mov edx, eax ;eax holds the result
	mov ebx, 0
.result_loop:
	mov edx, 0
	mov ecx, 8
	div ecx
	push edx ;save the result
	
	inc ebx
	cmp eax, 0
	jg .result_loop
.createNumber:
	pop edx
	createNewLink
	addLinkData dl
	dec ebx
	cmp ebx , 0
	jg .createNumber

	free_list ;free first link

	pushToStack
	mov ecx, [headLink]
	printDebug ecx
	
	popad
	mov esp,ebp
	pop ebp
	ret


clean_list:
	push ebp
	mov ebp, esp
	pushad
	mov ecx, dword[ebp+STK_UNIT*2] 		;pointer to the argument
	cmp dword[ecx+1], 0	 ;check if the next is null		
	je .nextNull
			
	push dword[ecx+1]	;clean the next link
	call clean_list				
	add esp, STK_UNIT
	
.nextNull:
	push ecx
	call free 	;free the current link
	add esp, STK_UNIT			
	
	popad
	mov esp, ebp
	pop ebp
	ret






num_of_bytes:	
	push ebp
	mov ebp, esp
	push ebx
	push ecx
	pushfd
	
	mov edx, 0
	mov ebx, [ebp + STK_UNIT*2]
	mov eax, 0

.numbersCount:	
	cmp ebx, 0	
	jz .end_count
	movzx ecx, byte[ebx] 
	inc eax		
	add edx , 3				
	mov ebx, dword[ebx+1]
	jmp num_of_bytes.numbersCount
.end_count:

	sub edx, 3

	cmp ecx, 2
	jl .addOne
	cmp ecx, 4
	jl .addTwo
	add edx , 3
	jmp .endL
.addTwo:
	add edx , 2
	jmp .endL
.addOne:
	inc edx

.endL:
	popfd
	pop ecx
	pop ebx
	mov esp, ebp
	pop ebp
	ret
