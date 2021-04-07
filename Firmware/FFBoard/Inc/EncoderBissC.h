/*
 * EncoderDummy.h
 *
 *  Created on: 19.12.2020
 *      Author: Leon
 */

#ifndef ENCODERBISSC_H_
#define ENCODERBISSC_H_

#include "cppmain.h"
#include "constants.h"
#include "ChoosableClass.h"
#include "CommandHandler.h"
#include "SPI.h"
#include "Encoder.h"
#include "cpp_target_config.h"

class EncoderBissC: public Encoder, public CommandHandler, public SPIDevice {
public:

	static ClassIdentifier info;
	const ClassIdentifier getInfo();

	EncoderBissC();
	virtual ~EncoderBissC();

	EncoderType getType();

	void updatePos();
	int32_t getPos();
	void setPos(int32_t pos);
	uint32_t getCpr();
	int16_t calcPhi_ext();

	ParseStatus command(ParsedCommand* cmd,std::string* reply);

	void initSPI(void);
	void acquirePosition();
	void SpiRxCplt(SPI_HandleTypeDef *hspi);

	const SPIConfig& getConfig() const override;
    void beginRequest(SPIPort::Pipe& pipe) override;

	uint16_t cspin;
	GPIO_TypeDef* csport;

private:
	void process(uint32_t* buf);
	SPIConfig spi_config;

	int clz64(uint64_t bytes);

	int32_t pos = 0;
	int32_t mtpos = 0;
	uint8_t spi_buf[8] = {0}, decod_buf[8] = {0};
	volatile bool spibusy = false;
	uint8_t bytes = 8;

	uint8_t POLY = 0x43;
	uint8_t tableCRC6n[64] = {0};
	int32_t numErrors = 0;
};

#endif /* ENCODERBISSC_H_ */
