/*
 * EncoderDummy.cpp
 *
 *  Created on: 19.12.2020
 *      Author: Leon
 */

#include "EncoderBissC.h"

ClassIdentifier EncoderBissC::info = {
		 .name = "BissC" ,
		 .id=4,
		 .hidden = false,
};

const ClassIdentifier EncoderBissC::getInfo(){
	return info;
}

EncoderBissC::EncoderBissC() : spi_config{*getExternalSPI_CSPins()[1]} {
	setPos(0);

	this->spi_config.peripheral.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
	this->spi_config.peripheral.FirstBit = SPI_FIRSTBIT_MSB;
	this->spi_config.peripheral.CLKPhase = SPI_PHASE_1EDGE;
	this->spi_config.peripheral.CLKPolarity = SPI_POLARITY_HIGH;
	this->spi_config.peripheral.DataSize = SPI_DATASIZE_8BIT;
	attachToPort(external_spi);

	//Init CRC table
	for(int i = 0; i < 64; i++){
		int crc = i;

		for (int j = 0; j < 6; j++){
			if (crc & 0x20){
				crc <<= 1;
				crc ^= POLY;
			} else {
				crc <<= 1;
			}
		}
		tableCRC6n[i] = crc;
	}
	//volatile int test = clz64(0b0000010000000000000000000000000000000000000000000000000000000000);
}

const SPIConfig& EncoderBissC::getConfig() const {
	return spi_config;
}

EncoderBissC::~EncoderBissC() {
	setPos(0);
}

void EncoderBissC::acquirePosition(){
	//TODO HAL_GPIO_WritePin(this->csport,this->cspin,conf.cspol ? GPIO_PIN_SET : GPIO_PIN_RESET);
	spibusy = true;
	//HAL_SPI_Receive_DMA(this->spi,spi_buf, this->bytes);
}

EncoderType EncoderBissC::getType(){
	return EncoderType::absolute;
}

void EncoderBissC::beginRequest(SPIPort::Pipe& pipe) {
	pipe.beginRx(spi_buf, 8);
}

int16_t EncoderBissC::calcPhiE_ext(){
	return 0; // TODO: calculate correct value
}

void EncoderBissC::updatePos(){
	
	int32_t lastpos = pos;
	memcpy(this->decod_buf,this->spi_buf,this->bytes);

	decod_buf[0] &= 0x3F; //remove first 2 bits (startbit, maybe one ack)

	//Put data into 64bit int to enable easy shifting
	volatile uint64_t rxData64 = (uint64_t)decod_buf[0] << 56;
	rxData64 |= (uint64_t)decod_buf[1] << 48;
	rxData64 |= (uint64_t)decod_buf[2] << 40;
	rxData64 |= (uint64_t)decod_buf[3] << 32;
	rxData64 |= (uint64_t)decod_buf[4] << 24;
	rxData64 |= (uint64_t)decod_buf[5] << 16;
	rxData64 |= (uint64_t)decod_buf[6] << 8;
	rxData64 |= (uint64_t)decod_buf[7];

	for(int i = 0; i < 64; i++){
		if(!(0x8000000000000000 & rxData64))
			rxData64<<=1;
		else
			break;
	}
	//TODO rxData64 <<= clz64(rxData64);

	rxData64 >>= 64 -(1+1+22+1+1+6); //Align bitstream to left (Startbit, CDS, 22-bit Position, Error, Warning, CRC)
	volatile uint8_t crcRx = rxData64 & 0x3F; //AND with 6-bit mask to get CRC
	volatile uint32_t dataRx = (rxData64 >> 6) & 0xFFFFFF; //Shift out CRC, AND with 24-bit mask to get raw data (position, error, warning)
	pos = (dataRx >> 2) & 0x3FFFFF; //Shift out error and warning, AND with 22-bit mask to get position

	volatile uint8_t crc = 0;  //CRC seed is 0b000000

	crc = tableCRC6n[((dataRx >> 18) & 0x3F) ^ crc];
	crc = tableCRC6n[((dataRx >> 12) & 0x3F) ^ crc];
	crc = tableCRC6n[((dataRx >> 6) & 0x3F) ^ crc];
	crc = tableCRC6n[((dataRx >> 0) & 0x3F) ^ crc];

	crc = 0x3F & ~crc; //CRC is output inverted

	if(crc != crcRx || pos < 0)
	{
		numErrors++;
		//TODO pos = -1;	//position is 22bit value, so in positive range. if crc error, negative value is indicator
	}


	//handle multiturn
	if(pos-lastpos > 1<<21){
		mtpos--;
	}else if(lastpos-pos > 1<<21){
		mtpos++;
	}

	if(!this->requestPending()){
		requestPort();
	}
}

int32_t EncoderBissC::getPos(){
	return pos + mtpos * getCpr();
}

int EncoderBissC::clz64(uint64_t bytes){
	int cnt = __CLZ(((uint32_t*)bytes)[1]);	//high word

	if(cnt == 32)
		cnt += __CLZ(((uint32_t*)bytes)[0]);	//low word
	return cnt;
}

void EncoderBissC::setPos(int32_t pos){
	this->pos = pos; //TODO change to create offset
}

uint32_t EncoderBissC::getCpr(){
	return 1<<22; //TODO flexible
}

ParseStatus EncoderBissC::command(ParsedCommand* cmd,std::string* reply){
	return ParseStatus::NOT_FOUND;
}
