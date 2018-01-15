	.data
srcmat:	.word	23
	.word	3
	.word	7
	.word	1
	.word	8
	.word	8
	.word	8
	.word	6
	.word	6
	.word	8
	.word	3
	.word	4
	.word	2
	.word	3
	.word	4
	.word	3
	.word	6
	.word	4
	.word	4
	.word	6
	.word	2
	.word	2
	.word	4
	.word	8
	.word	1
	.word	7
	.word	3
	.word	6
	.word	5
	.word	5
	.word	7
	.word	7
	.word	7
	.word	2
	.word	8
	.word	7
	.word	4
	.word	1
	.word	5
	.word	8
	.word	7
	.word	8
	.word	5
	.word	1
	.word	5
	.word	1
	.word	4
	.word	8
	.word	4
	.word	3
	.word	8
	.word	4
	.word	7
	.word	7
	.word	7
	.word	8
	.word	2
	.word	8
	.word	8
	.word	1
	.word	4
	.word	7
	.word	6
	.word	1
dstmat:	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
		.text
main:
	la	$5, srcmat 
	la	$6, desmat 
	ori	$7, $7, 8
	addiu	$8, $7, -1
outer1:	
	subu	$3, $8, $1	
	andi	$2, $0,	0	
inner:	
	subu	$4, $8, $2
	sll	$9, $1, 3
	addu 	$9, $9, $2
	sll	$9,  $9, 2
	addu	$11, $5, $9
	lw	$13, 0($11)
	sll	$10, $3, 3
	addu	$10, $10, $4
	sll 	$10, $10, 2
	addu 	$12, $6, $10
	sw	$13, 0($12)
	addiu 	$2, $2, 1
	bne	$7, $2, inner

outer2:
	addiu   $1, $1, 1
	bne	$7, $1, outer1	
end:
	addiu $22, $22, 0
	addiu $22, $22, 0
	addiu $22, $22, 0
