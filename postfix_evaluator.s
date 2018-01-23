#CSCE350
#MIPS project
# Team: Krista Capps & Francisco Almanza

.data
    M:        .space    800    # Put a number in bytes, which is N * sizeof(block_t).
    FreeList:    .word    M        # Initially it points to M.
    Top:     .word   0              # Top of Stack
    postfixspace:        .space    64
    tempchar:   .space 1
    NL:     .asciiz  "\n"
    msg1:   .asciiz  "Enter a postfix expression: "
    errmsg1: .asciiz "error: user entered char that is not int or operator\n"
    errmsg2: .asciiz "error: stack_pop attempted on empty stack\n"
    errmsg3: .asciiz "error: stack_push on full stack\n"
    errmsg4: .asciiz "error: memory deallocation attempted on NULL block\n"
    errmsg5: .asciiz "error: more than one element left in stack after postfix\n"

.text
main:   
        la $s0, M                #GLOBAL VAR: FreeList
        la $s1, FreeList         #GLOBAL VAR: Stack Top

        #For testing error message pop on empty stack:
        #jal stack_pop

        #initialize linked list
        jal mem_init

        li $v0, 4
        la $a0, msg1
        syscall                 #print enter postfix expression:
                                #read postfix expression as string
        readchar:               #loop to read in string
            la $s3, postfixspace
                loop1:
                    li $v0,8
                    la $a0,tempchar  #temporary place to put char
                    li $a1,2
                    syscall
                    lb $t0, tempchar
                    beq $t0,10,readdone  #end of string?
                    sb $t0, ($s3)
                    addi $s3,$s3,1        #address for next char
                j loop1
        #end of postfix expression
        readdone:
            la $a0, postfixspace
            addi $a1, $s3, 0
            jal eval            #call eval(postfixpace)

        addi $s4,$v0,0
        li $v0, 10 # "Exit" system call
        syscall

    #void mem init()
    #initiallizes array M with N elements of type block_t where N=100
    #data is set to zero and each block_t->next is set to the next block_t
    #$t1=i
    #$t2=N=100
    #t3=block size=8
    mem_init:
        la $t0, M
        li $t1, 0       #i=0
        li $t2, 100     #N
        li $t3, 8       #block sz
        loop:
            beq $t2, $t1, exit
            addi $t4,$t3,-4         #data sz=block sz - pointer sz
            beq $t4, 4, word        #if data size is word
                addi $t0,$t0,1      #else data size is byte
                addi $t4,$t0,4      #addr M[i+1].data (block)
                sw $t4, ($t0)       #M[i].next=M[i+1] address
                addi $t1,$t1,1      #i++
                addi $t0,$t0,4      #next block
                j loop

        word:
            addi $t0,$t0,4      #get M[i].next address
            addi $t4,$t0,4      #addr M[i+1] (block)
            sw $t4, ($t0)       #M[i].next=M[i+1] address
            addi $t1,$t1,1      #i++
            addi $t0,$t0,4      #get next block address
            j loop

        exit:
        jr $ra

   #block_t* mem_alloc()
   #function returns first available memory block from linked list data
   #updates linked list to move Freelist forward
   mem_alloc:
        la $t1, M
        addi $t2, $t1, 800      #total bytes in M
        addi $v0, $s0,0         #$v0=Freelist address
        beq $s0, $t2, exit2     #if( FreeList==NULL )
            addi $t3,$s0,0      #p=FreeList
            lw $t1, 4($s0)      #FreeList->next
            addi $s0, $t1,0     #Freelist=Freelist->next
            addi $v0,$t3,0      #return(p)
            jr $ra
        exit2:
            jr $ra              #error out of memory

   #void mem_dealloc(block_t *p)
   #function returns block p to linked list and updates Freelist
   mem_dealloc:
        addi $t1,$a0,0           #get parameter *p
        la $t0, M
        addi $t0, $t0, 800       #get last memory address
        bgt $t1,$t0, error1      #if p != NULL, if p > last allocated memory block
            addi $t1,$t1,4       #p->next
            sw $s0, ($t1)        #p->next=Freelist addr
            addi $t1,$t1,-4      #addr of p block
            addi $s0, $t1, 0     #FreeList = p
            jr $ra
        error1:
            #print null block error
            li $v0, 4
            la $a0, errmsg4
            syscall         #print error msg
            jr $ra              #called on NULL value of p

    #void set_data_int(data_t* p, int integer)
    #function sets the value of data_t p as integer
    set_data_int:
        addi $t0, $a0, 0        #get *p
        addi $t1, $a1, 0        #integer
        sw $t1, ($t0)           #p->data=integer
        jr $ra

    #void print_data(data_t* p)
    #function outputs the data of data_t p to the terminal
    print_data:
        addi $t0,$a0,0          #t0 = p
        lw $t5, ($t0)           #t5 = p->value
        addi $a0, $t5,0         #print p->value
        addi $v0,$0,1
        syscall
        jr $ra

    #void stack_push(data_t* d)
    #function accepts data item d, allocates block of memory for it
    #on the stack, then updates the value of the new block with d
    #also updates Top of stack
    stack_push:
        addi $t5,$ra,0          # save return address
        addi $t7,$a0,0          # get data, t7=d
        jal mem_alloc           # after mem_alloc, FreeList++
        addi $t6,$v0,0          # p=FreeListPast
        beq $v0,-1,stackfull    # mem_alloc returned error
        addi $a0, $t6,0         # addr for new p = old p
        addi $a1, $t7, 0        # get d value, put to integer parameter
        jal set_data_int        # set_data_int(p,d)

        la $t1,M
        beq $a0,$t1,firstelem   # if d is first stack element then Top->next should not be
                                # updated
        sw $s1,4($a0)           # Top->next=prevTop
        firstelem:
            addi $s1, $t6, 0        # Top=old Freelist
            addi $ra,$t5,0          # restore return address
            jr $ra
        stackfull:
            #print error:stack full
            li $v0, 4
            la $a0, errmsg3
            syscall         #print error msg
            jr $ra

    #data_t* stack_pop()
    #function returns block at top of stack, and updates Top to point to that block's next
    #deallocates memory which updates Freelist
    stack_pop:
        addi $t5,$ra,0          # save return address = t5
        addi $t6, $s1,0         # get Top, t6=Top
        la $t7, FreeList
        beq $t6,$t7,emptystack  # if(Top==Freelist) stack is empty
        lw $t7, 4($s1)          # get Top->prev, t7=Top->prev
        addi $s1, $t7,0         # Top=Top->prev
        addi $a0, $t6,0
        jal mem_dealloc         #deallocate old Top
        addi $v0,$t6,0          #return block_t old Top = t6
        addi $ra,$t5,0          #restore return address
        jr $ra
        emptystack:
            #print pop on empty stack error msg
            li $v0, 4
            la $a0, errmsg2
            syscall             #print error msg
            addi $ra,$t5,0      #restore return address
            jr $ra

    #int eval(char* e)
    #function evaluates a postfix expression string parameter
    #returns result value integer
    eval:
        addi $s6,$ra,0          #save return address
        addi $t4, $a0,0         #t4=char* e
        addi $s7, $a1,0         #s7=end of e
        postloop:
            lb $s2, ($t4)       #load char, s2=*e
            beq $t4,$s7, exit3  #if e[i] == end of e, postfix expression is done
            blt $s2,48,operator #if ascii < 0 then is an operator
            bgt $s2,57,error3   #if ascii > 57 then is not integer or operator
            addi $s2,$s2,-48    #otw convert to int
            addi $a0,$s2,0
            jal stack_push      #push(*e)
            addi $t4,$t4,1      #increment address, char is 1 byte, t4=e++
            j postloop          #get next char
            operator:
                beq $s2,43,addition        #if *e == +
                beq $s2,45,subtraction     #if *e == -
                beq $s2,42,multiplication  #if *e == *
                beq $s2,47,division        #if *e == /
                j error3           #otw error, *e is not operator or int

                addition:
                jal stack_pop       #get Top element
                lw $s4, ($v0)       #s4 = Top->value
                jal stack_pop       #get next Top element
                lw $s5, ($v0)       #s5 = Top->value
                add $a0,$s5,$s4     #value1+value2
                jal stack_push      #push(value1+value2)
                addi $t4,$t4,1      #increment address, char is 1 byte
                j postloop

                subtraction:
                jal stack_pop       #get Top element
                lw $s4, ($v0)       #s4 = Top->value
                jal stack_pop       #get next Top element
                lw $s5, ($v0)       #s5 = Top->value
                sub $a0,$s5,$s4     #value1-value2
                jal stack_push      #push(value1-value2)
                addi $t4,$t4,1      #increment address, char is 1 byte
                j postloop

                multiplication:
                jal stack_pop       #get Top element
                lw $s4, ($v0)       #s4 = Top->value
                jal stack_pop       #get next Top element
                lw $s5, ($v0)       #s5 = Top->value
                mul $a0,$s5,$s4     #value2*value1
                jal stack_push      #push(value2*value1)
                addi $t4,$t4,1      #increment address, char is 1 byte
                j postloop

                division:
                jal stack_pop       #get Top element
                lw $s4, ($v0)       #s4 = Top->value
                jal stack_pop       #get next Top element
                lw $s5, ($v0)       #s5 = Top->value
                div $s5,$s4         #value1/value2
                mflo $a0            #$a0=value1/value2
                jal stack_push      #push(value1/value2)
                addi $t4,$t4,1      #increment address, char is 1 byte
                j postloop

        exit3:
            jal stack_pop   #v0 will be result block_t
            sub $t2,$s1,$s0 #t2 = Freelist-Top
            bne $t2,8,error4
            addi $a0,$v0,0  #a0=result, setup for print_data
            addi $t2,$v0,0  #save return value = result = v0 = t2
            jal print_data  #print result
            addi $v0,$t2,0  #return result
            addi $ra,$s6,0  #restore return address
            jr $ra
        error3:
            #print error: user entered char that is not int or operator
            li $v0, 4
            la $a0, errmsg1
            syscall         #print error msg
            addi $v0,$0,-1
            addi $ra,$s6,0
            jr $ra
      error4:
            #print error more than one item on stack
            li $v0, 4
            la $a0, errmsg5
            syscall         #print error msg
            addi $v0,$0,-1
            addi $ra,$s6,0
            jr $ra

