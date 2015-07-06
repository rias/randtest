/*
	
	RANDTEST: A simple suit of randomness tests for bit sequences.
  

	Copyright (c) 2004, Rafael Alvarez
	All rights reserved.
	
	See LICENSE.md file for details.

	*Some of the tests are implemented from
	Handbook of Applied Cryptography, Alfred Menezes et al. 
	http://cacr.uwaterloo.ca/hac/
*/


/* headers */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>



/* data types */

typedef unsigned char bit;
typedef struct
{
	float result;
	float corrhi;
	float corrlo;
	char name[1024];
	char pass[1024];
	char extra[1024];
} TestResult;


/* prototypes */

void Usage();
void ReadSequence(char * filename, int mode, bit ** sequence, unsigned long * length);
TestResult TestFrequency(bit * sequence, unsigned long length);
TestResult TestSerial(bit * sequence, unsigned long length);
TestResult TestPoker8(bit * sequence, unsigned long length);
TestResult TestPoker16(bit * sequence, unsigned long length);
TestResult TestRuns(bit * sequence, unsigned long length);
TestResult TestAutocorrelation(bit * sequence, unsigned long length); 
TestResult TestLinearComplexity(bit * sequence, unsigned long length);
void PrintResult(TestResult *res, FILE *f);



/* code */

int main(int argc, char * argv[])
{
	bit * seq;
	unsigned long len;
	TestResult res;
	FILE * report;
	int mode;
	char dbuff[9], tbuff[9];
	char latex[1024];

	if((argc < 3)||(argc > 4))
	{
		Usage();
		return 1;
	}

	if(!strcmp(argv[1],"-u"))	mode = 0;
	else if(!strcmp(argv[1],"-p"))	mode = 1;
	else if(!strcmp(argv[1],"-t")) mode = 2;
	else
	{
		printf("\n\nInvalid argument '%s'!!!\n", argv[1]);
		Usage();
		return 2;
	}

	ReadSequence(argv[2], mode, &seq, &len);

	if(len == 0)
	{
		printf("\n\nSequence '%s' could not be read!!!\n", argv[2]);
		Usage();
		return 3;
	}

	if(argc == 4)
	{
		report = fopen(argv[3], "a+b");
		if(!report)
		{
			printf("\n\nCan't open report file '%s'!!!\n", argv[3]);
			Usage();
			return 4;
		}
	}
	else report = stdout;

	fprintf(report,"\n\nRANDTEST ( %s, %ld )\n\n", argv[2], len);

	res = TestFrequency(seq, len);
	PrintResult(&res,report);
	fflush(report);
	sprintf(latex,"%.4f\t& ",res.result);

	res = TestSerial(seq, len);
	PrintResult(&res,report);
	fflush(report);
	sprintf(latex,"%s%.4f\t& ",strdup(latex),res.result);

	res = TestPoker8(seq, len);
	PrintResult(&res,report);
	fflush(report);
	sprintf(latex,"%s%.2f\t& ",strdup(latex),res.result);

	res = TestPoker16(seq, len);
	PrintResult(&res,report);
	fflush(report);
	sprintf(latex,"%s%.0f\t& ",strdup(latex),res.result);

	res = TestRuns(seq, len);
	PrintResult(&res,report);
	fflush(report);
	sprintf(latex,"%s%.4f\t& ",strdup(latex),res.result);

	res = TestAutocorrelation(seq, len);
	PrintResult(&res,report);
	fflush(report);
	sprintf(latex,"%s%.4f\t& ",strdup(latex),res.result);

	res = TestLinearComplexity(seq, len);
	PrintResult(&res,report);
	fflush(report);
	sprintf(latex,"%s%.0f\t& ",strdup(latex),res.result);

	//fprintf(report,"%s\n",latex);

	fclose(report);

	return 0;
}

void Usage()
{
	printf("\nrandtest (c) Rafael Alvarez, 2004-2013\n\n");
	printf("Usage: randtest [-p|-u|-t] sequencefile <reportfile>\n\n"
		"-p packed mode, -u unpacked mode, -t text mode\n"
		"sequencefile is mandatory, reportfile is optional\n"
		"if no reportfile is given, report is presented on std output.\n\n");
}
 
void ReadSequence(char * filename, int mode, bit ** sequence, unsigned long * length)
{
	FILE * in;
	unsigned char data;
	unsigned long i;
	bit b;
	


	in = fopen(filename,"rb");
	
	if(!in)
	{
		*sequence = 0;
		*length = 0;
		return;
	}

	fseek(in,0,SEEK_END);
	
	if(mode == 0)	/* unpacked mode */
	{
		*length = ftell(in);
		*sequence = (bit *)malloc(*length);
		fseek(in,0,SEEK_SET);
		fread(*sequence,1,*length,in);
	}
	else if(mode == 1)	/* packed mode */
	{
		*length = ftell(in) * 8;
		*sequence = (bit *)malloc(*length);
		fseek(in,0,SEEK_SET);

		i = 0;
		while(i < *length)
		{
			fread(&data,1,1,in);
			(*sequence)[i+0] = (data & 128) ? 1 : 0;
			(*sequence)[i+1] = (data & 64) ? 1 : 0;
			(*sequence)[i+2] = (data & 32) ? 1 : 0;
			(*sequence)[i+3] = (data & 16) ? 1 : 0;
			(*sequence)[i+4] = (data & 8) ? 1 : 0;
			(*sequence)[i+5] = (data & 4) ? 1 : 0;
			(*sequence)[i+6] = (data & 2) ? 1 : 0;
			(*sequence)[i+7] = (data & 1) ? 1 : 0;
			i+=8;
		}
	}
	else if(mode == 2) /* text mode */
	{
		*length = ftell(in);
		*sequence = (bit *)malloc(*length);
		fseek(in,0,SEEK_SET);
		
		i=0;
		for(;(*length)>0;(*length)--)
		{
			fread(&b,1,1,in);
			if(b=='0')
			{
				(*sequence)[i++] = 0;
			}
			else if(b=='1')
			{
				(*sequence)[i++] = 1;
			}
		}
		*length = i;
	}
}

TestResult TestFrequency(bit * sequence, unsigned long length)
{
	unsigned long n0,n1,i;
	double x;
	TestResult res;

	n0 = n1 = 0;

	for(i=0; i<length; i++)
	{
		if(sequence[i] == 0) n0++;
		else n1++;
	}
	
	x = (double)n0;
	x = x-n1;
	x = x*x;
	x /= (double)length;

	res.result = (float)x;
	res.corrhi = 2.706f;
	res.corrlo = 10.830f;
	strcpy(res.name,"FREQNCY");
	sprintf(res.extra,"0=%ld 1=%ld",n0,n1);

	if(res.result <= res.corrhi) strcpy(res.pass,"OK.");
	else if(res.result <= res.corrlo) strcpy(res.pass,"MEH.");
	else strcpy(res.pass,"FAILED!");

	return res;
}

TestResult TestSerial(bit * sequence, unsigned long length)
{
	unsigned long n0,n1,n00,n01,n10,n11,i;
	double x,y,z;
	TestResult res;

	n0 = n1 = n00 = n01 = n10 = n11 = 0;

	for(i=0; i<(length - 1);i++)
	{
		if(sequence[i] == 0)
		{
			n0++;
			if(sequence[i+1] == 0)	n00++;
			else n01++;
		}
		else
		{
			n1++;
			if(sequence[i+1] == 0) n10++;
			else n11++;
		}
	}
	
	if(sequence[length-1] == 0)	n0++;
	else n1++;
	
	y = ((n00*n00)+(n01*n01)+(n10*n10)+(n11*n11))*4.0;
	z = ((n0*n0)+(n1*n1))*2.0;
	y = y/(length-1.0);
	z = z/(double)length;
	x = y - z;
	x += 1;
	
	res.result = (float)x;
	res.corrhi = 4.605f;
	res.corrlo = 13.820f;
	strcpy(res.name,"SERIAL");
	strcpy(res.extra,"");

	if(res.result <= res.corrhi) strcpy(res.pass,"OK.");
	else if(res.result <= res.corrlo) strcpy(res.pass,"MEH.");
	else strcpy(res.pass,"FAILED!");

	return res;	
}

TestResult TestPoker8(bit * sequence, unsigned long length)
{
	unsigned long table[256],i,k;
	unsigned char byte;
	double x;
	TestResult res;

	for(i=0;i<256;i++)	table[i]=0;

	k=0;
	for(i=0;i<(length-7);i+=8)
	{
		byte = sequence[i+7];	byte <<=1;
		byte |= sequence[i+6];	byte <<=1;
		byte |= sequence[i+5];	byte <<=1;
		byte |= sequence[i+4];	byte <<=1;
		byte |= sequence[i+3];	byte <<=1;
		byte |= sequence[i+2];	byte <<=1;
		byte |= sequence[i+1];	byte <<=1;
		byte |= sequence[i];

		table[byte]++;
		k++;
	}

	x = 0.0;
	for(i=0;i<256;i++)
		x += (double)table[i]*table[i];

	x = ((256.0/k)*x) - k;

	res.result = (float)x;
	res.corrhi = 284.3f;
	res.corrlo = 330.5f;
	strcpy(res.name,"POKER 8");
	strcpy(res.extra,"");

	if(res.result <= res.corrhi) strcpy(res.pass,"OK.");
	else if(res.result <= res.corrlo) strcpy(res.pass,"MEH.");
	else strcpy(res.pass,"FAILED!");

	return res;
}

TestResult TestPoker16(bit * sequence, unsigned long length)
{
	unsigned long table[65536],i,k;
	unsigned short word;
	double x;
	TestResult res;

	for(i=0;i<65536;i++)	table[i]=0;

	k=0;
	for(i=0;i<(length-15);i+=16)
	{
		word = sequence[i+15];	word <<=1;
		word |= sequence[i+14];	word <<=1;
		word |= sequence[i+13];	word <<=1;
		word |= sequence[i+12];	word <<=1;
		word |= sequence[i+11];	word <<=1;
		word |= sequence[i+10];	word <<=1;
		word |= sequence[i+9];	word <<=1;
		word |= sequence[i+8];	word <<=1;
		word |= sequence[i+7];	word <<=1;
		word |= sequence[i+6];	word <<=1;
		word |= sequence[i+5];	word <<=1;
		word |= sequence[i+4];	word <<=1;
		word |= sequence[i+3];	word <<=1;
		word |= sequence[i+2];	word <<=1;
		word |= sequence[i+1];	word <<=1;
		word |= sequence[i];

		table[word]++;
		k++;
	}

	x = 0.0;
	for(i=0;i<65536;i++)
		x += table[i]*table[i];

	x = ((65536.0/k)*x) - k;

	res.result = (float)x;
	res.corrhi = 65999.3936f;
	res.corrlo = 66659.4697f;
	strcpy(res.name,"POKER 16");
	strcpy(res.extra,"");

	if(res.result <= res.corrhi) strcpy(res.pass,"OK.");
	else if(res.result <= res.corrlo) strcpy(res.pass,"MEH.");
	else strcpy(res.pass,"FAILED!");
	
	return res;
}

TestResult TestRuns(bit * sequence, unsigned long length)
{
	unsigned long i,*b,*g,k;
	unsigned long bl,gl,rt;
	double *e,x;
	double xb,xg;
	TestResult res;
	float corrhi[20];
	float corrlo[20];

	corrhi[0] = 0.0000f;	corrhi[1] = 0.0000f;
	corrhi[2] = 4.6052f;	corrhi[3] = 7.7794f;
	corrhi[4] = 10.6446f;	corrhi[5] = 13.3616f;
	corrhi[6] = 15.9872f;	corrhi[7] = 18.5493f;
	corrhi[8] = 21.0641f;	corrhi[9] = 23.5418f;
	corrhi[10] = 25.9894f;	corrhi[11] = 28.4120f;
	corrhi[12] = 30.8133f;	corrhi[13] = 33.1962f;
	corrhi[14] = 35.5632f;	corrhi[15] = 37.9159f;
	corrhi[16] = 40.2560f;	corrhi[17] = 42.5847f;
	corrhi[18] = 44.9031f;	corrhi[19] = 47.2121f;

	corrlo[0] = 0.0000f;	corrlo[1] = 0.0000f;
	corrlo[2] = 13.8155f;	corrlo[3] = 18.4668f;
	corrlo[4] = 22.4577f;	corrlo[5] = 26.1245f;
	corrlo[6] = 29.5883f;	corrlo[7] = 32.9095f;
	corrlo[8] = 36.1233f;	corrlo[9] = 39.2524f;
	corrlo[10] = 42.3124f;	corrlo[11] = 45.3147f;
	corrlo[12] = 48.2679f;	corrlo[13] = 51.1786f;
	corrlo[14] = 54.0520f;	corrlo[15] = 56.8923f;
	corrlo[16] = 59.7031f;	corrlo[17] = 62.4872f;
	corrlo[18] = 65.2472f;	corrlo[19] = 67.9851f;

	//obtain k
	for(i=1;i<=length;i++)
	{
		x = (length-i+3.0)/pow(2.0,i+2.0);
		if(x<5.0)
			break;
	}
	k=i-1;

	e = (double *)malloc((k+1)*sizeof(double));
	b = (unsigned long *)malloc((k+1)*sizeof(unsigned long));
	g = (unsigned long *)malloc((k+1)*sizeof(unsigned long));


	for(i=1;i<=k;i++)
	{
		e[i] = (length-i+3.0)/pow(2.0,i+2.0);

		b[i] = 0;
		g[i] = 0;
	}

	
	if(sequence[0] == 0)
	{
		gl=1; bl=0;
		rt=0;
	}
	else
	{
		bl=1; gl=0;
		rt=1;
	}
	
	for(i=1;i<length;i++)
	{
		if(sequence[i] == 0)
		{
			if(rt == 0)
			{
				gl++;
			}
			else
			{
				if(bl <= k)
					g[bl]++;

				gl=1;
				bl=0;
				rt=0;
			}
		}
		else
		{
			if(rt == 1)
			{
				bl++;
			}
			else
			{
				if(gl <= k)
					b[gl]++;

				bl=1;
				gl=0;
				rt=1;
			}
		}
	}
	
	xb=xg=0.0;
	for(i=1;i<=k;i++)
	{
		xb += pow(b[i]-e[i],2)/e[i];
		xg += pow(g[i]-e[i],2)/e[i];
	}

	free(e);
	free(b);
	free(g);

	x = xb + xg;
	res.result = (float)x;
	strcpy(res.name,"RUNS");

	if(k>19)
	{
		res.corrhi = 0.0f;
		res.corrlo = 0.0f;
		sprintf(res.extra,"unsupported length. k=%ld",k);
	}
	else
	{
		res.corrhi = corrhi[k];
		res.corrlo = corrlo[k];
		sprintf(res.extra,"k=%ld",k);
		
		if(res.result <= res.corrhi) strcpy(res.pass,"OK.");
		else if(res.result <= res.corrlo) strcpy(res.pass,"MEH.");
		else strcpy(res.pass,"FAILED!");
	}

	return res;
}

TestResult TestAutocorrelation(bit * sequence, unsigned long length)
{
	unsigned long i,d,worstd,a;
	double x,y;
	double worst,avg;
	TestResult res;
	
	worst = 0.0;
	avg = 0.0;
	for(d=1;d<=length/2;d++)
	{
		a=0;
		for(i=0;i<length-d;i++)
		{
			if(sequence[i] != sequence[i+d])
				a++;
		}


		x = 2*a;
		x = x-length;
		x = x+d;
		y = length-d;

		x = pow(x,2.0);
		x = x/y;
		x = sqrt(x);

		if(x > worst)
		{
			worst = x;
			worstd = d;
		}
		avg += x;
	}

	avg = avg/(length*0.5);
	
	res.result = (float)avg;
	res.corrhi = 1.282f;
	res.corrlo = 3.090f;
	strcpy(res.name,"AUTOCORR");
	sprintf(res.extra,"worst = %.4f, worstd = %ld", worst, worstd);

	if(res.result <= res.corrhi) strcpy(res.pass,"OK.");
	else if(res.result <= res.corrlo) strcpy(res.pass,"MEH.");
	else strcpy(res.pass,"FAILED!");

	return res;
}

TestResult TestLinearComplexity(bit * sequence, unsigned long length)
{
	unsigned long n,L,m,i,j;
	unsigned char *C,*B,*T,d;
	TestResult res;
	
	C = (unsigned char *)malloc(length+4);
	B = (unsigned char *)malloc(length+4);
	T = (unsigned char *)malloc(length+4);
	
	n = 0;
	C[n] = B[n] = 1;
	L = 0;
	m = -1;

	while(n<length)
	{
		d = sequence[n];
		for(i=1;i<=L;++i)
		{
			d ^= C[i] & sequence[n-i];
		}

		if(d)
		{
			memcpy(T,C,n+1);
			for(i = n-m,j=0;i<=n+1;++i,++j)
			{
				C[i] ^= B[j];
			}

			if(L <= n/2)
			{
				L = n + 1 - L;
				m = n;
				memcpy(B,T,n+1);
			}
		}

		++n;
	}

	free(C);
	free(B);
	free(T);

	res.result = (float)L;
	res.corrhi = length/2.0f;
	res.corrlo = length/2.0f;
	strcpy(res.name,"LC");
	strcpy(res.extra,"");

	if(res.result >= res.corrhi) strcpy(res.pass,"OK.");
	else strcpy(res.pass,"FAILED!");

	return res;
}

void PrintResult(TestResult *res, FILE *f)
{
	fprintf(f,"%10s:%10s ( %.4f, %.4f ) {%.4f} %s\n", res->name, res->pass, res->corrhi, res->corrlo, res->result, res->extra);
}


