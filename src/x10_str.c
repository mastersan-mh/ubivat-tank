/*
 * преобразование числа типа BYTE в строку
 */
char *bytetostr(char bts, char * s)
{
//	str(bts,st);
	return s;
};
/*
 * преобразование числа типа WORD в строку
 */
char *wordtostr(int wts, char * s)
{
//	str(wts,s);
	return s;
};
/********преобразование числа типа INTEGER в строку********/
char * inttostr(int its, char * s)
{
//	str(its,s);
	return s;
};

/*
 * преобразование числа типа LONGINT в строку
 */
char * longinttostr(long lits, char * s)
{
//	str(lits,s);
	return s;
};

/*
 * преобразование числа типа REAL в строку
 */
char * realtostr(float rts, int width, int decimals, char * s)
{
//	str(rts:width:decimals,s);
	return s;
};


/********добавление символа в конец строки********/
char * strZ_addch(char * s0, char ch)
{
	int count = 0;
	while(s0[count]) count++;
	s0[count] = ch;
	count++;
	s0[count] = 0;
	return s0;
}

char strZ_UPcheng(char c)
{
	if(97<=c && c<=122) c -= 32;
	return c;
}

char * strZ_UPeng(char * s)
{
	int count = 0;

	while(s[count])
	{
		if(97<=s[count] && s[count]<=122) s[count] -= 32;
		count++;
	}
	return s;
}
