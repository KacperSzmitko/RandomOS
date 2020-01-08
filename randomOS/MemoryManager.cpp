#include "MemoryManager.h"

//VirtualMemory *vm = new VirtualMemory();

Memory::Memory()
{

}
Memory::Memory(std::shared_ptr<VirtualMemory> vm) : vm(vm)
{
	for (unsigned int i = 0; i < 128; i++)
	{
		this->ram[i] = int(0);
	}
}

uint8_t Memory::writeInMem(int pid, int logical, int8_t data)
{
	//if process is not in swap file return error
	if (vm->swapFile.find(pid) == vm->swapFile.end())
	{
		return 81;
	}
	//if process has no PageTable it assigns new one to it
	if (PageTable.find(pid) == PageTable.end())
	{
		this->PageTable.insert(std::make_pair(pid, this->ProcessPages));
	}
	//process has PageTable and is written in RAM
	if (this->PageTable.find(pid)->first >= 0 && this->PageTable.find(pid)->second[logical / 16].second == 1)
	{
		this->ram[(16 * this->PageTable.find(pid)->second[logical / 16].first) + (logical % 16)] = data;
		vm->updateQueue(PageTable.find(pid)->second[logical / 16].first);
		//update information about frame
		this->FrameTable[PageTable.find(pid)->second[logical / 16].first].pid = pid;
		this->FrameTable[PageTable.find(pid)->second[logical / 16].first].dirtyflag = 1;
		this->FrameTable[PageTable.find(pid)->second[logical / 16].first].page = logical / 16;
		//set byte of correctness
		this->PageTable.find(pid)->second[logical / 16].second = 1;
		return 0;
	}
	//process has PageTable but is not written in RAM
	if (this->PageTable.find(pid)->first >= 0 && this->PageTable.find(pid)->second[logical / 16].second == 0)
	{
		int frame = vm->getVictimFrameNumber();
		//if Page in this Frame has changed it is uploaded back to swapFile
		if (this->FrameTable[frame].dirtyflag == 1)
		{
			Page page;
			for (int i = 0; i < 16; i++)
			{
				page.data[i] = this->ram[(frame * 16) + i];
			}
			vm->updateSwapFilePage(FrameTable[frame].pid, logical / 16, page);
		}
		//Page that we want to change is being downloaded from swapFile
		std::pair<uint8_t,Page> dane = vm->getPage(pid, logical / 16);
		for (int i = 0; i < 16; i++)
		{
			this->ram[i + (16 * frame)] = dane.second.data[i];
		}
		if (this->FrameTable[frame].pid != pid || (this->FrameTable[frame].pid == pid && this->FrameTable[frame].page != logical / 16))
		{
			if (PageTable.find(FrameTable[frame].pid) != PageTable.end())
			{
				this->PageTable.find(FrameTable[frame].pid)->second[frame].first = -1;
				this->PageTable.find(FrameTable[frame].pid)->second[frame].second = 0;
			}

		}
		//the byte that we wanted to write is being written at the logical adress
		this->ram[(logical % 16) + (16 * frame)] = data;
		//update PageTable
		this->PageTable.find(pid)->second[logical / 16].first = frame;
		this->PageTable.find(pid)->second[logical / 16].second = 1;
		vm->updateQueue(frame);
		//update information about frame
		this->FrameTable[frame].pid = pid;
		this->FrameTable[frame].dirtyflag = 1;
		this->FrameTable[frame].page = logical / 16;
		return 0;
	}



}

void Memory::deleteFromMem(int frame)
{
	for (int index = 16 * frame; index < frame + 16; index++)
	{
		this->ram[index] = 0;

	}
	this->FrameTable[frame].dirtyflag = false;

}

void Memory::printMemory()
{
	for (int i = 0; i < 128; i++)
	{
		if (i % 16 == 0)std::cout << "frame " << i / 16 << std::endl;
		std::cout << int(this->ram[i]);
		if (i % 16 == 15) std::cout << std::endl;
	}
}

std::pair<uint8_t, int8_t&> Memory::getMemoryContent(int pid, int logical)
{	
	//used for throwing errors
	int8_t errorByte = -1;
	uint8_t errorCode = 0;

	//if process is not in swap file return error
	if (vm->swapFile.find(pid) == vm->swapFile.end())
	{
		return { 81, errorByte };
	}

	//if process has no PageTable it assigns new one to it
	if (this->PageTable.find(pid) == this->PageTable.end())
	{
		this->PageTable.insert(std::make_pair(pid, this->ProcessPages));
	}
	//process has PageTable and it is not written in RAM
	if (this->PageTable.find(pid)->first >= 0 && this->PageTable.find(pid)->second[logical / 16].first == -1)
	{
		std::cout << "STRONA: " << logical / 16 <<" BAJT: "<< logical %16<< " PID " << pid << " NIE MA " << std::endl;;

		int frame = vm->getVictimFrameNumber();
		//if Page in this Frame has changed it is uploaded back to swapFile
		if (this->FrameTable[frame].dirtyflag == 1)
		{
			Page page;
			for (int i = 0; i < 16; i++)
			{
				page.data[i] = this->ram[(frame * 16) + i];
			}
			errorCode= vm->updateSwapFilePage(this->FrameTable[frame].pid, logical / 16, page);
			if(errorCode!=0){ return { errorCode,errorByte }; }
		}
		//Page that wanted logical adress is being downloaded from swapFile

		std::pair<uint8_t, Page> dane = vm->getPage(pid, logical / 16);
		if (dane.first != 0) { return {dane.first,errorByte}; }

		for (int i = 0; i < 16; i++)
		{
			this->ram[i + (16 * frame)] = dane.second.data[i];
		}
		this->FrameTable[frame].pid = pid;
		this->FrameTable[frame].page = logical / 16;

		errorCode=vm->updateQueue(frame);
		if (errorCode != 0) { return {errorCode,errorByte }; }

		if (this->FrameTable[frame].pid != pid || (this->FrameTable[frame].pid == pid && this->FrameTable[frame].page != logical / 16))
		{
			if (PageTable.find(FrameTable[frame].pid) != PageTable.end())
			{
				this->PageTable.find(FrameTable[frame].pid)->second[frame].first = -1;
				std::cout <<"TUTAJ "<< this->PageTable.find(FrameTable[frame].pid)->first<<std::endl;
				this->PageTable.find(FrameTable[frame].pid)->second[frame].second = 0;
				std::cout<<"TUTAJ " << this->PageTable.find(FrameTable[frame].pid)->second[frame].second << std::endl;
			}

		}
		this->PageTable.find(pid)->second[logical / 16].first = frame;
		this->PageTable.find(pid)->second[logical / 16].second = 1;
		

		return{ 0, this->ram[frame * 16 + logical % 16] };
	}
	//process has PageTable and it is written in RAM
	if (this->PageTable.find(pid)->first >= 0 && this->PageTable.find(pid)->second[logical / 16].second == 1)
	{
		std::cout << "STRONA: "<<logical / 16 << " BAJT: " << logical % 16<< " PID " << pid <<" JEST "<<std::endl;
		this->PageTable.find(pid)->second[logical / 16].second = 1;
		this->FrameTable[PageTable.find(pid)->second[logical / 16].first].pid = pid;
		this->FrameTable[PageTable.find(pid)->second[logical / 16].first].page = logical / 16;

		errorCode=vm->updateQueue(this->PageTable.find(pid)->second[logical / 16].first);
		if (errorCode != 0) { return { errorCode,errorByte }; }

		return { 0,this->ram[(this->PageTable.find(pid)->second[logical / 16].first * 16) + (logical % 16)] };
	}


}

std::pair<std::vector<int>, std::vector<bool>> Memory::printPageTable(int pid, uint8_t& errorCode)
{
	errorCode = 0;
	std::vector<int> ints;
	std::vector<bool> bools;
	if (this->PageTable.find(pid) == PageTable.end()) { errorCode= 82; return { ints,bools };}

	for (int i = 0; i < 8; i++)
	{
		ints.push_back(this->PageTable.find(pid)->second[i].first);
		bools.push_back(this->PageTable.find(pid)->second[i].second);
	}
	return {ints,bools};
}