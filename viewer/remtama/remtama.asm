; * ----------------------------------------------------------------------------
; * "THE BEER-WARE LICENSE" (Revision 42):
; * Jeroen Domburg <jeroen@spritesmods.com> wrote this file. As long as you retain 
; * this notice you can do whatever you want with this stuff. If we meet some day, 
; * and you think this stuff is worth it, you can buy me a beer in return. 
; * ----------------------------------------------------------------------------


start:
	;Show WiFi image
	LDA #0
	JSR display_image
	sei	;disable ints
	;Clock cpu at full speed
;	lda #$0
;	sta $3001
	ldx #$0

imgloop:
	;Make $00-$01 point to start of display RAM
	lda #$0
	sta $00
	lda #$10
	sta $01

byteloop:
	lda $3012	;read port a
	and #$4
	bne imgloop

	lda #$0
	sta $02
	
bit0:
	lda $3012
	tax
	and #$2
	bne bit0
	rol $2
	txa
	and #$1
	ora $2
	sta $2
bit1:
	lda $3012
	tax
	and #$2
	beq bit1
	rol $2
	txa
	and #$1
	ora $2
	sta $2
bit2:
	lda $3012
	tax
	and #$2
	bne bit2
	rol $2
	txa
	and #$1
	ora $2
	sta $2
bit3:
	lda $3012
	tax
	and #$2
	beq bit3
	rol $2
	txa
	and #$1
	ora $2
	sta $2
bit4:
	lda $3012
	tax
	and #$2
	bne bit4
	rol $2
	txa
	and #$1
	ora $2
	sta $2
bit5:
	lda $3012
	tax
	and #$2
	beq bit5
	rol $2
	txa
	and #$1
	ora $2
	sta $2
bit6:
	lda $3012
	tax
	and #$2
	bne bit6
	rol $2
	txa
	and #$1
	ora $2
	sta $2
bit7:
	lda $3012
	tax
	and #$2
	beq bit7
	rol $2
	txa
	and #$1
	ora $2
	sta $2
	
	
	
	ldx #0
	lda $02
	sta ($00,x)

	clc
	lda #$1
	adc $0
	sta $0
	lda #$0
	adc $1

	;safety: keep in $1000 range
	and #$3
	ora #$10

	sta $1

	jmp byteloop

