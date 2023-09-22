; Copyright by Xiyue87 2022

.MODEL flat
.CODE

_goto_to_64 PROC

; load new GDT for long mode
mov eax, [esp + 4]
lgdt fword ptr [eax]

; set page table base
mov eax, [esp + 8]
mov cr3, eax

; enable PAE
mov eax, cr4
or eax, 00000020h           ; PAE flag(1 << 5)
mov cr4, eax

; enable IA32_EFER.LME
mov ecx, 0C0000080h         ; 0xc0000080 IA32_EFER
rdmsr
or eax, 00000100h           ; IA32_EFER.LME flag(1 << 8)
wrmsr

; enable paging
mov eax, cr0
or eax, 80000000h           ; PG flag(1 << 31)
mov cr0, eax

mov eax, 30h
mov ds, ax
mov es, ax
mov ss, ax

; currently we are still in 32 bit code segment
; retf will pop 32 bit CS:IP and return to 64 bit mode

mov ecx, [esp + 12]         ; return IP for 64 long mode
mov eax, 20h                ; return CS for 64 long mode
push eax
push ecx
retf

_goto_to_64 ENDP


END