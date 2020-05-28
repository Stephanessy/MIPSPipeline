#include<iostream>
#include<string>
#include<vector>
#include<bitset>
#include<fstream>
using namespace std;
#define MemSize 1000 // memory size, in reality, the memory size should be 2^32, but for this lab, for the space resaon, we keep it as this large number, but the memory is still 32-bit addressable.

struct IFStruct {
	bitset<32>  PC;
	bool        nop;
	IFStruct() 
	{
		nop = 0;
	}
};

struct IDStruct {
	bitset<32>  Instr;
	bool        nop;
	IDStruct() 
	{
		nop = 1;
	}
};

struct EXStruct {
	bitset<32>  Read_data1;
	bitset<32>  Read_data2;
	bitset<16>  Imm;
	bitset<5>   Rs;
	bitset<5>   Rt;
	bitset<5>   Wrt_reg_addr;
	bool        is_I_type;
	bool        rd_mem;
	bool        wrt_mem;
	bool        alu_op;     //1 for addu, lw, sw, 0 for subu 
	bool        wrt_enable;
	bool        nop;
	bool		isbr;
	EXStruct()
	{
		is_I_type = 0;
		rd_mem = 0;
		wrt_mem = 0;
		alu_op = 1;
		wrt_enable = 0;
		nop = 1;
		isbr = 0;
	}
};

struct MEMStruct {
	bitset<32>  ALUresult;
	bitset<32>  Store_data;
	bitset<5>   Rs;
	bitset<5>   Rt;
	bitset<5>   Wrt_reg_addr;
	bool        rd_mem;
	bool        wrt_mem;
	bool        wrt_enable;
	bool        nop;
	bool		isbr;
	MEMStruct()
	{
		rd_mem = 0;
		wrt_mem = 0;
		wrt_enable = 0;
		nop = 1;
		isbr = 0;
	}
};

struct WBStruct {
	bitset<32>  Wrt_data;
	bitset<5>   Rs;
	bitset<5>   Rt;
	bitset<5>   Wrt_reg_addr;
	bool        wrt_enable;
	bool        nop;
	WBStruct()
	{
		wrt_enable = 0;
		nop = 1;
	}
};

struct stateStruct {
	IFStruct    IF;
	IDStruct    ID;
	EXStruct    EX;
	MEMStruct   MEM;
	WBStruct    WB;
	stateStruct()
	{
		IF = IFStruct();
		ID = IDStruct();
		EX = EXStruct();
		MEM = MEMStruct();
		WB = WBStruct();
	}
};

class RF
{
public:
	bitset<32> Reg_data;
	RF()
	{
		Registers.resize(32);
		Registers[0] = bitset<32>(0);
	}

	bitset<32> readRF(bitset<5> Reg_addr)
	{
		Reg_data = Registers[Reg_addr.to_ulong()];
		return Reg_data;
	}

	void writeRF(bitset<5> Reg_addr, bitset<32> Wrt_reg_data)
	{
		Registers[Reg_addr.to_ulong()] = Wrt_reg_data;
	}

	void outputRF()
	{
		ofstream rfout;
		rfout.open("RFresult.txt", std::ios_base::app);
		if (rfout.is_open())
		{
			rfout << "State of RF:\t" << endl;
			for (int j = 0; j < 32; j++)
			{
				rfout << Registers[j] << endl;
			}
		}
		else cout << "Unable to open file" << "RFresult.txt" << endl;
		rfout.close();
	}

private:
	vector<bitset<32> >Registers;
};

class INSMem
{
public:
	bitset<32> Instruction;
	INSMem()
	{
		IMem.resize(MemSize);
		ifstream imem;
		string line;
		int i = 0;
		imem.open("imem.txt");
		if (imem.is_open())
		{
			while (getline(imem, line))
			{
				IMem[i] = bitset<8>(line);
				i++;
			}
		}
		else cout << "Unable to open file";
		imem.close();
	}

	bitset<32> readInstr(bitset<32> ReadAddress)
	{
		string insmem;
		insmem.append(IMem[ReadAddress.to_ulong()].to_string());
		insmem.append(IMem[ReadAddress.to_ulong() + 1].to_string());
		insmem.append(IMem[ReadAddress.to_ulong() + 2].to_string());
		insmem.append(IMem[ReadAddress.to_ulong() + 3].to_string());
		Instruction = bitset<32>(insmem);		//read instruction memory
		return Instruction;
	}

private:
	vector<bitset<8> > IMem;
};

class DataMem
{
public:
	bitset<32> ReadData;
	DataMem()
	{
		DMem.resize(MemSize);
		ifstream dmem;
		string line;
		int i = 0;
		dmem.open("dmem.txt");
		if (dmem.is_open())
		{
			while (getline(dmem, line))
			{
				DMem[i] = bitset<8>(line);
				i++;
			}
		}
		else cout << "Unable to open file " << "dmem.txt" << endl;
		dmem.close();
	}

	bitset<32> readDataMem(bitset<32> Address)
	{
		string datamem;
		datamem.append(DMem[Address.to_ulong()].to_string());
		datamem.append(DMem[Address.to_ulong() + 1].to_string());
		datamem.append(DMem[Address.to_ulong() + 2].to_string());
		datamem.append(DMem[Address.to_ulong() + 3].to_string());
		ReadData = bitset<32>(datamem);		//read data memory
		return ReadData;
	}

	void writeDataMem(bitset<32> Address, bitset<32> WriteData)
	{
		DMem[Address.to_ulong()] = bitset<8>(WriteData.to_string().substr(0, 8));
		DMem[Address.to_ulong() + 1] = bitset<8>(WriteData.to_string().substr(8, 8));
		DMem[Address.to_ulong() + 2] = bitset<8>(WriteData.to_string().substr(16, 8));
		DMem[Address.to_ulong() + 3] = bitset<8>(WriteData.to_string().substr(24, 8));
	}

	void outputDataMem()
	{
		ofstream dmemout;
		dmemout.open("dmemresult.txt");
		if (dmemout.is_open())
		{
			for (int j = 0; j < 1000; j++)
			{
				dmemout << DMem[j] << endl;
			}

		}
		else cout << "Unable to open file" << "dmemresult.txt" << endl;
		dmemout.close();
	}

private:
	vector<bitset<8> > DMem;
};

void printState(stateStruct state, int cycle)
{
	ofstream printstate;
	printstate.open("stateresult.txt", std::ios_base::app);
	if (printstate.is_open())
	{
		printstate << "State after executing cycle:\t" << cycle << endl;

		printstate << "IF.PC:\t" << state.IF.PC.to_ulong() << endl;
		printstate << "IF.nop:\t" << state.IF.nop << endl;

		printstate << "ID.Instr:\t" << state.ID.Instr << endl;
		printstate << "ID.nop:\t" << state.ID.nop << endl;

		printstate << "EX.Read_data1:\t" << state.EX.Read_data1 << endl;
		printstate << "EX.Read_data2:\t" << state.EX.Read_data2 << endl;
		printstate << "EX.Imm:\t" << state.EX.Imm << endl;
		printstate << "EX.Rs:\t" << state.EX.Rs << endl;
		printstate << "EX.Rt:\t" << state.EX.Rt << endl;
		printstate << "EX.Wrt_reg_addr:\t" << state.EX.Wrt_reg_addr << endl;
		printstate << "EX.is_I_type:\t" <<state.EX.is_I_type << endl;
		printstate << "EX.rd_mem:\t" << state.EX.rd_mem << endl;
		printstate << "EX.wrt_mem:\t" << state.EX.wrt_mem << endl;
		printstate << "EX.alu_op:\t" << state.EX.alu_op << endl;
		printstate << "EX.wrt_enable:\t" << state.EX.wrt_enable << endl;
		printstate << "EX.nop:\t" << state.EX.nop << endl;

		printstate << "MEM.ALUresult:\t" << state.MEM.ALUresult << endl;
		printstate << "MEM.Store_data:\t" << state.MEM.Store_data << endl;
		printstate << "MEM.Rs:\t" << state.MEM.Rs << endl;
		printstate << "MEM.Rt:\t" << state.MEM.Rt << endl;
		printstate << "MEM.Wrt_reg_addr:\t" << state.MEM.Wrt_reg_addr << endl;
		printstate << "MEM.rd_mem:\t" << state.MEM.rd_mem << endl;
		printstate << "MEM.wrt_mem:\t" << state.MEM.wrt_mem << endl;
		printstate << "MEM.wrt_enable:\t" << state.MEM.wrt_enable << endl;
		printstate << "MEM.nop:\t" << state.MEM.nop << endl;

		printstate << "WB.Wrt_data:\t" << state.WB.Wrt_data << endl;
		printstate << "WB.Rs:\t" << state.WB.Rs << endl;
		printstate << "WB.Rt:\t" << state.WB.Rt << endl;
		printstate << "WB.Wrt_reg_addr:\t" << state.WB.Wrt_reg_addr << endl;
		printstate << "WB.wrt_enable:\t" << state.WB.wrt_enable << endl;
		printstate << "WB.nop:\t" << state.WB.nop << endl;
	}
	else cout << "Unable to open file" << endl;
	printstate.close();
}

bitset<32> signextend(bitset<16> imm)
{
	string sestring;
	if (imm[15] == 0) {
		sestring = "0000000000000000" + imm.to_string<char, std::string::traits_type, std::string::allocator_type>();
	}
	else {
		sestring = "1111111111111111" + imm.to_string<char, std::string::traits_type, std::string::allocator_type>();
	}
	return (bitset<32>(sestring));
}

bitset<30> signextend30(bitset<16> imm)
{
	string sestring;
	if (imm[15] == 0) {
		sestring = "00000000000000" + imm.to_string<char, std::string::traits_type, std::string::allocator_type>();
	}
	else {
		sestring = "11111111111111" + imm.to_string<char, std::string::traits_type, std::string::allocator_type>();
	}
	return (bitset<30>(sestring));
}

unsigned long shiftbits(bitset<32> inst, int start)
{
	unsigned long ulonginst;
	return ((inst.to_ulong()) >> start);
}

int main()
{

	RF myRF = RF();
	INSMem myInsMem = INSMem();
	DataMem myDataMem = DataMem();

	stateStruct state = stateStruct();
	stateStruct newState = stateStruct();

	int cycle = 0;

	while (1) {
		/* --------------------- WB stage --------------------- */
		if (state.WB.nop == 0)
		{
			if (state.WB.wrt_enable == 1)// load
			{
				myRF.writeRF(state.WB.Wrt_reg_addr, state.WB.Wrt_data);
			}
		}


		/* --------------------- MEM stage --------------------- */
		if (state.MEM.nop == 0)
		{
			if (state.WB.Wrt_reg_addr == state.MEM.Rt && state.WB.nop == 0)// MEM to MEM forward
			{
				state.MEM.Store_data = state.WB.Wrt_data;
			}
			if (state.MEM.rd_mem == 1)//load data
			{
				newState.WB.Wrt_data = myDataMem.readDataMem(state.MEM.ALUresult);
			}
			else
			{
				newState.WB.Wrt_data = state.MEM.ALUresult;
			}

			if (state.MEM.wrt_mem == 1)//store data
			{
				myDataMem.writeDataMem(state.MEM.ALUresult, state.MEM.Store_data);
				newState.WB.Wrt_data = state.WB.Wrt_data;
			}
			if (state.MEM.isbr)
			{
				newState.WB.Wrt_data = state.WB.Wrt_data;
			}

			// update flip-flop
			newState.WB.Wrt_reg_addr = state.MEM.Wrt_reg_addr;
			newState.WB.wrt_enable = state.MEM.wrt_enable;
			newState.WB.Rs = state.MEM.Rs;
			newState.WB.Rt = state.MEM.Rt;
			newState.WB.nop = state.MEM.nop;
		}
		else
			newState.WB.nop = state.MEM.nop;


		/* --------------------- EX stage --------------------- */
		if (state.EX.nop == 0)
		{
			if (state.EX.wrt_enable == 1 && state.EX.is_I_type == 0 && state.EX.alu_op == 1)// add
			{
				if (state.WB.Wrt_reg_addr == state.EX.Rs && state.WB.nop == 0 && state.WB.wrt_enable == 1)// MEM to EX forward
				{
					state.EX.Read_data1 = state.WB.Wrt_data;
				}
				if (state.WB.Wrt_reg_addr == state.EX.Rt && state.WB.nop == 0 && state.WB.wrt_enable == 1)// MEM to EX forward
				{
					state.EX.Read_data2 = state.WB.Wrt_data;
				}
				if (state.MEM.Wrt_reg_addr == state.EX.Rs && state.MEM.nop == 0 && state.MEM.wrt_enable == 1)// EX to EX forward (should be behind MEM to EX forward)
				{
					state.EX.Read_data1 = state.MEM.ALUresult;
				}
				if (state.MEM.Wrt_reg_addr == state.EX.Rt && state.MEM.nop == 0 && state.MEM.wrt_enable == 1)// EX to EX forward
				{
					state.EX.Read_data2 = state.MEM.ALUresult;
				}

				newState.MEM.ALUresult = bitset<32>(state.EX.Read_data1.to_ulong() + state.EX.Read_data2.to_ulong());
			}
			else if (state.EX.wrt_enable == 1 && state.EX.is_I_type == 0 && state.EX.alu_op == 0)// sub
			{
				if (state.WB.Wrt_reg_addr == state.EX.Rs && state.WB.nop == 0 && state.WB.wrt_enable == 1)// MEM to EX forward
				{
					state.EX.Read_data1 = state.WB.Wrt_data;
				}
				if (state.WB.Wrt_reg_addr == state.EX.Rt && state.WB.nop == 0 && state.WB.wrt_enable == 1)// MEM to EX forward
				{
					state.EX.Read_data2 = state.WB.Wrt_data;
				}
				if (state.MEM.Wrt_reg_addr == state.EX.Rs && state.MEM.nop == 0 && state.MEM.wrt_enable == 1)// EX to EX forward (should be behind MEM to EX forward)
				{
					state.EX.Read_data1 = state.MEM.ALUresult;
				}
				if (state.MEM.Wrt_reg_addr == state.EX.Rt && state.MEM.nop == 0 && state.MEM.wrt_enable == 1)// EX to EX forward
				{
					state.EX.Read_data2 = state.MEM.ALUresult;
				}
				
				newState.MEM.ALUresult = bitset<32>(state.EX.Read_data1.to_ulong() - state.EX.Read_data2.to_ulong());
			}
			else if (state.EX.wrt_enable == 1 && state.EX.is_I_type == 1 && state.EX.alu_op == 1)// load
			{
				if (state.WB.Wrt_reg_addr == state.EX.Rt && state.WB.nop == 0 && state.WB.wrt_enable == 1)// MEM to EX forward, for add/sub to load dependency
				{
					state.EX.Read_data2 = state.WB.Wrt_data;
				}
				if (state.WB.Wrt_reg_addr == state.EX.Rs && state.WB.nop == 0 && state.WB.wrt_enable == 1)
				{
					state.EX.Read_data1 = state.WB.Wrt_data;
				}
				if (state.MEM.Wrt_reg_addr == state.EX.Rs && state.MEM.nop == 0 && state.MEM.wrt_enable == 1)// EX to EX forward (should be behind MEM to EX forward)
				{
					state.EX.Read_data1 = state.MEM.ALUresult;
				}
				if (state.MEM.Wrt_reg_addr == state.EX.Rt && state.MEM.nop == 0 && state.MEM.wrt_enable == 1)// EX to EX forward
				{
					state.EX.Read_data2 = state.MEM.ALUresult;
				}
				
				newState.MEM.ALUresult = bitset<32>(state.EX.Read_data1.to_ulong() + signextend(state.EX.Imm).to_ulong());
			}
			else if (state.EX.wrt_enable == 0 && state.EX.is_I_type == 1 && state.EX.alu_op == 1)// store
			{
				if (state.WB.Wrt_reg_addr == state.EX.Rt && state.WB.nop == 0 && state.WB.wrt_enable == 1)// MEM to EX forward, for add/sub to store dependency
				{
					state.EX.Read_data2 = state.WB.Wrt_data;
				}
				if (state.WB.Wrt_reg_addr == state.EX.Rs && state.WB.nop == 0 && state.WB.wrt_enable == 1)
				{
					state.EX.Read_data1 = state.WB.Wrt_data;
				}
				if (state.MEM.Wrt_reg_addr == state.EX.Rs && state.MEM.nop == 0 && state.MEM.wrt_enable == 1)// EX to EX forward (should be behind MEM to EX forward)
				{
					state.EX.Read_data1 = state.MEM.ALUresult;
				}
				if (state.MEM.Wrt_reg_addr == state.EX.Rt && state.MEM.nop == 0 && state.MEM.wrt_enable == 1)// EX to EX forward
				{
					state.EX.Read_data2 = state.MEM.ALUresult;
				}
				
				newState.MEM.ALUresult = bitset<32>(state.EX.Read_data1.to_ulong() + signextend(state.EX.Imm).to_ulong());
			}
			else// beq
			{
				newState.MEM.ALUresult = bitset<32>("00000000000000000000000000000000");
			}

			// update flip-flop
			newState.MEM.Rs = state.EX.Rs;
			newState.MEM.Rt = state.EX.Rt;
			newState.MEM.Wrt_reg_addr = state.EX.Wrt_reg_addr;
			newState.MEM.rd_mem = state.EX.rd_mem;
			newState.MEM.wrt_mem = state.EX.wrt_mem;
			newState.MEM.wrt_enable = state.EX.wrt_enable;
			newState.MEM.nop = state.EX.nop;
			newState.MEM.Store_data = state.EX.Read_data2;
			newState.MEM.isbr = state.EX.isbr;
		}
		else
			newState.MEM.nop = state.EX.nop;


		/* --------------------- ID stage --------------------- */
		if (state.ID.nop == 0)
		{
			bitset<6> opcode = bitset<6>(shiftbits(state.ID.Instr, 26));
			bitset<1> IType = bitset<1>((opcode.to_ulong() != 0 && opcode.to_ulong() != 2 && opcode.to_ulong() != 4) ? 1 : 0);
			bitset<1> JType = bitset<1>((opcode.to_ulong() == 2) ? 1 : 0);
			newState.EX.is_I_type = IType.to_ulong();
			bitset<1> IsBranch = bitset<1>((opcode.to_ulong() == 4) ? 1 : 0);
			newState.EX.isbr = IsBranch.to_ulong();
			bitset<1> IsLoad = bitset<1>((opcode.to_ulong() == 35) ? 1 : 0);
			bitset<1> IsStore = bitset<1>((opcode.to_ulong() == 43) ? 1 : 0);
			newState.EX.rd_mem = IsLoad.to_ulong();
			newState.EX.wrt_mem = IsStore.to_ulong();
			newState.EX.wrt_enable = (IsStore.to_ulong() || IsBranch.to_ulong() || JType.to_ulong()) ? 0 : 1;
			bitset<5> RReg1 = bitset<5>(shiftbits(state.ID.Instr, 21));
			bitset<5> RReg2 = bitset<5>(shiftbits(state.ID.Instr, 16));
			newState.EX.Rs = RReg1;
			newState.EX.Rt = RReg2;
			bitset<6> funct = bitset<6>(shiftbits(state.ID.Instr, 0));
			bitset<1> ALUop;
			if (opcode.to_ulong() == 35 || opcode.to_ulong() == 43 || funct.to_ulong() == 33 || opcode.to_ulong() == 4)
			{
				ALUop = bitset<1>(string("1"));
			}
			else
			{
				ALUop = bitset<1>(string("0"));
			}
			newState.EX.alu_op = ALUop.to_ulong();
			bitset<16> imm = bitset<16>(shiftbits(state.ID.Instr, 0));
			newState.EX.Imm = imm;
			if (IType.to_ulong())
			{
				newState.EX.Wrt_reg_addr = RReg2;
			}
			else
			{
				newState.EX.Wrt_reg_addr = bitset<5>(shiftbits(state.ID.Instr, 11));
			}
			newState.EX.Read_data1 = myRF.readRF(RReg1);
			newState.EX.Read_data2 = myRF.readRF(RReg2);
			newState.EX.nop = state.ID.nop;
			bitset<1> IsEq = (myRF.readRF(RReg1).to_ulong() == myRF.readRF(RReg2).to_ulong()) ? 1 : 0;

			// Load-Add stall
			if (state.EX.is_I_type == 1 && state.EX.rd_mem == 1)
			{
				if (state.EX.Rt == bitset<5>(shiftbits(state.ID.Instr, 21)) && state.EX.nop == 0 && bitset<6>(shiftbits(state.ID.Instr, 26)) == bitset<6>("000000"))
				{
					newState.EX.nop = 1;
				}
				if (state.EX.Rt == bitset<5>(shiftbits(state.ID.Instr, 16)) && state.EX.nop == 0 && bitset<6>(shiftbits(state.ID.Instr, 26)) == bitset<6>("000000"))
				{
					newState.EX.nop = 1;
				}
			}
			// Load-Store stall
			if (state.EX.is_I_type == 1 && state.EX.rd_mem == 1 && bitset<1>(((bitset<6>(shiftbits(state.ID.Instr, 26))).to_ulong() == 43) ? 1 : 0) == bitset<1>("1"))
			{
				if (state.EX.Rt == bitset<5>(shiftbits(state.ID.Instr, 21)))
				{
					newState.EX.nop = 1;
				}
			}
			if (IsBranch == bitset<1>("1"))
			{
				newState.EX.Wrt_reg_addr = bitset<5>("00000");
			}
			if (IsBranch == bitset<1>("1") && IsEq == bitset<1>("0"))// beq, if Rs!=Rt, need to jump. Otherwise skip.
			{
				state.IF.PC = bitset<32>(state.IF.PC.to_ulong() - 4);
			}
		}
		else
			newState.EX.nop = state.ID.nop;


		/* --------------------- IF stage --------------------- */
		if (state.IF.nop == 0)
		{
			bitset<32> instruction = myInsMem.readInstr(state.IF.PC);
			newState.ID.Instr = instruction;

			// beq
			/*if (bitset<1>((bitset<6>(shiftbits(instruction, 26)).to_ulong() == 4) ? 1 : 0) == bitset<1>("1") && bitset<1>((myRF.readRF(bitset<5>(shiftbits(instruction, 21))).to_ulong() == myRF.readRF(bitset<5>(shiftbits(instruction, 16))).to_ulong()) ? 1 : 0) == bitset<1>("0"))
			{
				//state.IF.PC = bitset<32>(state.IF.PC.to_ulong() - 4);
				newState.ID.Instr = myInsMem.readInstr(state.IF.PC);
				//state.IF.PC = bitset<32>(state.IF.PC.to_ulong() - 4);
			}*/

			if (instruction.to_string<char, std::string::traits_type, std::string::allocator_type>() == "11111111111111111111111111111111")
			{
				state.IF.nop = 1;
			}
			newState.ID.nop = state.IF.nop;

			// Load-Add stall
			if (state.EX.is_I_type == 1 && state.EX.rd_mem == 1)
			{
				if (state.EX.Rt == bitset<5>(shiftbits(state.ID.Instr, 21)) && state.EX.nop == 0 && bitset<6>(shiftbits(state.ID.Instr, 26)) == bitset<6>("000000"))
				{
					newState.ID = state.ID;
				}
				if (state.EX.Rt == bitset<5>(shiftbits(state.ID.Instr, 16)) && state.EX.nop == 0 && bitset<6>(shiftbits(state.ID.Instr, 26)) == bitset<6>("000000"))
				{
					newState.ID = state.ID;
				}
			}
			// Load-Store stall
			if (state.EX.is_I_type == 1 && state.EX.rd_mem == 1 && bitset<1>(((bitset<6>(shiftbits(state.ID.Instr, 26))).to_ulong() == 43) ? 1 : 0) == bitset<1>("1"))
			{
				if (state.EX.Rt == bitset<5>(shiftbits(state.ID.Instr, 21)))
				{
					newState.ID = state.ID;
				}
			}
			// beq
			if (bitset<1>((bitset<6>(shiftbits(state.ID.Instr, 26)).to_ulong() == 4) ? 1 : 0) == bitset<1>("1") && bitset<1>((myRF.readRF(bitset<5>(shiftbits(state.ID.Instr, 21))).to_ulong() == myRF.readRF(bitset<5>(shiftbits(state.ID.Instr, 16))).to_ulong()) ? 1 : 0) == bitset<1>("0"))
			{
				if (state.ID.nop == 0)
				{
					newState.ID.nop = 1;
				}
			}
		}
		else
			newState.ID.nop = state.IF.nop;


		if (state.IF.nop && state.ID.nop && state.EX.nop && state.MEM.nop && state.WB.nop)
			break;

		if (myInsMem.readInstr(state.IF.PC).to_string<char, std::string::traits_type, std::string::allocator_type>() != "11111111111111111111111111111111")
		{
			// beq
			if (bitset<1>((bitset<6>(shiftbits(state.ID.Instr, 26)).to_ulong() == 4) ? 1 : 0) == bitset<1>("1") && bitset<1>((myRF.readRF(bitset<5>(shiftbits(state.ID.Instr, 21))).to_ulong() == myRF.readRF(bitset<5>(shiftbits(state.ID.Instr, 16))).to_ulong()) ? 1 : 0) == bitset<1>("0"))
			{
				if (state.ID.nop == 0)
				{
					newState.IF.PC = bitset<32>(state.IF.PC.to_ulong() + 4 + bitset<32>(signextend30(bitset<16>(shiftbits(state.ID.Instr, 0))).to_string<char, std::string::traits_type, std::string::allocator_type>() + "00").to_ulong());
				}
				else
				{
					newState.IF.PC = state.IF.PC.to_ulong() + 4;
				}
			}
			else
			{
				newState.IF.PC = state.IF.PC.to_ulong() + 4;
			}
		}
		newState.IF.nop = state.IF.nop;

		// Load-Add stall
		if (state.EX.is_I_type == 1 && state.EX.rd_mem == 1)
		{
			if (state.EX.Rt == bitset<5>(shiftbits(state.ID.Instr, 21)) && state.EX.nop == 0 && bitset<6>(shiftbits(state.ID.Instr, 26)) == bitset<6>("000000"))
			{
				newState.IF = state.IF;
			}
			if (state.EX.Rt == bitset<5>(shiftbits(state.ID.Instr, 16)) && state.EX.nop == 0 && bitset<6>(shiftbits(state.ID.Instr, 26)) == bitset<6>("000000"))
			{
				newState.IF = state.IF;
			}
		}
		// Load-Store stall
		if (state.EX.is_I_type == 1 && state.EX.rd_mem == 1 && bitset<1>(((bitset<6>(shiftbits(state.ID.Instr, 26))).to_ulong() == 43) ? 1 : 0) == bitset<1>("1"))
		{
			if (state.EX.Rt == bitset<5>(shiftbits(state.ID.Instr, 21)))
			{
				newState.IF = state.IF;
			}
		}

		printState(newState, cycle); //print states after executing cycle 0, cycle 1, cycle 2 ... 
		cycle = cycle + 1;

		state = newState; /*The end of the cycle and updates the current state with the values calculated in this cycle */

	}

	myRF.outputRF(); // dump RF;	
	myDataMem.outputDataMem(); // dump data mem 
	return 0;
}