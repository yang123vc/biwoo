// ITU-g711����Դ��
// ��Ҫ������
// 13�����������U�ɱ�����롣
// 13�����������U�ɱ�����롣
// U�ɺ�A��8λ��������֮����໥ת����

#define	SIGN_BIT	(0x80)		/* A-law ����λ  */
#define	QUANT_MASK	(0xf)		/* ��������ֵ��  */
#define	NSEGS		(8)		    /* A-law �����. */
#define	SEG_SHIFT	(4)		    /* ��������λ��  */
#define	SEG_MASK	(0x70)		/* ����������.   */

static short seg_aend[8] = {0x1F, 0x3F, 0x7F, 0xFF, 0x1FF, 0x3FF, 0x7FF, 0xFFF};  //A�ɱ������Ԥ�Ʊ�
static short seg_uend[8] = {0x3F, 0x7F, 0xFF, 0x1FF,0x3FF, 0x7FF, 0xFFF, 0x1FFF}; //U�ɱ������Ԥ�Ʊ�

unsigned char _u2a[128] = {			/* U�ɵ�A��ת������� */
	1,	1,	2,	2,	3,	3,	4,	4,
	5,	5,	6,	6,	7,	7,	8,	8,
	9,	10,	11,	12,	13,	14,	15,	16,
	17,	18,	19,	20,	21,	22,	23,	24,
	25,	27,	29,	31,	33,	34,	35,	36,
	37,	38,	39,	40,	41,	42,	43,	44,
	46,	48,	49,	50,	51,	52,	53,	54,
	55,	56,	57,	58,	59,	60,	61,	62,
	64,	65,	66,	67,	68,	69,	70,	71,
	72,	73,	74,	75,	76,	77,	78,	79,
	80,	82,	83,	84,	85,	86,	87,	88,
	89,	90,	91,	92,	93,	94,	95,	96,
	97,	98,	99,	100,	101,	102,	103,	104,
	105,	106,	107,	108,	109,	110,	111,	112,
	113,	114,	115,	116,	117,	118,	119,	120,
	121,	122,	123,	124,	125,	126,	127,	128};

unsigned char _a2u[128] = {			/* A�ɵ�U��ת������� */
	1,	3,	5,	7,	9,	11,	13,	15,
	16,	17,	18,	19,	20,	21,	22,	23,
	24,	25,	26,	27,	28,	29,	30,	31,
	32,	32,	33,	33,	34,	34,	35,	35,
	36,	37,	38,	39,	40,	41,	42,	43,
	44,	45,	46,	47,	48,	48,	49,	49,
	50,	51,	52,	53,	54,	55,	56,	57,
	58,	59,	60,	61,	62,	63,	64,	64,
	65,	66,	67,	68,	69,	70,	71,	72,
	73,	74,	75,	76,	77,	78,	79,	80,
	80,	81,	82,	83,	84,	85,	86,	87,
	88,	89,	90,	91,	92,	93,	94,	95,
	96,	97,	98,	99,	100,	101,	102,	103,
	104,	105,	106,	107,	108,	109,	110,	111,
	112,	113,	114,	115,	116,	117,	118,	119,
	120,	121,	122,	123,	124,	125,	126,	127};
// ���ӳ���Ѱ�Ҷ�����
static short search(	short		val,	short		*table,	short		size)
{  
	short		i;
	for (i = 0; i < size; i++) 
	{
		if (val <= *table++)
		return (i);
	}
	return (size);
}

/*  ������������Ͷ�Ӧ��
 *	����������       	   �����������
 *	----------------      --------------
 *	0000000wxyza			000wxyz
 *	0000001wxyza			001wxyz
 *	000001wxyzab			010wxyz
 *	00001wxyzabc			011wxyz
 *	0001wxyzabcd			100wxyz
 *	001wxyzabcde			101wxyz
 *	01wxyzabcdef			110wxyz
 *	1wxyzabcdefg			111wxyz
 */
//16bit ���������Ϊ8����A�ɷ�������
//���� pcm_val--16���������룬���8����A�ɷ�������
unsigned char linear2alaw(short pcm_val)	/* 2's complement (16-bit range) */
{
	short		mask;
	short		seg;
	unsigned char	aval;

	pcm_val = pcm_val >> 3;

	if (pcm_val >= 0) {
		mask = 0xD5;		/* sign (7th) bit = 1 */
	} else {
		mask = 0x55;		/* sign bit = 0 */
		pcm_val = -pcm_val - 1;
	}
	seg = search(pcm_val, seg_aend, 8);  /* �����������ֵ��ȡ�����. */
	// ����ȡ�ķ���λ, �����, ��������ֵ���һ��8���������/
	if (seg >= 8)  return (unsigned char) (0x7F ^ mask);
	else {
		aval = (unsigned char) seg << SEG_SHIFT;
		if (seg < 2)aval |= (pcm_val >> 1) & QUANT_MASK;
		else
			aval |= (pcm_val >> seg) & QUANT_MASK;
		return (aval ^ mask);
	}
}

//8bitA�ɷ����������Ϊ16�������������
//����8����A�ɷ������� �����pcm_val--16����������
short alaw2linear(	unsigned char	a_val)
{
	short		t;
	short		seg;
	a_val ^= 0x55;
	t = (a_val & QUANT_MASK) << 4;
	seg = ((unsigned)a_val & SEG_MASK) >> SEG_SHIFT;
	switch (seg) {
	case 0:
		t += 8;		break;
	case 1:
		t += 0x108;		break;
	default:
		t += 0x108;		t <<= seg - 1;
	}
	return ((a_val & SIGN_BIT) ? t : -t);
}

#define	BIAS		(0x84)		//   ������ƫ��ֵ
#define CLIP         8159       //   �������������

/*
//16bit ���������Ϊ8����u�ɷ�������
//���� pcm_val--16���������룬���8����u�ɷ�������
 *	����������       	   �����������
 *	----------------      --------------
 *	00000001wxyza			000wxyz
 *	0000001wxyzab			001wxyz
 *	000001wxyzabc			010wxyz
 *	00001wxyzabcd			011wxyz
 *	0001wxyzabcde			100wxyz
 *	001wxyzabcdef			101wxyz
 *	01wxyzabcdefg			110wxyz
 *	1wxyzabcdefgh			111wxyz
 */
unsigned char linear2ulaw( 	short pcm_val)	/* 16���������� */
{
	short		mask;
	short		seg;
	unsigned char	uval;

	/* Get the sign and the magnitude of the value. */
	pcm_val = pcm_val >> 2;
	if (pcm_val < 0)
	{		pcm_val = -pcm_val;		mask = 0x7F;	} 
	else 
	{		mask = 0xFF;	}
    
	if ( pcm_val > CLIP ) pcm_val = CLIP;		/* ���� */
	pcm_val += (BIAS >> 2);

	/* Convert the scaled magnitude to segment number. */
	seg = search(pcm_val, seg_uend, 8);
	// ����ȡ�ķ���λ, �����, ��������ֵ���һ��8���������/
	if (seg >= 8)		
		return (unsigned char) (0x7F ^ mask);
	else {	uval = (unsigned char) (seg << 4) | ((pcm_val >> (seg + 1)) & 0xF);
		return (uval ^ mask);
	}

}
//8bit u�ɷ����������Ϊ16bits ���������
//����u_val 8����u�ɷ������� �����pcm_val--16����������
short ulaw2linear(	unsigned char	u_val)
{
	short		t;
	u_val = ~u_val;
	t = ((u_val & QUANT_MASK) << 3) + BIAS;
	t <<= ((unsigned)u_val & SEG_MASK) >> SEG_SHIFT;

	return ((u_val & SIGN_BIT) ? (BIAS - t) : (t - BIAS));
}

/*����A�ɵ�U��ת����������A�ɵ�U������ת�� */
unsigned char alaw2ulaw(	unsigned char	aval)
{
	aval &= 0xff;
	return (unsigned char) ((aval & 0x80) ? (0xFF ^ _a2u[aval ^ 0xD5]) :
	    (0x7F ^ _a2u[aval ^ 0x55]));
}
/*����U�ɵ�A��ת����������U�ɵ�A������ת�� */
unsigned char ulaw2alaw(	unsigned char	uval)
{
	uval &= 0xff;
	return (unsigned char) ((uval & 0x80) ? (0xD5 ^ (_u2a[0xFF ^ uval] - 1)) :
	    (unsigned char) (0x55 ^ (_u2a[0x7F ^ uval] - 1)));
}
