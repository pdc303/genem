int ea_mode_time[ADDRESSING_MODE_COUNT][2] = 
{
/*	BW   L		*/
	 0,  0,
	 0,  0,

	 4,  8,
	 4,  8,

	 6, 10,
	 8, 12,

	10, 14,
	 8, 12,

	12, 16,
	 8, 12,

	10, 14,
	 4,  8
};
/*
move_time[src mode][dest mode][size] 
	size = 0 : Byte/Word
	size = 1: Long
*/
int move_time[2][ADDRESSING_MODE_COUNT][9] = 
{
	 4,  4,  8,  8,  8, 12, 14, 12, 16,
	 4,  4,  8,  8,  8, 12, 14, 12, 16,
	 8,  8, 12, 12, 12, 16, 18, 16, 20,

	 8,  8, 12, 12, 12, 16, 18, 16, 20,
	10, 10, 14, 14, 14, 18, 20, 18, 22,
	12, 12, 16, 16, 16, 20, 22, 20, 24,

	14, 14, 18, 18, 18, 22, 24, 22, 26,
	12, 12, 16, 16, 16, 20, 22, 20, 24,
	16, 16, 20, 20, 20, 24, 26, 24, 28,

	12, 12, 16, 16, 16, 20, 22, 20, 24,
	14, 14, 18, 18, 18, 22, 24, 22, 26,
	 8,  8, 12, 12, 12, 16, 18, 16, 20,

	/* ---------------------------- */

	 4,  4, 12, 12, 12, 16, 18, 16, 20,
	 4,  4, 12, 12, 12, 16, 18, 16, 20,
	12, 12, 20, 20, 20, 24, 26, 24, 28,
	
	12, 12, 20, 20, 20, 24, 26, 24, 28,
	14, 14, 22, 22, 22, 26, 28, 26, 30,
	16, 16, 24, 24, 24, 28, 30, 28, 32,

	18, 18, 26, 26, 26, 30, 32, 30, 34,
	16, 16, 24, 24, 24, 28, 30, 28, 32,
	20, 20, 28, 28, 28, 32, 44, 32, 36,

	16, 16, 24, 24, 24, 28, 30, 28, 32,
	18, 18, 26, 26, 26, 30, 32, 30, 34,
	12, 12, 20, 20, 20, 24, 26, 24, 28
};

int lea_time[ADDRESSING_MODE_COUNT] = {
	[ADDRESSING_MODE_AN_IND] = 4,
	[ADDRESSING_MODE_AN_IND_DISPL] = 8,
	[ADDRESSING_MODE_AN_IND_INDEX] = 12,
	[ADDRESSING_MODE_ABS_SHORT] = 8,
	[ADDRESSING_MODE_ABS_LONG] = 12,
	[ADDRESSING_MODE_PC_IND_DISPL] = 8,
	[ADDRESSING_MODE_PC_IND_INDEX] = 12
};

/* base times */

/* instructions which have fairly simply timing conditions */
enum BASE_TIME {
	BASE_TIME_TST = 4,
	
	BASE_TIME_BCC_Y = 10, /* BCC, Branch Yes */
	BASE_TIME_BCC_N_B = 8, /* BCC, Branch No. Byte */
	BASE_TIME_BCC_N_W = 12, /* BCC, Branch No. Word */

	BASE_TIME_DBCC_N_CCTRUE = 20, /* DBCC, Branch not taken, CC True */
	BASE_TIME_DBCC_Y_CCFALSE = 18, /* DBCC, Branch taken, CC False */
	BASE_TIME_DBCC_N_CCFALSE = 26, /* DBCC, Branch not taken, CC False */
	
	BASE_TIME_ANDI_DN_BW = 8, /* ANDI, DN operand, Byte/Word */
	BASE_TIME_ANDI_DN_L = 14, /* ANDI, DN operand, Long */
	BASE_TIME_ANDI_M_BW = 12, /* ANDI, Memory operand, Byte/Word */
	BASE_TIME_ANDI_M_L = 20, /* ANDI, Memory operand, Long */
	
	BASE_TIME_CMPI_DN_BW = 8, /* CMPI, DN operand, Byte/Word */
	BASE_TIME_CMPI_DN_L = 14, /* CMPI, DN operand, Long */
	BASE_TIME_CMPI_M_BW = 8, /* CMPI, Memory operand, Byte/Word */
	BASE_TIME_CMPI_M_L = 12, /* CMPI, Memory operand, Long */
	
	BASE_TIME_CMP_L = 4, /* CMP, Long */
	BASE_TIME_CMP_BW = 6, /* CMP, Byte/Word */
	
	BASE_TIME_EORI_DN_BW = 8, /* EORI, DN operand, Byte/Word */
	BASE_TIME_EORI_DN_L = 16, /* EORI, DN operand, Long */
	BASE_TIME_EORI_M_BW = 12, /* EORI, Memory operand, Byte/Word */
	BASE_TIME_EORI_M_L = 20, /* EORI, Memory operand, Long */

	BASE_TIME_AND_EA_TO_REG_BW = 4, /* AND EA->REG, Byte/Word */
	BASE_TIME_AND_EA_TO_REG_L = 6, 
	
	BASE_TIME_AND_REG_TO_EA_BW = 8,
	BASE_TIME_AND_REG_TO_EA_L = 12,

	BASE_TIME_LSD_REG_BW = 6, /* LSL/LSR Register shift. Byte/Word */
	BASE_TIME_LSD_REG_L = 8, /* LSL/LSR Register shift. Long */
	BASE_TIME_LSD_MEM = 8, /* LSL/LSR Register shift */
	
	BASE_TIME_ROD_REG_BW = 6, /* ROL/ROR Register shift. Byte/Word */
	BASE_TIME_ROD_REG_L = 8, /* ROL/ROR Register shift. Long */
	BASE_TIME_ROD_MEM = 8, /* ROL/ROR Register shift */

	BASE_TIME_RTS = 32,
	BASE_TIME_MOVEQ = 4,

	BASE_TIME_CLR_REG_BW = 4, /* CLR, Register, Byte/Word */
	BASE_TIME_CLR_MEM_BW = 8, /* CLR, Memory, Byte/Word */
	BASE_TIME_CLR_REG_L = 6, /* CLR, Register, Byte/Word */
	BASE_TIME_CLR_MEM_L = 12, /* CLR, Memory, Byte/Word */

	BASE_TIME_ADD_EA_TO_REG_BW = 4, /* ADD, EA+REG->REG, Byte/Word */
	BASE_TIME_ADD_EA_TO_REG_L = 6, /* ADD, EA+REG->REG, Long */
	BASE_TIME_ADD_REG_TO_EA_BW = 8, /* ADD, REG+EA->EA, Byte/Word */
	BASE_TIME_ADD_REG_TO_EA_L = 12, /* ADD, REG+EA->EA, Long */
	
	BASE_TIME_ADDA_EA_TO_REG_BW = 4, /* ADDA, EA+REG->REG, Byte/Word */
	BASE_TIME_ADDA_EA_TO_REG_L = 6, /* ADDA, EA+REG->REG, Long */
	BASE_TIME_ADDA_REG_TO_EA_BW = BASE_TIME_ADD_REG_TO_EA_BW, /* ADDA, REG+EA->EA, Byte/Word */
	BASE_TIME_ADDA_REG_TO_EA_L = BASE_TIME_ADD_REG_TO_EA_L, /* ADDA, REG+EA->EA, Long */

	BASE_TIME_ADDQ_SUBQ_TO_REG_BW = 4, /* ADDQ/SUBQ, dest=reg, Byte/Word */
	BASE_TIME_ADDQ_SUBQ_TO_REG_L = 8, /* ADDQ/SUBQ, dest=reg, Long */
	BASE_TIME_ADDQ_SUBQ_TO_MEM_BW = 4, /* ADDQ/SUBQ, dest=mem, Byte/Word */
	BASE_TIME_ADDQ_SUBQ_TO_MEM_L = 8, /* ADDQ/SUBQ, dest=mem, Long */

	BASE_TIME_BTST_DYNAMIC_REG_L = 6, /* BTST, Dynamic bit number, dest=reg */
	BASE_TIME_BTST_DYNAMIC_MEM_B = 4, /* BTST, Dynamic bit number, dest=mem */
	BASE_TIME_BTST_STATIC_REG_L = 10, /* BTST, Static bit number, dest=reg */
	BASE_TIME_BTST_STATIC_MEM_B = 8, /* BTST, Static bit number, dest=mem */
};
