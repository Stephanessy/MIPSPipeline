# MIPSPipeline
using c++ to create a MIPS Pipeline execution model.

* `imem.txt` is used to initialize the Instruction Memory. Each line of the file corresponds to a Byte stored in the Instruction Memory in Big-Endian format. Four contiguous lines correspond to a whole instruction.

* The Data Memory is initialized using the `dmem.txt` file in the same format as `imem.txt`.

* Support a subset of MIPS instructions and a custom instruction `halt`.
  
  | Instructions |       Format       | OpCode(hex) | Funct. (hex) |
  |--------------|--------------------|-------------|--------------|
  |     Addu     | R-Type (ALU)       |     00      |      21      |
  |     Subu     | R-Type (ALU)       |     00      |      23      |
  |      Lw      | I-Type (Memory)    |     23      |      -       |
  |      Sw      | I-Type (Memory)    |     2B      |      -       |
  |     Beq      | I-Type (Control)   |     04      |      -       |
  |     Halt     | Custom Instruction |     FF      |      -       |

* **Note that in this implementation, we assume that `beq` (branch-if-equal) instruction operates like `bne` (branch-if-not-equal).**

* MIPS pipeline has the following 5 stages:
  * **Fetch(IF):** fetches an instruction from instruction memory. Updates PC.
  * **Decode(ID/RF):** reads from the register RF and generates control signals required in subsequent stages. In addition, branches are resolved in this stage by checking for the branch condition and computing the effective address.
  * **EXecute(EX):** performs an ALU operation.
  * **MEMory(MEM):** loads or stores a 32-bit word from data memory.
  * **WriteBack(WB):** writes back data to the RF.

* Each pipeline stages takes inputs from *flip-flops*.

* This implementation deals with 2 kinds of hazards:
  * **RAW Hazards:** RAW hazards are dealt with using either `forwarding` (if possible) or, if not, using `stalling` + `forwarding`.
  * **Control Flow Hazards:**
    * Branches are always assumed to be NOT TAKEN. That is, when a beq is fetched in the IF stage, the PC is speculatively updated as PC+4.
    * Branch conditions are resolved in the ID/RF stage. **This implementation does not deal with RAW dependency for branch instructions.**
    * Two operations are performed in the ID/RF stage:
      * Read_data1 and Read_data2 are compared to determine the branch outcome;
      * the effective branch address is computed.
    * If the branch is NOT TAKEN, execution proceeds normally. However, if the branch is TAKEN, the speculatively fetched instruction from PC+4 is quashed inits ID/RF stage using the nop bit and the next instruction is fetched from the effective branch address. Execution now proceeds normally.

* The `nop` bit:
  * The nop bit for any stage indicates whether it is performing a valid operation in the current clock cycle. The nop bit for the IF stage is initialized to 0 and for all other stages is initialized to 1. (This is because in the first clock cycle, only the IF stage performs a valid operation.)
  * In the absence of hazards, the value of the nop bit for a stage in the current clock cycle is equal to the nop bit of the prior stage in the previous clock cycle.

* The `halt` instruction: The halt instruction is a custom instruction indicating the end of simulation. When a HALT instruction is fetched in IF stage at cycle N, the nop bit of the IF stage in the next clock cycle (cycle N+1) is set to 1 and subsequently stays at 1. The nop bit of the ID/RF stage is set to 1 in cycle N+1 and subsequently stays at 1. The nop bit of the EX stage is set to 1 in cycle N+2 and subsequently stays at 1. The nop bit of the MEM stage is set to 1 in cycle N+3 and subsequently stays at 1. The nop bit of the WB stage is set to 1 in cycle N+4 and subsequently stays at 1.

* Output of the simulator: the values of all flip-flops at the end of each clock cycle. When the simulation terminates, the simulator also outputs the state of the RF and Dmem.
