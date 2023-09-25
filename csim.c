//===============================================================
// Abrham Negash Gelan
// 05-05-23
// csim.c
// Cache Lab - Cache Simulator
//===============================================================
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include "cachelab.h"


/*** TYPES ***/
typedef unsigned long long int ADDR_T;

// c style structs for your cache construction
struct c_line
{
  char valid;
  ADDR_T tag;
  int LRU;
};

/*** Global Arguments ***/
int s = 0;      		/* number of set bits */
int S;          		/* number of Sets = 2^s */
int E = 0;      		/* number of lines per set */
int b = 0;      		/* number of block bits */
int B;          		/* number of blocks = 2^b */
int verbose = 0;        /* toggle for verbose mode */
char *tracefile = 0;    /* filename of trace input file */

int num_hits = 0;
int num_misses = 0;
int num_evictions = 0;
int LRU_counter = 0;    // keep track of time stamp of each mem access


/*** function declarations ***/
void    printUsage      ( char *argv[] );

//===============================================================
//===============================================================
int main(int argc, char *argv[])
{

 	int c;
	while ( (c = getopt(argc,argv,"s:E:b:t:v:h")) != -1 )
	{
		switch (c)
		{
		case 's':	s = atoi(optarg);
					break;
		case 'E':	E = atoi(optarg);
					break;
		case 'b':	b = atoi(optarg);
					break;
		case 't':	tracefile = optarg;
					break;
		case 'v':	verbose = 1;
					break;
		case 'h':   printUsage(argv);
					exit(0);
		};
	}
	S = 1 << s;		// number of sets
	B = 1 << b;		// number of blocks per line

	// comment this out later once you are sure you have
	// command line options working.
	//printf("S[%d,%d] E[%d] B[%d,%d] v[%d] t[%s]\n",
		//S,s,E,B,b,verbose,tracefile);

	// your code here
  struct c_line **cache;
  //Set associative
  //Whole cache--> size of lines*num_sets
  cache= (struct c_line **)malloc(sizeof(struct c_line*) * S);
  for (int i=0;i<S;i++)
  {
    //A set --> line size*num_lines
    cache[i]=(struct c_line *)malloc(sizeof(struct c_line) * E);

    for (int j=0;j<E;j++)
    {
      cache[i][j].valid=0;
      cache[i][j].tag=0;
      cache[i][j].LRU=0;
    }
  }

  // Open file with fscan
  char inst_type;
  ADDR_T mem_addr;
  int num_bytes;

  FILE *fptr;
  fptr= fopen(tracefile,"r");

  while (fscanf(fptr," %c %llx,%d", &inst_type, &mem_addr, &num_bytes) >0)
  {
    if(inst_type!='I')                        //ignore I
    {
          ADDR_T mask=pow(2,s)-1;
          //parse line
          ADDR_T tag_bit= (mem_addr>>(b+s));
          ADDR_T set_index= (mem_addr>>b)&mask;


          //Cache lookup
          int hit = 0;                      //hit
          int empty_line = -1;              //miss
          int to_evict = 0;                 //miss and evict


          for (int i=0; i<E;i++)
          {
            // Valid is set, tag matches --> hit, num_hit++, set hit id
            if (cache[set_index][i].valid == 1 && cache[set_index][i].tag == tag_bit)
            {
                cache[set_index][i].LRU = LRU_counter++;
                num_hits++;
                hit = 1;
                break;
            }

            // Find empty line in case no hit
            if (cache[set_index][i].valid == 0 && empty_line == -1)
            {
                empty_line = i;
            }

            // Find line to evict in case no empty
            if (cache[set_index][i].LRU < cache[set_index][to_evict].LRU)
            {
                to_evict = i;
            }
        }

        // If hit and type was M, num_hit++ because M-> load and read
        if (hit)
        {
            if (inst_type == 'M')
            {
                num_hits++;
            }

        }
        //If not hit, then miss
        else
        {
          // If empty line, cache with no eviction, set valid bit
          if (empty_line != -1)
          {
              cache[set_index][empty_line].valid = 1;
              cache[set_index][empty_line].tag = tag_bit;
              cache[set_index][empty_line].LRU = LRU_counter++;
              num_misses++;
          }
          // If no empty line, cache with eviction, num_evictions++
          else
          {
              cache[set_index][to_evict].tag = tag_bit;
              cache[set_index][to_evict].LRU = LRU_counter++;
              num_misses++;
              num_evictions++;
          }

          // If type was M, num_hit++ because again M-> load and read
          if (inst_type == 'M')
          {
              num_hits++;
          }
        }
     }
   }

	// change this to reflect hits/misses/evictions counts
	printSummary(num_hits,num_misses,num_evictions);
    return 0;
}



//===============================================================
//===============================================================

void printUsage (char *argv[] )
{
    printf("Usage: \n");
    printf("%s -s #setbits -E #lines -b #blockbits -t tracefilename -v -h\n",argv[0]);
    printf("where s specifies number of sets S = 2^s\n");
    printf("where E specifies number of lines per set\n");
    printf("where b specifies size of blocks B = 2^b\n");
    printf("where t specifies name of tracefile\n");
    printf("where v turns no verbose mode (default is off)\n");
    printf("where h prints this help message\n");
}
