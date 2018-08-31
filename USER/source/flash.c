#include "flash.h"
#include "stm32f37x.h"
#include "stm32f37x_flash.h"



/********************************************************************
* ��    �ܣ�flash��ʼ��
* ��    �룺none
* ��    ����	none
* �� д �ˣ�stragen
* ��д���ڣ�2016.6.23
**********************************************************************/
void vInnerFlash_Init(void)
{

	FLASH_SetLatency(FLASH_Latency_2);  //����д��ʱ
	FLASH_ClearFlag(FLASH_FLAG_EOP| FLASH_FLAG_PGERR |FLASH_FLAG_WRPERR);  //�����־λ
	
}

/********************************************************************
* ��    �ܣ� ��EEPROMд���ֽ�
* ��    �룺 EEPROM��һ���洢��ַ��˫�ֶ���
* ��    ����	none
* �� д �ˣ�stragen
* ��д���ڣ�2016.6.23
**********************************************************************/
void EEProm_Write(uint32_t addr,uint8_t ByteData)
{
	FLASH_Unlock(); //����д����
	FLASH_ProgramHalfWord(addr, (uint16_t)ByteData);
	FLASH_Lock();
	if( *(__IO uint32_t *)addr != (uint16_t)ByteData)
		{
			while(1);//дflashʧ��
		}
}

/********************************************************************
* ��    �ܣ� ��EEPROM��һ����ַ�϶������ֽ�
* ��    �룺 EEPROM��һ���洢��ַ��˫�ֶ���
* ��    ����	none
* �� д �ˣ�stragen
* ��д���ڣ�2016.6.23
**********************************************************************/
 uint8_t EEProm_Read(uint32_t addr)
{
	  uint8_t ReadByte;
		ReadByte=(uint16_t) *(__IO uint32_t *)addr;
		return ReadByte;
}

