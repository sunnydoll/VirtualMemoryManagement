# VirtualMemoryManagement

The file Page.c contains the program which simulates virtual memory manager has two functions, getPNumandPOff function and main function.<br>
The getPNumandPOff function masks the rightmost 16 bits of each logical address and gets the page number and page offset.<br>

The main function accomplishes almost all of work. I used LRU algorithm for page replacement and updating of TLB, for that I
declared a number as a counter for each entry of TLB and physical memory. The page in the entry with the <b>biggest</b> counter will be replaced. And if there are more than one entries have equal counter, the first one of them based on the index will be chosen in replacement. I used arrays including 2D arrays and 1D arrays as my main tool to simulate the TLB, page table and physical memory. And I also added comments for my code.<br>
Since the program has three situations as follows:<br>
1.Basic part, before the modification mentioned in the textbook. That means the size of physical memory is 256, the same as the size of page table.<br>
2.Modification part, after modify the program as modification in the textbook. That means the size of physical memory is 128.<br>
3.Additional functionality part, after the program accomplished the additional functionality that was assigned by the professor. That means the program is able to set dirty bit to the pages and denote a copy back to the swap when page is replaced in memory.<br>

I accomplished each situation and save the output of each situation.<br>
1.The output1.txt is the output of the first situation, the result is the same as that in the correct.txt.<br>
2.The output2.txt is the output of the second situation.<br>
3.The output.txt is the output of the third situation.<br>
The program is in the third situation, so its output is the same as that in output.txt file and will be saved in the output.txt file.

Ps: One thing I noticed is that there are 1000 numbers in the addresses.txt, while there are 1001 numbers in the addresses2.txt, so that the result in output.txt is different from the result in output2.txt. And please put output.txt, addresses.txt, addresses2.txt and BACKING_STORE.bin file in the same directory with Page.c file before you try to run my program.

Thanks!

------Zhichao Cao
