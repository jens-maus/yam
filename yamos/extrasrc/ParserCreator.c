//-----------------------------------------------------------------------
//                        YAM parser creator v0.1b
//                    Written by dr.Titus (c) on 23.07.2001
//-----------------------------------------------------------------------

#include <stdio.h>

void main(int argc, char* argv[])
{
	long	i, j, max, len;
	long	StatTbl[16384];
	FILE	*ParserFL, *LowUpTblFL, *SourceFL;
	char	LowUpTbl[256], Parser[16384], Source[65536];
	unsigned char	a=0, b;

	if(argc!= 4)
	{
		printf("Usage: ParserCreator Source Destination LowerToUpperTable\n");
		exit(0);
	}


	if	(!(LowUpTblFL=fopen(argv[2],"rb")))
	{
		printf("Can't open Lower to Upper Translation table '%s'!\n", argv[2]);
//		printf("Не могу открыть таблицу перекодировки из маленьких букв в большие!\n");
		exit(0);
	}

	fread(LowUpTbl, 1, 256, LowUpTblFL);
	// Прочитать таблицу перекодировки из маленьких букв в большие
	// Read Translaton Table
	fclose(LowUpTblFL);
	
	
	if	(!(SourceFL=fopen(argv[1],"rb")))
	{
//		printf("Не могу открыть исходный текст!\n", argv[1]);
		printf("Can't open source text '%s' !\n", argv[1]);
		exit(0);
	}
	

	if	(!(ParserFL=fopen(argv[3],"wb")))
	{
//		printf("Не могу создать файл '%s!'\n", argv[3]);
		printf("Can't Create file '%s'!\n", argv[3]);
		
		fclose(SourceFL);
		exit(0);
	}

	for (i=0;i<16384;i++) StatTbl[i]=0;
//Подготовить таблицу статистики
// Prepare table of statistic

// Главный цикл анализа текста
// Main routine analyse text.
	
	while (!feof(SourceFL)) 
	{
		len=fread(Source, 1, 65536, SourceFL);

		for (i=0;i<len;i++)
		{
			b=Source[i];
			if ((a>127) && (b>127)) StatTbl[(a-128)*128+(b-128)]+=1; 
			a=b;
		}

	}

// Дополнить таблицу LowLow комбинациями UpLow и UpUp
// Add UpLow and UpUp to table LowLow 

	for (a=0;a<128;a++)
	{
		b=LowUpTbl[a+128]-128; //b-буква в UpperCase
		if (b!=a) 
		{
			for (i=0;i<128;i++)
			{	
				j=StatTbl[a*128+i]; //Low->Up
				if (j!=0) 
				{
					StatTbl[b*128+i]=j;
		
					StatTbl[b*128 + (LowUpTbl[i+128]-128)]=j;
				}		
			}
		}
	}
//
	max=0;
	for (i=0;i<16384;i++)
	{
		if (max<StatTbl[i]) max=StatTbl[i];
	}


	for (i=0;i<16384;i++)
	{		
		if (StatTbl[i] > 0) Parser[i]=(StatTbl[i]*127)/max;
		else Parser[i]=-128;
	}

	fwrite(Parser,1,16384,ParserFL);


eexit:
	exit(0);
}

