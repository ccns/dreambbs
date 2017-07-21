	int
str_rle(str)			/* run-length encoding */
	unsigned char *str;
{
	unsigned char *src, *dst;
	int cc, rl;

	dst = src = str;
	while ((cc = *src++))
	{
		if (cc > 8 && cc == src[0] && cc == src[1] && cc == src[2])
		{
			src += 2;
			rl = 4;
			while (*++src == cc)
			{
				if (++rl >= 255)
					break;
			}

			*dst++ = 8;
			*dst++ = rl;
		}
		*dst++ = cc;
	}

	*dst = '\0';
	return dst - str;
}
