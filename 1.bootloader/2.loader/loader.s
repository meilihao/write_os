.intel_syntax noprefix
.code16
.text
.globl _start
_start:
	mov	ax,	cs
	mov	ds,	ax
	mov	es,	ax
	mov	ax,	0x00
	mov	ss,	ax
	mov	sp,	0x7c00

# =======	display on screen : Start Loader......

	mov	ax,	0x1301
	mov	bx,	0x000f
	mov	dx,	0x0200		# row 2
	mov	cx,	12
	push	ax
	mov	ax,	ds
	mov	es,	ax
	pop	ax
	lea	bp,	StartLoaderMessage
	int	0x10

	jmp	$

# =======	display messages

StartLoaderMessage:	.ascii	"Start Loader"
