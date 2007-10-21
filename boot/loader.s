.global _loader           #Entry

# multiboot header 
.set ALIGN, 1<<0             
.set MEMINFO, 1<<1             
.set FLAGS_MBH, ALIGN | MEMINFO  
.set MAGIC_MBH, 0x1BADB002       
.set CHECKSUM,-(MAGIC_MBH+FLAGS_MBH) 

.align 4
.long MAGIC_MBH
.long FLAGS_MBH
.long CHECKSUM

.set STACKSIZE, 0x4000         
.comm stack, STACKSIZE, 32 

_loader:
   mov   $(stack + STACKSIZE), %esp 
   push  %eax                       
   push  %ebx             
   cli          

#Start Kernel's main-function
   call  _kmain           
   cli
   hlt                   

.global att_lgdt
.extern gp
att_lgdt:
   lgdt gp
   ret

.global idt_load
.extern idtp
idt_load:
    lidt idtp
    ret
