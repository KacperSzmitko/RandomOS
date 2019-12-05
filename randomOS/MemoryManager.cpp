// Memory.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "MemoryManager.h"




int Memory::findFreeFrame()
{
	for (int i = 0; i < 8; i++)
	{
		if (FrameTable[i].taken == false)
		{
			return i;
		}
		else
		{
			std::cout << " no free frame " << std::endl;
			return 8;

		}
	}
}

void Memory::writeInMem(uint8_t data, int pid, int logical)
{

	for (int i = 0; i < 16; i++)
	{
		if (FrameTable[i].pid == pid && FrameTable[i].ProcessPageNr == logical / 16)
		{
			this->ram[16 * i + logical%16] = data;
		}
		if(int frame = findFreeFrame()==8)
		{
			//virtual
		}
		if (int frame = findFreeFrame() < 8)
		{
			this->ram[logical%16 + 16 * frame] = data;
			this->FrameTable[frame].taken = true;
			this->FrameTable[frame].lastUse++;
			this->FrameTable[frame].ProcessPageNr = logical / 16;
			this->FrameTable[frame].pid = pid;
		}
	}
}

void Memory::deleteFromMem(int frame)
{
	for (int index = 16*frame; index < frame + 16; index++)
	{
		this->ram[index] = 0;
		
	}
	this->FrameTable[frame].taken == false;
	this->FrameTable[frame].lastUse = 0;
}

void Memory::printMemory()
{
	for (int i = 0; i < 128; i++)
	{
		std::cout << this->ram[i];
		if (i % 16 == 15) std::cout << std::endl;
	}
}

uint8_t& Memory::getMemoryContent(int logical,int pid)
{
	for (int i = 0; i < 8; i++)
	{
		if (FrameTable[i].pid == pid && logical / 16 == FrameTable[i].ProcessPageNr)
		{
			return ram[i * 16 + logical % 16];
		}
		else std::cout << "wrong logical address" << std::endl;
	}
	
	
	
	
}





int main()
{

}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
