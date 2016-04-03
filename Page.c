/**
* @author: Zhichao Cao
*/

#include<stdio.h> 
#include<stdlib.h> 
#include<string.h>


#define PAGE_NUM 256
#define PHY_NUM 128


void getPNumandPOff(int logical_address, int* offset, int* page_number)
{
    int mask = 0xFF;
    *offset = logical_address & mask;
    *page_number = (logical_address >> 8) & mask;
}

void main(int argc, const char* argv[])
{
	//Initial part
	int logical[1024];  //List of logical memory
	int physical[1024];  //List of physical memory
	int value[1024];  //List of unsigned value
	int operation[1024];  //List of i/o operation
	float sum = 0;
	int temp = 0;
	int tempValue = 0;
	int tempPNum = 0;
	char addresses[100];  //File name for the address.txt
	char numStr[1024];  //String read from addresses.txt
	char tempStr[1024];
	int bitRW = 0;
	int i = 0;
	int j = 0;
	int k = 0;
	int hit = 0;  //The TLB hit 
	int fault = 0;  //The page fault times
	double hitRate = 0;  //The TLB hit rate
	double faultRate = 0;  //The page fault rate
	int TLBmiss = 1;  //TLB miss
	int runtime = 0;  //The total time of loop for the program
	int mask = 0xFF;
	int pNumBin = 0;
	int pOffBin = 0;
	int pNum = 0;  //Page number for the number from file
	int pOff = 0;  //Offset number for the number from file
	int pFrame = 0;  //Frame number for the number from file
	int pAddress = 0;  //Address for the number fro file
	char page_Buffer[PAGE_NUM];  //Buffer for reading .bin file
	char strOutput[100];  //String for output and save it to the output.txt file

	int page[PAGE_NUM][PAGE_NUM];  //Page table 
	int page_Frame[PAGE_NUM];  //Page table frame bit
	int page_Valid[PAGE_NUM];  //Page table valid-invalid bit

	int TLB_Page[16];  //TLB page bit
	int TLB_Frame[16];  //TLB frame bit
	int TLB_Counter[16];  //TLB counter for LRU
	int TLB_MaxIndex = 0;  //TLB index for the element with biggest counter

	int frame[PHY_NUM][PAGE_NUM];  //Physical memory
	int frame_Page[PHY_NUM];  //Physical memory page bit
	int frame_Counter[PHY_NUM];  //Physical memory counter for LRU
	int frame_MaxIndex = 0;  //TPhysical memory index for the element with biggest counter	
	int frame_ModiBit[PHY_NUM];  //Modification bit

	FILE *fpRead;
	FILE *fpWrite;
	FILE *binRead;
	FILE *binWrite;

	//Initialize pagetable
	for(i = 0; i < PAGE_NUM; i++)
	{
		for(j = 0; j < PAGE_NUM; j++)
		{
			page[i][j] = -1;
		}
		page_Frame[i] = -1;
		page_Valid[i] = 0;
	}

	//Initialize TLB
	for(i = 0; i < 16; i++)
	{
		TLB_Page[i] = -1;
		TLB_Frame[i] = -1;
		TLB_Counter[i] = 0;
	}

	//Initialize physical memory
	for(i = 0; i < PHY_NUM; i++)
	{
		for(j = 0; j < PAGE_NUM; j++)
		{
			frame[i][j] = -1;
		}
		frame_Page[i] = -1;
		frame_Counter[i] = 0;
		frame_ModiBit[i] = 0;
	}	

	if (argc < 2)
    {
        printf("Please type: ./a.out addresses.txt or ./a.out addresses2.txt\n");
        exit(EXIT_FAILURE);
    }

	//Read the address file
	strcpy(addresses, argv[1]);
	if(strcmp(addresses, "addresses.txt"))
	{
		bitRW = 0;
	}
	else if(strcmp(addresses, "addresses2.txt"))
	{
		bitRW = 1;
	}
	fpRead = fopen(addresses, "r");
	if(fpRead == NULL) 
	{
        //File read failed
        printf("The porgram cannot get access to the %s, please put the file in the same directory with the code file.\n", addresses);
		return;
	}
	else
	{
		k=0;
		if(bitRW == 0)
		{
			while(fgets(numStr, 1024, fpRead) != NULL)
			{
				logical[k] = atol(numStr);
				operation[k] = 0;
				k++;
				sum++;
			}
		}
		else
		{
			while(fgets(numStr, 1024, fpRead) != NULL)
			{
				for(i = 0; i < 1024; i++)
				{
					if(numStr[i] == ' ')
					{
						break;
					}
					tempStr[i] = numStr[i];
				}
				if(numStr[i+1] == 'W' || numStr[i+1] == 'w')
				{
					operation[k] = 2;
				}
				else
				{
					operation[k] = 1;
				}
				logical[k] = atol(tempStr);
				k++;
				sum++;
			}
		}

		fclose(fpRead);
	}
		
	while(runtime < sum)
	{		
		for(i = 0; i < PAGE_NUM; i++)
		{
			page_Buffer[i] = ' ';
		}
		i = 0;
		j = 0;
		k = 0;
		getPNumandPOff(logical[runtime], &pOffBin, &pNumBin);
		pNum = logical[runtime] / 256;
		pOff = logical[runtime] % 256;
		pFrame = -1;

		//TLB query
		for(i = 0; i < 16; i++)
		{
			//Plus counter for LRU
			TLB_Counter[i]++;
		}
		for(i = 0; i < 16; i++)
		{
			//Check the TLB for frame number
			if(TLB_Page[i] == pNum)
			{
				hit++;
				pFrame = TLB_Frame[i];
				TLB_Counter[i] = 0;
				TLBmiss = 0;
				break;
			}
			else 
			{
				TLBmiss = 1;
			}
		}

		for(i = 0; i < PHY_NUM; i++)
		{
			frame_Counter[i]++;
		}

		if(TLBmiss == 1)
		{
			//Page table query
			pFrame = page_Frame[pNum];

			//Physical Memory query		
			if(page_Valid[pNum] == 1)
			{
				//If the page is valid
				physical[runtime] = pFrame * 256 + pOff;
				value[runtime] = frame[pFrame][pOff];
				frame_Counter[pFrame] = 0;
			}
			else
			{
				//The page fault occurs
				fault++;

				//Read from .bin file and update physical memory
				binRead = fopen("BACKING_STORE.bin", "rb");
				if(binRead == NULL) 
				{
					//File read failed
					printf("The porgram cannot get access to the BACKING_STORE.bin, please put the file in the same directory with the code file.\n");
					return;
				}
				else
				{
					temp = frame_Counter[0];
					frame_MaxIndex = 0;
					for(i = 0; i < PHY_NUM; i++)
					{
						//LRU algorithm get the least recently used frame
						if(frame_Counter[i] > temp)
						{
							temp = frame_Counter[i];
							frame_MaxIndex = i;
						}
					}
					//Update frame
					pFrame = frame_MaxIndex;
					tempPNum = frame_Page[pFrame];
					if(tempPNum >= 0)
					{
						page_Valid[tempPNum] = 0;  //Update page table valid-invalid bit
						if(frame_ModiBit[pFrame] == 2)
						{
							//Modify bit detected, write back to bin file before replacement
							binWrite = fopen("BACKING_STORE.bin", "rb");
							if(binWrite == NULL) 
							{
								//File read failed
								printf("The porgram cannot get access to the BACKING_STORE.bin, please put the file in the same directory with the code file.\n");
								return;
							}
							else
							{
								fseek(binWrite, frame_Page[pFrame]*PAGE_NUM, SEEK_SET);
								fwrite(&frame[pFrame], 1, PAGE_NUM, binWrite);
								fclose(binWrite);
							}
						}
					}
					fseek(binRead, pNumBin*PAGE_NUM, SEEK_SET);
					fread(page_Buffer, 1, PAGE_NUM, binRead);
					for(i = 0; i < PAGE_NUM; i++)
					{						
						tempValue = (int)page_Buffer[i];
						frame[pFrame][i] = tempValue;
					}
					frame_Page[pFrame] = pNum;
					frame_Counter[pFrame] = 0;
					frame_ModiBit[pFrame] = operation[runtime];
					physical[runtime] = pFrame * 256 + pOff;
					value[runtime] = frame[pFrame][pOff];
					fclose(binRead);
					//Update page table
					page_Valid[pNum] = 1;
					page_Frame[pNum] = pFrame;
				}
			}

			//Update TLB
			temp = TLB_Counter[0];
			TLB_MaxIndex = 0;
			for(i = 0; i < 16; i++)
			{
				if(tempPNum >= 0 && TLB_Page[i] == tempPNum)
				{
					TLB_MaxIndex = i;
					break;
				}
				if(TLB_Counter[i] > temp)
				{
					temp = TLB_Counter[i];
					TLB_MaxIndex = i;
				}
			}
			TLB_Page[TLB_MaxIndex] = pNum;
			TLB_Frame[TLB_MaxIndex] = pFrame;
			TLB_Counter[TLB_MaxIndex] = 0;
		}
		else
		{
			//TLB hit
			frame_Counter[pFrame] = 0;
			physical[runtime] = pFrame * 256 + pOff;
			value[runtime] = frame[pFrame][pOff];
		}

		runtime++;
	}
	
	//Calculate the rate of page fault and TLB hit
	faultRate = fault /sum;
	hitRate = hit / sum;

	//Write result to output.txt
	fpWrite = fopen("output.txt", "w");
	if(fpWrite == NULL) 
	{
		//File read failed
		printf("The porgram cannot get access to the output.txt, please put the file in the same directory with the code file.\n");
		return;
	}
	else
	{
		for(i = 0; i < sum; i++)
		{
			sprintf(strOutput, "Logical address: %d ", logical[i]); 
			fputs(strOutput, fpWrite);
			sprintf(strOutput, "Physical address: %d ", physical[i]); 
			fputs(strOutput, fpWrite);
			sprintf(strOutput, "Value: %d ", value[i]); 
			fputs(strOutput, fpWrite);
			fputs("\n", fpWrite);
		}
		fputs("\n", fpWrite);
		sprintf(strOutput, "The number of translated address is %.0f\n", sum); 
		fputs(strOutput, fpWrite);
		sprintf(strOutput, "The total number of page-fault is %d\n", fault); 
		fputs(strOutput, fpWrite);
		sprintf(strOutput, "The rate of page-fault is %.4f\n", faultRate); 
		fputs(strOutput, fpWrite);
		sprintf(strOutput, "The total number of TLB hit is %d\n", hit); 
		fputs(strOutput, fpWrite);
		sprintf(strOutput, "The rate of TLB hit is %.4f\n", hitRate); 
		fputs(strOutput, fpWrite);
		fclose(fpWrite);
	}

	printf("The number of translated address is %.0f\n", sum);
	printf("The total number of page-fault is %d\n", fault);
	printf("The rate of page-fault is %.4f\n", faultRate);
	printf("The total number of TLB hit is %d\n", hit);
	printf("The rate of TLB hit is %.4f\n", hitRate);

}
