unsigned int CalcHash(const char* String) {
	unsigned int Result= 0;
	const char *Pos= String;
	while (*Pos)
		Result= ((Result<<1) + (Result ^ *Pos++));
	return Result;
}

