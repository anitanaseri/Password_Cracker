/*
* 
* Anita Naseri 
* Assignment 2, May 2019
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <stddef.h>

#define listlength 10000
#define wordslength 6
#define SHA256_BLOCK_SIZE 32

#define SHA256_BLOCK_SIZE 32 // SHA256 outputs a 32 byte digest

/**************************** DATA TYPES ****************************/
typedef unsigned char BYTE; // 8-bit byte
typedef unsigned int WORD;  // 32-bit word, change to "long" for 16-bit machines

typedef struct
{
	BYTE data[64];
	WORD datalen;
	unsigned long long bitlen;
	WORD state[8];
} SHA256_CTX;
/****************************** MACROS ******************************/
#define ROTLEFT(a, b) (((a) << (b)) | ((a) >> (32 - (b))))
#define ROTRIGHT(a, b) (((a) >> (b)) | ((a) << (32 - (b))))

#define CH(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROTRIGHT(x, 2) ^ ROTRIGHT(x, 13) ^ ROTRIGHT(x, 22))
#define EP1(x) (ROTRIGHT(x, 6) ^ ROTRIGHT(x, 11) ^ ROTRIGHT(x, 25))
#define SIG0(x) (ROTRIGHT(x, 7) ^ ROTRIGHT(x, 18) ^ ((x) >> 3))
#define SIG1(x) (ROTRIGHT(x, 17) ^ ROTRIGHT(x, 19) ^ ((x) >> 10))

/**************************** VARIABLES *****************************/
static const WORD k[64] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
	0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
	0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
	0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
	0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
	0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
	};



char **getwords(FILE *fp, int *n);
void free_array(char **words, int rows);
void create_hash(BYTE text[], BYTE hash[SHA256_BLOCK_SIZE]);
BYTE *readSHAfilePasswords();
void ChangeStrToBYTE(char *input, BYTE *result);
void crack_noargument(char *word);

void sha256_final(SHA256_CTX *ctx, BYTE hash[]);
void sha256_update(SHA256_CTX *ctx, const BYTE data[], size_t len);
void sha256_init(SHA256_CTX *ctx);
void sha256_transform(SHA256_CTX *ctx, const BYTE data[]);

int checkPasswords(BYTE *passwords, char *guess); 

int main(int argc, char **argv)
{

	int i, nwords = 0;
	char **words = NULL; /* file given as argv[1] (default dictionary.txt) */
	char *fname = argc > 1 ? argv[0] : "common_passwords.txt";
	FILE *dictionary = fopen(fname, "r");

	if (!dictionary)
	{ /* validate file open */
		fprintf(stderr, "error: file open failed.\n");
		return 1;
	}
	words = getwords(dictionary, &nwords); 
	if (!(words))
	{
		fprintf(stderr, "error: getwords returned NULL.\n");
		return 1;
	}
	fclose(dictionary);

	printf("\n '%d' words read from '%s'\n\n", nwords, fname);

	for (i = 0; i < nwords; i++)
	{
		if (argc < 2)
		{
			crack_noargument(words[i]);
		}
		/*else if (argc == 1)
		{
			crack_oneargument(words[i], argcv0]);
		}

		else if (argc == 2)
		{
			crack_oneargument(words[i], , argv[0], argv[1]);
		}*/
		else
		{
			perror("can not accept 3 inputs");
			exit(EXIT_FAILURE);
		}
	}

	free_array(words, nwords);

	return 0;
}

/* read all words 1 per-line, from 'fp', return
 * pointer-to-pointers of allocated strings on 
 * success, NULL otherwise, 'n' updated with 
 * number of words read.
 */
char **getwords(FILE *fp, int *n)
{

	char **words = NULL;
	char buf[wordslength + 1] = {0};
	int maxlen = listlength > 0 ? listlength : 1;

	if (!(words = calloc(maxlen, sizeof *words)))
	{
		fprintf(stderr, "getwords() error: virtual memory exhausted.\n");
		return NULL;
	}

	while (fgets(buf, wordslength + 1, fp))
	{

		size_t wordlen = strlen(buf); /* get word length */

		if (buf[wordlen - 1] == '\n') /* strip '\n' */
			buf[--wordlen] = 0;

		words[(*n)++] = strdup(buf); /* allocate/copy */

		if (*n == maxlen)
		{ /* realloc as required, update maxlen */
			void *tmp = realloc(words, maxlen * 2 * sizeof *words);
			if (!tmp)
			{
				fprintf(stderr, "getwords() realloc: memory exhausted.\n");
				return words; /* to return existing words before failure */
			}
			words = tmp;
			memset(words + maxlen, 0, maxlen * sizeof *words);
			maxlen *= 2;
		}
	}

	return words;
}


void free_array(char **words, int rows)
{

	int i;
	for (i = 0; i < rows; i++)
	{
		free(words[i]);
	}
	free(words);
}

//create hash representation for BYTE array
void create_hash(BYTE text[], BYTE hash[SHA256_BLOCK_SIZE])
{
	SHA256_CTX ctx;

	sha256_init(&ctx);
	sha256_update(&ctx, text, strlen(text));
	sha256_final(&ctx, hash);
}

//read file containing SHA Passwords and returns the bite array
BYTE* readSHAfilePasswords(){
	FILE *dictionary;
	long dictionarysize;
	BYTE *listofHashes;

	dictionary = fopen("pwd4sha256", "rb");
	if (!dictionary)
	{ /* validate file open */
		fprintf(stderr, "error: hash file open failed.\n");
		exit(EXIT_FAILURE);
	}

	fseek(dictionary, 0, SEEK_END);
	dictionarysize = ftell(dictionary);
	rewind(dictionary);

	listofHashes = (BYTE *)malloc((dictionarysize * 4) * sizeof(BYTE));
	fread(listofHashes, dictionarysize, 1, dictionary);
	fclose(dictionary);

	return listofHashes;
}

void ChangeStrToBYTE(char *input, BYTE *result)
{
	int j = 0;
	int i=0;

	while (input[j] != '\0')
	{
		result[i++] = input[j++];
	}
}


//checks if guess in passwords and returns the index if it is
int checkPasswords(BYTE *passwords, char *guess)
{

	//convert word to BYTE string
	int len = strlen(guess);
	BYTE guessBYTE[len];
	ChangeStrToBYTE(guess, guessBYTE);

	//create hash for guess
	BYTE guessHash[32];
	create_hash(guessBYTE, guessHash);

	//check if guess in passwords
	//adapted from https://stackoverflow.com/questions/27490488/copying-part-of-an-array-into-another-variable
	for (int i = 0; i < 288; i++)
	{
		int sz = 32; // number of characters to copy

		BYTE check[sz + 1];
		int from = i; // here from is where you start to copy

		strncpy(check, passwords + from, sz);
		if (memcmp(check, guessHash, 32) == 0)
		{
			return i / 32;
		}
	}

	return -1;
}



void crack_noargument(char *word)
{
	BYTE *passwords;
	passwords = readSHAfilePasswords();

	if (checkPasswords(passwords, word) != -1){
		printf("%s\n", word);
		printf("%d\n", checkPasswords(passwords, word));
	}
		
}



/*********************** FUNCTION DEFINITIONS ***********************/
void sha256_transform(SHA256_CTX *ctx, const BYTE data[])
{
	WORD a, b, c, d, e, f, g, h, i, j, t1, t2, m[64];

	for (i = 0, j = 0; i < 16; ++i, j += 4)
		m[i] = (data[j] << 24) | (data[j + 1] << 16) | (data[j + 2] << 8) | (data[j + 3]);
	for (; i < 64; ++i)
		m[i] = SIG1(m[i - 2]) + m[i - 7] + SIG0(m[i - 15]) + m[i - 16];

	a = ctx->state[0];
	b = ctx->state[1];
	c = ctx->state[2];
	d = ctx->state[3];
	e = ctx->state[4];
	f = ctx->state[5];
	g = ctx->state[6];
	h = ctx->state[7];

	for (i = 0; i < 64; ++i)
	{
		t1 = h + EP1(e) + CH(e, f, g) + k[i] + m[i];
		t2 = EP0(a) + MAJ(a, b, c);
		h = g;
		g = f;
		f = e;
		e = d + t1;
		d = c;
		c = b;
		b = a;
		a = t1 + t2;
	}

	ctx->state[0] += a;
	ctx->state[1] += b;
	ctx->state[2] += c;
	ctx->state[3] += d;
	ctx->state[4] += e;
	ctx->state[5] += f;
	ctx->state[6] += g;
	ctx->state[7] += h;
}

void sha256_init(SHA256_CTX *ctx)
{
	ctx->datalen = 0;
	ctx->bitlen = 0;
	ctx->state[0] = 0x6a09e667;
	ctx->state[1] = 0xbb67ae85;
	ctx->state[2] = 0x3c6ef372;
	ctx->state[3] = 0xa54ff53a;
	ctx->state[4] = 0x510e527f;
	ctx->state[5] = 0x9b05688c;
	ctx->state[6] = 0x1f83d9ab;
	ctx->state[7] = 0x5be0cd19;
}

void sha256_update(SHA256_CTX *ctx, const BYTE data[], size_t len)
{
	WORD i;

	for (i = 0; i < len; ++i)
	{
		ctx->data[ctx->datalen] = data[i];
		ctx->datalen++;
		if (ctx->datalen == 64)
		{
			sha256_transform(ctx, ctx->data);
			ctx->bitlen += 512;
			ctx->datalen = 0;
		}
	}
}

void sha256_final(SHA256_CTX *ctx, BYTE hash[])
{
	WORD i;

	i = ctx->datalen;

	// Pad whatever data is left in the buffer.
	if (ctx->datalen < 56)
	{
		ctx->data[i++] = 0x80;
		while (i < 56)
			ctx->data[i++] = 0x00;
	}
	else
	{
		ctx->data[i++] = 0x80;
		while (i < 64)
			ctx->data[i++] = 0x00;
		sha256_transform(ctx, ctx->data);
		memset(ctx->data, 0, 56);
	}

	// Append to the padding the total message's length in bits and transform.
	ctx->bitlen += ctx->datalen * 8;
	ctx->data[63] = ctx->bitlen;
	ctx->data[62] = ctx->bitlen >> 8;
	ctx->data[61] = ctx->bitlen >> 16;
	ctx->data[60] = ctx->bitlen >> 24;
	ctx->data[59] = ctx->bitlen >> 32;
	ctx->data[58] = ctx->bitlen >> 40;
	ctx->data[57] = ctx->bitlen >> 48;
	ctx->data[56] = ctx->bitlen >> 56;
	sha256_transform(ctx, ctx->data);

	// Since this implementation uses little endian byte ordering and SHA uses big endian,
	// reverse all the bytes when copying the final state to the output hash.
	for (i = 0; i < 4; ++i)
	{
		hash[i] = (ctx->state[0] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 4] = (ctx->state[1] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 8] = (ctx->state[2] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 12] = (ctx->state[3] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 16] = (ctx->state[4] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 20] = (ctx->state[5] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 24] = (ctx->state[6] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 28] = (ctx->state[7] >> (24 - i * 8)) & 0x000000ff;
	}
}

/*void crack_oneargument()
{

}

void crack_twoargument()
{

}

*/


