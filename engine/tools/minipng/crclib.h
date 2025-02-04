/*
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.
*/
#ifndef _CRCLIB_H_
#define _CRCLIB_H_

typedef unsigned char byte;

void CRC32_Init(uint32_t* pulCRC);
void CRC32_ProcessBuffer(uint32_t* pulCRC, const void* pBuffer, int nBuffer);
void CRC32_ProcessByte(uint32_t* pulCRC, byte ch);
uint32_t CRC32_Final(uint32_t pulCRC);

#endif 
