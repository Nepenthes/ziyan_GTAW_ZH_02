#ifndef LD_CHIP_H
#define LD_CHIP_H

#define uint8 unsigned char
#define uint16 unsigned int
#define uint32 unsigned long

//	以下三个状态定义用来记录程序是在运行ASR识别还是在运行MP3播放
#define LD_MODE_IDLE		0x00
#define LD_MODE_ASR_RUN		0x08
#define LD_MODE_MP3		 	0x40


//	以下五个状态定义用来记录程序是在运行ASR识别过程中的哪个状态
#define LD_ASR_NONE				0x00	//	表示没有在作ASR识别
#define LD_ASR_RUNING			0x01	//	表示LD3320正在作ASR识别中
#define LD_ASR_FOUNDOK			0x10	//	表示一次识别流程结束后，有一个识别结果
#define LD_ASR_FOUNDZERO 		0x11	//	表示一次识别流程结束后，没有识别结果
#define LD_ASR_ERROR	 		0x31	//	表示一次识别流程中LD3320芯片内部出现不正确的状态

#define CLK_IN   	8	/* user need modify this value according to clock in */
#define LD_PLL_11			(uint8)((CLK_IN/2.0)-1)
#define LD_PLL_MP3_19		0x0f
#define LD_PLL_MP3_1B		0x18
#define LD_PLL_MP3_1D   	(uint8)(((90.0*((LD_PLL_11)+1))/(CLK_IN))-1)

#define LD_PLL_ASR_19 		(uint8)(CLK_IN*32.0/(LD_PLL_11+1) - 0.51)
#define LD_PLL_ASR_1B 		0x48
#define LD_PLL_ASR_1D 		0x1f

// LD chip fixed values.
#define        RESUM_OF_MUSIC               0x01
#define        CAUSE_MP3_SONG_END           0x20

#define        MASK_INT_SYNC				0x10
#define        MASK_INT_FIFO				0x04
#define    	   MASK_AFIFO_INT				0x01
#define        MASK_FIFO_STATUS_AFULL		0x08

void LD_reset(void);

void LD_Init_Common(void);

void LD_Init_ASR(void);

void LD_ReloadMp3Data(void);
void LD_ReloadMp3Data_2(void);

uint8 LD_ProcessAsr(uint32 RecogAddr);
void LD_AsrStart(void);
uint8 LD_AsrRun(void);
uint8 LD_AsrAddFixed(void);
uint8 LD_GetResult(void);

void LD_ReadMemoryBlock(uint8 dev, uint8 * ptr, uint32 addr, uint8 count);
void LD_WriteMemoryBlock(uint8 dev, uint8 * ptr, uint32 addr, uint8 count);

void PhoneticsMS_Thread(const void *argument);
void Phonetics_Init(void);
void Phonetics(void);

extern uint8  nLD_Mode;

//识别码（客户修改处）
#define CODE_FSON			1	 /*打开风扇*/
#define CODE_FSOFF		2	 /*关闭风扇*/
#define CODE_CHON			3	 /*开窗*/
#define CODE_CHOFF		4	 /*关窗*/
#define CODE_PWON			5	 /*打开喷雾*/
#define CODE_PWOFF		6	 /*关闭喷雾*/
#define CODE_JRON			7	 /*打开加热*/
#define CODE_JROFF		8	 /*关闭加热*/
#define CODE_SZDON		9	 /*打开生长灯*/
#define CODE_SZDOFF		10	 /*关闭生长灯*/

void  LD3320_delay(unsigned long uldata);

#define MIC_VOL 0x55
#endif
