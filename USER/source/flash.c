#include "flash.h"
#include "stm32f37x.h"
#include "stm32f37x_flash.h"



/********************************************************************
* 功    能：flash初始化
* 输    入：none
* 输    出：	none
* 编 写 人：stragen
* 编写日期：2016.6.23
**********************************************************************/
void vInnerFlash_Init(void)
{

	FLASH_SetLatency(FLASH_Latency_2);  //设置写延时
	FLASH_ClearFlag(FLASH_FLAG_EOP| FLASH_FLAG_PGERR |FLASH_FLAG_WRPERR);  //清除标志位
	
}

/********************************************************************
* 功    能： 向EEPROM写两字节
* 输    入： EEPROM的一个存储地址，双字对齐
* 输    出：	none
* 编 写 人：stragen
* 编写日期：2016.6.23
**********************************************************************/
void EEProm_Write(uint32_t addr,uint8_t ByteData)
{
	FLASH_Unlock(); //解锁写保护
	FLASH_ProgramHalfWord(addr, (uint16_t)ByteData);
	FLASH_Lock();
	if( *(__IO uint32_t *)addr != (uint16_t)ByteData)
		{
			while(1);//写flash失败
		}
}

/********************************************************************
* 功    能： 从EEPROM的一个地址上读两个字节
* 输    入： EEPROM的一个存储地址，双字对齐
* 输    出：	none
* 编 写 人：stragen
* 编写日期：2016.6.23
**********************************************************************/
 uint8_t EEProm_Read(uint32_t addr)
{
	  uint8_t ReadByte;
		ReadByte=(uint16_t) *(__IO uint32_t *)addr;
		return ReadByte;
}

